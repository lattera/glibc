/* Ceil (round to +inf) long double floating-point values.
   IBM extended format long double version.
   Copyright (C) 2006 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <math.h>
#include <fenv.h>
#include <math_ldbl_opt.h>
#include <float.h>
#include <ieee754.h>


#ifdef __STDC__
long double
__ceill (long double x)
#else
long double
__ceill (x)
     long double x;
#endif
{
  static const double TWO52 = 4503599627370496.0L;
  int mode = fegetround();
  union ibm_extended_long_double u;

  u.d = x;

  if (fabs (u.dd[0]) < TWO52)
    {
      double high = u.dd[0];
      fesetround(FE_UPWARD);
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
      fesetround(mode);
    }
  else if (fabs (u.dd[1]) < TWO52 && u.dd[1] != 0.0)
    {
      double high, low;
      /* In this case we have to round the low double and handle any
         adjustment to the high double that may be caused by rounding
         (up).  This is complicated by the fact that the high double
         may already be rounded and the low double may have the
         opposite sign to compensate.  */
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
	      high = nextafter (u.dd[0], 0.0);
	      low = u.dd[1] + (u.dd[0] - high);
	    }
          fesetround(FE_UPWARD);
	  low += TWO52;
	  low -= TWO52;
          fesetround(mode);
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
	      high = nextafter (u.dd[0], 0.0);
	      low = u.dd[1] + (u.dd[0] - high);
	    }
          fesetround(FE_UPWARD);
	  low -= TWO52;
	  low += TWO52;
          fesetround(mode);
	}
      u.dd[0] = high + low;
      u.dd[1] = high - u.dd[0] + low;
    }

  return u.d;
}

long_double_symbol (libm, __ceill, ceill);
