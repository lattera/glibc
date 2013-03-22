#include <math.h>
#include <math_private.h>
#include <math_ldbl_opt.h>

static const long double o_thres = 709.78271289338399678773454114191496482L;
static const long double u_thres = -744.44007192138126231410729844608163411L;

long double __expl(long double x)	/* wrapper exp  */
{
  long double z;
  z = __ieee754_expl(x);
  if (_LIB_VERSION == _IEEE_)
    return z;
  if (__finitel(x))
    {
      if (x >= o_thres)
	return __kernel_standard_l(x,x,206); /* exp overflow  */
      else if (x <= u_thres)
	return __kernel_standard_l(x,x,207); /* exp underflow  */
    }
  return z;
}
hidden_def (__expl)
long_double_symbol (libm, __expl, expl);
