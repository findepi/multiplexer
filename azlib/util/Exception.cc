
#include <stdlib.h>
#include "Exception.h"

using namespace azlib;

static bool _abort_on_exception = false;

namespace {
    struct _AbortOnExceptionInitializer {
	_AbortOnExceptionInitializer() {
	    _abort_on_exception = getenv("ABORT_ON_EXCEPTION") != NULL;
	}
    };
    static _AbortOnExceptionInitializer _abort_on_exception_initializer;
}

bool Exception::abort_on_exception() {
    return _abort_on_exception;
}

void Exception::abort_on_exception(bool aoe) {
    _abort_on_exception = aoe;
}

Exception::Exception() throw()
{}

Exception::Exception(const std::string explanation, const std::string& file, int line, const std::string& function) throw()
    : explanation_(explanation)
    , file_(file), function_(function)
    , line_(line)
{
    if (_abort_on_exception)
	abort();
}

Exception::~Exception() throw() {
}

const char* Exception::what() const throw() {
    return explanation_.c_str();
}

//std::string Exception::what() throw() {
    //return explanation_;
//}

