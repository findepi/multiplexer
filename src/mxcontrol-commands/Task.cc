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
#include "Task.h"

namespace mxcontrol {

    void Task::_add_multiplexer_client_options(po::options_description& generic) {
	generic.add_options()
	    ("multiplexer,M", po::value(&multiplexers_)->composing(),
		"multiplexer server address in a form [hostip]:port; hostip defaults to 127.0.0.1\n"
		"may be repeated")
	    ("multiplexer-password,p", po::value(&multiplexer_password_),
		"password used when connecting to multiplexer server")
	    ;
    }

    void Task::__create_multiplexer_client(boost::uint32_t peer_type) {
	using std::string;
	using std::cerr;

	io_service(); // force *io_service_ instantiation
	multiplexer_client_.reset(new multiplexer::Client(io_service_, peer_type));
	multiplexer_client_->set_multiplexer_password(multiplexer_password_);
	if (multiplexers_.size()) {
	    BOOST_FOREACH(const std::string& address, multiplexers_) {
		string::size_type colonpos = address.find(':');
		if (colonpos == string::npos || colonpos != address.rfind(':')) {
		    AZOUK_LOG(WARNING, LOWVERBOSITY, MESSAGE("invalid MX server address '" + address + "' ignored"));
		} else {
		    string host = address.substr(0, colonpos);
		    boost::uint16_t port = boost::lexical_cast<boost::uint16_t>(address.substr(colonpos + 1, address.size()));
		    if (host.empty())
			host = "127.0.0.1";
		    multiplexer_client_->connect(host, port);
		}
	    }
	}
    }

    void Task::parse_options(std::vector<std::string>& args) {
	po::options_description cmdline_options;
	cmdline_options
	    .add(_options_description())
	    .add(_hidden_options_description())
	    ;
	po::parsed_options parsed = po::command_line_parser(args)
	    .options(cmdline_options)
	    .positional(_positional_options_description())
	    .run();
	po::variables_map vm;
	po::store(parsed, vm);
	po::notify(vm);
    }

    void Task::print_help(std::ostream& out) {
	out
	    << _options_description()
	    ;
    }
};
