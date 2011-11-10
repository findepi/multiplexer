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
#include <boost/foreach.hpp>
#include <iomanip>
#include <iostream>
#include "mxcontrol-commands/TasksHolder.h"

#include "Help.h"

namespace mxcontrol {
    int Help::run() {
	std::ostream& output = show_help_forced ? std::cerr : std::cout;

	if (subcommand_.empty()) {
	    // general help
	    size_t longest_name = 0;
	    BOOST_FOREACH(const TasksHolder::TasksMap::value_type& entry, tasks_holder().tasks()) 
		longest_name = std::max(entry.first.size(), longest_name);

	    output
		<< "Usage: " << program_name << " <general-options> <command> <command-options>\n"
		<< "Sample usage: mxcontrol run_multiplexer 127.0.0.1:31889 --rules ../git_openbci/multiplexer.rules\n"
		<< "Commands:\n"
		;
	    BOOST_FOREACH(const TasksHolder::TasksMap::value_type& entry, tasks_holder().tasks()) {
		output << "  " << std::left << std::setw(longest_name + 4) << entry.first << entry.second->task()->short_description() << "\n";
	    }
	    output
		<< "\n"
		<< tasks_holder().general_options
		;
	} else if (!tasks_holder().is_command(subcommand_)) {
	    output
		<< program_name << ": unknown subcommand: " << subcommand_ << "\n"
		;
	    return 1;
	} else {
	    // help about a subcommand_
	    const TasksHolder::TasksMap::value_type& entry = *tasks_holder().tasks().find(subcommand_);
	    boost::shared_ptr<Task> t = entry.second->task();
	    output
		<< "Usage: " << program_name << " <general-options> " << entry.first << " " << t->short_synopsis(entry.first) << "\n"
		<< tasks_holder().general_options
		<< "\n"
		;
	    t->print_help(output);
	}

	return 0;
    }

    bool show_help_forced;
};

REGISTER_MXCONTROL_SUBCOMMAND(help, mxcontrol::Help);
