#include <sparc-ifunc.h>
#include <math_ldbl_opt.h>
#include <libm-alias-double.h>

SPARC_ASM_VIS3_IFUNC(fabs)

libm_alias_double (__fabs, fabs)

# undef weak_alias
# define weak_alias(a, b)
# undef compat_symbol
# define compat_symbol(a, b, c, d)

#define __fabs __fabs_generic

#include "../s_fabs.S"
