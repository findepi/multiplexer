#ifndef AZLIB_MACROS_H
#define AZLIB_MACROS_H

#ifdef __COUNTER__
# define AZOUK_MAKE_UNIQUE_NAME(name) name ## _ ## __COUNTER__
#else
# define AZOUK_MAKE_UNIQUE_NAME
#endif

#endif
