/*
 * ====================================================
 * Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved.
 *
 * Developed at SunPro, a Sun Microsystems, Inc. business.
 * Permission to use, copy, modify, and distribute this
 * software is freely granted, provided that this notice
 * is preserved.
 * ====================================================
 */


#include <math.h>
#include <math_private.h>
#include <math_ldbl_opt.h>

/*
 * wrapper expl(x)
 */
static const long double
o_threshold = 709.78271289338399678773454114191496482L,
u_threshold = -744.44007192138126231410729844608163411L;

long double __expl(long double x)	/* wrapper exp */
{
#ifdef _IEEE_LIBM
  return __ieee754_expl(x);
#else
  long double z;
  z = __ieee754_expl(x);
  if (_LIB_VERSION == _IEEE_)
    return z;
  if (__finitel(x))
    {
      if (x >= o_threshold)
	return __kernel_standard_l(x,x,206); /* exp overflow */
      else if (x <= u_threshold)
	return __kernel_standard_l(x,x,207); /* exp underflow */
    }
  return z;
#endif
}
hidden_def (__expl)
long_double_symbol (libm, __expl, expl);
