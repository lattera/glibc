#include <math_ldbl_opt.h>
#include <math/w_exp10.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __exp10, exp10l, GLIBC_2_1);
compat_symbol (libm, __pow10, pow10l, GLIBC_2_1);
#endif
