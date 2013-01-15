#include <sparc-ifunc.h>

SPARC_ASM_VIS3_VIS2_IFUNC(ceil)

weak_alias (__ceil, ceil)

# undef weak_alias
# define weak_alias(a, b)

#define __ceil __ceil_generic

#include "../s_ceil.S"
