//
// Azouk Libraries -- Libraries and goodies created for www.azouk.com.
// Copyright (C) 2008-2009 Azouk Network Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Author:
//      Piotr Findeisen <piotr.findeisen at gmail.com>
//

#ifndef MX_BASICCLIENT_H
#define MX_BASICCLIENT_H

#include <deque>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <boost/cstdint.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/logic/tribool.hpp>
#include <boost/logic/tribool_io.hpp>
#include "azlib/timer.h"
#include "azlib/util/Assert.h"
#include "multiplexer/ConnectionsManager.h"
#include "multiplexer/io/Connection.h"
#include "multiplexer/configuration.h"
#include "azlib/util/functors.h"
#include "azlib/spanset.h"
#include "azlib/triple.h"

namespace multiplexer {

    struct BasicClientTraits {
	typedef boost::asio::deadline_timer Timer;
	typedef boost::shared_ptr<Timer> TimerPointer;
	typedef boost::asio::ip::tcp::endpoint Endpoint;
    };

    class BasicClient;
    class Client;

    template <>
    struct ConnectionsManagerTraits<BasicClient> :
            public DefaultConnectionsManagerTraits {

	typedef DefaultConnectionsManagerTraits Base;

	struct MessagesBufferTraits : public Base::MessagesBufferTraits {
	    typedef ConnectionsManagerTraits::Base::MessagesBufferTraits Base;

	    typedef std::pair<boost::shared_ptr<boost::tribool>,
                        boost::shared_ptr<const RawMessage> >
                            temporary_value_type;
            typedef std::pair<boost::weak_ptr<boost::tribool>,
                    boost::shared_ptr<const RawMessage> > value_type;

            typedef azlib::SecondFromPairExtractor<value_type>
                ToRawMessagePointerConverter;

	    struct ToBufferRepresentationConverter :
                    public std::unary_function<
                        boost::shared_ptr<const RawMessage>,
                        temporary_value_type> {

		temporary_value_type operator() (
                        boost::shared_ptr<const RawMessage> raw) const {

		    return temporary_value_type(
                        temporary_value_type::first_type(
                            new boost::tribool(boost::logic::indeterminate)),
                        raw);
		}
	    };
            typedef azlib::FirstFromPairExtractor<temporary_value_type>
                SchedulingResultFunctor;

	    struct SendingResultNotifier : public Base::SendingResultNotifier {

                template <typename ConnectionsManagerImplementationWeakPointer,
                         typename QueueType>
                void notify_success(ConnectionsManagerImplementationWeakPointer,
                        QueueType& qe) const {

		    if (temporary_value_type::first_type tbp = qe.first.lock())
			*tbp = true;
		}

                template <typename ConnectionsManagerImplementationWeakPointer,
                         typename QueueType>
                void notify_error(ConnectionsManagerImplementationWeakPointer,
                        QueueType& qe) const {

		    if (temporary_value_type::first_type tbp = qe.first.lock())
			*tbp = false;
		}
	    };

	};

	struct ConnectionManagerPrivateDataInConnection {
	private:
	    BasicClientTraits::Endpoint expected_endpoint;
	    friend class BasicClient;
	};

	typedef multiplexer::Connection<BasicClient> Connection;
    };

    class ConnectionWrapper {
    public:
	inline operator bool () const { return lock(); }
    private:
	typedef ConnectionsManagerTraits<BasicClient>::Connection Connection;

    public:
	ConnectionWrapper()
	{}

    private:
	//ConnectionWrapper(Connection::pointer conn) : conn_(conn) {}
	ConnectionWrapper(Connection::pointer conn,
                const BasicClientTraits::Endpoint &endpoint)
	    : conn_(conn), endpoint_(endpoint)
	{}
	Connection::pointer lock() const { return conn_.lock(); }

    public:
	ConnectionWrapper& operator= (const ConnectionWrapper& other) {
	    if (this == &other) return *this;
	    conn_ = other.conn_;
	    endpoint_ = other.endpoint_;
	    return *this;
	}

    private:
	Connection::weak_pointer conn_;
	BasicClientTraits::Endpoint endpoint_;

	friend class BasicClient;
	friend class Client;
    };

    struct ExceptionDefinitions {
	/*
	 * exceptions raised by BasicClient et al
	 *
         * virtual what() is provided so that they are always distinguishable
         * types (even for Boost.Python which translates them to Python
         * exceptions)
	 */
	struct MxClientError : public azlib::Exception {
            const char* what() const throw() {
                return azlib::Exception::what();
            }
        };
	struct NotConnected : public MxClientError {
            const char* what() const throw() {
                return MxClientError::what();
            }
        };
	struct OperationTimedOut : public MxClientError {
            const char* what() const throw() {
                return MxClientError::what();
            }
        };
	struct OperationFailed : public MxClientError {
            const char* what() const throw() {
                return MxClientError::what();
            }
        };
    };

    /**
     * @class BasicClient
     *  manages connections to the Multiplexer servers
     */
    class BasicClient : public ConnectionsManager<BasicClient>,
            public boost::enable_shared_from_this<BasicClient>,
	    public BasicClientTraits, public ExceptionDefinitions {

    private:
	BasicClient(boost::asio::io_service& io_service, boost::uint32_t client_type);

    public:
	// definitions
	typedef ConnectionsManager<BasicClient> Base;

	typedef std::deque<
                azlib::triple<boost::shared_ptr<const RawMessage>,
                ConnectionWrapper, boost::shared_ptr<MultiplexerMessage> >
	    > IncomingMessagesBuffer;
        typedef MessagesBufferTraits::SchedulingResultFunctor::result_type
            BasicScheduledMessageTracker;

	// public constructor-like function
	typedef boost::shared_ptr<BasicClient> pointer;
	typedef boost::weak_ptr<BasicClient> weak_pointer;

	static pointer Create(boost::asio::io_service& io_service,
                unsigned short port) {
	    return pointer(new BasicClient(io_service, port));
	}

	// ConnectionsManager interface
	boost::shared_ptr<const RawMessage> get_welcome_message() {
	    if (!welcome_message_) {
		welcome_message_ = create_welcome_message(client_type_);
	    }
	    return welcome_message_;
	}

	void inline set_multiplexer_password(
		const std::string& multiplexer_password) {
	    welcome_message_.reset();
	    return Base::set_multiplexer_password(multiplexer_password);
	}

	void handle_message(Connection::pointer conn,
                boost::shared_ptr<const RawMessage> raw,
                boost::shared_ptr<MultiplexerMessage> mxmsg);

	// connectivity
	void shutdown();
	ConnectionWrapper async_connect(const Endpoint& peer_endpoint);
	bool wait_for_connection(ConnectionWrapper connwrap, float timeout)
            const;
	ConnectionWrapper connect(const Endpoint& peer_endpoint, float timeout);
	void connection_destroyed(Connection* conn);
	void reconnect_after_timeout(TimerPointer, Endpoint peer_endpoint,
                const boost::system::error_code&);

    public:
	std::auto_ptr<azlib::SimpleTimer> create_timer(float timeout) const;

    public:
	bool accept_peer_type(boost::uint32_t peer_type) const {
	    return peer_type == peers::MULTIPLEXER &&
                Base::accept_peer_type(peer_type);
	}

	// access to connections list
	ConnectionById::const_iterator begin() const {
            return connection_by_id_.begin();
        }
	ConnectionById::iterator begin() {
            return connection_by_id_.begin();
        }
	ConnectionById::const_iterator end() const {
            return connection_by_id_.end();
        }
	ConnectionById::iterator end() {
            return connection_by_id_.end();
        }

	// incoming messages
	inline bool has_incoming_messages() const {
            return !incoming_messages_.empty();
        }
	IncomingMessagesBuffer::value_type next_incoming_message();
	inline bool incoming_queue_full() const {
            return incoming_queue_max_size_ <= incoming_messages_.size();
        }

	void inline wait_for_incoming_message(float timeout = -1) const {
	    if (has_incoming_messages()) return;
	    std::auto_ptr<azlib::SimpleTimer> timer = create_timer(timeout);
	    return wait_for_incoming_message(*timer);
	}

	void inline wait_for_incoming_message(azlib::SimpleTimer& timer) const {
	    unsigned int cc = 1;
	    const bool DISABLE_IFCONNECTED_CHECK = true; // TODO
	    while (!has_incoming_messages() && !timer.expired() &&
                    ((cc = connections_count(true)), DISABLE_IFCONNECTED_CHECK))
		io_service_.run_one();

	    if (!has_incoming_messages()) {
		if (timer.expired())
		    MXTHROW(OperationTimedOut());
		else if (!cc)
		    MXTHROW(NotConnected());
		else {
                    // TODO logger.warning << "wait_for_incoming_message()
                    // operation failed for unknown reason";
		    MXTHROW(OperationFailed());
		}
	    }
	}

	// outgoing messages
	unsigned int schedule_all(boost::shared_ptr<const RawMessage> raw) {
	    unsigned int c = 0;
	    for (ConnectionById::const_iterator i = connection_by_id_.begin();
                    i != connection_by_id_.end(); ++ i) {

		if (Connection::pointer conn = i->second.lock()) {
		    if (conn->outgoing_queue_full() || !conn->living())
			continue;
		    if (conn->schedule(raw))
			++ c;
		}
	    }
	    return c;
	}

	BasicScheduledMessageTracker schedule_one(
                boost::shared_ptr<const RawMessage> raw) {

	    Connection::pointer conn;
	    ConnectionsList& connections =
                connections_by_type_[peers::MULTIPLEXER];
	    for (ConnectionsList::iterator i = connections.begin();
                    (i = choose_free_connections(connections, i)) !=
                        connections.end(); ++i) {

		if (!(conn = i->lock()))
		    continue;
		BasicScheduledMessageTracker r = conn->schedule(raw);
		if (!r)
		    continue;
		// round-robin: move *i to the end of multiplexers list
		connections.splice(connections.end(), connections, i);
		return r;
	    }
	    return BasicScheduledMessageTracker();
	}

	BasicScheduledMessageTracker schedule_one(
                boost::shared_ptr<const RawMessage> raw, ConnectionWrapper w,
                float timeout) {

	    Connection::pointer conn;
	    if ((conn = w.lock()) && conn->living()) {
		// TODO if we call schedule_one after long time of inactivity,
                // the `conn' may be still not notified about sockets having
                // been closed by the remote peer.  This way, we may take this
                // path of execution no knowing it will fail inevitably.
		return (BasicScheduledMessageTracker) conn->schedule(raw);
	    } else {
		assert(Endpoint().port() == 0);
		if (w.endpoint_.port() && timeout > 0) {
		    w = this->connect(w.endpoint_, timeout);
		    return schedule_one(raw, w, 0);
		} else
		    MXTHROW(NotConnected());
	    }
	}

	template <typename MessagesBuffer>
	void inline handle_orphaned_outgoing_messages(
                MessagesBuffer& outgoing_messages) {

	    std::list<Connection::pointer> working_connections;
	    BOOST_FOREACH(Connection::weak_pointer connwp,
                    connections_by_type_[peers::MULTIPLEXER])

	       if (Connection::pointer conn = connwp.lock())
		   if (conn->living())
		       working_connections.push_back(conn);

	    BOOST_FOREACH(typename MessagesBuffer::value_type& message,
                    outgoing_messages) {

		for (size_t n = working_connections.size(); n; --n) {
		    if (working_connections.front()->take_over(message)) {
                        // move front element to the end
			working_connections.splice(working_connections.end(),
                                working_connections,
                                working_connections.begin());
			message.first.reset();
		    } else
			working_connections.pop_front();
		}
		if (working_connections.empty())
		    break;
	    }
	}

	azlib::Random64::result_type random64() {
	    return random_();
	}

	boost::uint32_t inline client_type() const { return client_type_; }

    private:
        typedef std::map<Endpoint, Connection::weak_pointer>
            ConnectionByEndpoint;

	/* instance properties */
	boost::uint32_t client_type_;
	bool shuts_down_;

	// received messages
	IncomingMessagesBuffer incoming_messages_;
	unsigned int incoming_queue_max_size_;

	ConnectionByEndpoint connection_by_endpoint_;

	boost::shared_ptr<const RawMessage> welcome_message_;

	azlib::SpanSet<boost::uint64_t, 2048> last_seen_message_ids_;
    };

}; // namespace multiplexer
#endif
