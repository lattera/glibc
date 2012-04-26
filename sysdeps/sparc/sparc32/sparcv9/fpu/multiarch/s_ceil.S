#include <sparc-ifunc.h>
#include <math_ldbl_opt.h>

SPARC_ASM_VIS3_IFUNC(ceil)

weak_alias (__ceil, ceil)

#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __ceil, ceill, GLIBC_2_0)
#endif

# undef weak_alias
# define weak_alias(a, b)
# undef compat_symbol
# define compat_symbol(a, b, c, d)

#define __ceil __ceil_generic

#include "../s_ceil.S"
