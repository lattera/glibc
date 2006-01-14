#include <math_ldbl_opt.h>
#include <math/w_atanh.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __atanh, atanhl, GLIBC_2_0);
#endif
