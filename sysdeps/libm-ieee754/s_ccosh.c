/* Complex cosine hyperbole function for double.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <complex.h>
#include <math.h>


__complex__ double
__ccosh (__complex__ double x)
{
  __complex__ double retval;

  __real__ x = fabs (__real__ x);

  if (isfinite (__real__ x))
    {
      if (isfinite (__imag__ x))
	{
	  double exp_val = __exp (__real__ x);
	  double rec_exp_val = 1.0 / exp_val;

	  __real__ retval = 0.5 * (exp_val + rec_exp_val) * __cos (__imag__ x);
	  __imag__ retval = 0.5 * (exp_val + rec_exp_val) * __sin (__imag__ x);
	}
      else
	{
	  if (__real__ x == 0)
	    {
	      __imag__ retval = 0.0;
	      __real__ retval = __nan ("") + __nan ("");
	    }
	  else
	    {
	      __real__ retval = __nan ("");
	      __imag__ retval = __nan ("") + __nan ("");
	    }
	}
    }
  else if (__isinf (__real__ x))
    {
      if (__imag__ x == 0.0)
	{
	  __real__ retval = HUGE_VAL;
	  __imag__ retval = __imag__ x;
	}
      else if (isfinite (__imag__ x))
	{
	  __real__ retval = __copysign (HUGE_VAL, __cos (__imag__ x));
	  __imag__ retval = __copysign (HUGE_VAL, __sin (__imag__ x));
	}
      else
	{
	  /* The addition raises the invalid exception.  */
	  __real__ retval = HUGE_VAL;
	  __imag__ retval = __nan ("") + __nan ("");
	}
    }
  else
    {
      if (__imag__ x == 0.0)
	{
	  __real__ retval = __nan ("");
	  __imag__ retval = __imag__ x;
	}
      else
	{
	  __real__ retval = __nan ("");
	  __imag__ retval = __nan ("");
	}
    }

  return retval;
}
weak_alias (__ccosh, ccosh)
#ifdef NO_LONG_DOUBLE
strong_alias (__ccosh, __ccoshl)
weak_alias (__ccosh, ccoshl)
#endif
