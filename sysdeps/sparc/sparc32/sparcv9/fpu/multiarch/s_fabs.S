#include <sparc-ifunc.h>
#include <math_ldbl_opt.h>

SPARC_ASM_VIS3_IFUNC(fabs)

weak_alias (__fabs, fabs)
#if LONG_DOUBLE_COMPAT (libm, GLIBC_2_0)
compat_symbol (libm, __fabs, fabsl, GLIBC_2_0);
#endif

# undef weak_alias
# define weak_alias(a, b)
# undef compat_symbol
# define compat_symbol(a, b, c, d)

#define __fabs __fabs_generic

#include "../s_fabs.S"
