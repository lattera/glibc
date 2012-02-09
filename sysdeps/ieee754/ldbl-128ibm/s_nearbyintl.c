/* Round to int long double floating-point values without raising inexact.
   IBM extended format long double version.
   Copyright (C) 2006, 2008, 2012 Free Software Foundation, Inc.
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
  u.d = x;

  if (fabs (u.dd[0]) < TWO52)
    {
      double high = u.dd[0];
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
      u.dd[0] = high;
      u.dd[1] = 0.0;
      fesetenv (&env);
    }
  else if (fabs (u.dd[1]) < TWO52 && u.dd[1] != 0.0)
    {
      double high, low, tau;
      /* In this case we have to round the low double and handle any
         adjustment to the high double that may be caused by rounding
         (up).  This is complicated by the fact that the high double
         may already be rounded and the low double may have the
         opposite sign to compensate.  */
      feholdexcept (&env);
      if (u.dd[0] > 0.0)
	{
	  if (u.dd[1] > 0.0)
	    {
	      /* If the high/low doubles are the same sign then simply
	         round the low double.  */
	      high = u.dd[0];
	      low = u.dd[1];
	    }
	  else if (u.dd[1] < 0.0)
	    {
	      /* Else the high double is pre rounded and we need to
	         adjust for that.  */

	      tau = __nextafter (u.dd[0], 0.0);
	      tau = (u.dd[0] - tau) * 2.0;
	      high = u.dd[0] - tau;
	      low = u.dd[1] + tau;
	    }
	  low += TWO52;
	  low -= TWO52;
	}
      else if (u.dd[0] < 0.0)
	{
	  if (u.dd[1] < 0.0)
	    {
	      /* If the high/low doubles are the same sign then simply
	         round the low double.  */
	      high = u.dd[0];
	      low = u.dd[1];
	    }
	  else if (u.dd[1] > 0.0)
	    {
	      /* Else the high double is pre rounded and we need to
	         adjust for that.  */
	      tau = __nextafter (u.dd[0], 0.0);
	      tau = (u.dd[0] - tau) * 2.0;
	      high = u.dd[0] - tau;
	      low = u.dd[1] + tau;
	    }
	  low = TWO52 - low;
	  low = -(low - TWO52);
	}
      u.dd[0] = high + low;
      u.dd[1] = high - u.dd[0] + low;
      fesetenv (&env);
    }

  return u.d;
}

long_double_symbol (libm, __nearbyintl, nearbyintl);
