#if defined HAVE_FMA4_SUPPORT || defined HAVE_AVX_SUPPORT
# include <init-arch.h>
# include <math.h>

extern double __tan_sse2 (double);
extern double __tan_avx (double);
# ifdef HAVE_FMA4_SUPPORT
extern double __tan_fma4 (double);
# else
#  undef HAS_FMA4
#  define HAS_FMA4 0
#  define __tan_fma4 ((void *) 0)
# endif

libm_ifunc (tan, (HAS_FMA4 ? __tan_fma4 :
		  HAS_AVX ? __tan_avx : __tan_sse2));

# define tan __tan_sse2
#endif


#include <sysdeps/ieee754/dbl-64/s_tan.c>
