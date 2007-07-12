#include <math_ldbl_opt.h>
#include <sysdeps/ieee754/dbl-64/s_ilogb.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __ilogb, ilogbl, GLIBC_2_0);
#endif
