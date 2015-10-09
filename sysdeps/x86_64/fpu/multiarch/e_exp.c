#include <init-arch.h>
#include <math.h>
#include <math_private.h>

extern double __ieee754_exp_sse2 (double);
extern double __ieee754_exp_avx (double);
extern double __ieee754_exp_fma4 (double);

libm_ifunc (__ieee754_exp,
	    HAS_ARCH_FEATURE (FMA4_Usable) ? __ieee754_exp_fma4
	    : (HAS_ARCH_FEATURE (AVX_Usable)
	       ? __ieee754_exp_avx : __ieee754_exp_sse2));
strong_alias (__ieee754_exp, __exp_finite)

#define __ieee754_exp __ieee754_exp_sse2


#include <sysdeps/ieee754/dbl-64/e_exp.c>
