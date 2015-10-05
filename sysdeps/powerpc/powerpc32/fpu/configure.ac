GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/powerpc/powerpc32/fpu.

# Test whether integer to floating point conversions use fcfid.
AC_CACHE_CHECK([for fcfid use], [libc_cv_ppc_fcfid], [dnl
echo 'double foo (int x) { return (double) x; }' > conftest.c
libc_cv_ppc_fcfid=no
if AC_TRY_COMMAND(${CC-cc} -S $CFLAGS conftest.c -o conftest.s 1>&AS_MESSAGE_LOG_FD); then
changequote(,)dnl
  if grep '[ 	]fcfid' conftest.s > /dev/null 2>&1; then
    libc_cv_ppc_fcfid=yes
  fi
changequote([,])dnl
fi
rm -rf conftest*])
if test $libc_cv_ppc_fcfid = yes; then
  AC_DEFINE([HAVE_PPC_FCFID])
fi
