#include <complex.h>
#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/conjl.c>
long_double_symbol (libm, __conjl, conjl);
