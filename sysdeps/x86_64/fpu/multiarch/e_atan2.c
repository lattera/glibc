#include <init-arch.h>
#include <math.h>
#include <math_private.h>

extern double __ieee754_atan2_sse2 (double, double);
extern double __ieee754_atan2_avx (double, double);
extern double __ieee754_atan2_fma4 (double, double);

libm_ifunc (__ieee754_atan2,
	    HAS_ARCH_FEATURE (FMA4_Usable) ? __ieee754_atan2_fma4
	    : (HAS_ARCH_FEATURE (AVX_Usable)
	       ? __ieee754_atan2_avx : __ieee754_atan2_sse2));
strong_alias (__ieee754_atan2, __atan2_finite)

#define __ieee754_atan2 __ieee754_atan2_sse2


#include <sysdeps/ieee754/dbl-64/e_atan2.c>
