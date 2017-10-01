#define __READDIR __readdir64
#define __GETDENTS __getdents64
#define DIRENT_TYPE struct dirent64

#include <sysdeps/posix/readdir.c>

libc_hidden_def (__readdir64)
weak_alias (__readdir64, readdir64)
