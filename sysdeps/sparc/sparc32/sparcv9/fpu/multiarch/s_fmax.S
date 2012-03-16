#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(fmax)

weak_alias (__fmax, fmax)

# undef weak_alias
# define weak_alias(a, b)

#define __fmax __fmax_generic

#include "../s_fmax.S"
