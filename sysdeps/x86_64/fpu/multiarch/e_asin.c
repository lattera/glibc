#ifdef HAVE_FMA4_SUPPORT
# include <init-arch.h>
# include <math.h>
# include <math_private.h>

extern double __ieee754_acos_sse2 (double);
extern double __ieee754_asin_sse2 (double);
extern double __ieee754_acos_fma4 (double);
extern double __ieee754_asin_fma4 (double);

libm_ifunc (__ieee754_acos,
	    HAS_ARCH_FEATURE (FMA4_Usable)
	    ? __ieee754_acos_fma4
	    : __ieee754_acos_sse2);
strong_alias (__ieee754_acos, __acos_finite)

libm_ifunc (__ieee754_asin,
	    HAS_ARCH_FEATURE (FMA4_Usable)
	    ? __ieee754_asin_fma4
	    : __ieee754_asin_sse2);
strong_alias (__ieee754_asin, __asin_finite)

# define __ieee754_acos __ieee754_acos_sse2
# define __ieee754_asin __ieee754_asin_sse2
#endif


#include <sysdeps/ieee754/dbl-64/e_asin.c>
