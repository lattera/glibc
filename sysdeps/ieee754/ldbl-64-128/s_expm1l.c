#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <sysdeps/ieee754/ldbl-128/s_expm1l.c>
long_double_symbol (libm, __expm1l, expm1l);
