#include <math_ldbl_opt.h>
#include <sysdeps/powerpc/fpu/s_fdim.c>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __fdim, fdiml, GLIBC_2_1);
#endif
