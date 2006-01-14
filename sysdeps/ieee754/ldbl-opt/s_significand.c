#include <math_ldbl_opt.h>
#include <math/s_significand.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __significand, significandl, GLIBC_2_0);
#endif
