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

#ifndef AZLIB_HASH_SET_H
#define AZLIB_HASH_SET_H

#if defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__ >= 4) && \
                                                                 (__GNUC_MINOR__ >= 3)
// In g++ 4.3 ext/hash_set is deprecated.
// However, use of unordered_set requires -std=c++0x option that is considered
// experimental, so instead of using a non-deprecated header, we desiable the
// warning.
//# include <unordered_set>
# ifdef __DEPRECATED
#  undef __DEPRECATED
#  include <ext/hash_set>
#  define __DEPRECATED
# else
#  include <ext/hash_set>
#endif
# define AZLIB_hash_set_implementation ::__gnu_cxx::hash_set
#else
# include <ext/hash_set>
# define AZLIB_hash_set_implementation ::__gnu_cxx::hash_set
#endif // g++ >= 4.3

namespace azlib {
    using ::__gnu_cxx::hash_set;
};

#endif // AZLIB_HASH_SET_H
