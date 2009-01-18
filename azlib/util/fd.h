#ifndef AZLIB_UTIL_FD_H
#define AZLIB_UTIL_FD_H

#include <unistd.h>
#include <boost/noncopyable.hpp>

namespace azlib {
    namespace util {

	struct Fd : boost::noncopyable {
	    explicit Fd(int fd, bool own_fd = false)
		: fd_(fd), own_fd_(own_fd)
	    {}

	    inline int fd() const { return fd_; }
	    ~Fd() {
		if (own_fd_ && fd_ >= 0) {
		    close(fd_);
		}
	    }
	private:
	    int fd_;
	    bool own_fd_;
	}; // struct Fd

    }; // namespace util
}; // namespace azlib

#endif
