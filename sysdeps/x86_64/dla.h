#ifdef __FMA4__
# define DLA_FMA(x,y,z) \
	   ({ double __zz; \
	      asm ("vfmsubsd %3, %2, %1, %0"				      \
		   : "=x" (__zz) : "x" (x), "xm" (y), "x" (z));		      \
	      __zz; })
#endif

#include "sysdeps/ieee754/dbl-64/dla.h"
