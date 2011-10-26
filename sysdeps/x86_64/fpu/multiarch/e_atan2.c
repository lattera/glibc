#if defined HAVE_FMA4_SUPPORT || defined HAVE_AVX_SUPPORT
# include <init-arch.h>
# include <math_private.h>

extern double __ieee754_atan2_sse2 (double, double);
extern double __ieee754_atan2_avx (double, double);
# ifdef HAVE_FMA4_SUPPORT
extern double __ieee754_atan2_fma4 (double, double);
# else
#  undef HAS_FMA4
#  define HAS_FMA4 0
#  define __ieee754_atan2_fma4 ((void *) 0)
# endif

libm_ifunc (__ieee754_atan2,
	    HAS_FMA4 ? __ieee754_atan2_fma4
	    : (HAS_AVX ? __ieee754_atan2_avx : __ieee754_atan2_sse2));
strong_alias (__ieee754_atan2, __atan2_finite)

# define __ieee754_atan2 __ieee754_atan2_sse2
#endif


#include <sysdeps/ieee754/dbl-64/e_atan2.c>
