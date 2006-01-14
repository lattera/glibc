#include <math_ldbl_opt.h>
#include <math/w_atan2.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __atan2, atan2l, GLIBC_2_0);
#endif
