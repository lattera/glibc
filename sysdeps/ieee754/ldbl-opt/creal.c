#include <complex.h>
#include <math_ldbl_opt.h>
#include <math/creal.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __creal, creall, GLIBC_2_1);
#endif
