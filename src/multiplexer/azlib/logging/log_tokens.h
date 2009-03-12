#ifndef AZLIB_LOGGING_LOG_TOKENS_H
#define AZLIB_LOGGING_LOG_TOKENS_H
// vim:tw=0

#include "azlib/preproc/kwargs.h"

# /*
#  * helper macro definitions for __AZOUK_LOG and __AZOUK_ENTER
#  */

# /*
#  * __AZOUK_LOG_PROCESS_TOKENS_*
#  *	Called in __AZOUK_LOG at various stages.
#  */
#define __AZOUK_LOG_PROCESS_TOKENS_BEFORE_CHECK(data, tokens)	AZOUK_PP_KWARGS_PROCESS(__AZOUK_LOG_KW_BEFORE_CHECK_, data, tokens)

# /*
#  *	each token AA must be implement in azlib/preproc/kwargs.h
#  *	each token must provide support for all places it can occur in
#  */

# // __AZOUK_LOG_KW_BEFORE_CHECK_ namespace
#define __AZOUK_LOG_KW_BEFORE_CHECK_FLOW(flow, data)
#define __AZOUK_LOG_KW_BEFORE_CHECK_TEXT(text, data)
#define __AZOUK_LOG_KW_BEFORE_CHECK_DATA(type_id, type, setters, data)
#define __AZOUK_LOG_KW_BEFORE_CHECK_CTX(context, data)
#define __AZOUK_LOG_KW_BEFORE_CHECK_CONTEXT(context, data)
#define __AZOUK_LOG_KW_BEFORE_CHECK_MUSTLOG(data)			__azouk_log_mustlog = true;
#define __AZOUK_LOG_KW_BEFORE_CHECK_SKIPFILEIF(b, data)

# /*
#  * __AZOUK_EMIT_PROCESS_TOKENS_*
#  *	Called in __AZOUK_EMIT at various stages.
#  *
#  *    They must implement all tokens provided by callers, e.g. by __AZOUK_LOG
#  *    and __AZOUK_ENTER.
#  */
#define __AZOUK_EMIT_PROCESS_TOKENS_AFTER_CHECK(data, tokens)	AZOUK_PP_KWARGS_PROCESS(__AZOUK_EMIT_KW_AFTER_CHECK_, data, tokens)
#define __AZOUK_EMIT_PROCESS_TOKENS_BEFORE_EMIT(data, tokens)	AZOUK_PP_KWARGS_PROCESS(__AZOUK_EMIT_KW_BEFORE_EMIT_, data, tokens)

# // __AZOUK_EMIT_KW_AFTER_CHECK_ namespace
#define __AZOUK_EMIT_KW_AFTER_CHECK_FLOW(flow, data)
#define __AZOUK_EMIT_KW_AFTER_CHECK_TEXT(text, data)
#define __AZOUK_EMIT_KW_AFTER_CHECK_DATA(type_id, type, setters, data)
#define __AZOUK_EMIT_KW_AFTER_CHECK_CTX(context, data)			BOOST_PP_ARRAY_ELEM(2, data).append(".").append(context);
#define __AZOUK_EMIT_KW_AFTER_CHECK_CONTEXT(context, data)		BOOST_PP_ARRAY_ELEM(2, data) = (context);
#define __AZOUK_EMIT_KW_AFTER_CHECK_MUSTLOG(data)
#define __AZOUK_EMIT_KW_AFTER_CHECK_SKIPFILEIF(b, data)			if (b) { __azouk_log_flags |= ::azlib::logging::impl::SKIP_LOGGING_TO_STREAM; } else {}
# // added to support __AZOUK_ENTER calls
#define __AZOUK_EMIT_KW_AFTER_CHECK_LEVEL(level, data)
#define __AZOUK_EMIT_KW_AFTER_CHECK_VERBOSITY(verbosity, data)

# // __AZOUK_EMIT_KW_BEFORE_EMIT_ namespace
#define __AZOUK_EMIT_KW_BEFORE_EMIT_FLOW(flow, data)			BOOST_PP_ARRAY_ELEM(0, data).set_workflow(flow);
#define __AZOUK_EMIT_KW_BEFORE_EMIT_TEXT(text, data)			BOOST_PP_ARRAY_ELEM(0, data).set_text(text);
#define __AZOUK_EMIT_KW_BEFORE_EMIT_DATA(type_id, type, setters, data)  AZOUK_CREATE_MESSAGE( type, BOOST_PP_ARRAY_ELEM(1, data), setters ); \
									BOOST_PP_ARRAY_ELEM(0, data).set_data_type(type_id); \
									BOOST_PP_ARRAY_ELEM(0, data).set_data_class(#type);
#define __AZOUK_EMIT_KW_BEFORE_EMIT_CTX(context, data)
#define __AZOUK_EMIT_KW_BEFORE_EMIT_CONTEXT(context, data)
#define __AZOUK_EMIT_KW_BEFORE_EMIT_MUSTLOG(data)
#define __AZOUK_EMIT_KW_BEFORE_EMIT_SKIPFILEIF(b, data)
# // added to support __AZOUK_ENTER calls
#define __AZOUK_EMIT_KW_BEFORE_EMIT_LEVEL(level, data)
#define __AZOUK_EMIT_KW_BEFORE_EMIT_VERBOSITY(verbosity, data)


# /*
#  * __AZOUK_ENTER_PROCESS_TOKENS_*
#  *	Called in __AZOUK_ENTER at various stages.
#  */
#define __AZOUK_ENTER_PROCESS_TOKENS_BEFORE_CHECK(data, tokens) AZOUK_PP_KWARGS_PROCESS(__AZOUK_ENTER_KW_BEFORE_CHECK_, data, tokens)

# // __AZOUK_ENTER_KW_BEFORE_CHECK_ namespace
#define __AZOUK_ENTER_KW_BEFORE_CHECK_LEVEL(level, data)                BOOST_PP_ARRAY_ELEM(0, data) = (level);
#define __AZOUK_ENTER_KW_BEFORE_CHECK_VERBOSITY(verbosity, data)        BOOST_PP_ARRAY_ELEM(1, data) = (verbosity);
#define __AZOUK_ENTER_KW_BEFORE_CHECK_CTX(context, data)                BOOST_PP_ARRAY_ELEM(2, data).append(".").append(context)
#define __AZOUK_ENTER_KW_BEFORE_CHECK_CONTEXT(context, data)            BOOST_PP_ARRAY_ELEM(2, data) = (context);


// vim:tw=0:
#endif
