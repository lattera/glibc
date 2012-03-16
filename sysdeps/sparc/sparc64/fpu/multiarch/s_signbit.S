#include <sparc-ifunc.h>

SPARC_ASM_VIS3_IFUNC(signbit)

weak_alias (__signbit, signbit)

/* On 64-bit the double version will also always work for
   long-double-precision since in both cases the word with the
   sign bit in it is passed always in register %f0.  */
strong_alias (__signbit, __signbitl)
hidden_def (__signbitl)
weak_alias (__signbitl, signbitl)

# undef weak_alias
# define weak_alias(a, b)
# undef strong_alias
# define strong_alias(a, b)
# undef hidden_def
# define hidden_def(a)

#define __signbit __signbit_generic

#include "../s_signbit.S"
