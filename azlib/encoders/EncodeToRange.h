#ifndef MX_ENCODERS_ENCODETORANGE_H
#define MX_ENCODERS_ENCODETORANGE_H_

#include "azlib/util/Assert.h"
#include "azlib/encoders/BaseEncoder.h"

namespace azlib {
    namespace encoders {

	template <typename OutputIterator, template <typename> class Encoding>
	struct EncodeToRange : public BaseEncoder<EncodeToRange<OutputIterator, Encoding>, Encoding> {

	    EncodeToRange(OutputIterator begin, OutputIterator end)
		: begin_(begin), end_(end)
	    {}

	    void write_byte(unsigned char byte) {
		Assert(begin_ != end_);
		*(begin_)++ = byte;
	    }

	private:
	    OutputIterator begin_, end_;
	};

    };
};

#endif
