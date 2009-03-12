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
