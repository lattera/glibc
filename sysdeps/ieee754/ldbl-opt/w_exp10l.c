#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/w_exp10l.c>
long_double_symbol (libm, __exp10l, exp10l);
long_double_symbol (libm, __pow10l, pow10l);
