/*
 * wrapper exp2(x)
 */

#include <float.h>
#include <math.h>
#include <math_private.h>

static const double o_threshold = (double) DBL_MAX_EXP;
static const double u_threshold = (double) (DBL_MIN_EXP - DBL_MANT_DIG - 1);

double
__exp2 (double x)
{
  if (__builtin_expect (islessequal (x, u_threshold)
			|| isgreater (x, o_threshold), 0)
      && _LIB_VERSION != _IEEE_ && __finite (x))
    /* exp2 overflow: 44, exp2 underflow: 45 */
    return __kernel_standard (x, x, 44 + (x <= o_threshold));

  return __ieee754_exp2 (x);
}
weak_alias (__exp2, exp2)
#ifdef NO_LONG_DOUBLE
strong_alias (__exp2, __exp2l)
weak_alias (__exp2, exp2l)
#endif
