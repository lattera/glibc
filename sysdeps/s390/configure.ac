GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/s390.

dnl It is always possible to access static and hidden symbols in an
dnl position independent way.
AC_DEFINE(PI_STATIC_AND_HIDDEN)

dnl Accept as 2.24 or newer.
AC_CHECK_PROG_VER(AS, $AS, --version,
		  [GNU assembler.* \([0-9]*\.[0-9.]*\)],
		  [2.2[4-9]*|2.[3-9][0-9]*|[3-9].*|[1-9][0-9]*], critic_missing="$critic_missing The program AS is required in version >= 2.24 for target S390.")


AC_CACHE_CHECK(for __builtin_tbegin, libc_cv_gcc_builtin_tbegin, [dnl
cat > conftest.c <<\EOF
#include <htmintrin.h>
void testtransaction ()
{
  if (__builtin_tbegin (0) == _HTM_TBEGIN_STARTED)
    {
      __builtin_tend ();
    }
}
EOF
dnl
dnl test, if the tbegin instruction is used by __builtin_tbegin
if AC_TRY_COMMAND([${CC-cc} -mhtm -O2 -S conftest.c -o - | grep -w tbegin > /dev/null]) ;
then
  libc_cv_gcc_builtin_tbegin=yes
else
  libc_cv_gcc_builtin_tbegin=no
fi
rm -f conftest* ])

if test "$enable_lock_elision" = yes && test "$libc_cv_gcc_builtin_tbegin" = no ; then
   critic_missing="$critic_missing The used GCC has no support for __builtin_tbegin, which is needed for lock-elision on target S390."
fi

test -n "$critic_missing" && AC_MSG_ERROR([
*** $critic_missing])
