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
