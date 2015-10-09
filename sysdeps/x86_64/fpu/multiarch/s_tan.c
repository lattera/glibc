#include <init-arch.h>
#include <math.h>

extern double __tan_sse2 (double);
extern double __tan_avx (double);
extern double __tan_fma4 (double);

libm_ifunc (tan, (HAS_ARCH_FEATURE (FMA4_Usable) ? __tan_fma4 :
		  HAS_ARCH_FEATURE (AVX_Usable)
		  ? __tan_avx : __tan_sse2));

#define tan __tan_sse2


#include <sysdeps/ieee754/dbl-64/s_tan.c>
