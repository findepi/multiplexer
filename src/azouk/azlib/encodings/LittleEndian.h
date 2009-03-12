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

#ifndef MX_ENCODERS_LITTLEENDIAN_H
#define MX_ENCODERS_LITTLEENDIAN_H

#include <boost/cstdint.hpp>

namespace azlib {
    namespace encodings {

	template <typename T>
	struct LittleEndian;

	template <>
	struct LittleEndian<boost::uint32_t> {
	    template <typename Encoder>
	    static void encode(Encoder& enc, boost::uint32_t n) {
		enc.write_byte(n & 0xff);
		enc.write_byte((n >> 8) & 0xff);
		enc.write_byte((n >> 16) & 0xff);
		enc.write_byte((n >> 24) & 0xff);
	    }

	    template <typename Decoder>
	    static void decode(Decoder& dec, boost::uint32_t& n) {
		n = dec.read_byte()
		    + ((boost::uint32_t)dec.read_byte() << 8)
		    + ((boost::uint32_t)dec.read_byte() << 16)
		    + ((boost::uint32_t)dec.read_byte() << 24);
	    }
	};

    };
};

#endif
