#ifndef AZLIB_PREPROC_CREATE_MESSAGE_H
#define AZLIB_PREPROC_CREATE_MESSAGE_H

# /*
#  * azlib/preproc/create_message.h
#  *
#  *	Macro definitions for defining messages inline.
#  *
#  *	Can be used from within other macros (see logging) or
#  *	as a little dirty shorthand that saves typing the variable name in all lines.
#  */

#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/preprocessor/comparison/greater.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <boost/preprocessor/seq/push_back.hpp>
#include <boost/preprocessor/seq/seq.hpp>

#include "azlib/preproc/common.h"
#include "azlib/preproc/create_message/detail.h"

#if 0
# /*
#  * AZOUK_CREATE_MESSAGE_NO_NAME(type, setters)
#  *	Create an instance of type and runs the setters on it.
#  *	Here for illustrative purposes as it's not very useful -- you don't
#  *	control the name of the variable to which the new instance is bound.
#  */
# define AZOUK_CREATE_MESSAGE_NO_NAME(type, assignments_seq) \
    AZOUK_CREATE_MESSAGE(type, BOOST_PP_SEQ_CAT( (__) (type) (__data__) (__LINE__) ), assignments_seq)
#endif


# /*
#  * AZOUK_CREATE_MESSAGE(type, msg, assignments_seq)
#  *	Create an instance of type bound to the variable msg.
#  *	Then run setters on it.
#  *
#  *	Examples:
#  *	    AZOUK_CREATE_MESSAGE(MultiplexerMessage, mxmsg, (set_id(0)) (set_from(1)))
#  *	    AZOUK_CREATE_MESSAGE(MultiplexerMessage, mxmsg, BOOST_PP_SEQ_NIL)
#  */
#define AZOUK_CREATE_MESSAGE(type, msg, setters) \
    /* start AZOUK_CREATE_MESSAGE*/ \
    type msg; \
    AZOUK_INTIALIZE_MESSAGE(msg, setters); \
    // end AZOUK_CREATE_MESSAGE

# /*
#  * AZOUK_INTIALIZE_MESSAGE(msg, setters)
#  *	Run all setters on msg.
#  */
#define AZOUK_INTIALIZE_MESSAGE(msg, setters) \
    BOOST_PP_EXPR_IF( \
	    BOOST_PP_GREATER(BOOST_PP_SEQ_SIZE(BOOST_PP_SEQ_PUSH_BACK(setters, ~)), 1), \
	    /* do { BOOST_PP_SEQ_FOR_EACH(AZOUK_CREATE_MESSAGE__INVOKE_SETTER, msg, setters) } while (0) */ \
	    do { BOOST_PP_FOR((AZOUK_CREATE_MESSAGE__INVOKE_SETTER, msg, setters (nil)), \
		__AZOUK_INITIALIZE_MESSAGE_TUPLE_SIZE_GT_1, __AZOUK_INITIALIZE_MESSAGE_OP, \
		__AZOUK_INITIALIZE_MESSAGE_EXPAND) } while (0) \
	) \

# /*
#  * This macro definitions are copied from boost/preprocessor/seq/for_each.hpp
#  * to allow AZOUK_INTIALIZE_MESSAGE to be used inside BOOST_PP_SEQ_FOR_EACH iteration.
#  * Now the only limitation for AZOUK_INTIALIZE_MESSAGE is that it cannot be used inside of self, which is obvious.
#  */
#define __AZOUK_INITIALIZE_MESSAGE_TUPLE_SIZE_GT_1(r, state) BOOST_PP_GREATER(BOOST_PP_SEQ_SIZE(BOOST_PP_TUPLE_ELEM(3, 2, state)), 1)

#define __AZOUK_INITIALIZE_MESSAGE_OP(r, state) __AZOUK_INITIALIZE_MESSAGE_OP_I state
#define __AZOUK_INITIALIZE_MESSAGE_OP_I(macro, data, seq) (macro, data, BOOST_PP_SEQ_TAIL(seq))

#define __AZOUK_INITIALIZE_MESSAGE_EXPAND(r, x) __AZOUK_INITIALIZE_MESSAGE_EXPAND_IM(r, BOOST_PP_TUPLE_REM_3 x)
#define __AZOUK_INITIALIZE_MESSAGE_EXPAND_IM(r, im) __AZOUK_INITIALIZE_MESSAGE_EXPAND_I(r, im)
#define __AZOUK_INITIALIZE_MESSAGE_EXPAND_I(r, macro, data, seq) macro(r, data, BOOST_PP_SEQ_HEAD(seq))

#endif
