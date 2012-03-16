#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(lrint)

weak_alias (__lrint, lrint)

strong_alias (__lrint, __llrint)
weak_alias (__llrint, llrint)

# undef weak_alias
# define weak_alias(a, b)
# undef strong_alias
# define strong_alias(a, b)

#define __lrint __lrint_generic

#include "../s_lrint.S"
