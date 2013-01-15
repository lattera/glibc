#include <sparc-ifunc.h>
#include <math_ldbl_opt.h>

SPARC_ASM_VIS3_IFUNC(fdim)

weak_alias (__fdim, fdim)

#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __fdim, fdiml, GLIBC_2_1);
#endif

# undef weak_alias
# define weak_alias(a, b)
# undef compat_symbol
# define compat_symbol(a, b, c, d)

#define __fdim __fdim_generic

#include "../s_fdim.S"
