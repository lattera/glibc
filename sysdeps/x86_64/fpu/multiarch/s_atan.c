#ifdef HAVE_FMA4_SUPPORT
# include <init-arch.h>
# include <math.h>

extern double __atan_sse2 (double);
extern double __atan_fma4 (double);

libm_ifunc (atan, HAS_FMA4 ? __atan_fma4 : __atan_sse2);

# define atan __atan_sse2
#endif


#include <sysdeps/ieee754/dbl-64/s_atan.c>
