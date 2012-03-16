#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(sqrt)

weak_alias (__sqrt, sqrt)

# undef weak_alias
# define weak_alias(a, b)

#define __sqrt __sqrt_generic

#include "../w_sqrt.S"
