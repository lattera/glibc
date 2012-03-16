#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(llrint)

weak_alias (__llrint, llrint)

strong_alias (__llrint, __lllrint)
weak_alias (__lllrint, lllrint)

# undef weak_alias
# define weak_alias(a, b)
# undef strong_alias
# define strong_alias(a, b)

#define __llrint __llrint_generic

#include "../s_llrint.S"
