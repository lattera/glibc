#include <sparc-ifunc.h>
#include <math_ldbl_opt.h>

SPARC_ASM_VIS3_IFUNC(copysign)

weak_alias (__copysign, copysign)
#if LONG_DOUBLE_COMPAT (libm, GLIBC_2_0)
compat_symbol (libm, __copysign, copysignl, GLIBC_2_0);
#endif
#if LONG_DOUBLE_COMPAT (libc, GLIBC_2_0)
compat_symbol (libc, __copysign, copysignl, GLIBC_2_0);
#endif

# undef weak_alias
# define weak_alias(a, b)
# undef compat_symbol
# define compat_symbol(a, b, c, d)

#define __copysign __copysign_generic

#include "../../../fpu/s_copysign.S"
