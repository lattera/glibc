#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(isnanf)

hidden_def (__isnanf)
weak_alias (__isnanf, isnanf)

# undef weak_alias
# define weak_alias(a, b)
# undef hidden_def
# define hidden_def(a)

#define __isnanf __isnanf_generic

#include "../s_isnanf.S"
