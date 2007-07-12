#include <math_ldbl_opt.h>
#include <math/s_ldexp.c>
#ifdef IS_IN_libm
# if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __ldexp, ldexpl, GLIBC_2_0);
# endif
#elif LONG_DOUBLE_COMPAT(libc, GLIBC_2_0)
compat_symbol (libc, __ldexp, ldexpl, GLIBC_2_0);
#endif
