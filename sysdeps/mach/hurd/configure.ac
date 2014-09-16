GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.

dnl We need this setting because of the need for PLT calls in ld.so.
AC_DEFINE([NO_HIDDEN])

AC_CACHE_CHECK(Hurd header version, libc_cv_hurd_version, [dnl
AC_TRY_COMPILE(dnl
[#include <hurd/version.h>], [
#define NEED_VERSION 20020609
#if HURD_INTERFACE_VERSION < NEED_VERSION
# error Hurd version too old: HURD_INTERFACE_VERSION < NEED_VERSION
#endif],
	       libc_cv_hurd_version=ok,
	       libc_cv_hurd_version=bad)])
if test "x$libc_cv_hurd_version" != xok; then
  AC_MSG_ERROR(Hurd headers not installed or too old)
fi
