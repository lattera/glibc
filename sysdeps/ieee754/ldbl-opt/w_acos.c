#include <math_ldbl_opt.h>
#include <math/w_acos.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __acos, acosl, GLIBC_2_0);
#endif
