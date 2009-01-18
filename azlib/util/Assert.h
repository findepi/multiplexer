#ifndef MX_LIB_ASSERT_H
#define MX_LIB_ASSERT_H

#include <boost/preprocessor/stringize.hpp>
#include "azlib/util/Exception.h"

#define Assert(w) do { if (!(w)) ::azlib::_AssertionFailed(__FILE__, __LINE__, __PRETTY_FUNCTION__, BOOST_PP_STRINGIZE(w)); } while (0)
#define AssertMsg(w, args...) do { if (!(w)) ::azlib::_AssertionFailed(__FILE__, __LINE__, __PRETTY_FUNCTION__, BOOST_PP_STRINGIZE(w), args); } while (0)

#ifndef NDEBUG
# define DbgAssert Assert
# define DbgAssertMsg AssertMsg
#else
// TODO(kk) we may want to disable DbgAssert without disabling all assert()'s in the productional environment
# define DbgAssert(...)	
# define DbgAssertMsg(...)
#endif

namespace azlib {

    struct AssertionError : public Exception {
	public:
	    AssertionError(const std::string& file, unsigned int line, const std::string& function,
		    const std::string& question, const std::string& explanation) throw();
    };

    void _AssertionFailed(const char* file, unsigned int line, const char* function, const char* question, const std::string& explanation = "");

}; // namespace azlib

#endif
