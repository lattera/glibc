/* Complex exponential function.  m68k fpu version
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@issan.informatik.uni-dortmund.de>

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

#define __LIBC_M81_MATH_INLINES
#include <complex.h>
#include <math.h>

#ifndef SUFF
#define SUFF
#endif
#ifndef huge_val
#define huge_val HUGE_VAL
#endif
#ifndef float_type
#define float_type double
#endif

#define CONCATX(a,b) __CONCAT(a,b)
#define s(name) CONCATX(name,SUFF)
#define m81(func) __m81_u(s(func))

__complex__ float_type
s(__cexp) (__complex__ float_type x)
{
  __complex__ float_type retval;
  float_type sin_ix, cos_ix;

  if (m81(__finite) (__real__ x))
    {
      if (m81(__finite) (__imag__ x))
	{
	  float_type exp_val = s(__exp) (__real__ x);

	  __asm ("fsincos%.x %2,%1:%0" : "=f" (sin_ix), "=f" (cos_ix)
		 : "f" (__imag__ x));
	  __real__ retval = exp_val * cos_ix;
	  __imag__ retval = exp_val * sin_ix;
	}
      else
	/* If the imaginary part is +-inf or NaN and the real part is
	   not +-inf the result is NaN + iNaN.  */
	__real__ retval = __imag__ retval = 0.0/0.0;
    }
  else if (m81(__isinf) (__real__ x))
    {
      if (m81(__finite) (__imag__ x))
	{
	  if (m81(__signbit) (__real__ x) == 0 && __imag__ x == 0.0)
	    retval = huge_val;
	  else
	    {
	      float_type value = m81(__signbit) (__real__ x) ? 0.0 : huge_val;

	      __asm ("fsincos%.x %2,%1:%0" : "=f" (sin_ix), "=f" (cos_ix)
		     : "f" (__imag__ x));
	      __real__ retval = value * cos_ix;
	      __imag__ retval = value * sin_ix;
	    }
	}
      else if (m81(__signbit) (__real__ x) == 0)
	{
	  __real__ retval = huge_val;
	  __imag__ retval = 0.0/0.0;
	}
      else
	retval = 0.0;
    }
  else
    /* If the real part is NaN the result is NaN + iNaN.  */
    __real__ retval = __imag__ retval = 0.0/0.0;

  return retval;
}
#define weak_aliasx(a,b) weak_alias(a,b)
weak_aliasx (s(__cexp), s(cexp))
