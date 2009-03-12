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

#ifndef AZLIB_PROGRAM_H
#define AZLIB_PROGRAM_H

// include it only in your main program -- this header file defines simple main function

#include <iostream>
#include "azlib/util/Exception.h"
#include "azlib/util/Assert.h"
#include "azlib/util/type_utils.h"
#include "azlib/signals.h"

int AzoukMain(int argc, char** argv);

int main(int argc, char** argv) {
    using std::cerr;
    using std::endl;
    using namespace azlib::signals;

    try {
	get_exit_signal()(AzoukMain(argc, argv));
	AssertMsg(false, "exit_signal() returned");

    } catch (azlib::Exception& e) {
	cerr
	    << azlib::type_utils::type_name(e) << " in " << e.file() << ":" << e.line() << " (" << e.function() << ")\n"
	    << "    " << e.what() << endl;
	get_exit_signal()(1);
	AssertMsg(false, "exit_signal() returned");

    } catch (std::exception& e) {
	cerr
	    << azlib::type_utils::type_name(e) << ": " << e.what() << "\n";
	get_exit_signal()(1);
	AssertMsg(false, "exit_signal() returned");
    }
}

#endif
