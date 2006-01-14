#include <math_ldbl_opt.h>
#include <math/w_log2.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __log2, log2l, GLIBC_2_1);
#endif
