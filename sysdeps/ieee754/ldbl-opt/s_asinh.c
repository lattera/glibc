#include <math_ldbl_opt.h>
#include <sysdeps/ieee754/dbl-64/s_asinh.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __asinh, asinhl, GLIBC_2_0);
#endif
