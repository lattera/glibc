GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/powerpc/powerpc64.

# The Aix ld uses global .symbol_names instead of symbol_names
# and unfortunately early Linux PPC64 linkers use it as well.
AC_CACHE_CHECK(for support for omitting dot symbols,
libc_cv_omit_dot_syms, [dnl
libc_cv_omit_dot_syms=no
echo 'void foo (void) {}' > conftest.c
if AC_TRY_COMMAND(${CC-cc} $CFLAGS $CPPFLAGS -S conftest.c -o conftest.s 1>&AS_MESSAGE_LOG_FD); then
  if grep -w '\.foo' conftest.s > /dev/null; then
    :
  else
    libc_cv_omit_dot_syms=yes
  fi
fi
rm -f conftest.c conftest.s
])
if test x$libc_cv_omit_dot_syms != xyes; then
  AC_DEFINE(HAVE_ASM_GLOBAL_DOT_NAME)
fi

AC_CACHE_CHECK(for linker support for overlapping .opd entries,
libc_cv_overlapping_opd, [dnl
libc_cv_overlapping_opd=no
echo 'void foo (void) {}' > conftest.c
if AC_TRY_COMMAND(${CC-cc} $CFLAGS $CPPFLAGS -S conftest.c -o conftest.s 1>&AS_MESSAGE_LOG_FD); then
changequote(,)dnl
  if grep '\.TOC\.@tocbase' conftest.s > /dev/null; then
    if grep '\.TOC\.@tocbase[ 	]*,[ 	]*0' conftest.s > /dev/null; then
      :
    else
      libc_cv_overlapping_opd=yes
    fi
  fi
changequote([,])dnl
fi
rm -f conftest.c conftest.s
])
if test x$libc_cv_overlapping_opd = xyes; then
  AC_DEFINE(USE_PPC64_OVERLAPPING_OPD)
fi
