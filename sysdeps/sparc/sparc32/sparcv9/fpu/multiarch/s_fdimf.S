#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(fdimf)

weak_alias (__fdimf, fdimf)

# undef weak_alias
# define weak_alias(a, b)

#define __fdimf __fdimf_generic

#include "../s_fdimf.S"
