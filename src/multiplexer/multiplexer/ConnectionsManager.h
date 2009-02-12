#ifndef MX_CONNECTIONSMANAGER_H
#define MX_CONNECTIONSMANAGER_H

#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/random.hpp>
#include <boost/foreach.hpp>
#include <asio/io_service.hpp>

#include "azlib/debug.h"
#include "azlib/random.h"
#include "multiplexer/io/RawMessage.h"
#include "multiplexer/Config.h"
#include "azlib/logging.h"

namespace multiplexer {

    using namespace ::azlib::logging::consts;

    class WelcomeMessage;

    struct DefaultConnectionsManagerTraits {
	struct ConnectionManagerPrivateDataInConnection {};

	struct MessagesBufferTraits {
	    struct SendingResultNotifier {
		template <typename ConnectionsManagerImplementationWeakPointer, typename QueueType>
		void notify_success(ConnectionsManagerImplementationWeakPointer, QueueType&) const {}

		template <typename ConnectionsManagerImplementationWeakPointer, typename QueueType>
		void notify_error(ConnectionsManagerImplementationWeakPointer, QueueType&) const {}
	    };
	};
    };
    
    template <typename ConnectionsManagerImplementation> struct ConnectionsManagerTraits;

    namespace impl {
	boost::shared_ptr<const RawMessage>
	    create_welcome_message(boost::uint32_t peer_type,
		    boost::uint64_t instance_id,
		    const std::string& multiplexer_password_);
    };


    template <typename ConnectionsManagerImplementation>
    class ConnectionsManager {
    protected:
	ConnectionsManager(asio::io_service& io_service)
	    : io_service_(io_service)
	    , instance_id_(random_())
	{
	    AZDEBUG_MSG("my instance_id = " << instance_id_);
	}

    public:
	const static unsigned int DEFAULT_OUT_QUEUE_SIZE = 1024;

	typedef ConnectionsManagerTraits<ConnectionsManagerImplementation> ConnectionsManagerTraits;
	typedef typename ConnectionsManagerTraits::MessagesBufferTraits MessagesBufferTraits;
	typedef typename ConnectionsManagerTraits::Connection Connection;

	typedef std::list<typename Connection::weak_pointer> ConnectionsList;
	typedef std::map<boost::uint32_t, ConnectionsList> ConnectionsByType;
	typedef std::map<boost::uint64_t, typename Connection::weak_pointer> ConnectionById;

	typedef ::multiplexer::Config<std::map> Config;

	///*virtual*/ boost::shared_ptr<const RawMessage> get_welcome_message() = 0;

	bool inline accept_peer_type(boost::uint32_t peer_type) const {
	    return !config_.initialized() || config_.peer_by_type().find(peer_type) != config_.peer_by_type().end();
	}

	/*
	 * check_multiplexer_password
	 *	validates the multiplexer_password sent by a peer
	 */
	bool inline check_multiplexer_password(const std::string& multiplexer_password) {
	    return multiplexer_password == multiplexer_password_;
	}

	void inline set_multiplexer_password(const std::string& multiplexer_password) {
	    multiplexer_password_ = multiplexer_password;
	}

	inline const std::string& multiplexer_password() const {
	    return multiplexer_password_;
	}

	/**
	 * stores a *weak reference* to conn in internal data structures
	 */
	void register_connection(typename Connection::pointer conn, const WelcomeMessage& welcome) {

	    AZDEBUG_MSG("register_connection(" << (void*) conn.get() << "): id " << conn->peer_id() << ", type " << conn->peer_type());
	    if (!static_cast<const ConnectionsManagerImplementation&>(*this).accept_peer_type(conn->peer_type())) {
		std::cerr << "invalid peer type " << conn->peer_type() << "\n";
		conn->shutdown();
		return;
	    }

	    if (conn->peer_id() == instance_id()) {
		std::cerr << "ERROR connected to self\n";
		conn->shutdown();
		return;
	    }

	    // TODO log this:
	    // std::cerr << "registering a peer #" << conn->peer_id() << " of type " << conn->peer_type() << " (" << config_.peer_name_by_type(conn->peer_type()) << ")\n";

	    Config::PeerDescriptionById::const_iterator pdi = config_.peer_by_type().find(conn->peer_type());
	    if (pdi != config_.peer_by_type().end()) {
		conn->set_is_passive(pdi->second.is_passive());
	    }

	    // It's possible that
	    //	    connection_by_id_.find(welcome.id()) != connection_by_id_.end()
	    // because the peer can be reconnecting after losing its connection and we may still not know
	    // about the connection being lost.
	    typename ConnectionById::iterator prev = connection_by_id_.find(welcome.id());
	    if (prev != connection_by_id_.end()) {
		if (typename Connection::pointer prevptr = prev->second.lock()) {
		    prevptr->shutdown();
		}
	    }

	    connection_by_id_[welcome.id()] = conn;
	    connections_by_type_[welcome.type()].push_front(conn);

	    conn->set_outgoing_queue_max_size(outgoing_queue_max_size(conn->peer_type()));

	    //after_connection_registration(conn, welcome);
	}

	void inline after_connection_registration(typename
		Connection::pointer, const WelcomeMessage&) {
	}

	void unregister_connection(Connection* conn) {
	    // TODO log this
	    // std::cerr << "unregistering a peer #" << conn->peer_id() << " of type " << conn->peer_type() << " (" << config_.peer_name_by_type(conn->peer_type()) << ")\n";

	    if (connection_by_id_.find(conn->peer_id()) == connection_by_id_.end()) {
		// TODO log this:
		std::cerr << "unregistering a peer #" << conn->peer_id() << " of type " << conn->peer_type() << " (" << config_.peer_name_by_type(conn->peer_type()) << ")\n";
		std::cerr << "(connection not registerd.)\n";
		return;
	    }

	    //before_connection_deregistration(conn);

	    bool scan_connections_by_id = false;

	    connection_by_id_.erase(conn->peer_id());
	    ConnectionsList& cons = connections_by_type_[conn->peer_type()];
	    for (typename ConnectionsList::iterator next = cons.begin(), i; next != cons.end() && (i = next++, true); ) {
		typename Connection::pointer cptr = i->lock();
		if (!cptr) {
		    std::cerr << "detected dead connection; in connections_by_type_\n";
		    scan_connections_by_id = true;
		    cons.erase(i);
		} else if (cptr.get() == conn) {
		    cons.erase(i);
		}
	    }

	    if (scan_connections_by_id) {
		for (typename ConnectionById::iterator next = connection_by_id_.begin(), i; next != connection_by_id_.end() && (i = next++, true); ) {
		    Assert(!i->second.lock() || i->second.lock().get() != conn); // assume we are not in threaded env.
		    if (!i->second.lock()) {
			connection_by_id_.erase(i);
		    }
		}
	    }
	}

	void inline connection_destroyed(Connection*) {}

	unsigned int connections_count(bool exact) {
	    //AZDEBUG_MSG("connections_count(" << exact << ") while size = " << connection_by_id_.size());
	    if (exact) {
		for (typename ConnectionById::iterator next = connection_by_id_.begin(), i; next != connection_by_id_.end() && (i = next++, true); ) {
		    if (!i->second.lock())
			connection_by_id_.erase(i);
		}
	    }
	    return connection_by_id_.size();
	}

	unsigned int connections_count(bool exact) const {
	    if (exact) {
		unsigned int c = 0;
		BOOST_FOREACH (const typename ConnectionById::value_type& i, connection_by_id_)
		    if (i.second.lock())
			++ c;
		    else
			AZDEBUG_MSG("dangling weak_ref in connection_by_id_");
		return c;
	    }
	    return connection_by_id_.size();
	}
	
	static inline typename ConnectionsList::iterator choose_free_connections(ConnectionsList& connections, typename ConnectionsList::iterator begin) {
	    for (typename ConnectionsList::iterator i, next = begin; next != connections.end() && (i = next++, true); ) {
		if (typename Connection::pointer conn = i->lock()) {
		    if (conn->outgoing_queue_full() || !conn->living())
			continue;
		    return i;
		} else
		    connections.erase(i);
	    }
	    return connections.end();
	}


	template <typename MessagesBuffer>
	void inline handle_orphaned_outgoing_messages(MessagesBuffer&) {}

    public:
	/* Config management */
	inline const Config& config() const { return config_; }
	void clear_rules() { config_.clear(); }
	void read_rules(const std::string& file) { config_.read_configuration(file); }
	unsigned int outgoing_queue_max_size(boost::uint32_t peer_type) const {
	    if (config_.initialized()) {
		Assert(config_.peer_by_type().find(peer_type) != config_.peer_by_type().end());
		return config_.peer_by_type().find(peer_type)->second.queue_size();
	    }
	    return DEFAULT_OUT_QUEUE_SIZE;
	}

	boost::uint64_t instance_id() const {
	    return instance_id_;
	}

    protected:
	/* optional helpers */
	inline boost::shared_ptr<const RawMessage> create_welcome_message(boost::uint32_t peer_type) const {
	    return impl::create_welcome_message(peer_type, instance_id_,
		    multiplexer_password_);
	}

    protected:
	asio::io_service& io_service_;
	azlib::Random64 random_;
	boost::uint64_t instance_id_;
	Config config_;
	//unsigned int living_count_;
	ConnectionsByType connections_by_type_;
	ConnectionById connection_by_id_;

	std::string multiplexer_password_;
    }; // ConnectionsManager

}; // namespace multiplexer

#endif
