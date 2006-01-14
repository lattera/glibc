#include <math_ldbl_opt.h>
#include <math/w_hypot.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __hypot, hypotl, GLIBC_2_0);
#endif
