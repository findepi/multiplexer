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

#ifndef AZLIB_PREPROC_COMMON_H
#define AZLIB_PREPROC_COMMON_H

#include <boost/preprocessor/seq/cat.hpp>

# /*
#  * AZOUK_UNIQUE_INT
#  *	Expand to some int.
#  */
#ifdef __COUNTER__
# define AZOUK_UNIQUE_INT() __COUNTER__
#else
# define AZOUK_UNIQUE_INT() __LINE__
#endif

# /*
#  * AZOUK_REMOVE_PARENS
#  *	Remove parenthesis.
#  *
#  *	Example:
#  *	    #define foo(sth_in_parenthesis) AZOUK_REMOVE_PARENS sth_in_parenthesis
#  */
#define AZOUK_REMOVE_PARENS(arg...) arg

# /*
#  * AZOUK_UNIQUE_NAME(name)
#  *	Generate unique name starting with name as a base.
#  */
#define AZOUK_UNIQUE_NAME(name) BOOST_PP_SEQ_CAT( (__uniqname__) (name) (_)  (AZOUK_UNIQUE_INT()) (_) )

# /*
#  * AZOUK_ATTRIBUTE_ALWAYS_INLINE
#  *	Delcare __attribute__(always_inline) where it's possible.
#  *	TODO remove dependence on Google.
#  */
#include <google/protobuf/stubs/common.h> /* GOOGLE_ATTRIBUTE_ALWAYS_INLINE */
#ifndef GOOGLE_ATTRIBUTE_ALWAYS_INLINE
#  error "Hey, Google, define GOOGLE_ATTRIBUTE_ALWAYS_INLINE!"
#endif
#define AZOUK_ATTRIBUTE_ALWAYS_INLINE GOOGLE_ATTRIBUTE_ALWAYS_INLINE

# /*
#  * AZOUK_PP_REMOVE_BRACES
#  */
#define AZOUK_PP_REMOVE_BRACES(args...) args

#endif
