#include <sparc-ifunc.h>
#include <math_ldbl_opt.h>

SPARC_ASM_VIS3_IFUNC(trunc)

weak_alias (__trunc, trunc)

#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __trunc, truncl, GLIBC_2_1)
#endif

# undef weak_alias
# define weak_alias(a, b)
# undef compat_symbol
# define compat_symbol(a, b, c, d)

#define __trunc __trunc_generic

#include "../s_trunc.S"
