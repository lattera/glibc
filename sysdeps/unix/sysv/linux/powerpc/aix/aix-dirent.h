#include "aix-types.h"

struct aixdirent
  {
    aixino_t d_ino;
    aixoff_t d_off;
    unsigned short int d_reclen;
    unsigned short int d_namlen;
    char d_name[256];           /* We must not include limits.h! */
  };

struct aixdirent64
  {
    aixino64_t d_ino;
    aixoff64_t d_off;
    unsigned short int d_reclen;
    unsigned short int d_namlen;
    char d_name[256];           /* We must not include limits.h! */
  };
