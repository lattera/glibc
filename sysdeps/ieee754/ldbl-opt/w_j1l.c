#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/w_j1l.c>
long_double_symbol (libm, __j1l, j1l);
long_double_symbol (libm, __y1l, y1l);
