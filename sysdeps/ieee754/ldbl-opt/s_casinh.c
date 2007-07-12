#include <complex.h>
#include <math_ldbl_opt.h>
#include <math/s_casinh.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __casinh, casinhl, GLIBC_2_1);
#endif
