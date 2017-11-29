#include <sparc-ifunc.h>
#include <math_ldbl_opt.h>
#include <libm-alias-double.h>

SPARC_ASM_VIS3_IFUNC(nearbyint)

libm_alias_double (__nearbyint, nearbyint)

# undef weak_alias
# define weak_alias(a, b)
# undef compat_symbol
# define compat_symbol(a, b, c, d)

#define __nearbyint __nearbyint_generic

#include "../s_nearbyint.S"
