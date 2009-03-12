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

#ifndef MXCONTROL_HELP_H
#define MXCONTROL_HELP_H

#include "mxcontrol-commands/Task.h"

namespace mxcontrol {

    class Help : public Task {
    public:
	virtual int run();
	virtual std::string short_description() const { return "get some help"; }
	virtual std::string short_synopsis(const std::string&) const { return "[subcommand]"; }
	virtual void print_help(std::ostream& out) const {
	    out << "Get a list of available commands.\n";
	    out << "If subcommand is given, get subcommand options and usage information insted.\n";
	}

    protected:
	virtual void _initialize_hidden_options_description(po::options_description& hidden) {
	    hidden.add_options()
		("_subcommand_", po::value(&subcommand_))
		;
	}
	virtual void _initialize_positional_options_description(po::positional_options_description& positional) {
	    positional.add("_subcommand_", 1);
	}

    private:
	inline std::string _program_name() const { return tasks_holder().original_argc() ? tasks_holder().original_argv()[0] : "program"; }

    private:
	std::string subcommand_;
    };
};

#endif
