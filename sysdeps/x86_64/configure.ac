GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/x86_64.

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

dnl Check if -mavx works.
AC_CACHE_CHECK(for AVX support, libc_cv_cc_avx, [dnl
LIBC_TRY_CC_OPTION([-mavx], [libc_cv_cc_avx=yes], [libc_cv_cc_avx=no])
])
if test $libc_cv_cc_avx = yes; then
  AC_DEFINE(HAVE_AVX_SUPPORT)
fi
LIBC_CONFIG_VAR([config-cflags-avx], [$libc_cv_cc_avx])

dnl Check if asm supports AVX512.
AC_CACHE_CHECK(for AVX512 support in assembler, libc_cv_asm_avx512, [dnl
cat > conftest.s <<\EOF
        vmovdqu64 %zmm0, (%rsp)
EOF
if AC_TRY_COMMAND(${CC-cc} -c $ASFLAGS conftest.s 1>&AS_MESSAGE_LOG_FD); then
  libc_cv_asm_avx512=yes
else
  libc_cv_asm_avx512=no
fi
rm -f conftest*])
if test $libc_cv_asm_avx512 == yes; then
  AC_DEFINE(HAVE_AVX512_ASM_SUPPORT)
fi

dnl Check if -mavx512f works.
AC_CACHE_CHECK(for AVX512 support, libc_cv_cc_avx512, [dnl
LIBC_TRY_CC_OPTION([-mavx512f], [libc_cv_cc_avx512=$libc_cv_asm_avx512], [libc_cv_cc_avx512=no])
])
if test $libc_cv_cc_avx512 = yes; then
  AC_DEFINE(HAVE_AVX512_SUPPORT)
fi
LIBC_CONFIG_VAR([config-cflags-avx512], [$libc_cv_cc_avx512])

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

dnl Check whether asm supports Intel MPX
AC_CACHE_CHECK(for Intel MPX support, libc_cv_asm_mpx, [dnl
cat > conftest.s <<\EOF
        bndmov %bnd0,(%rsp)
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

dnl It is always possible to access static and hidden symbols in an
dnl position independent way.
AC_DEFINE(PI_STATIC_AND_HIDDEN)
# work around problem with autoconf and empty lines at the end of files
