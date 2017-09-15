/*
 * wrapper exp2(x)
 */

#include <math.h>
#include <math_private.h>
#include <math-svid-compat.h>
#include <libm-alias-double.h>

#if LIBM_SVID_COMPAT
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
libm_alias_double (__exp2, exp2)
#endif
