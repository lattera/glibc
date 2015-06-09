#define __statvfs64(file, buf) __no_statvfs64(file, buf)
#define statvfs64(file, buf) no_statvfs64(file, buf)
#include "../statvfs.c"
strong_alias (__statvfs, __statvfs64)
weak_alias (__statvfs, statvfs64)
