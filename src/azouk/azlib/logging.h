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

#ifndef AZLIB_LOGGING_H
#define AZLIB_LOGGING_H

#include <string>
#include <boost/preprocessor/stringize.hpp>
#include <boost/preprocessor/selection/max.hpp>
#include <boost/cstdint.hpp>

#include "azlib/preproc/create_message.h"
#include "azlib/signals.h" // this should be initialized before logging



# /*
#  * AZOUK_LOG(level, verbosity, tokens)
#  *	Emit log message with proper parameters runnin as little code as
#  *    possible it the message finally wounldn't be emitted.
#  *
#  *	`tokens' is a non empty list of:
#  *
#  *	    DEFAULT				no-op
#  *
#  *	    FLOW(flow)				add workflow information
#  *
#  *	    TEXT(text)				add human readable textual
#  *                                            information
#  *
#  *	    DATA(type_id, type, setters)	add data_type(type_id) information and generate inner data msg using setters
#  *						AZOUK_CREATE_MESSAGE(type, x, setters) must be valid
#  *
#  *	    CTX(context)			add context to process_context
#  *
#  *	    CONTEXT(context)			override process_context with context
#  *
#  *	    SKIPFILEIF(b)			don't emit log to logging stream if `b'
#  *
#  *
#  *	Examples (see azlib/util/str.h for information how to shorten lexical_cast):
#  *
#  *	    AZOUK_LOG(DEBUG, VERBOSE, CONTEXT("tarantula.multiplexer"));
#  *
#  *	    AZOUK_LOG(DEBUG, VERBOSE, CTX("multiplexer") FLOW(mxmsg.workflow())
#  *		    TEXT("received a message with id " +
#  *                        boost::lexical_cast<std::string>(mxmsg.id))
#  *		);
#  *
#  *	    AZOUK_LOG(DEBUG, VERBOSE, CONTEXT("tarantula.multiplexer")
#  *                FLOW(mxmsg.workflow())
#  *		    DATA(MALFORMED_MESSAGE_SO_SHUTDOWN, PeerCharacteristics,
#  *                    (set_peer_id(conn->peer_id()))
#  *                        (set_peer_type(conn->peer_type())))
#  *		);
#  *
#  */
#define AZOUK_LOG(tokens...) __AZOUK_LOG(AZOUK_UNIQUE_NAME(_log_msg_), \
        AZOUK_UNIQUE_NAME(_data_msg_), AZOUK_UNIQUE_NAME(_context_), tokens)

# /*
#  * helper for extracting maximal LEVEL or VERBOSITY
#  */
#define __AZOUK_LOGGING_MAX_OF_2ND_OR_PAIR_OP(s, state, t) BOOST_PP_MAX(state, \
        BOOST_PP_TUPLE_ELEM(2, 1, t))
#define __AZOUK_LOGGING_MAX_LEVEL() BOOST_PP_SEQ_FOLD_LEFT( \
        __AZOUK_LOGGING_MAX_OF_2ND_OR_PAIR_OP, 0, AZOUK_LOGGING_LEVELS_SEQ)
#define __AZOUK_LOGGING_MAX_VERBOSITY() BOOST_PP_SEQ_FOLD_LEFT( \
        __AZOUK_LOGGING_MAX_OF_2ND_OR_PAIR_OP, 0, AZOUK_LOGGING_VERBOSITIES_SEQ)


# /*
#  * AZOUK_ENTER(tokens...)
#  *    Logging macro used when entering a function.
#  *
#  *	`tokens' is a non empty list of:
#  *
#  *	    DEFAULT				no-op
#  *
#  *        LEVEL(level)                        set logging level of the
#  *                                            underlying AZOUK_LOG call
#  *
#  *        VERBOSITY(verbosity)                set logging verbosity of the
#  *                                            underlying AZOUK_LOG call
#  *
#  *	    CTX(context)			add context to process_context
#  *
#  *	    CONTEXT(context)			override process_context with
#  *                                            `context`
#  *
#  *	    FLOW(flow)				add workflow information
#  *
#  */
#define AZOUK_ENTER(tokens...)  __AZOUK_ENTER(tokens)

#define AZOUK_RETURN(value)     __AZOUK_RETURN(return, value)
#define AZOUK_LEAVE()           __AZOUK_RETURN(__AZOUK_LOG_CALL_return_void, \
                                    '<void>')

#define __AZOUK_LOG_CALL_return_void(void)  return;

namespace azlib {
    namespace logging {

	extern bool module_is_initialized;

	namespace consts {
	    /*
	     * logging levels
	     */
#define AZOUK_LOGGING_LEVELS_SEQ \
	    ((DEBUG, 1)) \
	    ((INFO, 2)) \
	    ((OK, 3)) \
	    ((WARNING, 4)) \
	    ((ERROR, 5)) \
	    ((CRITICAL, 6)) \
	    /**/

	    const static unsigned int MAX_LEVEL = __AZOUK_LOGGING_MAX_LEVEL();

	    /*
	     * verbosities
	     */
#define AZOUK_LOGGING_VERBOSITIES_SEQ \
	    /* allows to completely disable logging of specific LEVEL */ \
	    /* with set_maximal_logging_verbosity(LEVEL, ZEROVERBOSITY) */ \
	    ((ZEROVERBOSITY, 0)) \
	    \
	    /* messages that appear very rarely */ \
	    ((LOWVERBOSITY, 1)) \
	    \
	    /* messages that appear sometimes */ \
	    ((MEDIUMVERBOSITY, 2)) \
	    \
	    /* messages that are usually quite numerous */ \
	    ((HIGHVERBOSITY, 3)) \
	    \
	    /* messages that can flood you */ \
	    ((CHATTERBOX, 4)) \
	    /**/
#define AZOUK_LOGGING_DEFAULT_VERBOSITY() \
            (::azlib::logging::consts::HIGHVERBOSITY)

	    const static unsigned int MAX_VERBOSITY =
                __AZOUK_LOGGING_MAX_VERBOSITY();

	    /*
	     * logging_level_name<L>::name()
	     *	    Returns the level name known at compile time.
	     *	    Checks that the level L really is defined.
	     */
	    template <int> struct logging_level_name;

	    /*
	     * logging_get_level_name(level)
	     *	    Returns the level name not known at compile time.
	     */
	    static inline const char* logging_get_level_name(
                    const unsigned int level) AZOUK_ATTRIBUTE_ALWAYS_INLINE;

	    /*
	     * logging_verbosity_name<L>::name()
	     *	    Returns the verbosity name known at compile time.
	     *	    Checks that the verbosity L really is defined.
	     */
	    template <int> struct logging_verbosity_name;

	    /*
	     * logging_get_verbosity_name(verbosity)
	     *	    Returns the verbosity name not known at compile time.
	     */
	    static inline const char* logging_get_verbosity_name(
                    const unsigned int verbosity) AZOUK_ATTRIBUTE_ALWAYS_INLINE;

	}; // namespace consts

	/*
	 * process_context()
	 *	    Get context of the whole process as required for
         *	    AZOUK_LOG(., ., context).
	 */
	static inline const std::string& process_context()
            AZOUK_ATTRIBUTE_ALWAYS_INLINE;

	/*
	 * set_maximal_logging_verbosity(for_level, minimal_verbosity)
         *	Define what is the minimal verbosity of LogEntries with level
         *	for_level, which are emitted.
	 */
	void set_maximal_logging_verbosity(const unsigned int for_level,
                const unsigned int minimal_verbosity);

	/*
	 * set_logging_fd
	 *	Set to which FD logging stream is sent.
	 */
	void set_logging_fd(unsigned int logging_fd,
                bool close_on_delete = false, bool log_the_fact = true);

	void set_logging_file(const std::string& file);

	boost::uint64_t create_log_id();

	static inline const std::string& process_context();
	static inline void set_process_context(const std::string& s);
	void set_process_context_program_name(const std::string& s);

	void die(const std::string& text);

    }; // namespace logging
};

/*
 * export logging constants names
 */
using namespace azlib::logging::consts;

#include "azlib/logging/impl.h"


#endif
