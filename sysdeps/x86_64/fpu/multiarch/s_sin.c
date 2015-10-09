#include <init-arch.h>
#include <math.h>
#undef NAN

extern double __cos_sse2 (double);
extern double __sin_sse2 (double);
extern double __cos_avx (double);
extern double __sin_avx (double);
extern double __cos_fma4 (double);
extern double __sin_fma4 (double);

libm_ifunc (__cos, (HAS_ARCH_FEATURE (FMA4_Usable) ? __cos_fma4 :
		    HAS_ARCH_FEATURE (AVX_Usable)
		    ? __cos_avx : __cos_sse2));
weak_alias (__cos, cos)

libm_ifunc (__sin, (HAS_ARCH_FEATURE (FMA4_Usable) ? __sin_fma4 :
		    HAS_ARCH_FEATURE (AVX_Usable)
		    ? __sin_avx : __sin_sse2));
weak_alias (__sin, sin)

#define __cos __cos_sse2
#define __sin __sin_sse2


#include <sysdeps/ieee754/dbl-64/s_sin.c>
