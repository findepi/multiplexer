#ifndef AZLIB_UTIL_TYPE_FUNCTORS_H
#define AZLIB_UTIL_TYPE_FUNCTORS_H

namespace azlib {
    template <typename T>
    struct IdentityTypeFunctor {
	typedef T type;
    };
};

#endif
