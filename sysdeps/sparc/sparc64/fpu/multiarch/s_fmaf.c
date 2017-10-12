#include <sparc-ifunc.h>
#include <math.h>

extern __typeof (fmaf) __fmaf_vis3 attribute_hidden;
extern __typeof (fmaf) __fmaf_generic attribute_hidden;

sparc_libm_ifunc (__fmaf,
		  hwcap & HWCAP_SPARC_FMAF
		  ? __fmaf_vis3
		  : __fmaf_generic)
weak_alias (__fmaf, fmaf)
