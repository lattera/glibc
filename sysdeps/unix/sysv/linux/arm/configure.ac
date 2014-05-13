GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.
# Local configure fragment for sysdeps/unix/sysv/linux/arm.

libc_cv_gcc_unwind_find_fde=no
# Remove -fno-unwind-tables that was added in sysdeps/arm/preconfigure.ac.
CFLAGS=${CFLAGS% -fno-unwind-tables}

dnl The normal configure check for gcc -fexecptions fails because it can't
dnl find __aeabi_unwind_cpp_pr0.  Work around this here; our GCC definitely
dnl has -fexceptions.
libc_cv_gcc_exceptions=yes
exceptions=-fexceptions
