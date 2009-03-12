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

#ifndef AZLIB_UTIL_INTRUSIVE_VALUE_H
#define AZLIB_UTIL_INTRUSIVE_VALUE_H

namespace azlib {

    /*
     * forward decl.
     */
    template <typename T> struct IntrusiveValue;
    template <typename T> void intrusive_ptr_release(IntrusiveValue<T>*);
    template <typename T> void intrusive_ptr_add_ref(IntrusiveValue<T>*);


    /*
     * IntrusiveValue
     */
    template <typename T>
    struct IntrusiveValue {
	IntrusiveValue()
	    : references_(0)
	{}
	IntrusiveValue(const T& value)
	    : references_(0)
	    , value_(value)
	{}

	operator T& () { return value_; }
	operator const T& () const { return value_; }

	IntrusiveValue& operator= (const T& value) {
	    value_ = value;
	    return *this;
	}

	IntrusiveValue& operator= (const IntrusiveValue& other) {
	    if (&other != this) {
		value_ = other.value_;
	    }
	    return *this;
	}

    private:
	unsigned int references_;
	T value_;

	friend void intrusive_ptr_release<>(IntrusiveValue<T>*);
	friend void intrusive_ptr_add_ref<>(IntrusiveValue<T>*);
    };

    /*
     * boost::intrusive_ptr support
     */
    template <typename T> void intrusive_ptr_release(IntrusiveValue<T>* itr) {
	if (-- itr->references_ == 0)
	    delete itr;
    }

    template <typename T> void intrusive_ptr_add_ref(IntrusiveValue<T>* itr) {
	++ itr->references_;
    }

};

#endif
#ifndef AZLIB_UTIL_INTRUSIVE_VALUE_H
#define AZLIB_UTIL_INTRUSIVE_VALUE_H

namespace azlib {

    /*
     * forward decl.
     */
    template <typename T> struct IntrusiveValue;
    template <typename T> void intrusive_ptr_release(IntrusiveValue<T>*);
    template <typename T> void intrusive_ptr_add_ref(IntrusiveValue<T>*);


    /*
     * IntrusiveValue
     */
    template <typename T>
    struct IntrusiveValue {
	IntrusiveValue()
	    : references_(0)
	{}
	IntrusiveValue(const T& value)
	    : references_(0)
	    , value_(value)
	{}

	operator T& () { return value_; }
	operator const T& () const { return value_; }

	IntrusiveValue& operator= (const T& value) {
	    value_ = value;
	    return *this;
	}

	IntrusiveValue& operator= (const IntrusiveValue& other) {
	    if (&other != this) {
		value_ = other.value_;
	    }
	    return *this;
	}

    private:
	unsigned int references_;
	T value_;

	friend void intrusive_ptr_release<>(IntrusiveValue<T>*);
	friend void intrusive_ptr_add_ref<>(IntrusiveValue<T>*);
    };

    /*
     * boost::intrusive_ptr support
     */
    template <typename T> void intrusive_ptr_release(IntrusiveValue<T>* itr) {
	if (-- itr->references_ == 0)
	    delete itr;
    }

    template <typename T> void intrusive_ptr_add_ref(IntrusiveValue<T>* itr) {
	++ itr->references_;
    }

};

#endif
