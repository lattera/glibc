#include <math_ldbl_opt.h>
#include <sysdeps/ieee754/dbl-64/s_log1p.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __log1p, log1pl, GLIBC_2_0);
#endif
