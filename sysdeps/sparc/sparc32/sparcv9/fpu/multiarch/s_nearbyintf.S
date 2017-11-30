#include <sparc-ifunc.h>
#include <libm-alias-float.h>

SPARC_ASM_VIS3_IFUNC(nearbyintf)

libm_alias_float (__nearbyint, nearbyint)

# undef weak_alias
# define weak_alias(a, b)

#define __nearbyintf __nearbyintf_generic

#include "../s_nearbyintf.S"
