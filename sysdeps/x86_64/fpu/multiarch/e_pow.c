#ifdef HAVE_FMA4_SUPPORT
# include <init-arch.h>
# include <math.h>
# include <math_private.h>

extern double __ieee754_pow_sse2 (double, double);
extern double __ieee754_pow_fma4 (double, double);

libm_ifunc (__ieee754_pow,
	    HAS_ARCH_FEATURE (FMA4_Usable)
	    ? __ieee754_pow_fma4
	    : __ieee754_pow_sse2);
strong_alias (__ieee754_pow, __pow_finite)

# define __ieee754_pow __ieee754_pow_sse2
#endif


#include <sysdeps/ieee754/dbl-64/e_pow.c>
