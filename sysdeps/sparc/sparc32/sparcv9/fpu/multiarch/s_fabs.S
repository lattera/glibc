#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(fabs)

weak_alias (__fabs, fabs)

# undef weak_alias
# define weak_alias(a, b)

#define __fabs __fabs_generic

#include "../s_fabs.S"
