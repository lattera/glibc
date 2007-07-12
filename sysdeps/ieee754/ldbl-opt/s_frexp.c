#include <math_ldbl_opt.h>
#include <sysdeps/ieee754/dbl-64/s_frexp.c>
#ifdef IS_IN_libm
# if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __frexp, frexpl, GLIBC_2_0);
# endif
#elif LONG_DOUBLE_COMPAT(libc, GLIBC_2_0)
compat_symbol (libc, __frexp, frexpl, GLIBC_2_0);
#endif
