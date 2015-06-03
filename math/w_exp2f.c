/*
 * wrapper exp2f(x)
 */

#include <math.h>
#include <math_private.h>

float
__exp2f (float x)
{
  float z = __ieee754_exp2f (x);
  if (__builtin_expect (!isfinite (z) || z == 0, 0)
      && isfinite (x) && _LIB_VERSION != _IEEE_)
    /* exp2 overflow: 144, exp2 underflow: 145 */
    return __kernel_standard_f (x, x, 144 + !!signbit (x));

  return z;
}
weak_alias (__exp2f, exp2f)
