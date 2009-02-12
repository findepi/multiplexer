#ifndef AZLIB_DEBUG_H
#define AZLIB_DEBUG_H


#ifndef NDEBUG
# include "azlib/logging.h"
# define AZDEBUG_MSG(whazza...) ((void)(::std::cerr << "[36m[K" __FILE__ << ":" << __LINE__ << "[m[K: " << whazza << "\n"))

#else
# define AZDEBUG_MSG(...)
#endif // NDEBUG

#undef AZDEBUG_MSG
#define AZDEBUG_MSG(...)

#endif
