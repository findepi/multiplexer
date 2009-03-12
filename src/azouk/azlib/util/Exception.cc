//
// Azouk Libraries -- Libraries and goodies created for www.azouk.com.
// Copyright (C) 2008-2009 Azouk Network Ltd.
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Author:
//      Piotr Findeisen <piotr.findeisen at gmail.com>
//


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

