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

#ifndef AZLIB_RANDOM_H
#define AZLIB_RANDOM_H

#include <boost/random/linear_congruential.hpp>
#include <boost/cstdint.hpp>
#include "azlib/util/Assert.h"

namespace azlib {


    struct AutoSeedingRand48 : public boost::rand48 {
	AutoSeedingRand48();
    };

    // Generator
    struct Random64 {
	typedef boost::uint64_t result_type;
	result_type operator() () {
	    return ((boost::uint64_t) a_() << 32) | b_();
	}

    private:
	AutoSeedingRand48 a_, b_;
    };
}; // namespace azlib

#endif
