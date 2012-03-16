#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(copysignf)

weak_alias (__copysignf, copysignf)

# undef weak_alias
# define weak_alias(a, b)

#define __copysignf __copysignf_generic

#include "../../../fpu/s_copysignf.S"
