#include <sparc-ifunc.h>
#include <libm-alias-float.h>

SPARC_ASM_VIS3_IFUNC(copysignf)

libm_alias_float (__copysign, copysign)

# undef weak_alias
# define weak_alias(a, b)

#define __copysignf __copysignf_generic

#include "../../../fpu/s_copysignf.S"
