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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <boost/lexical_cast.hpp>
#include <boost/preprocessor/facilities/intercept.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/arithmetic/inc.hpp>
#include <boost/scoped_ptr.hpp>
//#include <google/protobuf/io/zero_copy_stream_impl.h>
//#include <google/protobuf/io/coded_stream.h>
//#include <google/protobuf/wire_format.h>
//#include <google/protobuf/wire_format_inl.h>
#include <azlib/util/Assert.h>
#include "azlib/initialization.h"
#include "azlib/random.h"
#include "azlib/util/str.h"
#include "azlib/protobuf/stream.h"

#include "logging.h"

namespace azlib {
    namespace logging {

	bool module_is_initialized = false;
	NoneType None;

	namespace impl {
	    // check that azlib/signals.h is initialized
	    AZOUK_TRIGGER_STATIC_INITILIZATION(Assert(azlib::signals::module_is_initialized), true);

	    boost::scoped_ptr<azlib::protobuf::MessageOutputStream> message_output_stream_;

	    std::string process_context_;

	    unsigned int maximal_logging_verbosity[MAX_LEVEL + 1] = {
		BOOST_PP_ENUM_PARAMS(BOOST_PP_INC(__AZOUK_LOGGING_MAX_LEVEL()), AZOUK_LOGGING_DEFAULT_VERBOSITY() BOOST_PP_INTERCEPT)
	    };

	    static std::string hostname;
	    static std::string process_name;

	    /*
	     * enum controlling the state of process_context_ variable
	     */
	    enum ContextAutoStateEnum { 
		NOTHING,
		SET_WITH_ALL_DEFAULTS,
		SET_WITH_DEFAULT_NAME,
		DURING_INITIALIZATION
	    } process_context_state_ = NOTHING;

	    /*
	     * initialize_hostname()
	     */
	    static inline void initialize_hostname() {
		std::string hostname;
		const unsigned int hostname_len_max = 1024;
		for (unsigned int hostname_len = 256; hostname_len < hostname_len_max + 1; hostname_len *= 2) {
		    hostname.resize(hostname_len);
		    if (gethostname(&hostname[0], hostname_len) == 0) {
			if (hostname_len < hostname_len_max && strlen(hostname.c_str()) >= hostname_len - 1)
			    continue;
			impl::hostname = hostname.c_str();
			break;
		    } else {
			perror("logging::impl::initialize_process_context_all_defaults");
			exit(EXIT_FAILURE);
		    }
		}
	    }
	    AZOUK_TRIGGER_STATIC_INITILIZATION(initialize_hostname(), hostname.empty());

	    static inline void initialize_process_name() {
		std::string proc = "/proc/" + boost::lexical_cast<std::string>(getpid()) + "/cmdline";
		std::ifstream cmdline(proc.c_str(), std::ofstream::binary);
		if (!cmdline) {
		    std::cerr << "logging::impl::initialize_process_context_all_defaults: " << proc << ": No such file or directory\n";
		    exit(EXIT_FAILURE);
		}

		cmdline >> proc; // read the process name and parameters
		std::string name = proc.c_str(); // extract the process name
		if (name.empty()) {
		    std::cerr << "logging::impl::initialize_process_context_all_defaults: coulnd't get the process name from /proc\n";
		}

		process_name.clear();
		std::string::size_type spos = name.rfind('/'); // get the basename
		process_name.append(name, (spos == std::string::npos && name.size() > spos) ? 0 : spos + 1, name.size());
	    }
	    AZOUK_TRIGGER_STATIC_INITILIZATION(initialize_process_name(), process_name.empty());

	    /*
	     * initialize_process_context_all_defaults)
	     *	    set process_context_ to <hostname>.<processname> or die
	     */
	    static inline void initialize_process_context_all_defaults() {
		Assert(process_context_state_ != DURING_INITIALIZATION);
		// set process_context_ temporarily
		process_context_ = "<unknown>";
		process_context_state_ = DURING_INITIALIZATION;

		// trigger initialization if not yet triggered
		if (hostname.empty()) initialize_hostname();
		if (process_name.empty()) initialize_process_name();
		Assert(!hostname.empty());
		Assert(!process_name.empty());
		
		// build the process context
		process_context_ = hostname + "." + process_name;
		process_context_state_ = SET_WITH_ALL_DEFAULTS;
	    }
	    AZOUK_TRIGGER_STATIC_INITILIZATION(initialize_process_context_all_defaults(), process_context_state_ == NOTHING);

	    void _emit_log(const LogEntry& log_msg) {
		if (!azlib::logging::module_is_initialized) {
		    std::cerr << "Warning: azlib::logging::_emit_log called before module_is_initialized. LogEntry ignored.\n";
		    return;
		}
		if (!message_output_stream_) {
		    // no logging stream set yet
		    return;
		}
		message_output_stream_->write(log_msg);
		message_output_stream_->flush();
	    }

	}; // namespace impl

	using namespace impl;

	void set_maximal_logging_verbosity(const unsigned int for_level, const unsigned int minimal_verbosity) {
	    Assert(for_level <= MAX_LEVEL);
	    maximal_logging_verbosity[for_level] = minimal_verbosity;
	}

	static inline void _shutdown_logging_streams() {
	    if (message_output_stream_) {
		message_output_stream_->flush();
		message_output_stream_.reset();
	    }
	}

	void set_logging_fd(unsigned int logging_fd, bool close_on_delete, bool log_the_fact) {
	    using namespace google::protobuf::io;
	    AssertMsg(module_is_initialized, "You can't call set_logging_fd before main()");

	    // create new CodedOutputStream based on logging_fd
	    _shutdown_logging_streams();
	    message_output_stream_.reset(new azlib::protobuf::FileMessageOutputStream(logging_fd, close_on_delete));
	    if (log_the_fact)
		AZOUK_LOG(DEBUG, LOWVERBOSITY, CTX("logging") TEXT("set logging FD to " + str(logging_fd)));
	}

	void set_logging_file(const std::string& file) {
	    using namespace google::protobuf::io;
	    AssertMsg(module_is_initialized, "You can't call set_logging_file before main()");

	    // create new CodedOutputStream based on file
	    int fd = open(file.c_str(), O_CREAT | O_WRONLY | O_APPEND, 0600);
	    if (fd < 0) {
		AZOUK_LOG(ERROR, LOWVERBOSITY, TEXT("Failed to open file '" + file + "' for writing logs.") CTX("logging"));

	    } else {
		try {
		    set_logging_fd(fd, /*close_on_delete*/ true, /*log_the_fact*/ false);
		} catch (...) {
		    close(fd);
		    throw;
		}
		AZOUK_LOG(DEBUG, LOWVERBOSITY, CTX("logging") TEXT("set logging file to '" + file + "'"));
	    }
	}

	boost::uint64_t create_log_id() {
	    static Random64 r;
	    return r();
	}

	void die(const std::string& text) {
	    AZOUK_LOG(ERROR, LOWVERBOSITY, TEXT(text) MUSTLOG);
	    signals::get_exit_signal()(1);
	}

	void set_process_context_program_name(const std::string& s) {
	    impl::process_context_ = hostname + "." + s;
	}

	AZOUK_TRIGGER_STATIC_INITILIZATION(atexit(_shutdown_logging_streams), true);
	// this should stay at the EOF
	AZOUK_TRIGGER_STATIC_INITILIZATION(module_is_initialized = true, true);
    }; // namespace logging
}; // namespace azlib
