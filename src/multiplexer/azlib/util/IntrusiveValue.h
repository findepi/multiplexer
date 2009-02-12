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
