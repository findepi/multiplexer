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

#ifndef AZLIB_UTIL_STR_H
#define AZLIB_UTIL_STR_H

#include <string>
#include <boost/lexical_cast.hpp>
#include "azlib/preproc/common.h"

namespace azlib {

    /*
     * str(t) -> lexical_cast<std::string>(t)
     */
    template <typename T> inline std::string str(T& t) AZOUK_ATTRIBUTE_ALWAYS_INLINE;
    template <typename T> inline std::string str(T& t) {
	return boost::lexical_cast<std::string>(t);
    }

    template <typename T> inline std::string str(const T& t) AZOUK_ATTRIBUTE_ALWAYS_INLINE;
    template <typename T> inline std::string str(const T& t) {
	return boost::lexical_cast<std::string>(t);
    }

};

#endif
