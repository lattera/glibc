#include <init-arch.h>
#include <math.h>

extern double __atan_sse2 (double);
extern double __atan_avx (double);
extern double __atan_fma4 (double);

libm_ifunc (atan, (HAS_ARCH_FEATURE (FMA4_Usable) ? __atan_fma4 :
		   HAS_ARCH_FEATURE (AVX_Usable)
		   ? __atan_avx : __atan_sse2));

#define atan __atan_sse2


#include <sysdeps/ieee754/dbl-64/s_atan.c>
