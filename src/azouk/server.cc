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


#include <cstdlib>
#include <boost/lexical_cast.hpp>
#include <boost/filesystem.hpp>
#include "azlib/program.h" /* main() */
#include "azlib/logging.h"

using namespace std;
using namespace azlib::logging::consts;

void usage(const char* progname) {
    std::cerr
	<< "Usage: " << progname << " [ [ host ] port ]\n"
	<< " Creates a MX server instance bound to host:port (default is 127.0.0.1:1980)\n"
	;
}

int AzoukMain(int argc, char** argv) {

    AZOUK_LOG(WARNING, LOWVERBOSITY, TEXT(string(argv[0]) + " is deprecated; use mxcontrol instead"));

    string host = "0.0.0.0";
    boost::uint16_t port = 1980;

    switch (argc) {
	case 0:
	    std::cerr << "you can't run server without argv[0]\n";
	    return 1;

	case 1:
	    // defaults are OK
	    break;
	case 2:
	    if (std::string("-h") == argv[1]) {
		usage(argv[0]);
		return 0;
	    }
	    port = boost::lexical_cast<boost::uint16_t>(argv[1]);
	    break;
	case 3:
	    host = argv[1];
	    port = boost::lexical_cast<boost::uint16_t>(argv[2]);
	    break;
	default:
	    usage(argv[0]);
	    return 1;
    }

    // export PATH=$PATH:`dirname $0`
    string path = argv[0];
    if (path.find('/') != string::npos) {
	path = boost::filesystem::path(path).remove_leaf().string(); // FIXME change remove_leaf() to remove_filename() when moved to boost 1.37
	char* PATH = getenv("PATH");
	setenv("PATH", (PATH ? string(PATH) + ":" + path : path).c_str(), 1);
    }

    return system((
		"mxcontrol run_multiplexer --address " + host + ":" + boost::lexical_cast<std::string>(port)
	    ).c_str());
}

