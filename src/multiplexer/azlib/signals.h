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

#ifndef AZLIB_SIGNALS_H
#define AZLIB_SIGNALS_H

#include <boost/signals.hpp>
#include <boost/preprocessor/seq/for_each.hpp>
#include <iostream> // force std::cerr initialization before
                    // _azouk_signals_initilizer
#include "azlib/util/Assert.h"

namespace azlib {
    namespace signals {

	extern bool module_is_initialized;

	/*
         * list the boost::signals that are declared here, bound to POSIX
         * signals
	 */
#define AZOUK_SIGNALS_POSIX_SIGNALS_SEQ (TERM) (INT) (USR1) (USR2) //(HUP)

	/*
	 * list those signals that cause exit
	 */
#define AZOUK_SIGNALS_QUIT_ON_SEQ (TERM)

	/*
	 * list other process-level signals declared here
	 */
	extern boost::signal<void(int status)>* exit_signal;
	static inline boost::signal<void(int)>& get_exit_signal() {
            DbgAssert(module_is_initialized);
            return *exit_signal;
        }

	//
	// for every signal SIG from AZOUK_SIGNALS_POSIX_SIGNALS_SEQ list
	// define
	//	extern boost::signal<void()>* SIG;
	// signal.
        // We use plain pointers instead of objects statically initialized, so
        // that we have more control over the exact initliazation sequence (see
        // _azouk_signals_initilizer class).
	//
#define AZOUK_SIGNALS_define_exports(r, d, elem) \
	extern boost::signal<void()>* elem; \
	static inline boost::signal<void()>& BOOST_PP_CAT(get_, elem)() \
            { DbgAssert(module_is_initialized); return *elem; }
	BOOST_PP_SEQ_FOR_EACH(AZOUK_SIGNALS_define_exports, ~,
                AZOUK_SIGNALS_POSIX_SIGNALS_SEQ);
#undef AZOUK_SIGNALS_define_exports

	namespace impl {
            // so called Swatchz counter initialization
            // (http://www.cs.indiana.edu/~welu/notes/node23.html)
	    class _azouk_signals_initilizer {
		private: static unsigned int count;
		public: _azouk_signals_initilizer();
	    };
	    namespace {
                _azouk_signals_initilizer _azouk_signals_initilizer_instance;
            };
	}; // namespace impl

    }; // namespace signals
}; // namespace azlib

#endif
