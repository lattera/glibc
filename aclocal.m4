dnl We define the macro GLIBC_PROVIDES to do an AC_PROVIDE for each macro
dnl which appears in configure.in before the sysdep configure scripts are run.
dnl Each sysdep configure.in does GLIBC_PROVIDES first, to avoid any
dnl AC_REQUIREs or AC_BEFOREs duplicating their code.
dnl
define(AC_FD_MSG,6)dnl Autoconf lossage.
define(AC_FD_CC,5)dnl Autoconf lossage.
AC_DEFUN([GLIBC_PROVIDES], [dnl
AC_PROVIDE([AC_PROG_INSTALL])dnl
AC_PROVIDE([AC_PROG_RANLIB])dnl
AC_PROVIDE([AC_PROG_CC])dnl
AC_PROVIDE([AC_PROG_CPP])dnl
# This file is generated from configure.in by Autoconf.  DO NOT EDIT!
])dnl
dnl
dnl Check for a symbol
dnl
AC_DEFUN(AC_CHECK_SYMBOL, [dnl
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
dnl 		      [version-extract-regexp], version-glob, fatal)
AC_DEFUN(AC_CHECK_PROG_VER,
[# Prepare to iterate over the program-name list.
set dummy $2; shift
AC_MSG_CHECKING([for [$]1])
AC_CACHE_VAL(ac_cv_prog_$1, [dnl
if test -n "[$]$1"; then
  ac_cv_prog_$1="[$]$1" # Let the user override the test.
else
  IFS="${IFS=   }"; ac_save_ifs="$IFS"; IFS="${IFS}:"
  for ac_word; do
    for ac_dir in $PATH; do
      test -z "$ac_dir" && ac_dir=.
      if test -f $ac_dir/$ac_word; then
        ac_cv_prog_$1="$ac_word"
        break
      fi
    done
    test -n "$ac_cv_prog_$1" && break
  done
  IFS="$ac_save_ifs"
fi])dnl
if test -z "$ac_cv_prog_$1"; then
  AC_MSG_RESULT(no)
  $1=:
  ac_verc_fail=t; ifelse([$6],,,[ac_verc_fatal=$6])
else
# Found it, now check the version.
ac_word=$ac_cv_prog_$1
dnl Do this by hand to avoid "(cached) (cached)".
  if test "[$]{ac_cv_prog_$1_ver+set}" != set; then
changequote(<<,>>)dnl
    ac_cv_prog_$1_ver=`$ac_word $3 2>&1 ifelse(<<$4>>,,,<<| sed -n 's/^.*$4.*$/\1/p'>>)`
  fi
  if test -n "$ac_cv_prog_$1_ver"; then
  case $ac_cv_prog_$1_ver in
    <<$5>>) ac_vers_ok=", ok";  $1=$ac_cv_prog_$1;;
changequote([,])dnl
         *) ac_vers_ok=", bad"; $1=:
	    ac_verc_fail=t; ifelse([$6],,,[ac_verc_fatal=$6]);;
  esac
  else
    ac_vers_ok="v. ?.??, bad"; $1=:
    ac_verc_fail=t; ifelse([$6],,,[ac_verc_fatal=$6])
  fi
AC_MSG_RESULT($ac_word $ac_cv_prog_$1_ver$ac_vers_ok)
fi
AC_SUBST($1)dnl
])

dnl These modifications are to allow for an empty cross compiler tree.
dnl In the situation that cross-linking is impossible, the variable
dnl `cross_linkable' will be substituted with "yes".
dnl The vercheck macros are expected to have been called already.
AC_DEFUN(AC_PROG_CC_LOCAL,
[AC_BEFORE([$0], [AC_PROG_CPP])dnl

AC_PROG_CC_WORKS_LOCAL
AC_PROG_CC_GNU
if test $ac_cv_prog_gcc != yes; then
  AC_MSG_ERROR([GNU libc must be compiled using GNU CC])
fi
])

AC_DEFUN(AC_PROG_CC_WORKS_LOCAL,
[AC_MSG_CHECKING([whether the C compiler ($CC $CFLAGS $LDFLAGS) works])
AC_CACHE_VAL(ac_cv_prog_cc_works,
[AC_LANG_SAVE
AC_LANG_C
AC_TRY_COMPILER([main(){return(0);}], ac_cv_prog_cc_works, ac_cv_prog_cc_cross)
AC_LANG_RESTORE])
AC_MSG_RESULT($ac_cv_prog_cc_works)
if test $ac_cv_prog_cc_works = no; then
 cross_linkable=no
 ac_cv_prog_cc_cross=yes
dnl AC_MSG_ERROR([installation or configuration problem: C compiler cannot create executables.])
else
 cross_linkable=yes
fi
AC_CACHE_CHECK(
[whether the C compiler ($CC $CFLAGS $LDFLAGS) is a cross-compiler],
ac_cv_prog_cc_cross, [:])
AC_SUBST(cross_linkable)
cross_compiling=$ac_cv_prog_cc_cross
])

AC_DEFUN(LIBC_PROG_FOO_GNU,
[# Most GNU programs take a -v and spit out some text including
# the word 'GNU'.  Some try to read stdin, so give them /dev/null.
if $1 -v </dev/null 2>&1 | grep GNU > /dev/null 2>&1; then
  $2
else
  $3
fi])

AC_DEFUN(LIBC_PROG_BINUTILS,
[# Was a --with-binutils option given?
if test -n "$path_binutils"; then
    # Make absolute; ensure a single trailing slash.
    path_binutils=`(cd $path_binutils; pwd) | sed 's%/*$%/%'`
    CC="$CC -B$path_binutils"
fi
AS=`$CC -print-file-name=as`
LD=`$CC -print-file-name=ld`

# Determine whether we are using GNU binutils.
AC_CACHE_CHECK(whether $AS is GNU as, libc_cv_prog_as_gnu,
[LIBC_PROG_FOO_GNU($AS, libc_cv_prog_as_gnu=yes, libc_cv_prog_as_gnu=no)])
rm -f a.out
gnu_as=$libc_cv_prog_as_gnu

AC_CACHE_CHECK(whether $LD is GNU ld, libc_cv_prog_ld_gnu,
[LIBC_PROG_FOO_GNU($LD, libc_cv_prog_ld_gnu=yes, libc_cv_prog_ld_gnu=no)])
gnu_ld=$libc_cv_prog_ld_gnu
])
