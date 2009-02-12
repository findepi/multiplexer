#include <signal.h>
#include <boost/preprocessor/facilities/empty.hpp>
#include <boost/preprocessor/cat.hpp>
#include <boost/bind.hpp>
#include <boost/lambda/lambda.hpp>
#include <iostream> /* cerr for print_that_called */
#include <limits>
#include "azlib/initialization.h"
#include "azlib/util/Assert.h"
#include "azlib/logging.h"
#include "azlib/signals.h"

namespace azlib {
    namespace signals {

	bool module_is_initialized = false;

#define AZOUK_SIGNALS_c_name(SIGNAME) BOOST_PP_CAT(SIG, SIGNAME)
#define AZOUK_SIGNALS_callback_name(SIGNAME) BOOST_PP_CAT(_, BOOST_PP_CAT(SIGNAME, _posix_callback))

	/*
	 * check that the module was already initialized in Swatchz counter manner
	 * That is: that SIG objects are created.
	 */
	AZOUK_TRIGGER_STATIC_INITILIZATION_CODE((Assert(module_is_initialized); Assert(TERM);), true);

	/*
	 * declare the signals extern-ed in header file
	 */
#define AZOUK_SIGNALS_declare(r, d, SIG) \
	/* implement the extern from header file */ \
	boost::signal<void()>* SIG = NULL;
	BOOST_PP_SEQ_FOR_EACH(AZOUK_SIGNALS_declare, ~, AZOUK_SIGNALS_POSIX_SIGNALS_SEQ);
	boost::signal<void(int)>* exit_signal = NULL;
#undef AZOUK_SIGNALS_declare


	/*
	 * necessary code to bind each SIG to proper SIG* constant signal
	 */
#define AZOUK_SIGNALS_export_and_create_c_callback(r, d, SIG) \
	/* create C callback for this signal */ \
	static void AZOUK_SIGNALS_callback_name(SIG)(int n) { \
	    DbgAssert(n == AZOUK_SIGNALS_c_name(SIG)); \
	    (*SIG)(); \
	}; \
	\
	/* statically bind AZOUK_SIGNALS_callback_name(SIG) to signal AZOUK_SIGNALS_c_name(SIG) */ \
	AZOUK_TRIGGER_STATIC_INITILIZATION_CODE_NAME(( \
		/* connect _SIG_posix_callback to signal SIGNAME */ \
		struct sigaction act; \
		memset(&act, 0, sizeof(struct sigaction)); \
		act.sa_handler = AZOUK_SIGNALS_callback_name(SIG); \
		sigaction(AZOUK_SIGNALS_c_name(SIG), &act, NULL); \
	    ), SIG, true); \
	\
	/* create a logging function that prints a signal was called */ \
	static void BOOST_PP_CAT(print_that_called_, SIG)() { \
	    AZOUK_LOG(INFO, LOWVERBOSITY, CTX("signals") TEXT("got " BOOST_PP_STRINGIZE(SIG)  " signal")); \
	}; \
	/* bind the above logging function to SIG */ \
	AZOUK_TRIGGER_STATIC_INITILIZATION_CODE_NAME(( \
		    using namespace boost::lambda; \
		    SIG->connect(std::numeric_limits<int>::min(), std::cerr << constant("signal " BOOST_PP_STRINGIZE(SIG) " called\n")); \
		    SIG->connect(std::numeric_limits<int>::min(), & BOOST_PP_CAT(print_that_called_, SIG)) \
	    ), BOOST_PP_CAT(print_that_called, SIG), true); \
	// and that's it

	BOOST_PP_SEQ_FOR_EACH(AZOUK_SIGNALS_export_and_create_c_callback, ~, AZOUK_SIGNALS_POSIX_SIGNALS_SEQ);
#undef AZOUK_SIGNALS_export_and_create_c_callback

	/*
	 * for every SIG in AZOUK_SIGNALS_QUIT_ON_SEQ
	 * define a C handler that calls exit with proper status
	 * and bind it to SIG
	 */
#define AZOUK_SIGNALS_quit_on(r, d, SIG) \
	static void BOOST_PP_CAT(exit_on_, SIG)() { \
	    exit(128 | AZOUK_SIGNALS_c_name(SIG)); \
	}; \
	AZOUK_TRIGGER_STATIC_INITILIZATION_CODE_NAME((SIG->connect(std::numeric_limits<int>::max(), &BOOST_PP_CAT(exit_on_, SIG))), SIG, true);
	BOOST_PP_SEQ_FOR_EACH(AZOUK_SIGNALS_quit_on, ~, AZOUK_SIGNALS_QUIT_ON_SEQ);
#undef AZOUK_SIGNALS_quit_on


	namespace impl {

	    unsigned int _azouk_signals_initilizer::count = 0;
	    _azouk_signals_initilizer::_azouk_signals_initilizer() {
		namespace L = boost::lambda;
		namespace S = boost::signals;

		if (count++ == 0) {
		    /*
		     * create SIGs before any other module initializers are called in modules that
		     * can use them
		     */
#define AZOUK_SIGNALS_create_signal(r, d, SIG) signals::SIG = new boost::signal<void()>();
		    BOOST_PP_SEQ_FOR_EACH(AZOUK_SIGNALS_create_signal, ~, AZOUK_SIGNALS_POSIX_SIGNALS_SEQ);
#undef AZOUK_SIGNALS_create_signal
		    signals::exit_signal = new boost::signal<void(int)>();

		    // when exit_signal is called...
		    exit_signal->connect(std::numeric_limits<int>::min(), 
			    std::cerr << L::constant("azlib::signals::exit_signal called: exiting with ") << L::_1 << "\n");
		    exit_signal->connect(std::numeric_limits<int>::max(), &exit, S::at_back);

		    // when TERM or INT is called call exit_signal with proper status
#define AZOUK_SIGNALS_connect_killers(r, d, SIG) signals::SIG->connect(boost::bind(ref(*signals::exit_signal), 128 | AZOUK_SIGNALS_c_name(SIG)));
		    BOOST_PP_SEQ_FOR_EACH(AZOUK_SIGNALS_connect_killers, ~, AZOUK_SIGNALS_QUIT_ON_SEQ);
#undef AZOUK_SIGNALS_connect_killers

		    module_is_initialized = true;
		}
	    }
	};

    }; // namespace signals
}; // namespace azlib
