#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(nearbyintf)

weak_alias (__nearbyintf, nearbyintf)

# undef weak_alias
# define weak_alias(a, b)

#define __nearbyintf __nearbyintf_generic

#include "../s_nearbyintf.S"
