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
#include <deque>
#include <vector>
#include <map>
#include <boost/program_options.hpp>
#include <boost/foreach.hpp>
#include <boost/scoped_ptr.hpp>
#include "azlib/util/Assert.h"
#include "azlib/program.h" /* main() */
#include "mxcontrol-commands/TasksHolder.h"
#include "mxcontrol-commands/Help.h"

using namespace std;
using namespace mxcontrol;

const char* program_name = "mxcontrol";

int AzoukMain(int argc, char** argv) {

    Assert(argc > 0);
    std::vector<std::string> args(argv + 1, argv + argc);

    //TasksHolder tasks_holder;
    tasks_holder()
	    .set_original_args(argc, argv)
	;

    // build program options
    bool show_help;
    bool show_version;
    tasks_holder().general_options.add_options()
	("help", po::bool_switch(&show_help), "produce help message")
	("version", po::bool_switch(&show_version), "print version string")
	("logging-fd", po::value<boost::uint16_t>(), "descriptor, to which binary logging stream is sent")
	("logging-file", po::value<std::string>(), "binary logging stream file (if --logging-fd not set)")
	;

    // parse the commandline
    po::options_description cmdline_options;
    cmdline_options
	.add(tasks_holder().general_options)
	;

    po::parsed_options parsed = po::command_line_parser(argc, argv)
	.options(cmdline_options)
	.allow_unregistered()
	.run();
    po::variables_map vm;
    po::store(parsed, vm);
    po::notify(vm);

    std::vector<std::string> unrecognized_ = po::collect_unrecognized(parsed.options, po::include_positional);
    std::deque<std::string> unrecognized(unrecognized_.begin(), unrecognized_.end());
    std::vector<std::string>().swap(unrecognized_); // free all memory

    // see what're the results of parsing
    if (show_version) {
	unrecognized.push_front("version");
    }

    if (show_help) {
	unrecognized.push_front("help");
    } else if (!unrecognized.size()) {
	unrecognized.push_back("help");
	show_help_forced = true;
    }

    if (!tasks_holder().is_command(unrecognized[0])) {
	cerr << argv[0] << ": unknown command: " << unrecognized[0] << "\n";

	// print what commands are available
	if (tasks_holder().tasks().size()) {
	    cerr << "Available commands:\n";
	    BOOST_FOREACH(const TasksHolder::TasksMap::value_type& te, tasks_holder().tasks())
		cerr << "\t" << te.first << "\n";
	} else {
	    cerr << "There are no available commands.\n";
	}

	return EXIT_FAILURE;
    }
    
    // initialize env
    if (vm.count("logging-fd")) {
	azlib::logging::set_logging_fd(vm["logging-fd"].as<boost::uint16_t>());
    } else if (vm.count("logging-file")) {
	azlib::logging::set_logging_file(vm["logging-file"].as<std::string>());
    }

    // run the command
    return tasks_holder().run(unrecognized);
}
