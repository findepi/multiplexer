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

#ifndef MX_ENCODERS_DECODEFROMRANGE_H
#define MX_ENCODERS_DECODEFROMRANGE_H

#include "azlib/util/Assert.h"
#include "azlib/encoders/BaseDecoder.h"

namespace azlib {
    namespace encoders {

	template <typename InputIterator, template <typename> class Encoding>
	struct DecodeFromRange : public BaseDecoder<DecodeFromRange<InputIterator, Encoding>, Encoding> {

	    DecodeFromRange(InputIterator begin, InputIterator end)
		: begin_(begin), end_(end)
	    {}

	    unsigned char read_byte() {
		Assert(begin_ != end_);
		return *(begin_)++;
	    }

	private:
	    InputIterator begin_, end_;
	};

    };
};

#endif
