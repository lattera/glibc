#ifdef HAVE_FMA4_SUPPORT
# include <init-arch.h>
# include <math.h>

extern double __tan_sse2 (double);
extern double __tan_fma4 (double);

libm_ifunc (tan, HAS_FMA4 ? __tan_fma4 : __tan_sse2);

# define tan __tan_sse2
#endif


#include <sysdeps/ieee754/dbl-64/s_tan.c>
