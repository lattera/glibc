#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(rintf)

weak_alias (__rintf, rintf)

# undef weak_alias
# define weak_alias(a, b)

#define __rintf __rintf_generic

#include "../s_rintf.S"
