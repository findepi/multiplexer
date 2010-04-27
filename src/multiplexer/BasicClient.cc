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

#include "config.h"
#include <boost/foreach.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include "multiplexer/multiplexer.constants.h"
#include "azlib/logging.h"
#include "azlib/repr.h"
#include "BasicClient.h"

using namespace azlib;
using namespace multiplexer;
using azlib::SimpleTimer;
using std::cerr;

BasicClient::BasicClient(boost::asio::io_service& io_service,
        boost::uint32_t client_type)
    : Base(io_service)
    , client_type_(client_type)
    , shuts_down_(false)
    , incoming_queue_max_size_(DEFAULT_INCOMING_QUEUE_MAX_SIZE)
{
}

void BasicClient::handle_message(Connection::pointer conn,
        boost::shared_ptr<const RawMessage> raw,
        boost::shared_ptr<MultiplexerMessage> mxmsg) {

    if (!mxmsg->id()) {
	// TODO logger.error << "received a message without id; dropping...";
	return;
    }

    if (!last_seen_message_ids_.insert(mxmsg->id())) {
	// dropping duplicated message
	return;
    }

    if (incoming_queue_full()) {
        AZOUK_LOG(WARNING, HIGHVERBOSITY, CTX("BasicClient.handle_message")
                MESSAGE("incoming_queue_full, dropping #" + repr(mxmsg->id()))
            );
	return; // drop
    }
    incoming_messages_.push_back(azlib::make_triple(raw,
                ConnectionWrapper(conn,
                    conn->managers_private_data().expected_endpoint),
                mxmsg));
}

// BasicClient::async_connect() helper function
static inline void handle_connect(BasicClient::Connection::pointer conn,
        const boost::system::error_code& error) {
    if (!error) {
	conn->start();
    } else {
	conn->shutdown();
    }
}

void BasicClient::shutdown() {
    shuts_down_ = true;
    for (ConnectionByEndpoint::iterator next = connection_by_endpoint_.begin(),
            i; next != connection_by_endpoint_.end() && (i = next++, true); )
	if (Connection::pointer conn = i->second.lock())
            // this does modify connection_by_endpoint_, so we have to use two
            // iterators
	    conn->shutdown();
}

void BasicClient::connection_destroyed(Connection* conn) {
    AZOUK_LOG(LOG_ERROR, LOWVERBOSITY, CTX("BasicClient")
            MESSAGE("connection_destroyed(" + repr(conn) + ")")
        );
    Connection::pointer c;
    ConnectionByEndpoint::iterator conni = connection_by_endpoint_.find(
                conn->managers_private_data().expected_endpoint);

    if (conni != connection_by_endpoint_.end() && (c = conni->second.lock()) &&
            c.get() == conn) {
	connection_by_endpoint_.erase(conni);
    }

    if (!shuts_down_) {
	// auto reconnect after AUTO_RECONNECT_TIME seconds
        AZOUK_LOG(DEBUG, LOWVERBOSITY, CTX("BasicClient")
                MESSAGE("scheduling reconnecting after " +
                    repr(AUTO_RECONNECT_TIME) + " seconds to " +
                    repr(conn->managers_private_data().expected_endpoint))
            );
        TimerPointer tp(new Timer(io_service_,
                    boost::posix_time::seconds(AUTO_RECONNECT_TIME)));
	tp->async_wait(boost::bind(&BasicClient::reconnect_after_timeout,
                    this->shared_from_this(),
		    tp, conn->managers_private_data().expected_endpoint,
		    boost::asio::placeholders::error
	    ));
    }
}

void BasicClient::reconnect_after_timeout(TimerPointer, Endpoint peer_endpoint,
        const boost::system::error_code& error) {
    if (!error) {
        if (connection_by_endpoint_.find(peer_endpoint) ==
                connection_by_endpoint_.end())
	    async_connect(peer_endpoint);
    } else {
        AZOUK_LOG(LOG_ERROR, HIGHVERBOSITY, CTX("BasicClient")
                MESSAGE("auto reconnect to " + repr(peer_endpoint) + "cancelled "
                    "by error" + repr(error))
            );
    }
}

ConnectionWrapper BasicClient::async_connect(
        const boost::asio::ip::tcp::endpoint& peer_endpoint) {

    // close any previous connections with the same endpoint
    ConnectionByEndpoint::iterator conni = connection_by_endpoint_.find(
            peer_endpoint);
    if (conni != connection_by_endpoint_.end()) {
	if (Connection::pointer conn = conni->second.lock())
	    conn->shutdown();
    }
#ifndef NDEBUG
    // discard weak references that point to no connections at all
    for (ConnectionByEndpoint::iterator next = connection_by_endpoint_.begin(),
            i; next != connection_by_endpoint_.end() && (i = next++, true); ) {
	if (!i->second.lock()) {
            AZOUK_LOG(LOG_ERROR, LOWVERBOSITY, CTX("BasicClient.connect")
                    MESSAGE("there should be no dangling weak references in "
                        "connection_by_endpoint_")
                );
	    connection_by_endpoint_.erase(i);
	}
    }
#endif

    // save newly created connection in the map
    Connection::pointer new_connection = Connection::Create(io_service_,
            this->shared_from_this());
    new_connection->managers_private_data().expected_endpoint = peer_endpoint;
    connection_by_endpoint_.insert(
            std::make_pair(peer_endpoint, new_connection));

    // start connection asynchronously
    new_connection->socket().async_connect(peer_endpoint,
            boost::bind(handle_connect, new_connection, _1));

    return ConnectionWrapper(new_connection,
            new_connection->managers_private_data().expected_endpoint);
}

bool BasicClient::wait_for_connection(ConnectionWrapper connwrap, float timeout)
        const {

    if (Connection::pointer conn = connwrap.lock()) {
	io_service_.reset();
	std::auto_ptr<SimpleTimer> timer = create_timer(timeout);
	Assert(timeout == 0 || !timer->expired());
	AZOUK_LOG(DEBUG, CHATTERBOX, CTX("basicClient")
                MESSAGE("waiting for connection " + repr(conn.get()) + "on IO=" +
                    repr(&io_service_))
            );
        while (!conn->shuts_down() && !conn->registered() && !timer->expired())
                {

	    unsigned int c = io_service_.run_one();
	    AssertMsg(c != 0, "io_service_.run_one() must return 1 here");
	}
	return conn->registered() && !conn->shuts_down();
    }
    return false;
}

ConnectionWrapper BasicClient::connect(
        const boost::asio::ip::tcp::endpoint& peer_endpoint, float timeout) {

    ConnectionWrapper connwrap = async_connect(peer_endpoint);
    wait_for_connection(connwrap, timeout);
    return connwrap;
}

BasicClient::IncomingMessagesBuffer::value_type
        BasicClient::next_incoming_message() {

    Assert(has_incoming_messages());
    IncomingMessagesBuffer::value_type next = incoming_messages_.front();
    incoming_messages_.pop_front();
    return next;
}


std::auto_ptr<azlib::SimpleTimer> BasicClient::create_timer(float timeout)
        const {

    using azlib::SimpleTimer;
    typedef std::auto_ptr<SimpleTimer> SimpleTimerPtr;
    if (timeout >= 0) {
	return SimpleTimerPtr(new SimpleTimer(io_service_, timeout));
    }
    SimpleTimerPtr ptr(new SimpleTimer(io_service_));
    Assert(ptr->expired() == false);
    Assert(ptr->expired() == false);
    Assert(ptr->expired() == false);
    return ptr;
}
