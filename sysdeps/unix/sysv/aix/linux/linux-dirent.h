#include "linuxtypes.h"

struct linuxdirent
  {
    __linux_ino_t d_ino;
    __linux_off_t d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];           /* We must not include limits.h! */
  };

struct linuxdirent64
  {
    __linux_ino64_t d_ino;
    __linux_off64_t d_off;
    unsigned short int d_reclen;
    unsigned char d_type;
    char d_name[256];           /* We must not include limits.h! */
  };
