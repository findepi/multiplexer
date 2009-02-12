
#include <iostream>

#include "Assert.h"

namespace azlib {

    AssertionError::AssertionError(const std::string& file, unsigned int line, const std::string& function,
	    const std::string& question, const std::string& explanation) throw()
	: Exception(explanation.empty() ? "Assertion failed `" + question + "'" : explanation + " (`" + question + "' failed)",
		file, line, function)
    {}

    void _AssertionFailed(const char* file, unsigned int line, const char* function, const char* question, const std::string& explanation) {
	std::cerr << file << ":" << line << ": " << explanation << (!explanation.empty() ? ": " : "") << "`" << question << "' failed in " << function << "\n";
	throw AssertionError(file, line, function, question, explanation);
    }

}; // namespace azlib
