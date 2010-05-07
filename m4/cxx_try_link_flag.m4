dnl check compiler/ld flags
AC_DEFUN([CXX_TRY_LINK_FLAG],
[dnl
	AC_MSG_CHECKING([whether $CXX supports $1])

	_save_ldflags="$LFLAGS"
	LDFLAGS="$LDFLAGS -Werror $1"
	AC_LINK_IFELSE([int main(void){ return 0;} ],
                       [try_ld_flag=yes],
                       [try_ld_flag=no])
	LDFLAGS="$_save_ldflags"

	if test "x$try_ld_flag" = "xyes"; then
		ifelse([$2], , :, [$2])
	else
		ifelse([$3], , :, [$3])
	fi
	AC_MSG_RESULT([$try_ld_flag])
])
