#ifdef __FMA4__
# define DLA_FMS(x,y,z) \
  ({ double __z;							      \
     asm ("vfmsubsd %3, %2, %1, %0"					      \
	  : "=x" (__z)							      \
	  : "x" ((double) (x)), "xm" ((double) (y)) , "x" ((double) (z)));    \
    __z; })
#endif

#include "sysdeps/ieee754/dbl-64/dla.h"
