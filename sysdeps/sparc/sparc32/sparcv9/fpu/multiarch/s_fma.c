#ifdef HAVE_AS_VIS3_SUPPORT
# include <sparc-ifunc.h>
# include <math.h>
# include <math_ldbl_opt.h>

extern double __fma_vis3 (double, double, double);
extern double __fma_generic (double, double, double);

sparc_libm_ifunc(__fma, hwcap & HWCAP_SPARC_FMAF ? __fma_vis3 : __fma_generic);
weak_alias (__fma, fma)
#if LONG_DOUBLE_COMPAT (libm, GLIBC_2_1)
compat_symbol (libm, __fma, fmal, GLIBC_2_1);
#endif

# define __fma __fma_generic
#endif

#include <sysdeps/ieee754/dbl-64/s_fma.c>
