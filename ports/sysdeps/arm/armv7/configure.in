GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/arm/armv7.

# We need binutils 2.21 to ensure that NEON alignments are assembled correctly.
libc_cv_arm_as_version_ok=yes
AC_CHECK_PROG_VER(AS, $AS, --version,
                  [GNU assembler.* \([0-9]*\.[0-9.]*\)],
                  [2.1[0-9][0-9]*|2.2[1-9]*|2.[3-9][0-9]*|[3-9].*|[1-9][0-9]*], libc_cv_arm_as_version_ok=no)

if test $libc_cv_arm_as_version_ok != yes; then
  AC_MSG_ERROR([as version too old, at least 2.21 is required])
fi
