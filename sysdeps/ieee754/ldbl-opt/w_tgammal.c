#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/w_tgammal.c>
long_double_symbol (libm, __tgammal, tgammal);
