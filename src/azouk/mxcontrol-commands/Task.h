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

#ifndef MXCONTROL_TASK_H
#define MXCONTROL_TASK_H

#include <ostream>
#include <vector>
#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/function.hpp>
#include "azlib/logging.h"
#include "multiplexer/Client.h"

namespace mxcontrol {

    namespace po = boost::program_options;

    class TasksHolder;

    class Task {
    public:
	//virtual po::options_description& options_description() { return _options_description(); }
	virtual ~Task() {}

	/*
	 * call parse_options before calling run()
	 */
	virtual void parse_options(std::vector<std::string>& args);

	/*
	 * do actual work
	 */
	virtual int run() = 0;

	/*
	 * describe itself
	 */
	virtual std::string short_description() const { return ""; }
	virtual std::string short_synopsis(const std::string& commandname) { return "<" + commandname + "-options>"; }

	/*
	 * used by help when printing subcommand extended info
	 */
	virtual void print_help(std::ostream&);

    protected:
	/*
	 * initializer functions that can be overwritten in a subclass
	 */
	virtual void _initialize_options_description(po::options_description&) {}
	virtual void _initialize_hidden_options_description(po::options_description&) {}
	virtual void _initialize_positional_options_description(po::positional_options_description&) {}

	/*
	 * use this in your implementations of _initialize*_options_description
	 * if you want use _multiplexer_client()
	 */
	void _add_multiplexer_client_options(po::options_description&);

	/*
	 * accesor functions that trigger lazy initialization
	 */
	inline po::options_description& _options_description() {
	    return _generic_create_options_description(options_description_, "Options", boost::bind(&Task::_initialize_options_description, this, _1));
	}
	inline po::options_description& _hidden_options_description() {
	    return _generic_create_options_description(hidden_options_description_, "Options", boost::bind(&Task::_initialize_hidden_options_description, this, _1));
	}
	inline po::positional_options_description _positional_options_description() {
	    return _generic_create_options_description(positional_options_description_, "", boost::bind(&Task::_initialize_positional_options_description, this, _1));
	}

	inline multiplexer::Client& _multiplexer_client(boost::uint32_t default_peer_type) {
	    if (!multiplexer_client_) {
		__create_multiplexer_client(default_peer_type);
	    }
	    return *multiplexer_client_;
	}

	inline boost::asio::io_service& io_service() {
	    if (!io_service_) {
		io_service_.reset(new boost::asio::io_service());
	    }
	    return *io_service_;
	}

    private:
	template <typename ValueType> inline ValueType* __construct(ValueType*, const std::string& param) const throw() { return new ValueType(param); }
	inline po::positional_options_description* __construct(po::positional_options_description*, const std::string&) const throw() { return new po::positional_options_description(); }

	void __create_multiplexer_client(boost::uint32_t peer_type);

	/*
	 * _generic_create_options_description
	 *	If `opptr' is a NULL ptr, create new ValueType passing name as a ctor argument (maybe changed by overloading __construct()).
	 *	If new value is created, it's initialized using `initializer', which should be of type `void(ValueType&)'.
	 */
	template <typename ValueType, typename InitializerFunction>
	inline ValueType& _generic_create_options_description(boost::scoped_ptr<ValueType>& opptr, const std::string& name, InitializerFunction initializer) const {
	    if (!opptr) {
		opptr.reset(__construct((ValueType*)NULL, name));
		initializer(*opptr);
	    }
	    return *opptr;
	}

    protected:
	std::string multiplexer_password_;

    private:
	boost::scoped_ptr<po::options_description> options_description_;
	boost::scoped_ptr<po::options_description> hidden_options_description_;
	boost::scoped_ptr<po::positional_options_description> positional_options_description_;

	boost::shared_ptr<boost::asio::io_service> io_service_;
	boost::scoped_ptr<multiplexer::Client> multiplexer_client_;

	std::vector<std::string> multiplexers_;
    }; // class Task
}; // namespace mxcontrol

#endif
