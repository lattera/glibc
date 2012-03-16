#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(copysign)

weak_alias (__copysign, copysign)

# undef weak_alias
# define weak_alias(a, b)

#define __copysign __copysign_generic

#include "../../../fpu/s_copysign.S"
