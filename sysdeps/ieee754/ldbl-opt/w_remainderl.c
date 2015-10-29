#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/w_remainderl.c>
long_double_symbol (libm, __remainderl, remainderl);
strong_alias (__remainderl, __dreml)
long_double_symbol (libm, __dreml, dreml);
