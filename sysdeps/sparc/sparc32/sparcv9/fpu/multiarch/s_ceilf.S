#include <sparc-ifunc.h>

SPARC_ASM_VIS3_VIS2_IFUNC(ceilf)

weak_alias (__ceilf, ceilf)

# undef weak_alias
# define weak_alias(a, b)

#define __ceilf __ceilf_generic

#include "../s_ceilf.S"
