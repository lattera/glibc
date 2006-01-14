#include <complex.h>
#include <math_ldbl_opt.h>
#include <math/s_cpow.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __cpow, cpowl, GLIBC_2_1);
#endif
