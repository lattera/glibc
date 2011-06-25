dnl We define the macro GLIBC_PROVIDES to do an AC_PROVIDE for each macro
dnl which appears in configure.in before the sysdep configure scripts are run.
dnl Each sysdep configure.in does GLIBC_PROVIDES first, to avoid any
dnl AC_REQUIREs or AC_BEFOREs duplicating their code.
dnl
define([GLIBC_PROVIDES], [dnl
AC_PROVIDE([AC_CONFIG_AUX_DIR_DEFAULT])dnl
AC_PROVIDE([AC_CONFIG_SUBDIRS])dnl
AC_PROVIDE([_AS_ECHO_N_PREPARE])dnl
AC_PROVIDE([_AS_ECHO_PREPARE])dnl
AC_PROVIDE([_AS_CR_PREPARE])dnl
AC_PROVIDE([_AS_TR_SH_PREPARE])dnl
AC_PROVIDE([AC_PROG_INSTALL])dnl
AC_PROVIDE([AC_PROG_RANLIB])dnl
AC_PROVIDE([AC_PROG_CC])dnl
AC_PROVIDE([AC_PROG_CPP])dnl
AC_PROVIDE([_AS_PATH_SEPARATOR_PREPARE])dnl
AC_PROVIDE([_AS_TEST_PREPARE])dnl
define([AS_MESSAGE_LOG_FD],5)dnl
define([AS_MESSAGE_FD],6)dnl
dnl Ripped out of AS_INIT, which does more cruft we do not want.
m4_wrap([m4_divert_pop([BODY])[]])
m4_divert_push([BODY])[]dnl
dnl End of ripped out of AS_INIT.
# This file is generated from configure.in by Autoconf.  DO NOT EDIT!
define([_AC_LANG], [C])dnl
])dnl
dnl
dnl Check for a symbol
dnl
AC_DEFUN([AC_CHECK_SYMBOL], [dnl
AC_MSG_CHECKING(for $1)
AC_CACHE_VAL(ac_cv_check_symbol_$1, [dnl
AC_TRY_LINK(,
changequote(,)dnl
extern char *$1[]; puts(*$1);,
changequote([,])dnl
	    ac_cv_check_symbol_$1=yes, ac_cv_check_symbol_$1=no)])
if test "$ac_cv_check_symbol_$1" = yes; then
changequote(,)dnl
  ac_tr_symbol=`echo $1 | tr '[a-z]' '[A-Z]'`
changequote([,])dnl
  AC_DEFINE_UNQUOTED(HAVE_${ac_tr_symbol})
fi
AC_MSG_RESULT($ac_cv_check_symbol_$1)])dnl
dnl

dnl Locate a program and check that its version is acceptable.
dnl AC_PROG_CHECK_VER(var, namelist, version-switch,
dnl 		      [version-extract-regexp], version-glob [, do-if-fail])
AC_DEFUN([AC_CHECK_PROG_VER],
[AC_CHECK_PROGS([$1], [$2])
if test -z "[$]$1"; then
  ac_verc_fail=yes
else
  # Found it, now check the version.
  AC_MSG_CHECKING([version of [$]$1])
changequote(<<,>>)dnl
  ac_prog_version=`<<$>>$1 $3 2>&1 ifelse(<<$4>>,,,
		   <<| sed -n 's/^.*patsubst(<<$4>>,/,\/).*$/\1/p'>>)`
  case $ac_prog_version in
    '') ac_prog_version="v. ?.??, bad"; ac_verc_fail=yes;;
    <<$5>>)
changequote([,])dnl
       ac_prog_version="$ac_prog_version, ok"; ac_verc_fail=no;;
    *) ac_prog_version="$ac_prog_version, bad"; ac_verc_fail=yes;;

  esac
  AC_MSG_RESULT([$ac_prog_version])
fi
ifelse([$6],,,
[if test $ac_verc_fail = yes; then
  $6
fi])
])

dnl These modifications are to allow for an empty cross compiler tree.
define([_AC_COMPILER_EXEEXT], [EXEEXT=
])

AC_DEFUN([LIBC_PROG_FOO_GNU],
[# Most GNU programs take a -v and spit out some text including
# the word 'GNU'.  Some try to read stdin, so give them /dev/null.
if $1 -o conftest -v </dev/null 2>&1 | grep GNU > /dev/null 2>&1; then
  $2
else
  $3
fi
rm -fr contest*])

AC_DEFUN([LIBC_PROG_BINUTILS],
[# Was a --with-binutils option given?
if test -n "$path_binutils"; then
    # Make absolute; ensure a single trailing slash.
    path_binutils=`(cd $path_binutils; pwd) | sed 's%/*$%/%'`
    CC="$CC -B$path_binutils"
fi
AS=`$CC -print-prog-name=as`
LD=`$CC -print-prog-name=ld`
AR=`$CC -print-prog-name=ar`
AC_SUBST(AR)
OBJDUMP=`$CC -print-prog-name=objdump`
AC_SUBST(OBJDUMP)
OBJCOPY=`$CC -print-prog-name=objcopy`
AC_SUBST(OBJCOPY)

# Determine whether we are using GNU binutils.
AC_CACHE_CHECK(whether $AS is GNU as, libc_cv_prog_as_gnu,
[LIBC_PROG_FOO_GNU($AS, libc_cv_prog_as_gnu=yes, libc_cv_prog_as_gnu=no)])
rm -f a.out
gnu_as=$libc_cv_prog_as_gnu

AC_CACHE_CHECK(whether $LD is GNU ld, libc_cv_prog_ld_gnu,
[LIBC_PROG_FOO_GNU($LD, libc_cv_prog_ld_gnu=yes, libc_cv_prog_ld_gnu=no)])
gnu_ld=$libc_cv_prog_ld_gnu
])
