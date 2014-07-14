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

dnl Check if -msse4 works.
AC_CACHE_CHECK(for SSE4 support, libc_cv_cc_sse4, [dnl
LIBC_TRY_CC_OPTION([-msse4], [libc_cv_cc_sse4=yes], [libc_cv_cc_sse4=no])
])
if test $libc_cv_cc_sse4 = yes; then
  AC_DEFINE(HAVE_SSE4_SUPPORT)
fi
LIBC_CONFIG_VAR([config-cflags-sse4], [$libc_cv_cc_sse4])

dnl Check if -Wa,-mtune=i686 works.
AC_CACHE_CHECK(for assembler -mtune=i686 support, libc_cv_as_i686, [dnl
LIBC_TRY_CC_OPTION([-Wa,-mtune=i686],
		   [libc_cv_as_i686=yes],
		   [libc_cv_as_i686=no])
])
LIBC_CONFIG_VAR([config-asflags-i686], [$libc_cv_as_i686])

dnl Check if -mavx works.
AC_CACHE_CHECK(for AVX support, libc_cv_cc_avx, [dnl
LIBC_TRY_CC_OPTION([-mavx], [libc_cv_cc_avx=yes], [libc_cv_cc_avx=no])
])
if test $libc_cv_cc_avx = yes; then
  AC_DEFINE(HAVE_AVX_SUPPORT)
fi
LIBC_CONFIG_VAR([config-cflags-avx], [$libc_cv_cc_avx])

dnl Check if -msse2avx works.
AC_CACHE_CHECK(for AVX encoding of SSE instructions, libc_cv_cc_sse2avx, [dnl
LIBC_TRY_CC_OPTION([-msse2avx],
		   [libc_cv_cc_sse2avx=yes],
		   [libc_cv_cc_sse2avx=no])
])
if test $libc_cv_cc_sse2avx = yes; then
  AC_DEFINE(HAVE_SSE2AVX_SUPPORT)
fi
LIBC_CONFIG_VAR([config-cflags-sse2avx], [$libc_cv_cc_sse2avx])

dnl Check if -mfma4 works.
AC_CACHE_CHECK(for FMA4 support, libc_cv_cc_fma4, [dnl
LIBC_TRY_CC_OPTION([-mfma4], [libc_cv_cc_fma4=yes], [libc_cv_cc_fma4=no])
])
if test $libc_cv_cc_fma4 = yes; then
  AC_DEFINE(HAVE_FMA4_SUPPORT)
fi
LIBC_CONFIG_VAR([have-mfma4], [$libc_cv_cc_fma4])

dnl Check if -mno-vzeroupper works.
AC_CACHE_CHECK(for -mno-vzeroupper support, libc_cv_cc_novzeroupper, [dnl
LIBC_TRY_CC_OPTION([-mno-vzeroupper],
		   [libc_cv_cc_novzeroupper=yes],
		   [libc_cv_cc_novzeroupper=no])
])
LIBC_CONFIG_VAR([config-cflags-novzeroupper], [$libc_cv_cc_novzeroupper])

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
