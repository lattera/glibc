#define __READDIR_R __readdir64_r
#define __GETDENTS __getdents64
#define DIRENT_TYPE struct dirent64

#include <sysdeps/unix/readdir_r.c>

#include <shlib-compat.h>

versioned_symbol (libc, __readdir64_r, readdir64_r, GLIBC_2_2);

#if SHLIB_COMPAT(libc, GLIBC_2_1, GLIBC_2_2)
strong_alias (__readdir64_r, __old_readdir64_r)
compat_symbol (libc, __old_readdir64_r, readdir64_r, GLIBC_2_1);
#endif
