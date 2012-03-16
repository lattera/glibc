#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(isinff)

hidden_def (__isinff)
weak_alias (__isinff, isinff)

# undef weak_alias
# define weak_alias(a, b)
# undef hidden_def
# define hidden_def(a)

#define __isinff __isinff_generic

#include "../s_isinff.S"
