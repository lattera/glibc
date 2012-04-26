#include <sparc-ifunc.h>
#include <math_ldbl_opt.h>

SPARC_ASM_VIS3_IFUNC(llrint)

weak_alias (__llrint, llrint)

strong_alias (__llrint, __lllrint)
weak_alias (__lllrint, lllrint)

#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __llrint, llrintl, GLIBC_2_1)
#endif

# undef weak_alias
# define weak_alias(a, b)
# undef strong_alias
# define strong_alias(a, b)
# undef compat_symbol
# define compat_symbol(a, b, c, d)

#define __llrint __llrint_generic

#include "../s_llrint.S"
