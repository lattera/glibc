/*
 * wrapper exp2f(x)
 */

#include <float.h>
#include <math.h>
#include <math_private.h>

static const float o_threshold = (float) FLT_MAX_EXP;
static const float u_threshold = (float) (FLT_MIN_EXP - FLT_MANT_DIG - 1);

float
__exp2f (float x)
{
  if (__builtin_expect (islessequal (x, u_threshold)
			|| isgreater (x, o_threshold), 0)
      && _LIB_VERSION != _IEEE_ && __finitef (x))
    /* exp2 overflow: 144, exp2 underflow: 145 */
    return __kernel_standard_f (x, x, 144 + (x <= o_threshold));

  return __ieee754_exp2f (x);
}
weak_alias (__exp2f, exp2f)
