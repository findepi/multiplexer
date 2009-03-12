#ifndef AZLIB_HASH_MAP_H
#define AZLIB_HASH_MAP_H

#if defined(__GNUC__) && defined(__GNUC_MINOR__) && (__GNUC__ >= 4) && \
                                                                 (__GNUC_MINOR__ >= 3)
// In g++ 4.3 ext/hash_map is deprecated.
// However, use of unordered_map requires -std=c++0x option that is considered
// experimental, so instead of using a non-deprecated header, we desiable the
// warning.
//# include <unordered_map>
# ifdef __DEPRECATED
#  undef __DEPRECATED
#  include <ext/hash_map>
#  define __DEPRECATED
# else
#  include <ext/hash_map>
#endif
# define AZLIB_hash_map_implementation ::__gnu_cxx::hash_map
#else
# include <ext/hash_map>
# define AZLIB_hash_map_implementation ::__gnu_cxx::hash_map
#endif // g++ >= 4.3

namespace azlib {
    using ::__gnu_cxx::hash_map;
};

#endif // AZLIB_HASH_MAP_H
