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
