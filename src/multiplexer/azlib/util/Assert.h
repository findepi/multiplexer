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

#ifndef MX_LIB_ASSERT_H
#define MX_LIB_ASSERT_H

#include <boost/preprocessor/stringize.hpp>
#include "azlib/util/Exception.h"

#define Assert(w) \
    do { \
        if (!(w)) \
            ::azlib::_AssertionFailed(__FILE__, __LINE__, __PRETTY_FUNCTION__, \
                    BOOST_PP_STRINGIZE(w)); \
    } while (0)

#define AssertMsg(w, args...) \
    do { \
        if (!(w)) \
            ::azlib::_AssertionFailed(__FILE__, __LINE__, __PRETTY_FUNCTION__, \
                    BOOST_PP_STRINGIZE(w), args); \
    } while (0)

#ifndef NDEBUG
# define DbgAssert Assert
# define DbgAssertMsg AssertMsg
#else
// TODO(kk) we may want to disable DbgAssert without disabling all assert()'s in
// the productional environment
# define DbgAssert(...)
# define DbgAssertMsg(...)
#endif

namespace azlib {

    struct AssertionError : public Exception {
	public:
	    AssertionError(const std::string& file, unsigned int line,
                    const std::string& function,
		    const std::string& question, const std::string& explanation)
                throw();
    };

    void _AssertionFailed(const char* file, unsigned int line,
            const char* function, const char* question,
            const std::string& explanation = "");

}; // namespace azlib

#endif
