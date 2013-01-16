#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(truncf)

weak_alias (__truncf, truncf)

# undef weak_alias
# define weak_alias(a, b)

#define __truncf __truncf_generic

#include "../s_truncf.S"
