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

#ifndef MX_CLIENT_H
#define MX_CLIENT_H

#include <utility>
#include <boost/shared_ptr.hpp>
#include <boost/cstdint.hpp>
#include <asio/io_service.hpp>
#include "azlib/debug.h"
#include "multiplexer/BasicClient.h"
#include "multiplexer/configuration.h"
#include "azlib/logging.h"
#include "azlib/util/str.h"

namespace multiplexer {

    using azlib::str;

    /**
     * @class Client
     *	    (hopefully thin) wrapper around BasicClient
     *	    that will have its counterpart written in Python (boost::python)
     */
    class Client : public ExceptionDefinitions {
    public:

	typedef BasicClient::BasicScheduledMessageTracker BasicScheduledMessageTracker;
	struct ScheduledMessageTracker {
	    ScheduledMessageTracker(BasicScheduledMessageTracker basic_tracker)
		: basic_tracker_(basic_tracker)
	    {}

	    inline operator bool () const { return (bool) basic_tracker_; }
	    bool inline in_queue() const { Assert(*this); return (bool) boost::logic::indeterminate(*basic_tracker_); }
	    bool inline is_sent() const { Assert(*this); return (bool) *basic_tracker_; }
	    bool inline is_lost() const { Assert(*this); return (bool) !*basic_tracker_; }
	private:
	    BasicScheduledMessageTracker basic_tracker_;
	};


	Client(boost::uint32_t client_type);
	Client(boost::shared_ptr<asio::io_service> io_service, boost::uint32_t client_type);
	Client(asio::io_service& io_service, boost::uint32_t client_type);
	~Client();
	
	// connectivity
	void shutdown() { return basic_client_->shutdown(); }
	ConnectionWrapper async_connect(const asio::ip::tcp::endpoint& peer_endpoint) { return basic_client_->async_connect(peer_endpoint); }
	bool wait_for_connection(ConnectionWrapper connwrap, float timeout = DEFAULT_TIMEOUT) { return basic_client_->wait_for_connection(connwrap, timeout); }
	ConnectionWrapper connect(const asio::ip::tcp::endpoint& peer_endpoint, float timeout = DEFAULT_TIMEOUT) { return basic_client_->connect(peer_endpoint, timeout); }

	// IPv4 in nnn.nnn... form or IPv6 in hhh:hhh:... form
	ConnectionWrapper async_connect(const std::string& host, boost::uint16_t port) {
	    return async_connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port));
	}
	// IPv4 in nnn.nnn... form or IPv6 in hhh:hhh:... form
	ConnectionWrapper connect(const std::string& host, boost::uint16_t port, float timeout = DEFAULT_TIMEOUT) {
	    AZOUK_LOG(INFO, MEDIUMVERBOSITY, CTX("multiplexer.client") TEXT("connecting to " + host + ":" + str(port)));
	    return connect(asio::ip::tcp::endpoint(asio::ip::address::from_string(host), port), timeout);
	}

	unsigned int inline connections_count() { return basic_client_->connections_count(true); }
	boost::uint64_t inline instance_id() const { return basic_client_->instance_id(); }
	boost::uint32_t inline client_type() const { return basic_client_->client_type(); }

	void set_multiplexer_password(
		const std::string& multiplexer_password) {
	    basic_client_->set_multiplexer_password(multiplexer_password);
	}

	inline const std::string& multiplexer_password() const {
	    return basic_client_->multiplexer_password();
	}


	// incoming messages
	BasicClient::IncomingMessagesBuffer::value_type read_raw_message(float timeout = DEFAULT_READ_TIMEOUT) {
	    AZDEBUG_MSG("read_raw_message() called");
	    basic_client_->wait_for_incoming_message(timeout);
	    Assert(basic_client_->has_incoming_messages());
	    return basic_client_->next_incoming_message();
	}

	std::pair<boost::shared_ptr<MultiplexerMessage>, ConnectionWrapper> receive_message(float timeout = DEFAULT_READ_TIMEOUT) {
	    BasicClient::IncomingMessagesBuffer::value_type raw = read_raw_message(timeout);
	    return std::make_pair(raw.third, raw.second);
	}

	template <typename T>
	ScheduledMessageTracker schedule_one(const T& msg) { return ScheduledMessageTracker(basic_client_->schedule_one(_serialize(msg))); }
	
	template <typename T>
	ScheduledMessageTracker schedule_one(const T& msg, ConnectionWrapper w, float timeout = DEFAULT_TIMEOUT) {
	    return ScheduledMessageTracker(basic_client_->schedule_one(_serialize(msg), w, timeout));
	}

	void flush(ScheduledMessageTracker tracker, float timeout = DEFAULT_TIMEOUT) const {
	    std::auto_ptr<azlib::SimpleTimer> timer = basic_client_->create_timer(timeout);
	    std::size_t n;
	    while (tracker && tracker.in_queue() && !timer->expired()) {
		n = io_service_.run_one();
		Assert(n);
	    }
	}

	void flush_all(float timeout = DEFAULT_TIMEOUT) const {

	    std::auto_ptr<azlib::SimpleTimer> timer = basic_client_->create_timer(timeout);
	    std::size_t n;

	    BasicClient::Connection::pointer conn;
            std::list<BasicClient::Connection::weak_pointer> current_connections;
            for (BasicClient::ConnectionById::const_iterator conni =
                    basic_client_->begin(); conni !=  basic_client_->end();
                    ++conni) {
                current_connections.push_back(conni->second);
            }
            std::list<BasicClient::Connection::weak_pointer>::iterator
                conni = current_connections.begin(),
                connend = current_connections.end();

	    while (!timer->expired() && conni != connend) {

		// find a Connection that has some outgoing messages
		while (conni != connend && (
			    !(conn = conni->lock()) || conn->outgoing_queue_empty()
			)) {
		    ++ conni;
		}

		n = io_service_.run_one();
		Assert(n);
	    }
	}

	template <typename T>
	unsigned int schedule_all(T& msg) { return basic_client_->schedule_all(_serialize(msg)); }

	template <typename T>
	unsigned int schedule_all(const T& msg) { return basic_client_->schedule_all(_serialize(msg)); }

	azlib::Random64::result_type random64() const { return basic_client_->random64(); }

    protected:

	boost::shared_ptr<const RawMessage> _serialize(const MultiplexerMessage& msg) {
	    return boost::shared_ptr<const RawMessage>(RawMessage::FromMessage(msg));
	}
	boost::shared_ptr<const RawMessage> _serialize(std::string* serialized) {
	    return boost::shared_ptr<const RawMessage>(new RawMessage(serialized));
	}
	boost::shared_ptr<const RawMessage> _serialize(boost::shared_ptr<const RawMessage> raw) { return raw; }

    private:
	boost::shared_ptr<asio::io_service> io_service_ptr_;
    protected:
	asio::io_service& io_service_;
	boost::shared_ptr<BasicClient> basic_client_;
    };


}; // namespace multiplexer

#endif
