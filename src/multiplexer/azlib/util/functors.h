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

#ifndef AZLIB_UTIL_FUNCTORS_H
#define AZLIB_UTIL_FUNCTORS_H

#include <functional>

namespace azlib {
    
    template <typename What, typename From>
    struct ConstructingFunctor : std::unary_function<From, What> {
	What operator() (From& f) const { return What(f); }
	What operator() (const From& f) const { return What(f); }
    };

    template <typename What>
    struct ReferencingFunctor : std::unary_function<What, What> {
	What& operator() (What& w) const { return w; }
	What operator() (const What& w) const { return w; }
    };

    template <typename What>
    struct DefaultConstructingFactory {
	What operator() () const { return What(); }
    };

    template <typename Pair>
    struct FirstFromPairExtractor : std::unary_function<Pair, typename Pair::first_type> {
	typename Pair::first_type& operator() (Pair& p) const { return p.first; }
	const typename Pair::first_type& operator() (const Pair& p) const { return p.first; }
    };

    template <typename Pair>
    struct SecondFromPairExtractor : std::unary_function<Pair, typename Pair::second_type> {
	typename Pair::second_type& operator() (Pair& p) const { return p.second; }
	const typename Pair::second_type& operator() (const Pair& p) const { return p.second; }
    };
};

#endif
