#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(nearbyint)

weak_alias (__nearbyint, nearbyint)

# undef weak_alias
# define weak_alias(a, b)

#define __nearbyint __nearbyint_generic

#include "../s_nearbyint.S"
