#define __GETDENTS __getdents64
#define DIRENT_TYPE struct dirent64
#include <sysdeps/unix/sysv/linux/getdents.c>
