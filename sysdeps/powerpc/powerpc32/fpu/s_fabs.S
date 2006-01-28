#include <math_ldbl_opt.h>
#include <sysdeps/powerpc/fpu/s_fabs.S>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __fabs, fabsl, GLIBC_2_0)
#endif
