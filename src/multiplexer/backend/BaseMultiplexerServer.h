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

#ifndef BASEMULTIPLEXERSERVER_H
#define BASEMULTIPLEXERSERVER_H

#include <utility>
#include <string>
#include <vector>

#include <boost/cstdint.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/shared_ptr.hpp>

#include "azlib/util/Exception.h"
#include "azouk/util/kwargs.h"
#include "multiplexer/Client.h"
#include "multiplexer/Multiplexer.pb.h" /* generated */

namespace multiplexer {
    namespace backend {

	typedef std::pair<std::string, boost::uint16_t> MultiplexerAddress;
	typedef std::vector<MultiplexerAddress> MultiplexerAddresses;
	typedef boost::uint32_t PeerType;

	using azouk::util::kwargs::Kwargs;
	using azouk::util::kwargs::KwargsKeys;

	class BaseMultiplexerServer {

	public:
	    // for use with send_message(..., multiplexer=(ONE|ALL|a ConnectionWrapper), ...)
	    static const int ONE = 1;
	    static const int ALL = 2;

	protected:
	    BaseMultiplexerServer(const MultiplexerAddresses& addresses,
		    PeerType type);

	    BaseMultiplexerServer(multiplexer::Client* conn, PeerType type);

	public:
	    virtual ~BaseMultiplexerServer();

	    void serve_forever();

	    void loop() {
		serve_forever();
	    }

	protected:
	    virtual void handle_message(MultiplexerMessage&) = 0;

	protected:
	    template <typename Message>
	    static Message inline parse_message(const MultiplexerMessage& mxmsg) {
		return parse_message<Message>(mxmsg.message());
	    }

	    template <typename Message>
	    static Message inline parse_message(const std::string& from) {
		Message message;
		message.ParseFromString(from);
		return message;
	    }

	protected:
	    void notify_start();

	    /*
	     * required kwargs:
	     *	    message:	const MultiplexerMessage* OR
	     *			const std::string* OR
	     *			std::string
	     * possible kwargs:
	     *	    to:		boost::uint64_t
	     *	    references: boost::uint64_t
	     *	    type:	boost::uint32_t
	     *	    workflow:	std::string OR
	     *			const std::string*
	     *	    multiplexer:    int OR
	     *			    ConnectionWrapper
	     * TODO flush, timeout
	     * TODO other MultiplexerMessage keys ??
	     *
	     * returns
	     *	    TODO add doc on return type
	     */
	    boost::any send_message(Kwargs kwargs);

	    void no_response() {
		_has_sent_response = true;
	    }

	    void close();

	    void report_error(const std::string& message); // not_implemented
	    bool exception_occurred(const std::exception& e); // not_implemented
	    void set_exception_policy(); // not_implemented
	    void remove_exception_policy(); // not_implemented

	private:
	    void __handle_message();
	    void __handle_internal_message();

	public:
	    bool working;
	protected:
	    bool _has_sent_response;
	private:
	    boost::scoped_ptr<multiplexer::Client> __conn;
	protected:
	    multiplexer::Client* conn;
	    boost::shared_ptr<MultiplexerMessage> last_mxmsg;
	    ConnectionWrapper last_connwrap;
	private:
	};

    };
};

#endif
