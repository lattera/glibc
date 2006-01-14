#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/w_lgammal.c>
long_double_symbol (libm, __lgammal, lgammal);
long_double_symbol (libm, __gammal, gammal);
