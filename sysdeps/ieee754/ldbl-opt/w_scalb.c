#include <math_ldbl_opt.h>
#include <math/w_scalb.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __scalb, scalbl, GLIBC_2_0);
#endif
