/* Adapted for use as nearbyint by Ulrich Drepper <drepper@cygnus.com>.  */
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

#if defined(LIBM_SCCS) && !defined(lint)
static char rcsid[] = "$NetBSD: s_rint.c,v 1.8 1995/05/10 20:48:04 jtc Exp $";
#endif

/*
 * rint(x)
 * Return x rounded to integral value according to the prevailing
 * rounding mode.
 * Method:
 *	Using floating addition.
 * Exception:
 *	Inexact flag raised if x not equal to rint(x).
 */

#include <fenv.h>
#include <math.h>
#include <math-barriers.h>
#include <math_private.h>
#include <libm-alias-double.h>

static const double
  TWO52[2] = {
  4.50359962737049600000e+15, /* 0x43300000, 0x00000000 */
 -4.50359962737049600000e+15, /* 0xC3300000, 0x00000000 */
};

double
__nearbyint (double x)
{
  fenv_t env;
  int32_t i0, j0, sx;
  double w, t;
  GET_HIGH_WORD (i0, x);
  sx = (i0 >> 31) & 1;
  j0 = ((i0 >> 20) & 0x7ff) - 0x3ff;
  if (j0 < 52)
    {
      if (j0 < 0)
	{
	  libc_feholdexcept (&env);
	  w = TWO52[sx] + math_opt_barrier (x);
	  t = w - TWO52[sx];
	  math_force_eval (t);
	  libc_fesetenv (&env);
	  GET_HIGH_WORD (i0, t);
	  SET_HIGH_WORD (t, (i0 & 0x7fffffff) | (sx << 31));
	  return t;
	}
    }
  else
    {
      if (j0 == 0x400)
	return x + x;                   /* inf or NaN */
      else
	return x;                       /* x is integral */
    }
  libc_feholdexcept (&env);
  w = TWO52[sx] + math_opt_barrier (x);
  t = w - TWO52[sx];
  math_force_eval (t);
  libc_fesetenv (&env);
  return t;
}
libm_alias_double (__nearbyint, nearbyint)
