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

#ifndef MX_CONNECTION_H
#define MX_CONNECTION_H

#include <string>
#include <deque>

#include <boost/enable_shared_from_this.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/weak_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/numeric/conversion/cast.hpp>
#include <boost/random/linear_congruential.hpp>
#include <google/protobuf/message.h>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/read.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/placeholders.hpp>
#include <boost/asio/deadline_timer.hpp>

#include "azlib/util/type_functors.h"
#include "azlib/util/functors.h"
#include "azlib/repr.h"
#include "azlib/logging.h"
#include "multiplexer/multiplexer.constants.h" /* generated */
#include "multiplexer/io/RawMessage.h"
#include "multiplexer/configuration.h"

namespace multiplexer {

    //class ConnectionsManager;

    /**
     * @class Connection
     *	manages the connection to the MX server
     */
    template <class ConnectionsManagerImplementation>
    class Connection : public boost::enable_shared_from_this<Connection<ConnectionsManagerImplementation> > {

	public:
	    typedef multiplexer::ConnectionsManagerTraits<ConnectionsManagerImplementation> ConnectionsManagerTraits;
	    typedef typename ConnectionsManagerTraits::MessagesBufferTraits MessagesBufferTraits;
	    typedef std::deque<typename MessagesBufferTraits::value_type> MessagesBuffer;

	    struct ChannelState { enum _ChannelState { FREE, BUSY, BROKEN, READING_HEADER, READING_BODY }; };
	    typedef typename ChannelState::_ChannelState ChannelStateT;

	/* instance members */
	private:
	    Connection(boost::asio::io_service& io_service, boost::shared_ptr<ConnectionsManagerImplementation> manager)
		: socket_(io_service)
	        , peer_type_(0)
	        , peer_id_(0)
		, is_passive_(false)
	        , is_living_(false)
	        , shuts_down_(false)
	        , is_registered_(false)
	        , manager_(manager)
	        , outgoing_channel_state_(ChannelState::FREE)
	        , outgoing_queue_max_size_(1)
	        , incoming_message_(new RawMessage())
	        , incoming_channel_state_(ChannelState::FREE)
		, send_heartbit_timer_(io_service)
		, require_hearbit_timer_(io_service)
		//, send_heartbit_timer_(io_service, boost::posix_time::microseconds(HEARTBIT_INTERVAL * 1000000))
		//, require_hearbit_timer_(io_service, boost::posix_time::microseconds(NO_HEARTBIT_SO_DROP_INTERVAL * 1000000))
	    {
		// create heartbit message
		MultiplexerMessage mxmsg;
		mxmsg.set_id(0);
		mxmsg.set_type(types::HEARTBIT);
		mxmsg.set_from(manager->instance_id());
		heartbit_message_.reset(RawMessage::FromMessage(mxmsg));
	    }

	public:
	    typedef boost::shared_ptr<Connection> pointer;
	    typedef boost::weak_ptr<Connection> weak_pointer;
	    typedef boost::shared_ptr<ConnectionsManagerImplementation> ManagerPointer;

	    /**
	     * Create
	     *  A factory function that ensures that every instance is managed by a shared_ptr,
	     *  so that having a shared_ptr is always equal to having living instance.
	     */
	    static pointer Create(boost::asio::io_service& io_service, boost::shared_ptr<ConnectionsManagerImplementation> manager) {
		pointer p(new Connection(io_service, manager));
                AZOUK_LOG(DEBUG, HIGHVERBOSITY, MESSAGE("created new Connection " +
                            repr((void*)p.get())));
		return p;
	    }
	    
	    ~Connection() {
                AZOUK_LOG(DEBUG, HIGHVERBOSITY, MESSAGE("destroying Connection " +
                            repr((void*)this)));
		if (!shuts_down_)
		    shutdown();
	    }

            boost::asio::ip::tcp::socket& socket() { return socket_; }

	public:
	    ///* Connection's interface important functions */
	    // starts the processing with inital handshake and so on
	    void start() {
		start_only_read();
		start_rest();
	    }

	    void start_only_read() {
                AZOUK_LOG(DEBUG, HIGHVERBOSITY, MESSAGE("starting Connection " +
                            repr((void*)this)));
		Assert(!is_living_);
		Assert(!shuts_down_);
		is_living_ = true;
		_start_read();
	    }

	    void start_rest() {
		if (shuts_down_)
		    return;
		Assert(is_living_);
		ManagerPointer manager = manager_.lock();
		if (manager && schedule(manager->get_welcome_message(), true, true)) {
		    _send_heartbit_later(); // included in schedule() >> _process_send_queue()
		    _require_heartbit_later();
		} else {
		    shutdown();
		}
	    }

	    inline void set_is_passive(bool is_passive) {
		is_passive_ = is_passive;
		_require_heartbit_later();
	    }

	    void _handle_write_out_message(const boost::system::error_code& error, size_t bytes_transferred) {
		if (error) {
                    AZOUK_LOG(DEBUG, HIGHVERBOSITY,
                            MESSAGE("write error on " + repr((void*)this)
                                + " error=" + repr(error)
                                + " bytes_transferred=" +
                                repr(bytes_transferred)
                        ));
                }
	    }

	    // stops the processing ASAP
	    void shutdown(std::string out_message = "") {
                AZOUK_LOG(DEBUG, HIGHVERBOSITY, MESSAGE("shutdown called on " +
                            repr((void*)this)));
		if (shuts_down_) {
                    AZOUK_LOG(LOG_ERROR, HIGHVERBOSITY,
                            MESSAGE("shutdown called twice on " + repr((void*)this))
                        );
		    return;
		}
		if(out_message.compare("") != 0)
			boost::asio::async_write(socket_, boost::asio::buffer(out_message + "\n"), boost::bind(&Connection::_handle_write_out_message,
				this->shared_from_this(), /* making shared_ptr we ensure *this is not GCed in the middle */
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
			    );

		// mark as dead
		shuts_down_ = true;
		ManagerPointer manager = manager_.lock();

		// notify our Manager
		if (is_registered_ && manager) manager->unregister_connection(this);
		is_registered_ = false;
		if (manager) manager->connection_destroyed(this);
		is_living_ = false;

		// stop doing I/O
		outgoing_queue_max_size_ = 0;
		incoming_channel_state_ = ChannelState::BROKEN;
		outgoing_channel_state_ = ChannelState::BROKEN;
		socket_.cancel();
		try { socket_.shutdown(boost::asio::ip::tcp::socket::shutdown_both); } catch (...) {} // socket may be already closed in one or both directions
		incoming_message_.reset();

		// cancel outgoing messages
		_orphane_outgoing_messages();

		// die and let live or somehow ;)
		manager_.reset();
	    }

	    // schedules a message to be sent
	    //	`force' - skip checking queue fullness
	    //	`asap' - insert at the front of the queue
	    typename MessagesBufferTraits::SchedulingResultFunctor::result_type schedule(
		    boost::shared_ptr<const RawMessage> msg, bool force = false, bool asap = false) {

		using azlib::repr;

		if (!is_living_)
		    scheduling_result_type_default_functor_(); // drop

		Assert(msg->usability() == RawMessage::WRITING);
		Assert(!shuts_down_);

		if (!force && outgoing_queue_full()) {
                    AZOUK_LOG(WARNING, HIGHVERBOSITY,
                            MESSAGE("outgoing queue full, dropping message"));
		    return scheduling_result_type_default_functor_(); // drop
		}

		typename MessagesBufferTraits::ToBufferRepresentationConverter::result_type
		    value = raw_to_buffer_converter_(msg);
		if (asap) {
		    // insert `msg' as close to the outgoing_queue_.front() as possible
		    if (outgoing_channel_state_ == ChannelState::FREE) {
			Assert(outgoing_queue_.empty());
			outgoing_queue_.push_front(value);
		    } else {
			// outgoing_queue_.front() is being sent; let's insert msg just behind it
			Assert(!outgoing_queue_.empty());
			typename MessagesBuffer::iterator pos = outgoing_queue_.begin();
			Assert(pos != outgoing_queue_.end());
			++ pos;
			outgoing_queue_.insert(pos, value);
		    }
		} else {
		    outgoing_queue_.push_back(value);
		}

		_process_send_queue();
		AssertMsg(outgoing_queue_.empty() == (outgoing_channel_state_ == ChannelState::FREE),
			repr(outgoing_queue_.empty()) + " == (" + repr(outgoing_channel_state_)
			+ " == " + repr(ChannelState::FREE) + ") failed");
		return scheduling_result_type_functor_(value);
	    }

	    // schedule message previously scheduled with other Connection
	    bool inline take_over(typename MessagesBuffer::value_type internal) {
		if (!is_living_ || outgoing_queue_full())
		    return false;
		outgoing_queue_.push_back(internal);
		_process_send_queue();
		return true;
	    }

	    inline boost::uint32_t peer_type() const { return peer_type_; }
	    inline boost::uint64_t peer_id() const { return peer_id_; }
	    inline bool registered() const { return is_registered_; }

	    inline void set_outgoing_queue_max_size(unsigned int ms) { outgoing_queue_max_size_ = ms; }
	    inline bool outgoing_queue_full() const { return outgoing_queue_max_size_ <= outgoing_queue_.size(); }
	    inline bool outgoing_queue_empty() const { return outgoing_queue_.empty(); }
	    inline bool living() const { return is_living_; }
	    inline bool shuts_down() const { return shuts_down_; }

	private:
	    // keeping the connection
	    template <typename WaitHandler>
	    void _do_later(boost::asio::deadline_timer& timer, const float seconds, WaitHandler handler) {
		//timer.cancel();
                timer.expires_from_now(boost::posix_time::microseconds(
					       boost::numeric_cast<long>(seconds * 1e6)));
		timer.async_wait(handler);
	    }

	    void _send_heartbit_later() {
		if (outgoing_queue_.empty())
		    _do_later(send_heartbit_timer_, HEARTBIT_INTERVAL,
			    boost::bind(&Connection::_send_heartbit_now, this->shared_from_this(), boost::asio::placeholders::error));
	    }

	    void _send_heartbit_now(const boost::system::error_code& error) {
		if (error == boost::asio::error::operation_aborted || shuts_down_)
		    return;
		if (!schedule(heartbit_message_, true, true)) {
		    AZOUK_LOG(LOG_ERROR, MEDIUMVERBOSITY, CTX("connection")
			    MESSAGE("failed to schedule Heartbit message, we may lose the connection")
			    // TODO add peer_type() with DATA()
			);
		}
	    }

	    inline void _require_heartbit_later() {
		if (is_passive_) {
		    // we can't require Heartbits from passive clients
		    require_hearbit_timer_.cancel();
		    return;
		}

		_do_later(require_hearbit_timer_, NO_HEARTBIT_SO_PREPARE_DROP_INTERVAL,
			boost::bind(&Connection::_require_heartbit_soon, this->shared_from_this(), boost::asio::placeholders::error));
	    }

	    void _require_heartbit_soon(const boost::system::error_code& error) {
		// make dropping connections two-phase, so that
		// passive clients don't drop the connections after long period of idleness
		if (error == boost::asio::error::operation_aborted || shuts_down_)
		    return;
		_do_later(require_hearbit_timer_, NO_HEARTBIT_SO_REALLY_DROP_INTERVAL,
			boost::bind(&Connection::_require_heartbit_now, this->shared_from_this(), boost::asio::placeholders::error));
	    }

	    void _require_heartbit_now(const boost::system::error_code& error) {
		if (error == boost::asio::error::operation_aborted || shuts_down_)
		    return;
		// TODO logger.error << "no received messages for " << NO_HEARTBIT_SO_PREPARE_DROP_INTERVAL + NO_HEARTBIT_SO_REALLY_DROP_INTERVAL
		    //<< " seconds; shutting down... (peer_type = " << peer_type_ << ")";
		shutdown("no received messages for " + repr(NO_HEARTBIT_SO_PREPARE_DROP_INTERVAL + NO_HEARTBIT_SO_REALLY_DROP_INTERVAL) +
                    " seconds");
	    }

	    // receiving
	    void _start_read() {
		if (incoming_channel_state_ == ChannelState::BROKEN)
		    return;

		Assert(incoming_channel_state_ == ChannelState::FREE);

		incoming_channel_state_ = ChannelState::READING_HEADER;
                boost::asio::async_read(socket_, boost::asio::buffer(incoming_message_->get_header_buffer()),
			boost::bind(&Connection::_handle_read_header, this->shared_from_this(),
			    boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
			);
	    }
	    void _handle_read_header(const boost::system::error_code& error, size_t bytes_transferred) {
		if (incoming_channel_state_ == ChannelState::BROKEN)
		    return;

		if (bytes_transferred == 0) {
                    AZOUK_LOG(DEBUG, HIGHVERBOSITY,
                            MESSAGE("peer shut its end down \"gracefully\""));
		    incoming_channel_state_ = ChannelState::BROKEN; // this shouldn't be needed
		    shutdown();
		    return;
		}
		if (error) {
                    AZOUK_LOG(LOG_ERROR, HIGHVERBOSITY,
                            MESSAGE("read header error on " + repr((void*)this)
                                + "error=" + repr(error)
                                + "bytes_transferred=" + repr(bytes_transferred)
                        ));
		    shutdown("read header error=" + repr(error)
                                + "bytes_transferred=" + repr(bytes_transferred));
		    return;
		}

		Assert(incoming_channel_state_ == ChannelState::READING_HEADER);
		Assert(bytes_transferred == incoming_message_->get_header_length());
		if(!incoming_message_->unpack_header()){
		    AZOUK_LOG(LOG_ERROR, HIGHVERBOSITY,
                            MESSAGE("read header error on " + repr((void*)this)
                                + " declared contents is too long"));
		    shutdown("read header error declared contents is too long");
		    return;
		}
		incoming_channel_state_ = ChannelState::READING_BODY;
		_require_heartbit_later();
                boost::asio::async_read(socket_, boost::asio::buffer(incoming_message_->get_body_buffer()),
			boost::bind(&Connection::_handle_read_body, this->shared_from_this(), boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
			);
	    }
	    void _handle_read_body(const boost::system::error_code& error, size_t bytes_transferred) {
		if (incoming_channel_state_ == ChannelState::BROKEN)
		    return;

		if (error || bytes_transferred != incoming_message_->get_body_length()) {
                    AZOUK_LOG(LOG_ERROR, HIGHVERBOSITY,
                            MESSAGE("read body error on " + repr((void*)this)
                                + "error=" + repr(error)
                                + "bytes_transferred=" + repr(bytes_transferred)
                                + "expected_transferred=" +
                                   repr(incoming_message_->get_body_length())));
		    shutdown("read body error=" + repr(error)
                                + "bytes_transferred=" + repr(bytes_transferred)
                                + "expected_transferred=" +
                                   repr(incoming_message_->get_body_length()));
		    return;
		}

		Assert(bytes_transferred == incoming_message_->get_body_length());
		Assert(incoming_channel_state_ == ChannelState::READING_BODY);
		incoming_channel_state_ = ChannelState::FREE;

		if (!incoming_message_->verify()) {
		     AZOUK_LOG(LOG_ERROR, HIGHVERBOSITY,
                            MESSAGE("incomming message verification failed"));
		    shutdown("incomming message verification failed");

		} else {
		    _require_heartbit_later();
		    _receive_message();
		    _start_read();
		}
	    }

	    void _receive_message() {
		boost::shared_ptr<const RawMessage> message = incoming_message_;
		incoming_message_.reset(new RawMessage());
		
		ManagerPointer manager = manager_.lock();
		if (!manager) {
                    AZOUK_LOG(DEBUG, HIGHVERBOSITY,
                            MESSAGE("manager is gone for " + repr((void*)this)));
		    shutdown();
		    return;
		}

		boost::shared_ptr<MultiplexerMessage> mxmsg(new MultiplexerMessage());
		if (!mxmsg->ParseFromString(message->get_message())) {
                    AZOUK_LOG(DEBUG, HIGHVERBOSITY,
                            MESSAGE("received invalid message on " +
                                repr((void*)this) + "(maybe id=" +
                                repr(mxmsg->id()) + ")")
                        );
		    shutdown();
		    return;
		}

		if (_receive_internal_message(*mxmsg, manager))
		    return; // already handled

		manager->handle_message(this->shared_from_this(), message, mxmsg);
	    }

	    inline bool _receive_internal_message(MultiplexerMessage& mxmsg, ManagerPointer manager) {
		switch (mxmsg.type()) {
		    case types::CONNECTION_WELCOME:
			if (!is_registered_) {
			    WelcomeMessage welcome;
			    bool ok = false;
			    do {
				if (mxmsg.type() != types::CONNECTION_WELCOME) break;
				if (!welcome.ParseFromString(mxmsg.message())) break;
				if (!manager->check_multiplexer_password(welcome.multiplexer_password())) {
				    AZOUK_LOG(WARNING, MEDIUMVERBOSITY, CTX("connection")
					    MESSAGE("valid WelcomeMessage with invalid multiplexer_password rcvd")
					);
				    break;
				}
				ok = true;
			    } while (0);

			    if (!ok) {
				// TODO logger.error << "received invalid CONNECTION_WELCOME message; shutting down";
				shutdown("received invalid CONNECTION_WELCOME message; shutting down");

			    } else {
				peer_id_ = welcome.id();
				peer_type_ = welcome.type();
				manager->register_connection(this->shared_from_this(), welcome);
				manager->after_connection_registration(this->shared_from_this(), welcome);
				is_registered_ = true;
			    }
			} else {
			    // TODO logger.error << "received repeated CONNECTION_WELCOME message; shutting down";
			    shutdown("received repeated CONNECTION_WELCOME message; shutting down");
			}
			return true;
			// types::CONNECTION_WELCOME

		    case types::HEARTBIT:
			return true;

		    default:
			return false;
		}
	    }

	    // sending
	    void _process_send_queue() {
		// check if the outgoing channel is free and we have anything to send
		if (outgoing_channel_state_ == ChannelState::FREE && !outgoing_queue_.empty() && !shuts_down_) {
		    Assert(is_living_);
		    typename ConnectionsManagerTraits::MessagesBufferTraits::ToRawMessagePointerConverter to_raw_converter;
		    const typename ConnectionsManagerTraits::MessagesBufferTraits::ToRawMessagePointerConverter::result_type
			raw = to_raw_converter(outgoing_queue_.front());

		    // start writing
		    outgoing_channel_state_ = ChannelState::BUSY;
		    Assert(raw->get_message_buffer().size());
		    boost::asio::async_write(socket_, raw->get_message_buffer(),
			    boost::bind(&Connection::_handle_write,
				this->shared_from_this(), /* making shared_ptr we ensure *this is not GCed in the middle */
				boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred)
			    );
		}
	    }
	    void _handle_write(const boost::system::error_code& error, size_t bytes_transferred) {
                if (error) {
                    AZOUK_LOG(DEBUG, HIGHVERBOSITY,
                            MESSAGE("write error on " + repr((void*)this)
                                + " error=" + repr(error)
                                + " bytes_transferred=" +
                                repr(bytes_transferred)
                        ));
                }

		if (outgoing_channel_state_ == ChannelState::BROKEN)
		    return;
		if (bytes_transferred == 0) {
		    // is it exceptional?
                    AZOUK_LOG(DEBUG, HIGHVERBOSITY,
                            MESSAGE("i have managed to write nothing. Shut down")
                        );
		    shutdown();
		    return;
		}

		// TODO handle error conditions (pass it to handle_write(..., false) ???
		Assert(outgoing_channel_state_ == ChannelState::BUSY);
		outgoing_channel_state_ = ChannelState::FREE;

		Assert(outgoing_queue_.size());
		typename ConnectionsManagerTraits::MessagesBufferTraits::ToRawMessagePointerConverter to_raw_converter;
		const typename ConnectionsManagerTraits::MessagesBufferTraits::ToRawMessagePointerConverter::result_type
		    raw = to_raw_converter(outgoing_queue_.front());
		Assert(bytes_transferred == raw->get_header_length() + raw->get_body_length());

		message_sending_notifier_.notify_success(manager_, outgoing_queue_.front());
		outgoing_queue_.pop_front();
		_send_heartbit_later();

		Assert(error != boost::asio::error::message_size); /* was the message too big? */
		Assert(!error); // TODO handle this

		_process_send_queue();
	    }

	    void inline _orphane_outgoing_messages() {
		if (outgoing_queue_.empty())
		    return;
		if (ManagerPointer manager = manager_.lock()) {
		    manager->handle_orphaned_outgoing_messages(outgoing_queue_);
		    if (outgoing_queue_.empty())
			return;
		}
		// we haven't managed to transfer messages ownership to the manager for eventual resending
                AZOUK_LOG(WARNING, HIGHVERBOSITY,
                        MESSAGE("Connection shutdown on " + repr((void*)this)
                            + ", dropping about " + repr(outgoing_queue_.size())
                            + " outgoing messages"
                    ));
		BOOST_FOREACH (typename MessagesBuffer::value_type& qe, outgoing_queue_)
		    message_sending_notifier_.notify_error(manager_, qe);
		outgoing_queue_.clear();
	    }

	private:
	    typename ConnectionsManagerTraits::ConnectionManagerPrivateDataInConnection& managers_private_data() { return managers_private_data_; }
	    const typename ConnectionsManagerTraits::ConnectionManagerPrivateDataInConnection& managers_private_data() const { return managers_private_data_; }

	    /* members */
	private:
	    boost::asio::ip::tcp::socket socket_;
	    boost::uint32_t peer_type_;
	    boost::uint64_t peer_id_;
	    bool is_passive_;

	    bool is_living_;
	    bool shuts_down_;
	    bool is_registered_;
	    boost::weak_ptr<ConnectionsManagerImplementation> manager_;

	    /* outgoing channel */
	    MessagesBuffer outgoing_queue_;
	    ChannelStateT outgoing_channel_state_;
	    boost::uint32_t outgoing_queue_max_size_;

	    /* incoming channel */
	    boost::shared_ptr<RawMessage> incoming_message_;
	    ChannelStateT incoming_channel_state_;

	    /* heartbiting */
	    boost::asio::deadline_timer send_heartbit_timer_;
	    boost::asio::deadline_timer require_hearbit_timer_;
	    boost::shared_ptr<const RawMessage> heartbit_message_;

	    /* Manager's data pool */
	    typename ConnectionsManagerTraits::ConnectionManagerPrivateDataInConnection managers_private_data_;

	    /* some functors */
	    typename MessagesBufferTraits::SendingResultNotifier message_sending_notifier_;
	    typename MessagesBufferTraits::ToBufferRepresentationConverter raw_to_buffer_converter_;
	    typename MessagesBufferTraits::SchedulingResultFunctor scheduling_result_type_functor_;
	    azlib::DefaultConstructingFactory<typename MessagesBufferTraits::SchedulingResultFunctor::result_type> scheduling_result_type_default_functor_;

	    friend class azlib::IdentityTypeFunctor<ConnectionsManagerImplementation>::type;

    }; // class Connection


}; // namespace multiplexer

#endif
