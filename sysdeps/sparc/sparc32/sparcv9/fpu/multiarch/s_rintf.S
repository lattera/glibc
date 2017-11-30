#include <sparc-ifunc.h>
#include <libm-alias-float.h>

SPARC_ASM_VIS3_IFUNC(rintf)

libm_alias_float (__rint, rint)

# undef weak_alias
# define weak_alias(a, b)

#define __rintf __rintf_generic

#include "../s_rintf.S"
