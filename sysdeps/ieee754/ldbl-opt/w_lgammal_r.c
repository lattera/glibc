#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/w_lgammal_r.c>
long_double_symbol (libm, __lgammal_r, lgammal_r);
