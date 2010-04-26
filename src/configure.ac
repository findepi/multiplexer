#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([azouk], [1.0.0], [Krzysztof Kulewski <kulewski@gmail.com>])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE(foreign dist-lzma std-options -Wall -Werror -Wno-portability)
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([azouk/mxcontrol.cc])
AC_CONFIG_HEADERS([config/config.h])
AC_CONFIG_MACRO_DIR([m4])

# Checks for programs.
AC_PROG_AWK
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LN_S
AC_PROG_LIBTOOL

## look for protoc, the ProtoBuf compiler
AC_CHECK_PROG([PROTOC], [protoc], [protoc])

## used for the version file
AC_CHECK_PROG([SHA1SUM], [sha1sum], [sha1sum])
AC_CHECK_PROG([CUT], [cut], [cut])


BOOST_REQUIRE
BOOST_ASIO
BOOST_NUMERIC_CONVERSION
BOOST_PREPROCESSOR
BOOST_UTILITY
BOOST_FOREACH

BOOST_PROGRAM_OPTIONS()
BOOST_SIGNALS()
BOOST_SYSTEM()
BOOST_DATE_TIME()

AC_CHECK_LIB([pthread], [sem_open])
dnl AC_LANG_CPLUSPLUS
AC_CHECK_LIB([protoc], [main])
AC_CHECK_LIB([protobuf], [main])

dnl AC_LANG_CPLUSPLUS
dnl AC_CHECK_LIB([boost_program_options],
dnl 	     [boost::program_options::option_description::match])
dnl AC_CHECK_LIB([boost_python], [boost::python::eval])
dnl AC_CHECK_LIB([boost_signals], [main])
dnl AC_CHECK_LIB([boost_system], [main])
dnl AC_CHECK_LIB([dl], [main])
dnl AC_CHECK_LIB([protobuf], [main])
dnl AC_CHECK_LIB([util], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

# Checks for library functions.
AC_CHECK_FUNCS([atexit gethostname memset setenv socket])

AC_CONFIG_FILES([Makefile
                 azouk/Makefile])
AC_OUTPUT
