#include <sparc-ifunc.h>
#include <math_ldbl_opt.h>
#include <libm-alias-double.h>

SPARC_ASM_VIS3_IFUNC(llrint)

libm_alias_double (__llrint, llrint)

# undef weak_alias
# define weak_alias(a, b)
# undef strong_alias
# define strong_alias(a, b)
# undef compat_symbol
# define compat_symbol(a, b, c, d)

#define __llrint __llrint_generic

#include "../s_llrint.S"
