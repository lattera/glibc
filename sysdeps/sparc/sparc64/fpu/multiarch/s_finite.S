#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(finite)

hidden_def (__finite)
weak_alias (__finite, finite)

# undef weak_alias
# define weak_alias(a, b)
# undef hidden_def
# define hidden_def(a)

#define __finite __finite_generic

#include "../s_finite.S"
