GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/powerpc/powerpc32.

# See whether gas has R_PPC_REL16 relocs.
AC_CACHE_CHECK(for R_PPC_REL16 gas support, libc_cv_ppc_rel16, [dnl
cat > conftest.s <<\EOF
	.text
	addis 11,30,_GLOBAL_OFFSET_TABLE_-.@ha
EOF
if AC_TRY_COMMAND(${CC-cc} -c $CFLAGS conftest.s 1>&AS_MESSAGE_LOG_FD); then
  libc_cv_ppc_rel16=yes
else
  libc_cv_ppc_rel16=no
fi
rm -f conftest*])
if test $libc_cv_ppc_rel16 = no; then
  AC_MSG_ERROR(R_PPC_REL16 is not supported. Binutils is too old.)
fi

# See whether GCC uses -msecure-plt.
AC_CACHE_CHECK(for -msecure-plt by default, libc_cv_ppc_secure_plt, [dnl
echo 'int foo (void) { extern int bar; return bar; }' > conftest.c
libc_cv_ppc_secure_plt=no
if AC_TRY_COMMAND(${CC-cc} -S $CFLAGS conftest.c -fpic -o conftest.s 1>&AS_MESSAGE_LOG_FD); then
  if grep '_GLOBAL_OFFSET_TABLE_-.*@ha' conftest.s > /dev/null 2>&1; then
    libc_cv_ppc_secure_plt=yes
  fi
fi
rm -rf conftest*])
if test $libc_cv_ppc_secure_plt = yes; then
  AC_DEFINE(HAVE_PPC_SECURE_PLT)
fi
