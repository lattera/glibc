GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/i386.

# We no longer support i386 since it lacks the atomic instructions
# required to implement NPTL threading.
if test "$config_machine" = i386; then
  AC_MSG_ERROR([
*** ERROR: Support for i386 is deprecated.
*** Please use host i786, i686, i585 or i486.
*** For example: /src/glibc/configure --host=i686-pc-linux-gnu ..."])
fi

# The GNU C Library can't be built for i386.  There are several reasons for
# this restriction.  The primary reason is that i386 lacks the atomic
# operations required to support the current NPTL implementation.  While it is
# possible that such atomic operations could be emulated in the kernel to date
# no such work has been done to enable this.  Even with NPTL disabled you still
# have no atomic.h implementation.  Given the declining use of i386 we disable
# support for building with `-march=i386' or `-mcpu=i386.' We don't explicitly
# check for i386, instead we make sure the compiler has support for inlining
# the builtin __sync_val_compare_and_swap. If it does then we should have no
# problem building for i386.
LIBC_COMPILER_BUILTIN_INLINED(
  [__sync_val_compare_and_swap],
  [int a, b, c; __sync_val_compare_and_swap (&a, b, c);],
  [-O0],
  [libc_cv_unsupported_i386=no],
  [AC_MSG_ERROR([
*** Building with -march=i386/-mcpu=i386 is not supported.
*** Please use host i786, i686, i586, or i486.
*** For example: /source/glibc/configure CFLAGS='-O2 -march=i686' ...])])

AC_CHECK_HEADER([cpuid.h], ,
  [AC_MSG_ERROR([gcc must provide the <cpuid.h> header])],
  [/* No default includes.  */])

dnl Check whether asm supports Intel MPX
AC_CACHE_CHECK(for Intel MPX support, libc_cv_asm_mpx, [dnl
cat > conftest.s <<\EOF
        bndmov %bnd0,(%esp)
EOF
if AC_TRY_COMMAND(${CC-cc} -c $ASFLAGS conftest.s 1>&AS_MESSAGE_LOG_FD); then
  libc_cv_asm_mpx=yes
else
  libc_cv_asm_mpx=no
fi
rm -f conftest*])
if test $libc_cv_asm_mpx == yes; then
  AC_DEFINE(HAVE_MPX_SUPPORT)
fi

dnl Check if -mavx2 works.
AC_CACHE_CHECK(for AVX2 support, libc_cv_cc_avx2, [dnl
LIBC_TRY_CC_OPTION([-mavx2], [libc_cv_cc_avx2=yes], [libc_cv_cc_avx2=no])
])
if test $libc_cv_cc_avx2 = yes; then
  AC_DEFINE(HAVE_AVX2_SUPPORT)
fi
LIBC_CONFIG_VAR([config-cflags-avx2], [$libc_cv_cc_avx2])

AC_DEFINE(USE_REGPARMS)

dnl It is always possible to access static and hidden symbols in an
dnl position independent way.
AC_DEFINE(PI_STATIC_AND_HIDDEN)
