#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(fmaxf)

weak_alias (__fmaxf, fmaxf)

# undef weak_alias
# define weak_alias(a, b)

#define __fmaxf __fmaxf_generic

#include "../s_fmaxf.S"
