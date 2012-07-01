#define DIRENT_SET_DP_INO(dp, value) \
  do { (dp)->d_ino = (value); (dp)->__pad = 0; } while (0)
#include <sysdeps/unix/sysv/linux/getdents.c>
