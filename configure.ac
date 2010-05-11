#                                               -*- Autoconf -*-
# Process this file with autoconf to produce a configure script.

AC_PREREQ([2.65])
AC_INIT([Azouk Multiplexer], [1.0.0],
        [Piotr Findeisen <piotr.findeisen@gmail.com>],
        [multiplexer], [http://github.com/findepi/multiplexer])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE(foreign dist-lzma std-options -Wall -Werror -Wno-portability)
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

AC_CONFIG_SRCDIR([src/mxcontrol.cc])
AC_CONFIG_HEADERS([config/config.h])
AC_CONFIG_MACRO_DIR([m4])

AC_DEFINE([GNULIB_NAMESPACE], [gnulib], [Use gnulib with a namespace])

AC_CANONICAL_BUILD

# Checks for programs.
AC_PROG_CXX
AC_PROG_CC
AC_PROG_CPP
AC_PROG_AWK
gl_EARLY
AC_PROG_INSTALL
AC_PROG_LN_S
AC_PROG_MAKE_SET

# look for protoc, the ProtoBuf compiler
AX_WITH_PROG_REQUIRED([PROTOC], [protoc],
             [Google protocol buffers compiler protoc])

# sha1sum is used for the version file
AX_WITH_PROG_REQUIRED([SHA1SUM], [sha1sum],
             [SHA1 sum generator sha1sum])

LT_PREREQ([2.2.6])
LT_INIT([win32-dll])

gl_INIT

AC_LANG([C++])

# boost libless headers
BOOST_REQUIRE
BOOST_ASIO
BOOST_NUMERIC_CONVERSION
BOOST_PREPROCESSOR
BOOST_UTILITY
BOOST_FOREACH

# boost libfull headers
BOOST_PROGRAM_OPTIONS
BOOST_SIGNALS
BOOST_SYSTEM
BOOST_DATE_TIME

AC_CHECK_LIB([pthread], [sem_open])
AC_CHECK_LIB([protobuf], [main])

## a hack for now
## Server.o:Server.cc:(.text$_ZN5boost4asio6detail23win_iocp_socket_serviceINS0_2ip3tcpEE16accept_operationINS0_12basic_socketIS4_NS0_21stream_socket_serviceIS4_EEEENS_3_bi6bind_tIvNS_4_mfi3mf2IvN11multiplexer6ServerENS_10shared_ptrINSF_10ConnectionISG_EEEERKNS_6system10error_codeEEENSB_5list3INSB_5valueIPSG_EENSR_ISK_EEPFNS_3argILi1EEEvEEEEEE18do_completion_implEPNS1_19win_iocp_io_service9operationEmj[boost::asio::detail::win_iocp_socket_service<boost::asio::ip::tcp>::accept_operation<boost::asio::basic_socket<boost::asio::ip::tcp, boost::asio::stream_socket_service<boost::asio::ip::tcp> >, boost::_bi::bind_t<void, boost::_mfi::mf2<void, multiplexer::Server, boost::shared_ptr<multiplexer::Connection<multiplexer::Server> >, boost::system::error_code const&>, boost::_bi::list3<boost::_bi::value<multiplexer::Server*>, boost::_bi::value<boost::shared_ptr<multiplexer::Connection<multiplexer::Server> > >, boost::arg<1> (*)()> > >::do_completion_impl(boost::asio::detail::win_iocp_io_service::operation*, unsigned long, unsigned int)]+0x44a): undefined reference to `_AcceptEx@32'
## use -lmswsock if available
AC_CHECK_LIB([mswsock], [main])
## see http://lists.boost.org/boost-users/2006/11/23684.php

# Checks for header files.
AC_CHECK_HEADERS([fcntl.h stdlib.h unistd.h])
AC_CHECK_HEADER([google/protobuf/text_format.h], [],
        [AC_MSG_ERROR([protobuf headers not found, install libprotobuf-dev])])

# Checks for typedefs, structures, and compiler characteristics.
AC_HEADER_STDBOOL
AC_C_INLINE
AC_TYPE_SIZE_T
AC_TYPE_UINT16_T
AC_TYPE_UINT32_T
AC_TYPE_UINT64_T

dnl check linker flags
AX_CHECK_LINKER_FLAGS([-Wl,--enable-auto-import],
                      [LDFLAGS="${LDFLAGS} -Wl,--enable-auto-import"], [])

# Checks for library functions.
AC_CHECK_FUNCS([atexit gethostname memset setenv socket])

AC_CONFIG_FILES([Makefile
                 src/Makefile
                 lib/Makefile])
AC_OUTPUT

dir=$(cd $(dirname $0) && pwd)
echo "running ${dir}/tools/configure --host ${build} in tools"
mkdir -p tools && cd tools && echo && ${dir}/tools/configure --host ${build}
