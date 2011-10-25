#ifdef HAVE_FMA4_SUPPORT
# include <init-arch.h>
# include <math.h>
# undef NAN

extern double __cos_sse2 (double);
extern double __cos_fma4 (double);
extern double __sin_sse2 (double);
extern double __sin_fma4 (double);

libm_ifunc (__cos, HAS_FMA4 ? __cos_fma4 : __cos_sse2);
weak_alias (__cos, cos)

libm_ifunc (__sin, HAS_FMA4 ? __sin_fma4 : __sin_sse2);
weak_alias (__sin, sin)

# define __cos __cos_sse2
# define __sin __sin_sse2
#endif


#include <sysdeps/ieee754/dbl-64/s_sin.c>
