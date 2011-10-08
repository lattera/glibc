#include <math.h>
#include <math_private.h>

long double
__ieee754_exp2l (long double x)
{
  /* This is a very stupid and inprecise implementation.  It'll get
     replaced sometime (soon?).  */
  return __ieee754_expl (M_LN2l * x);
}
