#include <math_ldbl_opt.h>
#include <math/w_remainder.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __remainder, remainderl, GLIBC_2_0);
strong_alias (__remainder, __drem)
compat_symbol (libm, __drem, dreml, GLIBC_2_0);
#endif
