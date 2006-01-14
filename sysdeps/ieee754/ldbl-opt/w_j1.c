#include <math_ldbl_opt.h>
#include <math/w_j1.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, j1, j1l, GLIBC_2_0);
compat_symbol (libm, y1, y1l, GLIBC_2_0);
#endif
