#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(fminf)

weak_alias (__fminf, fminf)

# undef weak_alias
# define weak_alias(a, b)

#define __fminf __fminf_generic

#include "../s_fminf.S"
