#include <math_ldbl_opt.h>
#include <math/w_drem.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __drem, dreml, GLIBC_2_0);
#endif
