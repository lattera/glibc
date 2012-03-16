#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(floor)

weak_alias (__floor, floor)

# undef weak_alias
# define weak_alias(a, b)

#define __floor __floor_generic

#include "../s_floor.S"
