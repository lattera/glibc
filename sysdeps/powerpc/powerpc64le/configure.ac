GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/powerpc/powerpc64le.

dnl Require binary128 floating point support on powerpc64le (available in
dnl GCC 6.2).
AC_CACHE_CHECK([if $CC supports binary128 floating point type],
	       libc_cv_compiler_powerpc64le_binary128_ok, [dnl
save_CFLAGS="$CFLAGS"
CFLAGS="$CFLAGS -Werror -mfloat128"
AC_COMPILE_IFELSE([AC_LANG_SOURCE([[
__float128 a, b, c, d, e;
int i;

__float128
foobar (__float128 x)
{
  a = __builtin_nansq ("0");
  b = __builtin_huge_valq ();
  c = __builtin_infq ();
  d = __builtin_fabsq (x);
  e = __builtin_nanq ("0");
  i = __builtin_signbit (x);
  return __builtin_copysignq (x, x);
}
]])],
		  [libc_cv_compiler_powerpc64le_binary128_ok=yes],
		  [libc_cv_compiler_powerpc64le_binary128_ok=no])
CFLAGS="$save_CFLAGS"])
AS_IF([test "$libc_cv_compiler_powerpc64le_binary128_ok" != "yes"],
      [critic_missing="$critic_missing binary128 floating point type (GCC >= 6.2) is required on powerpc64le."])

test -n "$critic_missing" && AC_MSG_ERROR([*** $critic_missing])
