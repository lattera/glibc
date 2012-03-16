#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(sqrtf)

weak_alias (__sqrtf, sqrtf)

# undef weak_alias
# define weak_alias(a, b)

#define __sqrtf __sqrtf_generic

#include "../w_sqrtf.S"
