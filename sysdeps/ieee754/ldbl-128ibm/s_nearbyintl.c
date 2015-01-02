/* Round to int long double floating-point values without raising inexact.
   IBM extended format long double version.
   Copyright (C) 2006-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* This has been coded in assembler because GCC makes such a mess of it
   when it's coded in C.  */

#include <math.h>
#include <math_private.h>
#include <fenv.h>
#include <math_ldbl_opt.h>
#include <float.h>
#include <ieee754.h>


long double
__nearbyintl (long double x)
{
  fenv_t env;
  static const long double TWO52 = 4503599627370496.0L;
  union ibm_extended_long_double u;
  u.ld = x;

  if (fabs (u.d[0].d) < TWO52)
    {
      double xh = u.d[0].d;
      double high = u.d[0].d;
      feholdexcept (&env);
      if (high > 0.0)
	{
	  high += TWO52;
	  high -= TWO52;
          if (high == -0.0) high = 0.0;
	}
      else if (high < 0.0)
	{
	  high -= TWO52;
	  high += TWO52;
          if (high == 0.0) high = -0.0;
	}
      if (u.d[1].d > 0.0 && (xh - high == 0.5))
        high += 1.0;
      else if (u.d[1].d < 0.0 && (-(xh - high) == 0.5))
        high -= 1.0;
      u.d[0].d = high;
      u.d[1].d = 0.0;
      math_force_eval (u.d[0]);
      math_force_eval (u.d[1]);
      fesetenv (&env);
    }
  else if (fabs (u.d[1].d) < TWO52 && u.d[1].d != 0.0)
    {
      double high, low, tau;
      /* In this case we have to round the low double and handle any
         adjustment to the high double that may be caused by rounding
         (up).  This is complicated by the fact that the high double
         may already be rounded and the low double may have the
         opposite sign to compensate.  */
      feholdexcept (&env);
      if (u.d[0].d > 0.0)
	{
	  if (u.d[1].d > 0.0)
	    {
	      /* If the high/low doubles are the same sign then simply
	         round the low double.  */
	      high = u.d[0].d;
	      low = u.d[1].d;
	    }
	  else if (u.d[1].d < 0.0)
	    {
	      /* Else the high double is pre rounded and we need to
	         adjust for that.  */

	      tau = __nextafter (u.d[0].d, 0.0);
	      tau = (u.d[0].d - tau) * 2.0;
	      high = u.d[0].d - tau;
	      low = u.d[1].d + tau;
	    }
	  low += TWO52;
	  low -= TWO52;
	}
      else if (u.d[0].d < 0.0)
	{
	  if (u.d[1].d < 0.0)
	    {
	      /* If the high/low doubles are the same sign then simply
	         round the low double.  */
	      high = u.d[0].d;
	      low = u.d[1].d;
	    }
	  else if (u.d[1].d > 0.0)
	    {
	      /* Else the high double is pre rounded and we need to
	         adjust for that.  */
	      tau = __nextafter (u.d[0].d, 0.0);
	      tau = (u.d[0].d - tau) * 2.0;
	      high = u.d[0].d - tau;
	      low = u.d[1].d + tau;
	    }
	  low = TWO52 - low;
	  low = -(low - TWO52);
	}
      u.d[0].d = high + low;
      u.d[1].d = high - u.d[0].d + low;
      math_force_eval (u.d[0]);
      math_force_eval (u.d[1]);
      fesetenv (&env);
    }

  return u.ld;
}

long_double_symbol (libm, __nearbyintl, nearbyintl);
