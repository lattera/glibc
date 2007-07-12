#include <complex.h>
#include <math_ldbl_opt.h>
#include <math/s_cexp.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __cexp, cexpl, GLIBC_2_1);
#endif
