#ifndef AZLIB_UTIL_STR_H
#define AZLIB_UTIL_STR_H

#include <string>
#include <boost/lexical_cast.hpp>
#include "azlib/preproc/common.h"

namespace azlib {

    /*
     * str(t) -> lexical_cast<std::string>(t)
     */
    template <typename T> inline std::string str(T& t) AZOUK_ATTRIBUTE_ALWAYS_INLINE;
    template <typename T> inline std::string str(T& t) {
	return boost::lexical_cast<std::string>(t);
    }

    template <typename T> inline std::string str(const T& t) AZOUK_ATTRIBUTE_ALWAYS_INLINE;
    template <typename T> inline std::string str(const T& t) {
	return boost::lexical_cast<std::string>(t);
    }

};

#endif
