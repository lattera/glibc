/* Complex sine hyperbole function for double.
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

#include "math_private.h"


__complex__ double
__csinh (__complex__ double x)
{
  __complex__ double retval;
  int negate = signbit (__real__ x);
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  __real__ x = fabs (__real__ x);

  if (rcls >= FP_ZERO)
    {
      /* Real part is finite.  */
      if (icls >= FP_ZERO)
	{
	  /* Imaginary part is finite.  */
	   double sinh_val = __ieee754_sinh (__real__ x);

	  __real__ retval = sinh_val * __cos (__imag__ x);
	  __imag__ retval = sinh_val * __sin (__imag__ x);

	  if (negate)
	    __real__ retval = -__real__ retval;
	}
      else
	{
	  if (rcls == FP_ZERO)
	    {
	      /* Real part is 0.0.  */
	      __real__ retval = __copysign (0.0, negate ? -1.0 : 1.0);
	      __imag__ retval = __nan ("") + __nan ("");
	    }
	  else
	    {
	      __real__ retval = __nan ("");
	      __imag__ retval = __nan ("");
	    }
	}
    }
  else if (rcls == FP_INFINITE)
    {
      /* Real part is infinite.  */
      if (icls == FP_ZERO)
	{
	  /* Imaginary part is 0.0.  */
	  __real__ retval = negate ? -HUGE_VAL : HUGE_VAL;
	  __imag__ retval = __imag__ x;
	}
      else if (icls > FP_ZERO)
	{
	  /* Imaginary part is finite.  */
	  __real__ retval = __copysign (HUGE_VAL, __cos (__imag__ x));
	  __imag__ retval = __copysign (HUGE_VAL, __sin (__imag__ x));

	  if (negate)
	    __real__ retval = -__real__ retval;
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
      __real__ retval = __nan ("");
      __imag__ retval = __imag__ x == 0.0 ? __imag__ x : __nan ("");
    }

  return retval;
}
weak_alias (__csinh, csinh)
#ifdef NO_LONG_DOUBLE
strong_alias (__csinh, __csinhl)
weak_alias (__csinh, csinhl)
#endif
