GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/unix/sysv/linux/powerpc/powerpc64/.

# Define default-abi according to compiler flags.
AC_CACHE_CHECK([whether the compiler is using the PowerPC64 ELFv2 ABI],
  [libc_cv_ppc64_elfv2_abi],
  [AC_EGREP_CPP(yes,[#if _CALL_ELF == 2
                      yes
                     #endif
  ], libc_cv_ppc64_elfv2_abi=yes, libc_cv_ppc64_elfv2_abi=no)])
if test $libc_cv_ppc64_elfv2_abi = yes; then
  LIBC_CONFIG_VAR([default-abi], [64-v2])
else
  LIBC_CONFIG_VAR([default-abi], [64-v1])
fi
