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

#ifndef AZLIB_TRIPLE_H
#define AZLIB_TRIPLE_H

namespace azlib {

    template <typename First, typename Second, typename Third>
    struct triple {
	typedef First first_type;
	typedef Second second_type;
	typedef Third third_type;

	triple(const first_type& f, const second_type& s, const third_type& t)
	    : first(f), second(s), third(t)
	{}

	template <typename FirstP, typename SecondP, typename ThirdP>
	triple(const triple<FirstP, SecondP, ThirdP>& other)
	    : first(other.first), second(other.second), third(other.third)
	{}

	triple()
	{}

	// TODO write assignments et al

	first_type first;
	second_type second;
	third_type third;
    };

    template <typename First, typename Second, typename Third>
    triple<First, Second, Third> make_triple(const First& f, const Second& s, const Third& t) {
	return triple<First, Second, Third>(f, s, t);
    }

}; // namespace azlib

#endif
