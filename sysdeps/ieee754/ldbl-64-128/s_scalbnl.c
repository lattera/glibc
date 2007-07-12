#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <sysdeps/ieee754/ldbl-128/s_scalbnl.c>
#ifdef IS_IN_libm
long_double_symbol (libm, __scalbnl, scalbnl);
#else
long_double_symbol (libc, __scalbnl, scalbnl);
#endif
