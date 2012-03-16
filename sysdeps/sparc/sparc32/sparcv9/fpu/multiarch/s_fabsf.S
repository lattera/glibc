#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(fabsf)

weak_alias (__fabsf, fabsf)

# undef weak_alias
# define weak_alias(a, b)

#define __fabsf __fabsf_generic

#include "../../../fpu/s_fabsf.S"
