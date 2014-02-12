#include <shlib-compat.h>

#if SHLIB_COMPAT (libm, GLIBC_2_15, GLIBC_2_18)
# define __sqrtf_finite __sqrtf_finite1
#endif

#include <sysdeps/ieee754/flt-32/e_sqrtf.c>

/* Work around forgotten symbol in alphaev6 build.  */
#if SHLIB_COMPAT (libm, GLIBC_2_15, GLIBC_2_18)
# undef __sqrtf_finite
compat_symbol (libm, __sqrtf_finite1, __sqrtf_finite, GLIBC_2_15);
versioned_symbol (libm, __ieee754_sqrtf, __sqrtf_finite, GLIBC_2_18);
#endif
