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
