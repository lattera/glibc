#define DIRENT_SET_DP_INO(dp, value) \
  do { (dp)->d_ino = (value); (dp)->__pad = 0; } while (0)
#define __getdents64 __no___getdents64_decl
#include <sysdeps/unix/sysv/linux/getdents.c>
#undef __getdents64
weak_alias(__getdents, __getdents64);
