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
#include <boost/asio/io_service.hpp>
#include "multiplexer/BasicClient.h"
#include "multiplexer/configuration.h"
#include "azlib/logging.h"
#include "azlib/repr.h"
#include "azlib/triple.h"
#include "azlib/vector.h"

namespace multiplexer {

    using boost::shared_ptr;
    using azlib::repr;
    using azlib::triple;
    using azlib::contains;

    typedef azlib::triple<boost::shared_ptr<const RawMessage>,
            ConnectionWrapper, boost::shared_ptr<MultiplexerMessage> >
                IncomingMessage;

    /**
     * @class Client
     *	    wrapper around BasicClient
     *	    that will have its counterpart written in Python (boost::python)
     */
    class Client : public ExceptionDefinitions {
    public:

        typedef BasicClient::BasicScheduledMessageTracker
            BasicScheduledMessageTracker;

	struct ScheduledMessageTracker {
	    ScheduledMessageTracker(BasicScheduledMessageTracker basic_tracker)
		: basic_tracker_(basic_tracker)
	    {}

	    inline operator bool () const { return (bool) basic_tracker_; }
	    bool inline in_queue() const {
                Assert(*this);
                return (bool) boost::logic::indeterminate(*basic_tracker_);
            }
	    bool inline is_sent() const {
                Assert(*this);
                return (bool) *basic_tracker_;
            }
	    bool inline is_lost() const {
                Assert(*this);
                return (bool) !*basic_tracker_;
            }
	private:
	    BasicScheduledMessageTracker basic_tracker_;
	};


	Client(boost::uint32_t client_type);
        Client(shared_ptr<boost::asio::io_service> io_service, boost::uint32_t
                client_type);
	Client(boost::asio::io_service& io_service, boost::uint32_t client_type);
	~Client();

	// connectivity
	void shutdown() { return basic_client_->shutdown(); }
        ConnectionWrapper async_connect(const boost::asio::ip::tcp::endpoint&
                peer_endpoint) {
            return basic_client_->async_connect(peer_endpoint);
        }
        bool wait_for_connection(ConnectionWrapper connwrap, float
                timeout=DEFAULT_TIMEOUT) {
            return basic_client_->wait_for_connection(connwrap, timeout);
        }
        ConnectionWrapper connect(const boost::asio::ip::tcp::endpoint& peer_endpoint,
                float timeout=DEFAULT_TIMEOUT) {
            return basic_client_->connect(peer_endpoint, timeout);
        }

	// IPv4 in nnn.nnn... form or IPv6 in hhh:hhh:... form
        ConnectionWrapper async_connect(const std::string& host, boost::uint16_t
                port) {
	    return async_connect(boost::asio::ip::tcp::endpoint(
                        boost::asio::ip::address::from_string(host), port));
	}
	// IPv4 in nnn.nnn... form or IPv6 in hhh:hhh:... form
        ConnectionWrapper connect(const std::string& host, boost::uint16_t port,
                float timeout=DEFAULT_TIMEOUT) {
            AZOUK_LOG(INFO, MEDIUMVERBOSITY, CTX("multiplexer.client")
                    MESSAGE("connecting to " + host + ":" + repr(port)));
            return connect(boost::asio::ip::tcp::endpoint(
                        boost::asio::ip::address::from_string(host), port),
                    timeout);
	}

	unsigned int inline connections_count() {
            return basic_client_->connections_count(true);
        }
	boost::uint64_t inline instance_id() const {
            return basic_client_->instance_id();
        }
	boost::uint32_t inline client_type() const {
            return basic_client_->client_type();
        }

	void set_multiplexer_password(
		const std::string& multiplexer_password) {
	    basic_client_->set_multiplexer_password(multiplexer_password);
	}

	inline const std::string& multiplexer_password() const {
	    return basic_client_->multiplexer_password();
	}


	// incoming messages
        BasicClient::IncomingMessagesBuffer::value_type read_raw_message(float
                timeout=DEFAULT_READ_TIMEOUT) {
            std::auto_ptr<azlib::SimpleTimer> timer =
                basic_client_->create_timer(timeout);
            return read_raw_message(*timer);
	}
        BasicClient::IncomingMessagesBuffer::value_type read_raw_message(
                azlib::SimpleTimer& timer) {
	    basic_client_->wait_for_incoming_message(timer);
	    Assert(basic_client_->has_incoming_messages());
	    return basic_client_->next_incoming_message();
        }

        std::pair<shared_ptr<MultiplexerMessage>, ConnectionWrapper>
                receive_message(float timeout=DEFAULT_READ_TIMEOUT) {

	    BasicClient::IncomingMessagesBuffer::value_type raw =
                read_raw_message(timeout);
	    return std::make_pair(raw.third, raw.second);
	}

	template <typename T>
	ScheduledMessageTracker schedule_one(const T& msg) {
            return ScheduledMessageTracker(
                    basic_client_->schedule_one(_serialize(msg)));
        }

	template <typename T>
        ScheduledMessageTracker schedule_one(const T& msg, ConnectionWrapper w,
                float timeout=DEFAULT_TIMEOUT) {
	    return ScheduledMessageTracker(
                    basic_client_->schedule_one(_serialize(msg), w, timeout));
	}

        void flush(ScheduledMessageTracker tracker,
                float timeout=DEFAULT_TIMEOUT) const {

	    std::auto_ptr<azlib::SimpleTimer> timer =
                basic_client_->create_timer(timeout);
            return flush(tracker, *timer);
	}

        void flush(ScheduledMessageTracker tracker,
                azlib::SimpleTimer& timer) const {

	    std::size_t n;
	    while (tracker && tracker.in_queue() && !timer.expired()) {
		n = io_service_.run_one();
		Assert(n);
	    }
        }

	void flush_all(float timeout=DEFAULT_TIMEOUT) const {

	    std::auto_ptr<azlib::SimpleTimer> timer =
                basic_client_->create_timer(timeout);
	    std::size_t n;

	    BasicClient::Connection::pointer conn;
            std::list<BasicClient::Connection::weak_pointer>
                current_connections;
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
		while (conni != connend &&
                        (!(conn = conni->lock()) ||
                             conn->outgoing_queue_empty())
			) {
		    ++ conni;
		}

		n = io_service_.run_one();
		Assert(n);
	    }
	}

        /**
         * query
         */
        std::pair<shared_ptr<const RawMessage>,
            shared_ptr<MultiplexerMessage> >
                query(shared_ptr<const MultiplexerMessage> mxmsg,
                        float timeout=DEFAULT_TIMEOUT) {
            return _query(*mxmsg, timeout);
        }

        std::pair<shared_ptr<const RawMessage>,
            shared_ptr<MultiplexerMessage> >
                query(const std::string& message, boost::uint32_t type, float
                        timeout=DEFAULT_TIMEOUT) {

            MultiplexerMessage mxmsg;
            mxmsg.set_id(random64());
            mxmsg.set_from(instance_id());
            mxmsg.set_type(type);
            mxmsg.set_message(message);
            return _query(mxmsg, timeout);
        }

	template <typename T>
	unsigned int schedule_all(T& msg) {
            return basic_client_->schedule_all(_serialize(msg));
        }

	template <typename T>
	unsigned int schedule_all(const T& msg) {
            return basic_client_->schedule_all(_serialize(msg));
        }

	azlib::Random64::result_type random64() const {
            return basic_client_->random64();
        }

    protected:

        std::pair<shared_ptr<const RawMessage>,
            shared_ptr<MultiplexerMessage> >
                _query(const MultiplexerMessage& query, float timeout) {

            bool first_request_delivery_errored = false;

            std::auto_ptr<azlib::SimpleTimer> timer;

            IncomingMessage result;

            try {
                timer = basic_client_->create_timer(timeout);
                result = _send_and_receive(query, *timer);
                if (result.third->type() != types::DELIVERY_ERROR)
                    return std::make_pair(result.first, result.third);

                first_request_delivery_errored = true;

            } catch (OperationTimedOut&) {
            }

            // Find working backends.
            BackendForPacketSearch search;
            search.set_packet_type(query.type());
            AZOUK_CREATE_MESSAGE(MultiplexerMessage, mxmsg,
                    (set_from(instance_id())) (set_id(random64()))
                    (set_type(types::BACKEND_FOR_PACKET_SEARCH))
                );
            search.SerializeToString(mxmsg.mutable_message());

            timer = basic_client_->create_timer(timeout);
            result = _send_and_receive(mxmsg, *timer, true, true,
                    query.id(), types::REQUEST_RECEIVED);

            if (result.third->type() == types::DELIVERY_ERROR) {
                if (first_request_delivery_errored)
                    MXTHROW(OperationFailed());
                timer = basic_client_->create_timer(timeout);
                std::vector<uint64_t> accept_ids;
                accept_ids.push_back(query.id());
                result = _receive(*timer, accept_ids, 0, -1);
                return std::make_pair(result.first, result.third);
            }
            if (result.third->references() == query.id())
                return std::make_pair(result.first, result.third);

            if (result.third->type() != types::PING)
                MXTHROW(OperationFailed());

            // Resent original query directly to the working backend.a
            AZOUK_CREATE_MESSAGE(MultiplexerMessage, direct_query,
                    (set_from(instance_id())) (set_id(random64()))
                    (set_to(result.third->from())) (set_type(query.type()))
                    (set_message(query.message()))
                );

            timer = basic_client_->create_timer(timeout);
            result = _send_and_receive(direct_query, *timer, false, false,
                    query.id(), types::REQUEST_RECEIVED, mxmsg.id(),
                    result.second);
            if (result.third->type() == types::DELIVERY_ERROR)
                MXTHROW(OperationFailed());
            return std::make_pair(result.first, result.third);
        }

        IncomingMessage _send_and_receive(
                    const MultiplexerMessage& mxmsg,
                    azlib::SimpleTimer& timer,
                    bool schedule_all=false,
                    bool handle_delivery_errors=false,
                    boost::uint64_t accept_id=0,
                    boost::uint32_t ignore_type=0,
                    boost::uint64_t ignore_id=-1,
                    ConnectionWrapper connection = ConnectionWrapper()
                ) {

            if (connection)
                flush(schedule_one(mxmsg, connection), timer);
            else
                flush(schedule_one(mxmsg), timer);

            IncomingMessage result;

            std::vector<uint64_t> accept_ids;
            accept_ids.push_back(mxmsg.id());
            if (accept_id)
                accept_ids.push_back(accept_id);

            if (schedule_all) {
                int sends = this->schedule_all(mxmsg);
                Assert(sends >= 0);
                while (!timer.expired()) {
                    result = _receive(timer, accept_ids, ignore_type,
                            ignore_id);
                    if (result.third->type() == types::DELIVERY_ERROR &&
                            handle_delivery_errors) {
                        if ((-- sends) > 0)
                            continue;
                    }
                    return result;
                }

            } else {
                ScheduledMessageTracker tracker = schedule_one(mxmsg);
                flush(tracker, timer);
                return _receive(timer, accept_ids, ignore_type, ignore_id);
            }
        }

        IncomingMessage _receive(azlib::SimpleTimer& timer,
                const std::vector<uint64_t>& accept_ids,
                uint32_t ignore_type,
                uint64_t ignore_id
                ) {

            IncomingMessage result;

            while (!timer.expired()) {
                result = read_raw_message(timer);
                const MultiplexerMessage& mxmsg = *result.third;

                if (mxmsg.type() == ignore_type)
                    continue;
                if (contains(accept_ids, mxmsg.references()))
                    return result;
                if (ignore_id != mxmsg.references()) {
                    AZOUK_LOG(WARNING, HIGHVERBOSITY,
                            MESSAGE("message (id=" + repr(mxmsg.id()) + ", type=" +
                                repr(mxmsg.type()) + ", from=" +
                                repr(mxmsg.from()) + ", references=" +
                                repr(mxmsg.references()) + ") while waiting "
                                "for reply for " + repr(accept_ids))
                            );
                }
            }

            MXTHROW(OperationTimedOut());
        }

        /**
         * _serialize
         */
        shared_ptr<const RawMessage> _serialize(
                const MultiplexerMessage& msg) {
	    return shared_ptr<const RawMessage>(
                    RawMessage::FromMessage(msg));
	}
	shared_ptr<const RawMessage> _serialize(
                std::string* serialized) {
	    return shared_ptr<const RawMessage>(
                    new RawMessage(serialized));
	}
	shared_ptr<const RawMessage> _serialize(
                shared_ptr<const RawMessage> raw) {
            return raw;
        }

    private:
	shared_ptr<boost::asio::io_service> io_service_ptr_;
    protected:
	boost::asio::io_service& io_service_;
	shared_ptr<BasicClient> basic_client_;
    };


}; // namespace multiplexer

#endif
