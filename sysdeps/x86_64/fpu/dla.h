#include <features.h>

#ifdef __FMA4__
# if __GNUC_PREREQ (4, 6)
#  define DLA_FMS(x,y,z) \
  __builtin_fma (x, y, -(z))
# else
#  define DLA_FMS(x,y,z) \
  ({ double __z;							      \
     asm ("vfmsubsd %3, %2, %1, %0"					      \
	  : "=x" (__z)							      \
	  : "x" ((double) (x)), "xm" ((double) (y)) , "x" ((double) (z)));    \
    __z; })
# endif
#endif

#include "sysdeps/ieee754/dbl-64/dla.h"
