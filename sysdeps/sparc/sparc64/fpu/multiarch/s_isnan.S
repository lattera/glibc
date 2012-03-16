#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(isnan)

hidden_def (__isnan)
weak_alias (__isnan, isnan)

# undef weak_alias
# define weak_alias(a, b)
# undef hidden_def
# define hidden_def(a)

#define __isnan __isnan_generic

#include "../s_isnan.S"
