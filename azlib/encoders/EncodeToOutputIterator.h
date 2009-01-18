#ifndef MX_ENCODERS_ENCODETOOUTPUTITERATOR_H
#define MX_ENCODERS_ENCODETOOUTPUTITERATOR_H

#include "encoders/BaseEncoder.h"

namespace azlib {
    namespace encoders {

	template <typename OutputIterator, template <typename> class Encoding>
	struct EncodeToOutputIterator : public BaseEncoder<EncodeToOutputIterator<OutputIterator, Encoding>, Encoding> {

	    EncodeToOutputIterator(OutputIterator it) : it_(it)
	    {}

	    void write_byte(unsigned char byte) {
		*(it_)++ = byte;
	    }

	private:
	    OutputIterator it_;
	};

    };
};

#endif
