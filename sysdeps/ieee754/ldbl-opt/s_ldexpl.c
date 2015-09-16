#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/s_ldexpl.c>
strong_alias (__ldexpl, __ldexpl_2)
#if IS_IN (libm)
long_double_symbol (libm, __ldexpl, ldexpl);
long_double_symbol (libm, __ldexpl_2, scalbnl);
#else
long_double_symbol (libc, __ldexpl, ldexpl);
long_double_symbol (libc, __ldexpl_2, scalbnl);
#endif
