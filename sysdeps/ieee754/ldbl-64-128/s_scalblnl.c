#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <sysdeps/ieee754/ldbl-128/s_scalblnl.c>
#ifdef IS_IN_libm
long_double_symbol (libm, __scalblnl, scalblnl);
#else
long_double_symbol (libc, __scalblnl, scalblnl);
#endif
