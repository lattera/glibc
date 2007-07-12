/* dbl-64/s_sin.c uses NAN and sincos identifiers internally.  */
#define sincos sincos_disable
#include <math_ldbl_opt.h>
#undef NAN
#undef sincos
#include <sysdeps/ieee754/dbl-64/s_sin.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __sin, sinl, GLIBC_2_0);
compat_symbol (libm, __cos, cosl, GLIBC_2_0);
#endif
