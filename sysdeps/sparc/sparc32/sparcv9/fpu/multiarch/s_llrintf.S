#include <sparc-ifunc.h>
#include <libm-alias-float.h>

SPARC_ASM_VIS3_IFUNC(llrintf)

libm_alias_float (__llrint, llrint)

strong_alias (__llrintf, __lllrintf)
weak_alias (__lllrintf, lllrintf)

# undef weak_alias
# define weak_alias(a, b)
# undef strong_alias
# define strong_alias(a, b)

#define __llrintf __llrintf_generic

#include "../s_llrintf.S"
