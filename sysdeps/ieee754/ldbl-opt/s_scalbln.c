#include <math_ldbl_opt.h>
#include <sysdeps/ieee754/dbl-64/s_scalbln.c>
#if IS_IN (libm)
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __scalbln, scalblnl, GLIBC_2_1);
#endif
#elif LONG_DOUBLE_COMPAT(libc, GLIBC_2_1)
compat_symbol (libc, __scalbln, scalblnl, GLIBC_2_1);
#endif
