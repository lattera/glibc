#if defined HAVE_FMA4_SUPPORT || defined HAVE_AVX_SUPPORT
# include <init-arch.h>
# include <math.h>

extern double __tan_sse2 (double);
extern double __tan_avx (double);
# ifdef HAVE_FMA4_SUPPORT
extern double __tan_fma4 (double);
# else
#  undef HAS_ARCH_FEATURE
#  define HAS_ARCH_FEATURE(feature) 0
#  define __tan_fma4 ((void *) 0)
# endif

libm_ifunc (tan, (HAS_ARCH_FEATURE (FMA4_Usable) ? __tan_fma4 :
		  HAS_ARCH_FEATURE (AVX_Usable)
		  ? __tan_avx : __tan_sse2));

# define tan __tan_sse2
#endif


#include <sysdeps/ieee754/dbl-64/s_tan.c>
