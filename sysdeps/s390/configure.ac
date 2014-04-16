GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/s390.

dnl It is always possible to access static and hidden symbols in an
dnl position independent way.
AC_DEFINE(PI_STATIC_AND_HIDDEN)

dnl Accept as 2.24 or newer.
AC_CHECK_PROG_VER(AS, $AS, --version,
		  [GNU assembler.* \([0-9]*\.[0-9.]*\)],
		  [2.2[4-9]*|2.[3-9][0-9]*|[3-9].*|[1-9][0-9]*], critic_missing="$critic_missing The program AS is required in version >= 2.24 for target S390.")

test -n "$critic_missing" && AC_MSG_ERROR([
*** $critic_missing])
