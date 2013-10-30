dnl glibc configure fragment for libidn add-on
GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.

libc_add_on_canonical=

AS_IF([test "$shared" = yes], [
  libc_add_on_subdirs=.

  # Get this defined in config.h for main source code to test.
  AC_DEFINE([HAVE_LIBIDN])
], [AC_MSG_WARN([libidn add-on ignored with --disable-shared])])
