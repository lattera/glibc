#define __READDIR __readdir64
#define __GETDENTS __getdents64
#define DIRENT_TYPE struct dirent64

#include <sysdeps/unix/readdir.c>

#include <shlib-compat.h>

versioned_symbol (libc, __readdir64, readdir64, GLIBC_2_2);

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)
strong_alias (__readdir64, __old_readdir64)
compat_symbol (libc, __old_readdir64, readdir64, GLIBC_2_1);
#endif
