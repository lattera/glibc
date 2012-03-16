#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(rint)

weak_alias (__rint, rint)

# undef weak_alias
# define weak_alias(a, b)

#define __rint __rint_generic

#include "../s_rint.S"
