
#include <boost/foreach.hpp>
#include <iomanip>
#include <iostream>
#include "mxcontrol-commands/TasksHolder.h"

#include "Help.h"

namespace mxcontrol {
    int Help::run() {
	if (subcommand_.empty()) {
	    // general help
	    size_t longest_name = 0;
	    BOOST_FOREACH(const TasksHolder::TasksMap::value_type& entry, tasks_holder().tasks()) 
		longest_name = std::max(entry.first.size(), longest_name);

	    std::cerr
		<< "Usage: " << _program_name() << " <general-options> <command> <command-options>\n"
		<< "Commands:\n"
		;
	    BOOST_FOREACH(const TasksHolder::TasksMap::value_type& entry, tasks_holder().tasks()) {
		std::cerr << "  " << std::left << std::setw(longest_name + 4) << entry.first << entry.second->task()->short_description() << "\n";
	    }
	    std::cerr
		<< "\n"
		<< tasks_holder().general_options
		;
	} else if (!tasks_holder().is_command(subcommand_)) {
	    std::cerr
		<< _program_name() << ": unknown subcommand: " << subcommand_ << "\n"
		;
	    return 1;
	} else {
	    // help about a subcommand_
	    const TasksHolder::TasksMap::value_type& entry = *tasks_holder().tasks().find(subcommand_);
	    boost::shared_ptr<Task> t = entry.second->task();
	    std::cerr
		<< "Usage: " << _program_name() << " <general-options> " << entry.first << " " << t->short_synopsis(entry.first) << "\n"
		<< tasks_holder().general_options
		<< "\n"
		;
	    t->print_help(std::cerr);
	}

	return 0;
    }
};

REGISTER_MXCONTROL_SUBCOMMAND(help, mxcontrol::Help);
