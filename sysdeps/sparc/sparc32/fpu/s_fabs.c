#include <math.h>
#include <math_ldbl_opt.h>

double __fabs (double x)
{
  return __builtin_fabs (x);
}
weak_alias (__fabs, fabs)
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_0)
compat_symbol (libm, __fabs, fabsl, GLIBC_2_0);
#endif
