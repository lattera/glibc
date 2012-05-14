#include <init-arch.h>

extern double __ieee754_expf_sse2 (double);
extern double __ieee754_expf_ia32 (double);

double __ieee754_expf (double);
libm_ifunc (__ieee754_expf,
	    HAS_SSE2 ? __ieee754_expf_sse2 : __ieee754_expf_ia32);

extern double __expf_finite_sse2 (double);
extern double __expf_finite_ia32 (double);

double __expf_finite (double);
libm_ifunc (__expf_finite,
	    HAS_SSE2 ? __expf_finite_sse2 : __expf_finite_ia32);
