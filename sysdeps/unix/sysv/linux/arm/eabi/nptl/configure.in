dnl configure fragment for NPTL and ARM/Linux EABI.
GLIBC_PROVIDES dnl See aclocal.m4 in the top level source directory.

dnl The normal configure check for gcc -fexecptions fails because it can't
dnl find __aeabi_unwind_cpp_pr0.  Work around this here; our GCC definitely
dnl has -fexceptions.
libc_cv_gcc_exceptions=yes
exceptions=-fexceptions
