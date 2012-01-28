#if defined HAVE_FMA4_SUPPORT || defined HAVE_AVX_SUPPORT
# include <init-arch.h>
# include <math.h>
# undef NAN

extern double __cos_sse2 (double);
extern double __sin_sse2 (double);
extern double __cos_avx (double);
extern double __sin_avx (double);
# ifdef HAVE_FMA4_SUPPORT
extern double __cos_fma4 (double);
extern double __sin_fma4 (double);
# else
#  undef HAS_FMA4
#  define HAS_FMA4 0
#  define __cos_fma4 ((void *) 0)
#  define __sin_fma4 ((void *) 0)
# endif

libm_ifunc (__cos, (HAS_FMA4 ? __cos_fma4 :
		    HAS_AVX ? __cos_avx : __cos_sse2));
weak_alias (__cos, cos)

libm_ifunc (__sin, (HAS_FMA4 ? __sin_fma4 :
		    HAS_AVX ? __sin_avx : __sin_sse2));
weak_alias (__sin, sin)

# define __cos __cos_sse2
# define __sin __sin_sse2
#endif


#include <sysdeps/ieee754/dbl-64/s_sin.c>
