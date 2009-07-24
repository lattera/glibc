GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/i386.

AC_CACHE_CHECK([for .cfi_personality and .cfi_lsda pseudo-ops],
	       libc_cv_asm_cfi_personality, [dnl
  cat > conftest.s <<EOF
${libc_cv_dot_text}
foo:
	.cfi_startproc
	.cfi_personality 0, foo
	.cfi_lsda 0, foo
	.cfi_endproc
EOF
  if AC_TRY_COMMAND(${CC-cc} $ASFLAGS -c conftest.s 1>&AS_MESSAGE_LOG_FD); then
    libc_cv_asm_cfi_personality=yes
  else
    libc_cv_asm_cfi_personality=no
  fi
  rm -f conftest*
])
if test x"$libc_cv_asm_cfi_personality" != xyes; then
  AC_MSG_ERROR([assembler too old, .cfi_personality support missing])
fi
