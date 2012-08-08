#define __READDIR_R __readdir64_r
#define __GETDENTS __getdents64
#define DIRENT_TYPE struct dirent64

#include <sysdeps/posix/readdir_r.c>

weak_alias (__readdir64_r, readdir64_r)
