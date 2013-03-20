GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/sparc.

# Check for support of VIS3 et al. instructions in the assembler.
AC_CACHE_CHECK(for sparc assembler VIS3 support, libc_cv_sparc_as_vis3, [dnl
cat > conftest.S <<\EOF
	.text
foo:	fmadds		%f1, %f2, %f3, %f5
	fmaddd		%f2, %f4, %f8, %f10
	fhadds		%f2, %f3, %f5
	fhaddd		%f4, %f8, %f10
	pdistn		%f2, %f4, %g1
	movdtox		%f10, %o0
	movstouw	%f9, %o1
	movstosw	%f7, %o2
	movxtod		%o3, %f18
	movwtos		%o4, %f15
	flcmps		%fcc0, %f3, %f5
	flcmpd		%fcc1, %f4, %f6
EOF
dnl
if AC_TRY_COMMAND([${CC-cc} -c $CFLAGS -Wa,-Av9d conftest.S]); then
  libc_cv_sparc_as_vis3=yes
else
  libc_cv_sparc_as_vis3=no
fi
rm -f conftest*])
if test $libc_cv_sparc_as_vis3 = yes; then
  AC_DEFINE(HAVE_AS_VIS3_SUPPORT)
fi
LIBC_CONFIG_VAR([have-as-vis3], [$libc_cv_sparc_as_vis3])

# Check for a GCC emitting GOTDATA relocations.
AC_CACHE_CHECK(for sparc gcc GOTDATA reloc support, libc_cv_sparc_gcc_gotdata, [dnl
changequote(,)dnl
cat > conftest.c <<\EOF
int data;
int foo(void)
{
	return data;
}
EOF
changequote([,])dnl
dnl
libc_cv_sparc_gcc_gotdata=no
if AC_TRY_COMMAND(${CC-cc} -S $CFLAGS -O2 -fPIC conftest.c 1>&AS_MESSAGE_LOG_FD); then
  if grep -q 'gdop_hix22' conftest.s \
     && grep -q 'gdop_lox10' conftest.s; then
    libc_cv_sparc_gcc_gotdata=yes
  fi
fi
rm -f conftest*])
if test $libc_cv_sparc_gcc_gotdata = yes; then
  AC_DEFINE(HAVE_GCC_GOTDATA)
fi

if test $libc_cv_sparc_gcc_gotdata = yes; then
  AC_DEFINE(PI_STATIC_AND_HIDDEN)
fi
