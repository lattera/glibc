#ifndef _INTERNALTYPES_H
#include "../internaltypes.h"

union sparc_pthread_barrier
{
  struct pthread_barrier b;
  struct sparc_pthread_barrier_s
    {
      unsigned int curr_event;
      int lock;
      unsigned int left;
      unsigned int init_count;
      unsigned char left_lock;
      unsigned char pshared;
    } s;
};

#endif
