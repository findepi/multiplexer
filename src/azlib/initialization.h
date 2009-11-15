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
    namespace {                                                     \
	static struct BOOST_PP_CAT(name, _struct) {		    \
	    inline BOOST_PP_CAT(name, _struct)() {		    \
		if (condition) {				    \
		    AZOUK_PP_REMOVE_BRACES code;		    \
		}						    \
	    }							    \
	} BOOST_PP_CAT(name, _instance)				    \
    }                                                               \
    //

#endif
