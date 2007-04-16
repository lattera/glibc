#ifndef _MATH_PRIVATE_H

#define math_opt_barrier(x) \
({ __typeof(x) __x;					\
   __asm ("" : "=t" (__x) : "0" (x));			\
   __x; })
#define math_force_eval(x) \
do							\
  {							\
    if (sizeof (x) <= sizeof (double))			\
      __asm __volatile ("" : : "m" (x));		\
    else						\
      __asm __volatile ("" : : "f" (x));		\
  }							\
while (0)

#include <math/math_private.h>
#endif
