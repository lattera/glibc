#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(signbitf)

weak_alias (__signbitf, signbitf)

# undef weak_alias
# define weak_alias(a, b)

#define __signbitf __signbitf_generic

#include "../s_signbitf.S"
