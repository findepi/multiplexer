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
