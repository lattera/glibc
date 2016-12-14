/*
 * wrapper exp2l(x)
 */

#include <math.h>
#include <math_private.h>

long double
__exp2l (long double x)
{
  long double z = __ieee754_exp2l (x);
  if (__builtin_expect (!isfinite (z) || z == 0, 0)
      && isfinite (x) && _LIB_VERSION != _IEEE_)
    /* exp2 overflow: 244, exp2 underflow: 245 */
    return __kernel_standard_l (x, x, 244 + !!signbit (x));

  return z;
}
weak_alias (__exp2l, exp2l)
