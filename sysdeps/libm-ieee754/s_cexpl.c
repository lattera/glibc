/* Return value of complex exponential function for long double complex value.
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


__complex__ long double
__cexpl (__complex__ long double x)
{
  __complex__ long double retval;

  if (isfinite (__real__ x))
    {
      if (isfinite (__imag__ x))
	{
	  retval = __expl (__real__ x) * (__cosl (__imag__ x)
					  + 1i * __sinl (__imag__ x));
	}
      else
	/* If the imaginary part is +-inf or NaN and the real part is
	   not +-inf the result is NaN + iNan.  */
	retval = __nanl ("") + 1.0i * __nanl ("");
    }
  else if (__isinfl (__real__ x))
    {
      if (isfinite (__imag x))
	{
	  if (signbit (__real__ x) == 0 && __imag__ x == 0.0)
	    retval = HUGE_VALL;
	  else
	    retval = ((signbit (__real__ x) ? 0.0 : HUGE_VALL)
		      * (__cosl (__imag__ x) + 1i * __sinl (__imag__ x)));
	}
      else if (signbit (__real__ x))
	retval = HUGE_VALL + 1.0i * __nanl ("");
      else
	retval = 0.0;
    }
  else
    /* If the real part is NaN the result is NaN + iNan.  */
    retval = __nanl ("") + 1.0i * __nanl ("");

  return retval;
}
weak_alias (__cexpl, cexpl)
