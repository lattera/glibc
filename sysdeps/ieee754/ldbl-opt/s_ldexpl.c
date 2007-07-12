#include <math_ldbl_opt.h>
#undef weak_alias
#define weak_alias(n,a)
#include <math/s_ldexpl.c>
#ifdef IS_IN_libm
long_double_symbol (libm, __ldexpl, ldexpl);
#else
long_double_symbol (libc, __ldexpl, ldexpl);
#endif
