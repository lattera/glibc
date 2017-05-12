#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(signbitf)

# undef weak_alias
# define weak_alias(a, b)

#define __signbitf __signbitf_generic

#include "../s_signbitf.S"
