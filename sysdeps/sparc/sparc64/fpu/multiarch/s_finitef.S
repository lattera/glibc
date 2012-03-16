#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(finitef)

hidden_def (__finitef)
weak_alias (__finitef, finitef)

# undef weak_alias
# define weak_alias(a, b)
# undef hidden_def
# define hidden_def(a)

#define __finitef __finitef_generic

#include "../s_finitef.S"
