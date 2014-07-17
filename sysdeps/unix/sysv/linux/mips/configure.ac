sinclude(./aclocal.m4)dnl Autoconf lossage
GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/unix/sysv/linux/mips.

AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([
    #if (_MIPS_SIM != _ABIO32)
    #error Not O32 ABI
    #endif])],
  [libc_mips_abi=o32],
  [AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([
      #if (_MIPS_SIM != _ABIN32)
      #error Not N32 ABI
      #endif])],
    [libc_mips_abi=n32],
    [AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM([
        #if (_MIPS_SIM != _ABI64)
        #error Not 64 ABI
        #endif])],
      [libc_mips_abi=n64],
      [])])])

if test -z "$libc_mips_abi"; then
  AC_MSG_ERROR([could not determine what ABI the compiler is using])
fi

AC_COMPILE_IFELSE(
  [AC_LANG_PROGRAM([
    #if !defined(__mips_soft_float)
    #error Not soft ABI
    #endif])],
  [libc_mips_float=soft],
  [AC_COMPILE_IFELSE(
    [AC_LANG_PROGRAM([
      #if !defined(__mips_hard_float)
      #error Not hard ABI
      #endif])],
    [libc_mips_float=hard],
    [])])

if test -z "$libc_mips_float"; then
  AC_MSG_ERROR([could not determine if compiler is using hard or soft floating point ABI])
fi

AC_CACHE_CHECK([whether the compiler is using the 2008 NaN encoding],
  libc_cv_mips_nan2008, [AC_EGREP_CPP(yes, [dnl
#ifdef __mips_nan2008
yes
#endif], libc_cv_mips_nan2008=yes, libc_cv_mips_nan2008=no)])

libc_mips_nan=
if test x"$libc_cv_mips_nan2008" = xyes; then
  libc_mips_nan=_2008
fi

LIBC_CONFIG_VAR([default-abi],
  [${libc_mips_abi}_${libc_mips_float}${libc_mips_nan}])

case $machine in
mips/mips64/n64/*)
  LIBC_SLIBDIR_RTLDDIR([lib64], [lib64])
  ;;
mips/mips64/n32/*)
  LIBC_SLIBDIR_RTLDDIR([lib32], [lib32])
  ;;
esac

libc_cv_gcc_unwind_find_fde=yes

if test -z "$arch_minimum_kernel"; then
  if test x$libc_cv_mips_nan2008 = xyes; then
    # FIXME: Adjust this setting to the actual first upstream kernel
    # version to support the 2008 NaN encoding and then remove this
    # comment.
    arch_minimum_kernel=10.0.0
  fi
fi
