GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.

dnl The standard hppa assembler uses `;' to start comments and `!'
dnl as a line separator.
AC_CACHE_CHECK(for assembler line separator,
	       libc_cv_asm_line_sep, [dnl
cat > conftest.s <<EOF
nop ; is_old_puffin
EOF
if AC_TRY_COMMAND(${CC-cc} -c $ASFLAGS conftest.s 1>&AS_MESSAGE_LOG_FD); then
  libc_cv_asm_line_sep='!'
else
  if test -z "$enable_hacker_mode"; then
    echo "*** You need a newer assembler to compile glibc"
    rm -f conftest*
    exit 1
  fi
  libc_cv_asm_line_sep=';'
fi
rm -f conftest*])
AC_DEFINE_UNQUOTED(ASM_LINE_SEP, $libc_cv_asm_line_sep)

# Check for support of thread-local storage handling in assembler and
# linker.
AC_CACHE_CHECK(for hppa TLS support, libc_cv_hppa_tls, [dnl
cat > conftest.s <<\EOF
; Setup tls data
.section ".tdata","awT",@progbits
foo:	.data	32
	.text
; Test general dyanmic relocations
test0:
	addil 	LT'foo-$tls_gdidx$, %r19
	ldo 	RT'foo-$tls_gdidx$(%r1), %r26
	b 	__tls_get_addr
	nop
; Test local dynamic relocations
test1:
	addil 	LT'foo-$tls_ldidx$, %r19
	b 	__tls_get_addr
	ldo 	RT'foo-$tls_ldidx$(%r1), %r26
	ldo 	RR'foo-$tls_dtpoff$(%r1), %r25
	; More variables can be loaded...
; Test initial exec reloctiosn
test2:
	mfctl 	%cr27, %r26
	addil 	LT'foo-$tls_ieoff$, %r19
	ldw 	RT'foo-$tls_ieoff$(%r1), %r25
	add 	%r26, %r25, %r24
; Test local exec relocations
test3:
	mfctl 	%cr27, %r26
	addil 	LR'foo-$tls_leoff$, %r26
	ldo 	RR'foo-$tls_leoff$(%r1), %r25
; Done all the TLS tests.
EOF
dnl
if AC_TRY_COMMAND(${CC-cc} -c $CFLAGS conftest.s 1>&AS_MESSAGE_LOG_FD); then
  libc_cv_hppa_tls=yes
else
  libc_cv_hppa_tls=no
fi
rm -f conftest*])
if test $libc_cv_hppa_tls = no; then
  AC_MSG_ERROR([the assembler must support TLS])
fi
