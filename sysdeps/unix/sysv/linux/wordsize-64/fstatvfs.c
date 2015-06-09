#define __fstatvfs64(file, buf) __no_fstatvfs64(file, buf)
#define fstatvfs64(file, buf) no_fstatvfs64(file, buf)
#include "../fstatvfs.c"
strong_alias (__fstatvfs, __fstatvfs64)
weak_alias (__fstatvfs, fstatvfs64)
