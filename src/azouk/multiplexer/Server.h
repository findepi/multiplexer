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

#ifndef MX_SERVER_H
#define MX_SERVER_H

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <asio/io_service.hpp>
#include <asio/placeholders.hpp>
#include <boost/random/linear_congruential.hpp>

#include "azlib/logging.h"
#include "azlib/util/functors.h"
#include "azlib/repr.h"
#include "multiplexer/ConnectionsManager.h"
#include "multiplexer/io/Connection.h"
#include "multiplexer/Config.h"
#include "build/multiplexer/Multiplexer.pb.h" /* generated */
#include "build/multiplexer/multiplexer.constants.h" /* generated */
#include "build/azouk/logging/type_id_constants.h" /* generated */

namespace multiplexer {

    using azlib::repr;

    //class MultiplexerMessage;
    //class MultiplexerMessageDescription;
    //class MultiplexerMessageDescription_RoutingRule;

    class Server; // forward

    template <>
    struct ConnectionsManagerTraits<Server> : public DefaultConnectionsManagerTraits {

	struct MessagesBufferTraits : public DefaultConnectionsManagerTraits::MessagesBufferTraits {
	    typedef boost::shared_ptr<const RawMessage> value_type;
	    typedef azlib::ReferencingFunctor<value_type> ToRawMessagePointerConverter;
	    typedef azlib::ReferencingFunctor<value_type> ToBufferRepresentationConverter;

	    typedef azlib::ConstructingFunctor<bool, value_type> SchedulingResultFunctor;
	};

	typedef multiplexer::Connection<Server> Connection;
    };


    /**
     * @class Server
     *  manages the Multiplexer server
     */
    class Server : public ConnectionsManager<Server>, public boost::enable_shared_from_this<Server> {

    private:
	Server(asio::io_service& io_service, const std::string& host, unsigned short port);

    public:
	typedef ConnectionsManager<Server> Base;
	// public constructor-like function
	typedef boost::shared_ptr<Server> pointer;
	typedef boost::weak_ptr<Server> weak_pointer;
	static pointer Create(asio::io_service& io_service, const std::string& host, unsigned short port) {
	    return pointer(new Server(io_service, host, port));
	}

	// ConnectionsManager interface
	boost::shared_ptr<const RawMessage> get_welcome_message() {
	    if (!welcome_message_) {
		welcome_message_ = create_welcome_message(
			    multiplexer::peers::MULTIPLEXER);
	    }
	    return welcome_message_;
	}

	void inline set_multiplexer_password(
		const std::string& multiplexer_password) {
	    welcome_message_.reset();
	    return Base::set_multiplexer_password(multiplexer_password);
	}

	/**
	 * Here is the most imprtant part of Multiplexer Server (The Core Component of the Azouk Enterprise Message Bus)
	 * Here we handle routing rules, etc.
	 */
	void handle_message(Connection::pointer conn, boost::shared_ptr<const RawMessage> raw, boost::shared_ptr<MultiplexerMessage> msg);

	// other interface functions
	void start();

	bool inline accept_peer_type(boost::uint32_t peer_type) const {
	    return peer_type > peers::MAX_MULTIPLEXER_SPECIAL_PEER_TYPE && Base::accept_peer_type(peer_type);
	}

    private:
	void _start_accept() {
	    Connection::pointer new_connection = Connection::Create(acceptor_.io_service(), this->shared_from_this());

	    acceptor_.async_accept(new_connection->socket(),
		    boost::bind(&Server::_handle_accept, this, new_connection, asio::placeholders::error)
		    );
	}

	void _handle_accept(Connection::pointer new_connection, const asio::error_code& error) {
	    _start_accept();
	    if (!error) {
		// we don't run new_connection->start() until the connection
		// is authenticated
		new_connection->start_only_read();
	    } else {
		// the connection is dropped, or -- in fact -- has never been estabilished
		new_connection->shutdown();
	    }
	}

    public:
	void after_connection_registration(Connection::pointer new_connection,
		const WelcomeMessage&) {
	    new_connection->start_rest();
	}

    private:
	/**
	 * MessageMetaHandler
	 *	 a helper class that stored the internal state of message handling process
	 */
	struct MessageMetaHandler : boost::noncopyable {
	    MessageMetaHandler(const MultiplexerMessage& m, Connection::pointer c, boost::shared_ptr<const RawMessage> r)
		: msg(m), conn(c), raw(r)
	    {}

	    inline void failed(const MultiplexerMessageDescription::RoutingRule& rule, boost::uint32_t type) {
		__create_delivery_error_message(rule.include_original_packet_in_report());
		delivery_error_message->add_failed_type(type);
	    }

	    inline void failed(const boost::uint64_t to) {
		__create_delivery_error_message(msg.include_original_packet_in_report());
		delivery_error_message->set_failed_to(to);
	    }

	    inline void unknown() {
		__create_delivery_error_message(msg.include_original_packet_in_report());
		delivery_error_message->set_is_known_type(false);
	    }

	private:
	    inline void __create_delivery_error_message(bool include_original_packet_in_report) {
		if (delivery_error_message)
		    return;
		delivery_error_message.reset(new DeliveryError());
		delivery_error_message->set_packet_id(this->msg.id());
		if (include_original_packet_in_report) {
		    *delivery_error_message->mutable_original_message() = this->msg;
		}
	    }

	public:
	    boost::scoped_ptr<DeliveryError> delivery_error_message;
	    const MultiplexerMessage& msg;
	    const Connection::pointer conn;
	    const boost::shared_ptr<const RawMessage> raw;
	}; // struct MessageMetaHandler

	void _handle_message(Connection::pointer conn, const MultiplexerMessage& msg, boost::shared_ptr<const RawMessage> raw) {
	    // the below check had to be disabled after introduction of log collectors
	    // that act like proxies -- they send messages not from them
	    //if (msg.from() != conn->peer_id() && msg.from() != instance_id_) {
		// ... log error ...
		//return;
	    //}

            AZOUK_ENTER(VERBOSITY(HIGHVERBOSITY) DEFAULT);

	    AZOUK_LOG(DEBUG, HIGHVERBOSITY, CTX("multiplexer.server") FLOW(msg.workflow())
		    TEXT("handle_message(id=" + repr(msg.id()) + ", type=" + repr(msg.type()) + ")")
		    DATA(type_id_constants::MXSERVER_INCOMING_MULTIPLEXER_MESSAGE, MultiplexerMessage,
			(set_id(msg.id())) (set_from(msg.from())) (set_to(msg.to()))
			(set_type(msg.type())) (set_timestamp(msg.timestamp()))
			(set_references(msg.references())) (set_workflow(msg.workflow()))
		    )
		    SKIPFILEIF(!(msg.logging_method() & multiplexer::LoggingMethod::FILE))
		);

	    MessageMetaHandler meta_handler(msg, conn, raw);
	    do {
		if (_handle_message_inlined_rules(meta_handler))
		    break; // already handled
		if (_handle_meta_message(meta_handler))
		    break; // already handled

		// default message handler
		const Config::MessageDescriptionById& definitions = config_.message_description_by_id();
		Config::MessageDescriptionById::const_iterator defpos = definitions.find(msg.type());
		if (defpos == definitions.end()) {
		    // we don't know the type of the packet
		    std::cerr << "received unknown message of type " << msg.type() << "; dropping a packet\n";
		    meta_handler.unknown();
		    break; // won't be handled at all
		}

		// it's known type
		const MultiplexerMessageDescription& desc = defpos->second;
		AZOUK_LOG(DEBUG, CHATTERBOX, CTX("multiplexer.server") FLOW(msg.workflow())
			TEXT("received a message of type " + repr(msg.type()) + " (" + desc.name() + "); scheduling...")
			SKIPFILEIF(!(msg.logging_method() & multiplexer::LoggingMethod::FILE))
		    );
		_schedule(meta_handler, desc);

	    } while (0);

	    _handle_delivery_errors(meta_handler);

            AZOUK_LEAVE();

	}

	inline void _handle_delivery_errors(MessageMetaHandler& meta_handler) {
	    if (!meta_handler.delivery_error_message)
		return; // no errors

	    AZOUK_LOG(ERROR, HIGHVERBOSITY, CTX("multiplexer.server") FLOW(meta_handler.msg.workflow())
		    TEXT("errors when delivering " + repr(meta_handler.msg.id()))
		    SKIPFILEIF(!(meta_handler.msg.logging_method() & multiplexer::LoggingMethod::FILE))
		);

	    if (!meta_handler.msg.from()) // sanity check
		return;
	    if (meta_handler.msg.from() == instance_id_) // sanity check
		return;

	    // construct MultiplexerMessage for DeliveryError message
	    MultiplexerMessage mxmsg;
	    mxmsg.set_id(random_());
	    mxmsg.set_from(instance_id_);
	    mxmsg.set_to(meta_handler.msg.from());
	    mxmsg.set_report_delivery_error(false);
	    mxmsg.set_type(types::DELIVERY_ERROR);
	    meta_handler.delivery_error_message->SerializeToString(mxmsg.mutable_message());
	    mxmsg.set_references(meta_handler.msg.id());
	    mxmsg.set_workflow(meta_handler.msg.workflow());
	    boost::shared_ptr<const RawMessage> raw(RawMessage::FromMessage(mxmsg));
	    _handle_message(meta_handler.conn, mxmsg, raw);
	    meta_handler.delivery_error_message.reset();
	}

	inline bool _handle_message_inlined_rules(MessageMetaHandler& meta_handler) {
	    // handle msg.to()
	    if (meta_handler.msg.to()) {
		ConnectionById::iterator ci = connection_by_id_.find(meta_handler.msg.to());
		Connection::pointer c;
		if (ci == connection_by_id_.end() || !(c = ci->second.lock())) {
		    if (meta_handler.msg.report_delivery_error())
			meta_handler.failed(meta_handler.msg.to());
		    std::cerr << "received a packet to " << meta_handler.msg.to() << " while it's not connected; dropping a packet\n";
		    return true;
		}
		c->schedule(meta_handler.raw);
		return true;
	    }

	    // handle msg.override_rrules()
	    if (meta_handler.msg.override_rrules().size()) {
		_schedule(meta_handler, meta_handler.msg.override_rrules());
		return true;
	    }
	    return false;
	}

	inline bool _handle_meta_message(MessageMetaHandler& meta_handler) {
	    if (meta_handler.msg.type() > types::MAX_MULTIPLEXER_META_PACKET)
		return false; // this is not a meta message

	    switch (meta_handler.msg.type()) {
		case types::CONNECTION_WELCOME:
		    std::cerr << "received a CONNECTION_WELCOME message after estabilishing a connection\n";
		    meta_handler.conn->shutdown();
		    return true;

		case types::PING:
		    // TODO logger.error << "PING message without inlined routing rules is not supported";
		    // TODO: support PING to self
		    return true;

		case types::DELIVERY_ERROR:
		    // TODO logger.error << "DELIVERY_ERROR message without inlined routing rules";
		    return true;

		case types::BACKEND_FOR_PACKET_SEARCH:
		    {
			// TODO logger.debug << "received BACKEND_FOR_PACKET_SEARCH packet";
			BackendForPacketSearch inner;
			if (!inner.ParseFromString(meta_handler.msg.message())) {
			    std::cerr << "garbled BACKEND_FOR_PACKET_SEARCH packet; droppping...\n";
			    AZOUK_LOG(ERROR, HIGHVERBOSITY, CTX("multiplexer.server")
				    FLOW(meta_handler.msg.workflow())
				    TEXT("garbled BACKEND_FOR_PACKET_SEARCH packet")
				);
			    meta_handler.unknown();
			    return true;
			}
			const MultiplexerMessageDescription* descp = config_.message_description(inner.packet_type());
			if (!descp || !descp->to().size()) {
			    AZOUK_LOG(ERROR, HIGHVERBOSITY, CTX("multiplexer.server")
				    FLOW(meta_handler.msg.workflow())
				    TEXT("BACKEND_FOR_PACKET_SEARCH: unknown packet type " +
					repr(inner.packet_type()) + " or type with no routing rules")
				);
			    meta_handler.unknown();
			    return true;
			}

			// dynamically create routing rule for this BACKEND_FOR_PACKET_SEARCH
			MultiplexerMessageDescription::RoutingRule rule = descp->to().Get(0);

			rule.set_whom(MultiplexerMessageDescription::RoutingRule::ALL);
			rule.set_report_delivery_error(true);
			rule.set_include_original_packet_in_report(false);

			if (rule.peer_type() == peers::ALL_TYPES) {
			    BOOST_FOREACH(ConnectionsByType::value_type& cbt, connections_by_type_)
				_schedule(meta_handler, cbt.second, rule, cbt.first);
			} else {
			    _schedule(meta_handler, connections_by_type_[rule.peer_type()], rule);
			}
		    }
		    return true;

		default:
		    std::cerr << "ERROR: unknown meta packet of type " << meta_handler.msg.type() << "; dropping\n";
		    return true;
	    }
	}

	// schedule using MultiplexerMessageDescription
	inline unsigned int _schedule(MessageMetaHandler& meta_handler, const MultiplexerMessageDescription& desc) {
	    return _schedule(meta_handler, desc.to());
	}

	// schedule using list of rules
	// equivalent to calling _schedule for every element of the list
	inline unsigned int _schedule(MessageMetaHandler& meta_handler,
		const ::google::protobuf::RepeatedPtrField<MultiplexerMessageDescription::RoutingRule>& rrules) {

	    unsigned int schedules_counter = 0;
	    BOOST_FOREACH(const MultiplexerMessageDescription::RoutingRule& rule, rrules)
		schedules_counter += _schedule(meta_handler, rule);
	    return schedules_counter;
	}

	// schedule using single routing rule
	inline unsigned int _schedule(MessageMetaHandler& meta_handler, const MultiplexerMessageDescription::RoutingRule& rule) {

	    unsigned int schedules_counter = 0;
	    if (rule.peer_type() == peers::ALL_TYPES) {
		// handle spacial case: meta type "ALL_TYPES"
		BOOST_FOREACH(ConnectionsByType::value_type& cbt, connections_by_type_)
		    schedules_counter += _schedule(meta_handler, cbt.second, rule, cbt.first);
	    } else {

		// general case rule.peer_type() is really a peer type
		schedules_counter += _schedule(meta_handler, connections_by_type_[rule.peer_type()], rule);
	    }

	    if (!schedules_counter) {
		const unsigned int level = rule.delivery_error_is_error() ? ERROR : WARNING;
		AZOUK_LOG(level, HIGHVERBOSITY, CTX("multiplexer.server") FLOW(meta_handler.msg.workflow())
			TEXT("routing while none present of type " + repr(rule.peer_type()) + " (" + config_.peer_name_by_type(rule.peer_type()) + ")")
		    );
	    }
	    return schedules_counter;
	}

	inline unsigned int _schedule(MessageMetaHandler& meta_handler, ConnectionsList& connections, const MultiplexerMessageDescription::RoutingRule& rule) {
	    return _schedule(meta_handler, connections, rule, rule.peer_type());
	}

	inline unsigned int _schedule(MessageMetaHandler& meta_handler, ConnectionsList& connections, const MultiplexerMessageDescription::RoutingRule& rule,
		boost::uint32_t peer_type) {
	    unsigned int schedules_counter = (unsigned int)-1;
	    switch (rule.whom()) {
		case MultiplexerMessageDescription::RoutingRule::ALL:
		    // send message to ALL of given type
		    schedules_counter = send_to_all(connections, meta_handler.raw);
		    break;

		case MultiplexerMessageDescription::RoutingRule::ANY:
		    // send message to some (ANY) of given type
		    schedules_counter = send_to_one(connections, meta_handler.raw);
		    break;
	    }
	    AssertMsg(schedules_counter != (unsigned int)-1, "unhandled Whom type " + boost::lexical_cast<std::string>(rule.whom())); // never reached

	    if (!schedules_counter && rule.report_delivery_error())
		meta_handler.failed(rule, peer_type);
	    return schedules_counter;
	}

	static inline unsigned int send_to_all(ConnectionsList& connections, const boost::shared_ptr<const RawMessage>& raw) {
	    unsigned int schedules_counter = 0;
	    for (ConnectionsList::iterator i, next = connections.begin(); next != connections.end() && (i = next++, true); ) {
		if (Connection::pointer conn = i->lock()) {
		    if (conn->living() && conn->schedule(raw))
			++ schedules_counter;
		} else
		    connections.erase(i); // dead connection
	    }

	    return schedules_counter;
	}

	static inline unsigned int send_to_one(ConnectionsList& connections, const boost::shared_ptr<const RawMessage>& raw) {
	    // for now ANY means round robin
	    Connection::pointer conn;
	    for (ConnectionsList::iterator i = connections.begin(); (i = choose_free_connections(connections, i)) != connections.end(); ++i) {
		if (!(conn = i->lock()) || !conn->schedule(raw))
		    continue;
		connections.splice(connections.end(), connections, i);
		return 1;
	    }
	    return 0;
	}

    private:
	/* instance properties */
	asio::ip::tcp::acceptor acceptor_;
	boost::shared_ptr<const RawMessage> welcome_message_;

    }; // class Server

}; // namespace multiplexer

#endif
