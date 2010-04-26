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


#include <boost/foreach.hpp>

#include "BaseMultiplexerServer.h"

namespace multiplexer {
    namespace backend {

	BaseMultiplexerServer::BaseMultiplexerServer(const MultiplexerAddresses& addresses, PeerType type)
	    : working(true)
	    , _has_sent_response(false)
	    , __conn(new multiplexer::Client(type))
	    , conn(__conn.get())
	{
	    BOOST_FOREACH(const MultiplexerAddress& address, addresses) {
		conn->connect(address.first, address.second);
	    }
	}

	BaseMultiplexerServer::BaseMultiplexerServer(multiplexer::Client* conn_, PeerType type)
	    : working(true)
	    , _has_sent_response(false)
	    , conn(conn_)
	{
	    Assert(conn->client_type() == type);
	}

	BaseMultiplexerServer::~BaseMultiplexerServer() {
	}

	void BaseMultiplexerServer::serve_forever() {
	    while (working) {
		std::pair<boost::shared_ptr<MultiplexerMessage>,
		    ConnectionWrapper> received =
			conn->receive_message();
		last_mxmsg = received.first;
		last_connwrap = received.second;
		__handle_message();
	    }
	}

	boost::any BaseMultiplexerServer::send_message(Kwargs kwargs) {
	    DbgAssert(kwargs.check_keys(
			KwargsKeys()
			    ("message")("to")("type")("references")
			    ("workflow")
		));
	    Assert(kwargs.has_key("message"));
	    DbgAssert(kwargs.unsafe_is<const MultiplexerMessage*>
			("message") ||
		    kwargs.unsafe_is<const std::string*>("message") ||
		    kwargs.unsafe_is<std::string>("message")
		);
	    DbgAssert(kwargs.empty_or<boost::uint32_t>("type"));
	    DbgAssert(kwargs.empty_or<boost::uint64_t>("references"));
	    DbgAssert(kwargs.empty_or<boost::uint64_t>("to"));
	    DbgAssert(kwargs.empty_or<std::string>("workflow") ||
		    kwargs.unsafe_is<const std::string*>("workflow"));
	    
	    // defaults
	    kwargs.set_default("workflow", last_mxmsg->workflow());
	    kwargs.set_default("references", last_mxmsg->id());
	    kwargs.set_default("to", last_mxmsg->from());
	    kwargs.set_default("multiplexer", last_connwrap);

	    boost::scoped_ptr<MultiplexerMessage> _mxmsg;
	    const MultiplexerMessage* mxmsg;
	    if (!kwargs.unsafe_is<const MultiplexerMessage*>("message")) {
		// Construct new MultiplexerMessage using some info from kwargs.
		_mxmsg.reset(new MultiplexerMessage());

		// set message
		if (kwargs.unsafe_is<std::string>("message"))
		    _mxmsg->set_message(kwargs.get<const std::string&>("message"));
		else if (kwargs.unsafe_is<const std::string*>("message"))
		    _mxmsg->set_message(*kwargs.get<const std::string*>("message"));
		else
		    AssertMsg(false, "impossible");
		// type
		_mxmsg->set_type(kwargs.get<boost::uint32_t>("type"));
		// to
		_mxmsg->set_to(kwargs.get<boost::uint64_t>("to"));
		// references
		_mxmsg->set_references(
			kwargs.get<boost::uint64_t>("references"));
		// workflow
		if (kwargs.unsafe_is<const std::string*>("workflow"))
		    _mxmsg->set_workflow(*kwargs.get<const std::string*>("workflow"));
		else if (kwargs.unsafe_is<std::string>("workflow"))
		    _mxmsg->set_workflow(kwargs.get<const std::string&>("workflow"));

		mxmsg = _mxmsg.get();

	    } else {
		mxmsg = kwargs.get<const MultiplexerMessage*>("message");
	    }

	    if (kwargs.unsafe_is<int>("multiplexer")) {
		switch (kwargs.get<int>("multiplexer")) {
		    case ALL:
			return conn->schedule_all(*mxmsg);
		    case ONE:
			return conn->schedule_one(*mxmsg);
		    default:
			AssertMsg(false, "impossible");
		}
	    } else if (kwargs.unsafe_is<ConnectionWrapper>("multiplexer")) {
		return conn->schedule_one(*mxmsg, kwargs.get<const ConnectionWrapper&>("multiplexer"));
	    } 
	    AssertMsg(false, "impossible");
	    return false; // unreachable
	}

	void BaseMultiplexerServer::notify_start() {
	    DbgAssertMsg(!_has_sent_response, "If you use notify_start(), place it as a first function in your handle_message() code");
	    send_message(Kwargs()
		    .set("message", std::string(""))
		    .set("type", types::REQUEST_RECEIVED)
		    .set("references", last_mxmsg->id())
		);
	    _has_sent_response = false;
	}

	void BaseMultiplexerServer::__handle_message() {
	    _has_sent_response = false;
	    try {
		if (last_mxmsg->type() <= types::MAX_MULTIPLEXER_META_PACKET) {
		    __handle_internal_message();
		    if (!_has_sent_response) {
			AZOUK_LOG(WARNING, LOWVERBOSITY,
				MESSAGE("__handle_internal_message() finished w/o exception and w/o any response")
				);
		    }
		} else {
		    handle_message(*last_mxmsg);
		    if (!_has_sent_response) {
			AZOUK_LOG(WARNING, LOWVERBOSITY,
				MESSAGE("handle_message() finished w/o exception and w/o any response")
				);
		    }
		}
	    } catch (std::exception& e) {
		std::cerr << "Exception " << e.what() << " caught\n";
		if (!_has_sent_response) {
		    AZOUK_LOG(DEBUG, MEDIUMVERBOSITY,
			    MESSAGE(std::string("sending BACKEND_ERROR notification for Exception ") + e.what())
			);
		    //report_error(e.what()); // TODO
		}
		//bool handled = exception_occurred(e); // TODO
		//if (!handled) {
		    //throw;
		//}
	    }
	}

	void BaseMultiplexerServer::__handle_internal_message() {
	    const MultiplexerMessage& mxmsg = *last_mxmsg;
	    switch (mxmsg.type()) {
		case types::BACKEND_FOR_PACKET_SEARCH:
		    send_message(Kwargs()
			    .set("message", std::string())
			    //.set("flush", true)
			    .set("type", types::PING)
			    );
		    break;

		case types::PING:
		    if (!mxmsg.references()) {
			DbgAssert(mxmsg.id());
			send_message(Kwargs()
				.set("message", mxmsg.message())
				//.set("flush", true)
				.set("type", types::PING)
			    );
		    } else {
			no_response();
		    }
		    break;

		default:
		    AZOUK_LOG(ERROR, LOWVERBOSITY,
			    MESSAGE("received unknown meta-packet type=" + repr(mxmsg.type()))
			);
	    } // switch
	}

	void BaseMultiplexerServer::close() {
	    conn->shutdown();
	    __conn.reset();
	    conn = NULL;
	}

    };
};

