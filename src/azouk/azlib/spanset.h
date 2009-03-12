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

#ifndef AZLIB_SPANSET_H
#define AZLIB_SPANSET_H

#include <set>
#include "azlib/util/Assert.h"

namespace azlib {

    /**
     * SpanSet
     * implements a set that remembers only N last seen elements
     */
    template <
	typename T,	    // the type of elements held in the set
	int N = 2048,	    // number of elements remembered
	typename InnerSetImpl = std::set<T>
			    // the type of subset for checking existence
    >
    struct SpanSet {

	SpanSet() {}

	bool inline insert(const T& t) {
	    DbgAssert(queue_.size() == set_.size());
	    if (set_.insert(t).second) {
		if (queue_.size() == N) {
		    set_.erase(queue_.front());
		    queue_.pop_front();
		}
		queue_.push_back(t);
		DbgAssert(queue_.size() == set_.size());
		return true;
	    }
	    DbgAssert(queue_.size() == set_.size());
	    return false;
	}

    private:
	InnerSetImpl	set_;
	std::deque<T>	queue_;
    }; // struct SpanSet
}; // namespace azlib

#endif
