
#include <boost/foreach.hpp>

#include "kwargs.h"

namespace azouk {
    namespace util {
	namespace kwargs {

	    bool Kwargs::check_keys(const KwargsKeys& keys) {
		BOOST_FOREACH(const KwValue& kw, *__values)
		    if (!keys.__keys.count(kw.first))
			return false;
		return true;
	    }

	};
    };
};
