#include <sparc-ifunc.h>
#include <libm-alias-float.h>

SPARC_ASM_VIS3_IFUNC(fabsf)

libm_alias_float (__fabs, fabs)

# undef weak_alias
# define weak_alias(a, b)

#define __fabsf __fabsf_generic

#include "../../../fpu/s_fabsf.S"
