/*
 * wrapper exp2(x)
 */

#include <math.h>
#include <math_private.h>

double
__exp2 (double x)
{
  double z = __ieee754_exp2 (x);
  if (__builtin_expect (!isfinite (z) || z == 0, 0)
      && isfinite (x) && _LIB_VERSION != _IEEE_)
    /* exp2 overflow: 44, exp2 underflow: 45 */
    return __kernel_standard (x, x, 44 + !!signbit (x));

  return z;
}
weak_alias (__exp2, exp2)
#ifdef NO_LONG_DOUBLE
strong_alias (__exp2, __exp2l)
weak_alias (__exp2, exp2l)
#endif
