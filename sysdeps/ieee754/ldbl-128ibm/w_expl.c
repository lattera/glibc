#include <math.h>
#include <math_private.h>
#include <math_ldbl_opt.h>

long double __expl(long double x)	/* wrapper exp  */
{
  long double z;
  z = __ieee754_expl(x);
  if (_LIB_VERSION == _IEEE_)
    return z;
  if (isfinite(x))
    {
      if (!isfinite (z))
	return __kernel_standard_l(x,x,206); /* exp overflow  */
      else if (z == 0.0L)
	return __kernel_standard_l(x,x,207); /* exp underflow  */
    }
  return z;
}
hidden_def (__expl)
long_double_symbol (libm, __expl, expl);
