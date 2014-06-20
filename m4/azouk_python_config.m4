# SYNOPSIS
#
#   AZOUK_PYTHON_CONFIG
#
# DESCRIPTION
#
#   Try to find python-config based on $PYTHON set by $AM_PATH_PYTHON
#
#   It set PYTHON_INCLUDE_DIR to the directory holding the header files, and
#   PYTHON_LIB to the name of the Python library, based on a call to
#   python-config.
#
#   This macro calls AC_SUBST on PYTHON_INCLUDE_DIR and PYTHON_LIB.
#
# LICENSE
#
#   Copyright (c) 2010 Zbigniew Jędrzejewski-Szmek
#
#   This program is free software; you can redistribute it and/or modify it
#   under the terms of the GNU General Public License as published by the
#   Free Software Foundation; either version 2 of the License, or (at your
#   option) any later version.
#
#   This program is distributed in the hope that it will be useful, but
#   WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
#   Public License for more details.
#
#   You should have received a copy of the GNU General Public License along
#   with this program. If not, see <http://www.gnu.org/licenses/>.
#
#   As a special exception, the respective Autoconf Macro's copyright owner
#   gives unlimited permission to copy, distribute and modify the configure
#   scripts that are the output of Autoconf when processing the Macro. You
#   need not follow the terms of the GNU General Public License when using
#   or distributing such scripts, even though portions of the text of the
#   Macro appear in them. The GNU General Public License (GPL) does govern
#   all other use of the material that constitutes the Autoconf Macro.
#
#   This special exception to the GPL applies to versions of the Autoconf
#   Macro released by the Autoconf Archive. When you make and distribute a
#   modified version of the Autoconf Macro, you may extend this special
#   exception to the GPL to apply to your modified version as well.

#serial 1

AC_DEFUN([AZOUK_PYTHON_CONFIG],
[dnl
AC_CHECK_PROGS([PYTHON_CONFIG], [${PYTHON}-config python-config])
AS_IF(test -z "$PYTHON_CONFIG",
      [AC_MSG_ERROR("python-config not found")])

PYTHON_INCLUDES=`$PYTHON_CONFIG --includes`
AC_SUBST(PYTHON_INCLUDES)

PYTHON_LDFLAGS=`$PYTHON_CONFIG --ldflags`
AC_SUBST(PYTHON_LDFLAGS)
])dnl
