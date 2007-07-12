#include <math_ldbl_opt.h>
#include <sysdeps/powerpc/fpu/s_fmin.S>
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __fmin, fminl, GLIBC_2_1)
#endif
