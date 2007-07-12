#include <math_ldbl_opt.h>
#include <math/s_fmax.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __fmax, fmaxl, GLIBC_2_1);
#endif
