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


#include <boost/asio.hpp>
#include "multiplexer/Server.h"
#include "azlib/repr.h"
#include "mxcontrol-commands/StartMultiplexerServer.h"
#include "mxcontrol-commands/TasksHolder.h"

namespace mxcontrol {

    int StartMultiplexerServer::run() {
	using std::string;
	using azlib::repr;

	string host = host_port_;
	boost::uint16_t port = 1980;

	string::size_type colonpos = host_port_.find(':');
	Assert(colonpos < host_port_.size() || colonpos == string::npos);

	if (colonpos < host_port_.size()) {
	    // port specified
	    Assert(host_port_[colonpos] == ':');
	    string(&host_port_[0], &host_port_[colonpos]).swap(host);
	    string portstring(&host_port_[0] + colonpos + 1, &host_port_[0] + host_port_.size());
	    AssertMsg(portstring.find(':') == string::npos, "Invalid address spec: two colons");
	    port = boost::lexical_cast<boost::uint16_t>(portstring);
	}

	// TODO support for name resolving (e.g. host = "localhost" by default)
        boost::asio::io_service io_service;
	multiplexer::Server::pointer server = multiplexer::Server::Create(io_service, host, port);
	server->clear_rules();
	server->read_rules(rules_file_);
	server->set_multiplexer_password(multiplexer_password_);
	server->start();
	AZOUK_LOG(INFO, LOWVERBOSITY, TEXT("starting MX server on " + host + ":" + repr(port)));
	io_service.run();

	return 0;
    }
};

REGISTER_MXCONTROL_SUBCOMMAND(run_multiplexer, mxcontrol::StartMultiplexerServer);
