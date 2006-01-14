#include <math_ldbl_opt.h>
#include <math/w_jn.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, jn, jnl, GLIBC_2_0);
compat_symbol (libm, yn, ynl, GLIBC_2_0);
#endif
