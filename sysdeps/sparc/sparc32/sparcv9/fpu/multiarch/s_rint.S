#include <sparc-ifunc.h>
#include <math_ldbl_opt.h>

SPARC_ASM_VIS3_IFUNC(rint)

weak_alias (__rint, rint)

#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __rint, rintl, GLIBC_2_0)
#endif

# undef weak_alias
# define weak_alias(a, b)
# undef compat_symbol
# define compat_symbol(a, b, c, d)

#define __rint __rint_generic

#include "../s_rint.S"
