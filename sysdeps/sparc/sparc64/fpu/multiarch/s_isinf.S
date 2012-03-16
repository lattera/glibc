#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(isinf)

hidden_def (__isinf)
weak_alias (__isinf, isinf)

# undef weak_alias
# define weak_alias(a, b)
# undef hidden_def
# define hidden_def(a)

#define __isinf __isinf_generic

#include "../s_isinf.S"
