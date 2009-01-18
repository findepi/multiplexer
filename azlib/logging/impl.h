#ifndef AZLIB_LOGGING_IMPL_H
#define AZLIB_LOGGING_IMPL_H

#include <time.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <sstream>
#include <boost/preprocessor/array/elem.hpp>
#include <boost/cstdint.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <google/protobuf/text_format.h>

#include "azlib/preproc/common.h"
#include "azlib/logging/log_tokens.h"
#include "build/azlib/logging/Logging.pb.h" /* generated */
#include "azlib/util/Assert.h"

# /*
#  * AZOUK_SHOULD_LOG(level, verbosity, context)
#  *	Checks if the current (level, verbosity) is logged from context.
#  *	Use this and don't call azlib::logging::should_log directly,
#  *	as context parameter may be not used.
#  */
#define AZOUK_SHOULD_LOG(level, verbosity, context) \
    (::azlib::logging::impl::should_log(level, verbosity))

# /*
#  * AZOUK_LOGGING_EMIT_LOG(level, verbosity, type, type_str, data_type_str, context, log_msg, data_msg)
#  *	Emit log with parameters.
#  *	Use this and don't call emit_log directly. Obviously.
#  */
#define AZOUK_LOGGING_EMIT_LOG(log_msg, data_msg, level, verbosity, log_flags) \
    (::azlib::logging::impl::emit_log(level, log_msg, data_msg, log_flags))

# /*
#  * AZOUK_LOGGING_CURRENT__FILE__
#  *	Where the AZOUK_LOG originated from.
#  */
//#if defined(__BASE_FILE__)
//# define AZOUK_LOGGING_CURRENT__FILE__() (__FILE__ ":" __BASE_FILE__)
//#else
# define AZOUK_LOGGING_CURRENT__FILE__() (__FILE__)
//#endif

# /*
#  * __AZOUK_LOG
#  *	    AZOUK_LOG's helper (implementation)
#  */
#define __AZOUK_LOG(log_msg, data_msg, contexttmp, level, verbosity, tokens) \
    /* start AZOUK_LOG */ \
    do { \
	::azlib::logging::NoneType& data_msg = ::azlib::logging::None; \
	::azlib::logging::impl::do_nothing(data_msg); /* so that we don't have warning if DATA(...) is used */ \
	bool __azouk_log_mustlog = false; \
	__AZOUK_LOG_PROCESS_TOKENS_BEFORE_CHECK( \
		(5, (log_msg, data_msg, contexttmp, level, verbosity)), \
		tokens \
	    ) \
	if (__azouk_log_mustlog || ::azlib::logging::impl::should_log(level, verbosity)) { \
	    ::std::string contexttmp = ::azlib::logging::process_context(); \
	    unsigned int __azouk_log_flags = ::azlib::logging::impl::NO_FLAGS; \
	    __AZOUK_LOG_PROCESS_TOKENS_AFTER_CHECK( \
		    (5, (log_msg, data_msg, contexttmp, level, verbosity)), \
		    tokens \
		) \
	    AZOUK_CREATE_MESSAGE(::azlib::logging::LogEntry, log_msg, \
		    (set_id(::azlib::logging::create_log_id())) \
		    (set_pid(getpid())) \
		    (set_level(level)) (set_verbosity(verbosity)) (set_context(contexttmp)) \
		    (set_timestamp(::azlib::logging::impl::current_timestamp())) \
		    (set_source_file(AZOUK_LOGGING_CURRENT__FILE__())) (set_source_line(__LINE__)) \
		    (set_compilation_datetime(__DATE__ " " __TIME__)) \
		); \
	    __AZOUK_LOG_PROCESS_TOKENS_BEFORE_EMIT( \
		    (5, (log_msg, data_msg, contexttmp, level, verbosity)), \
		    tokens \
		) \
	    AZOUK_LOGGING_EMIT_LOG(log_msg, data_msg, level, verbosity, __azouk_log_flags); \
	} \
    } while (0) \
    // end AZOUK_LOG


namespace azlib {
    namespace logging {

	struct NoneType {};
	extern NoneType None;

	namespace consts {

	    /*
	     * defintions of levels
	     *	    static const unsigned int DEBUG = ...;
	     *
	     * and specializations logging_level_name<L>
	     *	    logging_level_name<DEBUG>::name() { return "DEBUG"; }
	     */
#define AZOUK_define_logging_level(r, d, tup) \
	    static const unsigned int BOOST_PP_TUPLE_ELEM(2, 0, tup) = BOOST_PP_TUPLE_ELEM(2, 1, tup); \
	    template<> struct logging_level_name<BOOST_PP_TUPLE_ELEM(2, 0, tup)> \
		{ static inline const char* name() AZOUK_ATTRIBUTE_ALWAYS_INLINE { return BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2, 0, tup)); } };

	    BOOST_PP_SEQ_FOR_EACH(AZOUK_define_logging_level, ~, AZOUK_LOGGING_LEVELS_SEQ);

#undef AZOUK_define_logging_level

	    /*
	     * definition of logging_get_level_name
	     */
	    static inline const char* logging_get_level_name(const unsigned int level) {
		switch (level) {
		    /* generate the switch:
		     *	    case DEBUG: return "DEBUG";
		     *	    ...
		     */
#define AZOUK_get_logging_level_name(r, d, tup) \
		    case BOOST_PP_TUPLE_ELEM(2, 0, tup): return BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2, 0, tup));
		    BOOST_PP_SEQ_FOR_EACH(AZOUK_get_logging_level_name, ~, AZOUK_LOGGING_LEVELS_SEQ)
#undef AZOUK_get_logging_level_name
		    default: return "UNKNOWN";
		}
	    }

	    /*
	     * defintions of verbosities
	     *	    static const unsigned int LOWVERBOSITY = ...;
	     *
	     * and specializations logging_verbosity_name<L>
	     *	    logging_verbosity_name<LOWVERBOSITY>::name() { return "LOWVERBOSITY"; }
	     */
#define AZOUK_define_logging_verbosity(r, d, tup) \
	    static const unsigned int BOOST_PP_TUPLE_ELEM(2, 0, tup) = BOOST_PP_TUPLE_ELEM(2, 1, tup); \
	    template<> struct logging_verbosity_name<BOOST_PP_TUPLE_ELEM(2, 0, tup)> \
		{ static inline const char* name() AZOUK_ATTRIBUTE_ALWAYS_INLINE { return BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2, 0, tup)); } };

	    BOOST_PP_SEQ_FOR_EACH(AZOUK_define_logging_verbosity, ~, AZOUK_LOGGING_VERBOSITIES_SEQ)
#undef AZOUK_define_logging_verbosity

	    /*
	     * definition of logging_get_verbosity_name
	     */
	    static inline const char* logging_get_verbosity_name(const unsigned int verbosity) {
		switch (verbosity) {
		    /* generate the switch:
		     *	    case LOWVERBOSITY: return "LOWVERBOSITY";
		     *	    ...
		     */
#define AZOUK_get_logging_verbosity_name(r, d, tup) \
		    case BOOST_PP_TUPLE_ELEM(2, 0, tup): return BOOST_PP_STRINGIZE(BOOST_PP_TUPLE_ELEM(2, 0, tup));
		    BOOST_PP_SEQ_FOR_EACH(AZOUK_get_logging_verbosity_name, ~, AZOUK_LOGGING_VERBOSITIES_SEQ)
#undef AZOUK_get_logging_verbosity_name
		    default: return "UNKNOWN";
		}
	    }

	}; // namespace consts

	namespace impl {

	    // flags
	    static const unsigned int NO_FLAGS = 0;
	    static const unsigned int SKIP_LOGGING_TO_STREAM = 1;

	    /*
	     * do_nothing
	     *	function to fool compiler that a variable is used even if it's not
	     */
	    template <typename T>
	    inline void do_nothing(const T&) {}

	    extern std::string process_context_;
	    extern unsigned int maximal_logging_verbosity[];


	    /*
	     * current_timestamp()
	     *	-> current timestamp as uint64_t
	     */
	    static inline boost::uint64_t current_timestamp() AZOUK_ATTRIBUTE_ALWAYS_INLINE;
	    static inline boost::uint64_t current_timestamp() {
		return time(NULL);
	    }

	    /*
	     * should_log
	     *	Invoked from AZOUK_SHOULD_LOG and __AZOUK_LOG.
	     */
	    inline bool should_log(unsigned int level, unsigned int verbosity) AZOUK_ATTRIBUTE_ALWAYS_INLINE;
	    inline bool should_log(unsigned int level, unsigned int verbosity) {
		DbgAssert(level <= MAX_LEVEL);
		DbgAssert(verbosity <= MAX_VERBOSITY);
		return impl::maximal_logging_verbosity[level] >= verbosity;
	    }


	    /*
	     * _emit_log
	     *	Emit log to logging stream (no cerr).
	     */
	    void _emit_log(const LogEntry& log_msg);

	    /*
	     * emit_log
	     *  Write log_msg to cerr and logging stream.
	     *	Invoked from AZOUK_LOGGING_EMIT_LOG.
	     */

	    // shortcut for writing an (already initialized) LogEntry on cerr and on binary logging stream
	    static inline void emit_log(const unsigned int level, const LogEntry& log_msg, unsigned int flags = 0) {
		if (!(flags & SKIP_LOGGING_TO_STREAM))
		    _emit_log(log_msg);
		std::ostringstream cerr;
		cerr
		    << "[" << logging_get_level_name(level) << "]"
		    << "  ts=" << log_msg.timestamp()
		    << "  pid=" << log_msg.pid()
		    << "  ctx=" << log_msg.context()
		    << "  flw=\"" << log_msg.workflow() << "\""
		    << "  txt=\"" << log_msg.text() << "\""
		    ;
		if (log_msg.has_source_file()) {
		    cerr << "  from=" << log_msg.source_file();
		    if (log_msg.has_source_line())
			cerr << ":" << log_msg.source_line();
		}
		cerr << "\n";
		std::cerr << cerr.str();
	    }

	    // as the above but without level->str optimization
	    static inline void emit_log(const LogEntry& log_msg, unsigned int flags = 0) {
		return emit_log(log_msg.level(), log_msg, flags);
	    }

	    // specialization for DataType being NoneType (AZOUK_LOG called without DATA kwarg)
	    inline void emit_log(const unsigned int level, const LogEntry& log_msg, azlib::logging::NoneType&, unsigned int flags = 0) {
		return emit_log(level, log_msg, flags);
	    }

	    // specialization for DataType being a ProtoBuf Message
	    template <typename DataType>
	    inline typename boost::enable_if_c<boost::is_base_of<google::protobuf::Message, DataType>::value, void>::type
	    emit_log(const unsigned int level, LogEntry& log_msg, DataType& data_msg, unsigned int flags = 0) {
		data_msg.SerializeToString(log_msg.mutable_data());
		std::string text_format;
		google::protobuf::TextFormat::PrintToString(data_msg, &text_format);

		emit_log(level, log_msg, None, flags);
		std::cerr
		    << text_format << "\n"
		    << "\n"
		    ;
	    }


	}; // namespace impl

	static inline const std::string& process_context() {
	    return impl::process_context_;
	}

	static inline void set_process_context(const std::string& s) {
	    impl::process_context_ = s;
	}


    }; // namespace logging
}; // namespace azlib

#endif
