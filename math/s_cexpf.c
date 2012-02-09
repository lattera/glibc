/* Return value of complex exponential function for float complex value.
   Copyright (C) 1997, 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <complex.h>
#include <fenv.h>
#include <math.h>
#include <math_private.h>


__complex__ float
__cexpf (__complex__ float x)
{
  __complex__ float retval;
  int rcls = fpclassify (__real__ x);
  int icls = fpclassify (__imag__ x);

  if (__builtin_expect (rcls >= FP_ZERO, 1))
    {
      /* Real part is finite.  */
      if (__builtin_expect (icls >= FP_ZERO, 1))
	{
	  /* Imaginary part is finite.  */
	  float exp_val = __ieee754_expf (__real__ x);
	  float sinix, cosix;

	  __sincosf (__imag__ x, &sinix, &cosix);

	  if (isfinite (exp_val))
	    {
	      __real__ retval = exp_val * cosix;
	      __imag__ retval = exp_val * sinix;
	    }
	  else
	    {
	      __real__ retval = __copysignf (exp_val, cosix);
	      __imag__ retval = __copysignf (exp_val, sinix);
	    }
	}
      else
	{
	  /* If the imaginary part is +-inf or NaN and the real part
	     is not +-inf the result is NaN + iNaN.  */
	  __real__ retval = __nanf ("");
	  __imag__ retval = __nanf ("");

	  feraiseexcept (FE_INVALID);
	}
    }
  else if (__builtin_expect (rcls == FP_INFINITE, 1))
    {
      /* Real part is infinite.  */
      if (__builtin_expect (icls >= FP_ZERO, 1))
	{
	  /* Imaginary part is finite.  */
	  float value = signbit (__real__ x) ? 0.0 : HUGE_VALF;

	  if (icls == FP_ZERO)
	    {
	      /* Imaginary part is 0.0.  */
	      __real__ retval = value;
	      __imag__ retval = __imag__ x;
	    }
	  else
	    {
	      float sinix, cosix;

	      __sincosf (__imag__ x, &sinix, &cosix);

	      __real__ retval = __copysignf (value, cosix);
	      __imag__ retval = __copysignf (value, sinix);
	    }
	}
      else if (signbit (__real__ x) == 0)
	{
	  __real__ retval = HUGE_VALF;
	  __imag__ retval = __nanf ("");

	  if (icls == FP_INFINITE)
	    feraiseexcept (FE_INVALID);
	}
      else
	{
	  __real__ retval = 0.0;
	  __imag__ retval = __copysignf (0.0, __imag__ x);
	}
    }
  else
    {
      /* If the real part is NaN the result is NaN + iNaN.  */
      __real__ retval = __nanf ("");
      __imag__ retval = __nanf ("");

      if (rcls != FP_NAN || icls != FP_NAN)
	feraiseexcept (FE_INVALID);
    }

  return retval;
}
#ifndef __cexpf
weak_alias (__cexpf, cexpf)
#endif
