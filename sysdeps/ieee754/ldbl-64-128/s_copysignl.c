#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <sysdeps/ieee754/ldbl-128/s_copysignl.c>
#if IS_IN (libm)
long_double_symbol (libm, __copysignl, copysignl);
#else
long_double_symbol (libc, __copysignl, copysignl);
#endif
