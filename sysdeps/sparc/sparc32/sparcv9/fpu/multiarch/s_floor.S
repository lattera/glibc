#include <sparc-ifunc.h>
#include <math_ldbl_opt.h>

SPARC_ASM_VIS3_VIS2_IFUNC(floor)

weak_alias (__floor, floor)

#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __floor, floorl, GLIBC_2_0)
#endif

# undef weak_alias
# define weak_alias(a, b)
# undef compat_symbol
# define compat_symbol(a, b, c, d)

#define __floor __floor_generic

#include "../s_floor.S"
