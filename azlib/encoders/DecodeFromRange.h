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
