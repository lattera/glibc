GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/sparc.

# Check for a GCC emitting GOTDATA relocations.
AC_CACHE_CHECK(for sparc gcc GOTDATA reloc support, libc_cv_sparc_gcc_gotdata, [dnl
changequote(,)dnl
cat > conftest.c <<\EOF
int data;
int foo(void)
{
	return data;
}
EOF
changequote([,])dnl
dnl
libc_cv_sparc_gcc_gotdata=no
if AC_TRY_COMMAND(${CC-cc} -S $CFLAGS -O2 -fPIC conftest.c 1>&AS_MESSAGE_LOG_FD); then
  if grep -q 'gdop_hix22' conftest.s \
     && grep -q 'gdop_lox10' conftest.s; then
    libc_cv_sparc_gcc_gotdata=yes
  fi
fi
rm -f conftest*])
if test $libc_cv_sparc_gcc_gotdata = yes; then
  AC_DEFINE(HAVE_GCC_GOTDATA)
fi

if test $libc_cv_sparc_gcc_gotdata = yes; then
  AC_DEFINE(PI_STATIC_AND_HIDDEN)
fi
