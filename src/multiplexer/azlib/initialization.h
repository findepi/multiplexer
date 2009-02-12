#ifndef AZLIB_INITIALIZATION_H
#define AZLIB_INITIALIZATION_H

#include "azlib/preproc/common.h"

# /*
#  * AZOUK_TRIGGER_STATIC_INITILIZATION(stmt, condition)
#  *	statically:
#  *	    if condition:
#  *		stmt
#  *
#  * Note:
#  *	stmt must be a valid statement (if it's function call, it must have '(' ')').
#  *	This allows to call initializers taking arguments;
#  */
#define AZOUK_TRIGGER_STATIC_INITILIZATION(stmt, condition) \
    AZOUK_TRIGGER_STATIC_INITILIZATION_CODE((stmt), condition)

# /*
#  * AZOUK_TRIGGER_STATIC_INITILIZATION_CODE
#  *	Like AZOUK_TRIGGER_STATIC_INITILIZATION but code can be any C++ code
#  *	in braces.
#  */
#define AZOUK_TRIGGER_STATIC_INITILIZATION_CODE(code, condition) \
    AZOUK_TRIGGER_STATIC_INITILIZATION_CODE_NAME(code, _, condition)

# /*
#  * AZOUK_TRIGGER_STATIC_INITILIZATION_CODE_NAME
#  *	Like AZOUK_TRIGGER_STATIC_INITILIZATION_CODE. `name' is used as a static initializer
#  *	name seed (use when auto generation of name fails.
#  */
#define AZOUK_TRIGGER_STATIC_INITILIZATION_CODE_NAME(code, name, condition) \
    __AZOUK_TRIGGER_STATIC_INITILIZATION_CODE(AZOUK_UNIQUE_NAME(BOOST_PP_CAT(name, _trigger_static_initialization)), code, condition)

#define __AZOUK_TRIGGER_STATIC_INITILIZATION_CODE(name, code, condition)    \
	static struct BOOST_PP_CAT(name, _struct) {		    \
	    inline BOOST_PP_CAT(name, _struct)() {		    \
		if (condition) {				    \
		    AZOUK_PP_REMOVE_BRACES code;		    \
		}						    \
	    }							    \
	} BOOST_PP_CAT(name, _instance)				    \
	//

#endif
