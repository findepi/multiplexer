#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
#
AC_INIT([azouk-tools], [1.0.0], [who@where.com])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE(foreign dist-lzma std-options -Wall -Werror -Wno-portability)
AM_SILENT_RULES([yes])

AC_CONFIG_SRCDIR([generate_constants.cc])
AC_CONFIG_HEADERS([config/config.h])
AC_CONFIG_MACRO_DIR([../m4])

CXXFLAGS=${CXXFLAGS-""}

AC_PROG_CXX
AC_PROG_MKDIR_P
AX_WITH_PROG_REQUIRED([PROTOC], [protoc],
             [Google protocol buffers compiler protoc])

AC_LANG([C++])

BOOST_REQUIRE
BOOST_FOREACH

AC_CHECK_LIB([pthread], [sem_open])
AC_CHECK_LIB([protobuf], [main])
## use -lmswsock if available
AC_CHECK_LIB([mswsock], [main])

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])
AC_CHECK_HEADER([google/protobuf/text_format.h], [],
        [AC_MSG_ERROR([protobuf headers not found, install libprotobuf-dev])])

dnl check linker flags
AX_CHECK_LINKER_FLAGS([-Wl,--enable-auto-import],
                      [LDFLAGS="${LDFLAGS} -Wl,--enable-auto-import"], [])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_TYPE_UINT32_T

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
