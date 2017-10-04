/* Always use dbl-64 version because long double is emulated in software.  */
#include <math_ldbl_opt.h>
#include <sysdeps/ieee754/dbl-64/s_fma.c>
#if LONG_DOUBLE_COMPAT (libm, GLIBC_2_1)
compat_symbol (libm, __fma, fmal, GLIBC_2_1);
#endif
