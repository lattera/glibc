GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/arm.

dnl It is always possible to access static and hidden symbols in an
dnl position independent way.
dnl NOTE: This feature was added by the GCC TLS patches.  We should test for
dnl it.  Until we do, don't define it.
#AC_DEFINE(PI_STATIC_AND_HIDDEN)

# We check to see if the compiler and flags are
# selecting the hard-float ABI and if they are then
# we set libc_cv_arm_pcs_vfp to yes which causes
# HAVE_ARM_PCS_VFP to be defined in config.h and
# in include/libc-symbols.h and thus available to
# shlib-versions to select the appropriate name for
# the dynamic linker via %ifdef.
AC_CACHE_CHECK([whether the compiler is using the ARM hard-float ABI],
  [libc_cv_arm_pcs_vfp],
  [AC_EGREP_CPP(yes,[#ifdef __ARM_PCS_VFP
		      yes
		     #endif
  ], libc_cv_arm_pcs_vfp=yes, libc_cv_arm_pcs_vfp=no)])
if test $libc_cv_arm_pcs_vfp = yes; then
  AC_DEFINE(HAVE_ARM_PCS_VFP)
  LIBC_CONFIG_VAR([default-abi], [hard])
else
  LIBC_CONFIG_VAR([default-abi], [soft])
fi

AC_CACHE_CHECK([whether the build tools support the GNU descriptor TLS scheme],
  [libc_cv_arm_tls_desc],
  [old_CFLAGS="$CFLAGS"
  CFLAGS="$CFLAGS -mtls-dialect=gnu2"
  AC_COMPILE_IFELSE([AC_LANG_PROGRAM([asm (".word\tfoo(tlsdesc)");], [])],
    [libc_cv_arm_tls_desc=yes], [libc_cv_arm_tls_desc=no])
  CFLAGS="$old_CFLAGS"])
if test $libc_cv_arm_tls_desc = yes; then
  LIBC_CONFIG_VAR([have-arm-tls-desc], [yes])
else
  LIBC_CONFIG_VAR([have-arm-tls-desc], [no])
fi

AC_CACHE_CHECK([whether PC-relative relocs in movw/movt work properly],
	       libc_cv_arm_pcrel_movw, [
cat > conftest.s <<\EOF
	.syntax unified
	.arm
	.arch armv7-a

	.text
	.globl foo
	.type foo,%function
foo:	movw r0, #:lower16:symbol - 1f - 8
	movt r0, #:upper16:symbol - 1f - 8
1:	add r0, pc
	@ And now a case with a local symbol.
	movw r0, #:lower16:3f - 2f - 8
	movt r0, #:upper16:3f - 2f - 8
2:	add r0, pc
	bx lr

.data
	.globl symbol
	.hidden symbol
symbol:	.long 23
3:	.long 17
EOF
libc_cv_arm_pcrel_movw=no
${CC-cc} $CFLAGS $CPPFLAGS $LDFLAGS \
	 -nostartfiles -nostdlib -shared \
	 -o conftest.so conftest.s 1>&AS_MESSAGE_LOG_FD 2>&AS_MESSAGE_LOG_FD &&
LC_ALL=C $READELF -dr conftest.so > conftest.dr 2>&AS_MESSAGE_LOG_FD &&
{
  cat conftest.dr 1>&AS_MESSAGE_LOG_FD
  fgrep 'TEXTREL
R_ARM_NONE' conftest.dr > /dev/null || libc_cv_arm_pcrel_movw=yes
}
rm -f conftest*])
if test $libc_cv_arm_pcrel_movw = yes; then
  AC_DEFINE([ARM_PCREL_MOVW_OK])
fi

libc_cv_gcc_unwind_find_fde=no

# Remove -fno-unwind-tables that was added in sysdeps/arm/preconfigure.ac.
CFLAGS=${CFLAGS% -fno-unwind-tables}

dnl The normal configure check for gcc -fexecptions fails because it can't
dnl find __aeabi_unwind_cpp_pr0.  Work around this here; our GCC definitely
dnl has -fexceptions.
libc_cv_gcc_exceptions=yes
exceptions=-fexceptions
