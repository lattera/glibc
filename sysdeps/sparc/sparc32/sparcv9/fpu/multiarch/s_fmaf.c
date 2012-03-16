#ifdef HAVE_AS_VIS3_SUPPORT
# include <sparc-ifunc.h>
# include <math.h>

extern float __fmaf_vis3 (float, float, float);
extern float __fmaf_generic (float, float, float);

sparc_libm_ifunc(__fmaf, hwcap & HWCAP_SPARC_FMAF ? __fmaf_vis3 : __fmaf_generic);
weak_alias (__fmaf, fmaf)

# define __fmaf __fmaf_generic
#endif

#include <sysdeps/ieee754/dbl-64/s_fmaf.c>
