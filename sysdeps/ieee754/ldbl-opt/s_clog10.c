#include <complex.h>
#include <math_ldbl_opt.h>
#include <math/s_clog10.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __clog10, __clog10l, GLIBC_2_1);
compat_symbol (libm, clog10, clog10l, GLIBC_2_1);
#endif
