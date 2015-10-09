#include <init-arch.h>
#include <math.h>
#include <math_private.h>

extern double __ieee754_log_sse2 (double);
extern double __ieee754_log_avx (double);
extern double __ieee754_log_fma4 (double);

libm_ifunc (__ieee754_log,
	    HAS_ARCH_FEATURE (FMA4_Usable) ? __ieee754_log_fma4
	    : (HAS_ARCH_FEATURE (AVX_Usable)
	       ? __ieee754_log_avx : __ieee754_log_sse2));
strong_alias (__ieee754_log, __log_finite)

#define __ieee754_log __ieee754_log_sse2


#include <sysdeps/ieee754/dbl-64/e_log.c>
