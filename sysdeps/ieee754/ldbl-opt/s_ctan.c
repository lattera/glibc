#include <complex.h>
#include <math_ldbl_opt.h>
#include <math/s_ctan.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __ctan, ctanl, GLIBC_2_1);
#endif
