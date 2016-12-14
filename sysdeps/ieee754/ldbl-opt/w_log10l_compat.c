#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/w_log10l_compat.c>
long_double_symbol (libm, __log10l, log10l);
