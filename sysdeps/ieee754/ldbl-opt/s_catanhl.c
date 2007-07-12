#include <complex.h>
#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/s_catanhl.c>
long_double_symbol (libm, __catanhl, catanhl);
