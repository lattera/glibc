#include <sparc-ifunc.h>

SPARC_ASM_VIS3_VIS2_IFUNC(floorf)

weak_alias (__floorf, floorf)

# undef weak_alias
# define weak_alias(a, b)

#define __floorf __floorf_generic

#include "../s_floorf.S"
