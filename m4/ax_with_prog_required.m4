#serial 1

AC_DEFUN([AX_WITH_PROG_REQUIRED],[
    pushdef([DESCRIPTION], $3)

    AX_WITH_PROG($1, $2, $4, $5)

    AS_IF(test -z "$$1",
        [AC_MSG_ERROR(DESCRIPTION [not found])])

    popdef([DESCRIPTION])
])
