#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(fmin)

weak_alias (__fmin, fmin)

# undef weak_alias
# define weak_alias(a, b)

#define __fmin __fmin_generic

#include "../s_fmin.S"
