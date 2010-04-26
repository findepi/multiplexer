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

#include <boost/asio/placeholders.hpp>
#include <boost/foreach.hpp>
#include <boost/asio/ip/tcp.hpp>
//#include "multiplexer/io/MessageHandler.h"
#include "multiplexer/Multiplexer.pb.h" /* generated */

#include "Server.h"

using namespace multiplexer;

using std::cerr;
using std::endl;


Server::Server(boost::asio::io_service& io_service, const std::string& host, unsigned short port)
    : Base(io_service)
      // TODO support for name resolving
    , acceptor_(io_service, boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string(host), port))
    //, acceptor_(io_service, asio::ip::tcp::endpoint(
		////asio::ip::address_v4(
		    //*asio::ip::tcp::resolver(io_service).resolve(asio::ip::tcp::resolver_query(host))
		    ////)
		    //,
		//port))
{
}

void Server::start() { _start_accept(); }

void Server::handle_message(Connection::pointer conn, boost::shared_ptr<const RawMessage> raw, boost::shared_ptr<MultiplexerMessage> msg) {
    if (msg->from() == instance_id_) {
	AZOUK_LOG(ERROR, MEDIUMVERBOSITY, CTX("multiplexer.server") MESSAGE("received message from self")
		FLOW(msg->workflow())
		SKIPFILEIF(!(msg->logging_method() & multiplexer::LoggingMethod::FILE))
	    );
	return;
    }
    return _handle_message(conn, *msg, raw);
}
