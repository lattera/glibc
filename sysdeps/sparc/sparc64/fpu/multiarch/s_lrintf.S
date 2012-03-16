#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(lrintf)

weak_alias (__lrintf, lrintf)

strong_alias (__lrintf, __llrintf)
weak_alias (__llrintf, llrintf)

# undef weak_alias
# define weak_alias(a, b)
# undef strong_alias
# define strong_alias(a, b)

#define __lrintf __lrintf_generic

#include "../s_lrintf.S"
