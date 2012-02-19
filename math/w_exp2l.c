/*
 * wrapper exp2l(x)
 */

#include <float.h>
#include <math.h>
#include <math_private.h>

static const long double o_threshold = (long double) LDBL_MAX_EXP;
static const long double u_threshold
  = (long double) (LDBL_MIN_EXP - LDBL_MANT_DIG - 1);

long double
__exp2l (long double x)
{
  if (__builtin_expect (islessequal (x, u_threshold)
			|| isgreater (x, o_threshold), 0)
      && _LIB_VERSION != _IEEE_ && __finitel (x))
    /* exp2 overflow: 244, exp2 underflow: 245 */
    return __kernel_standard (x, x, 244 + (x <= o_threshold));

  return __ieee754_exp2l (x);
}
weak_alias (__exp2l, exp2l)
