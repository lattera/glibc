dnl configure fragment for new libpthread implementation.
GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.

if test "x$libc_cv_gcc___thread" != xyes; then
  AC_MSG_ERROR(compiler support for __thread is required)
fi

if test "x${libc_cv_visibility_attribute}" != xyes ||
   test "x${libc_cv_broken_visibility_attribute}" != xno; then
  AC_MSG_ERROR(working compiler support for visibility attribute is required)
fi

dnl Iff <unwind.h> is available, make sure it is the right one and it
dnl contains struct _Unwind_Exception.
AC_CACHE_CHECK(dnl
for forced unwind support, libc_cv_forced_unwind, [dnl
old_CPPFLAGS="$CPPFLAGS"
# Without inhibit_libc #define'd, GCC's unwind.h (at least for ia64)
# will try to include <stdlib.h>, which doesn't exist yet if we're
# building libc in a bare environment.
CPPFLAGS="$CPPFLAGS -Dinhibit_libc=1"
AC_TRY_COMPILE([#include <unwind.h>], [
struct _Unwind_Exception exc;
struct _Unwind_Context *context;
_Unwind_GetCFA (context)],
libc_cv_forced_unwind=yes, libc_cv_forced_unwind=no)
CPPFLAGS="$old_CPPFLAGS"])
if test $libc_cv_forced_unwind = yes; then
  AC_DEFINE(HAVE_FORCED_UNWIND)
dnl Check for C cleanup handling.
  old_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -Werror -fexceptions"
  AC_CACHE_CHECK([for C cleanup handling], libc_cv_c_cleanup, [dnl
  AC_TRY_COMPILE([
extern void some_function (void);
void cl (void *a) { }], [
  int a __attribute__ ((cleanup (cl)));
  some_function ()],
libc_cv_c_cleanup=yes, libc_cv_c_cleanup=no)])
  CFLAGS="$old_CFLAGS"
  if test $libc_cv_c_cleanup = no; then
    AC_MSG_ERROR([the compiler must support C cleanup handling])
  fi
else
  AC_MSG_ERROR(forced unwind support is required)
fi
