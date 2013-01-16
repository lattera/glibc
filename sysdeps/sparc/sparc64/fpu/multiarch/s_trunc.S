#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(trunc)

weak_alias (__trunc, trunc)

# undef weak_alias
# define weak_alias(a, b)

#define __trunc __trunc_generic

#include "../s_trunc.S"
