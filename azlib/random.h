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
