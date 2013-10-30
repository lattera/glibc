GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/powerpc.

# Accept binutils which knows about ".machine".
AC_CACHE_CHECK(for .machine support, libc_cv_ppc_machine, [dnl
cat > conftest.s <<\EOF
	.machine "altivec"
	blr
EOF
if AC_TRY_COMMAND(${CC-cc} -c $CFLAGS conftest.s 1>&AS_MESSAGE_LOG_FD); then
  libc_cv_ppc_machine=yes
else
  libc_cv_ppc_machine=no
fi
rm -f conftest*])
if test $libc_cv_ppc_machine != yes; then
  AC_MSG_ERROR([
*** A binutils version which can handle
***    .machine "altivec"
*** is needed.])
fi
