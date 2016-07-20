/* Looks like we can use ieee854 s_cbrtl.c as is for IBM extended format. */
#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)

#define _Float128 long double
#define L(x) x ## L

#include <sysdeps/ieee754/ldbl-128/s_cbrtl.c>
long_double_symbol (libm, __cbrtl, cbrtl);
