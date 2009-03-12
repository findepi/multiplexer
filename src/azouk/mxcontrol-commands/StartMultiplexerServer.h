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


#include "mxcontrol-commands/Task.h"

namespace mxcontrol {

    class StartMultiplexerServer : public Task {
    public:
	virtual int run();
	virtual std::string short_synopsis(const std::string& commandname) { return "<" + commandname + "-options> [--address] address:port"; }

    protected:
	virtual void _initialize_options_description(po::options_description& generic) {
	    generic.add_options()
		("rules", po::value(&rules_file_)
			->default_value("multiplexer.rules"),
		    "file from which routing rules will be read")
		("address,M", po::value(&host_port_)
			->default_value("0.0.0.0:1980"),
		    "local address to listen on")
		("multiplexer-password,p", po::value(&multiplexer_password_),
		    "password required when authorizing new connections")
		;
	}

	virtual void _initialize_positional_options_description(po::positional_options_description& positional) {
	    positional.add("address", 1);
	}

    private:
	std::string host_port_;
	std::string rules_file_;
    };

};
