/* Complex cosine hyperbole function.  m68k fpu version
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@issan.informatik.uni-dortmund.de>.

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
s(__ccosh) (__complex__ float_type x)
{
  __complex__ float_type retval;

  __real__ x = s(fabs) (__real__ x);

  if (m81(__finite) (__real__ x))
    {
      if (m81(__finite) (__imag__ x))
	{
	  float_type cosh_val;
	  float_type sin_ix, cos_ix;

	  __asm ("fsincos%.x %2,%1:%0" : "=f" (sin_ix), "=f" (cos_ix)
		 : "f" (__imag__ x));
	  cosh_val = m81(__ieee754_cosh) (__real__ x);
	  __real__ retval = cos_ix * cosh_val;
	  __imag__ retval = sin_ix * cosh_val;
	}
      else if (__real__ x == 0)
	{
	  __imag__ retval = 0.0;
	  __real__ retval = huge_val - huge_val;
	}
      else
	__real__ retval = __imag__ retval = huge_val - huge_val;
    }
  else if (m81(__isinf) (__real__ x))
    {
      if (__imag__ x == 0)
	{
	  __real__ retval = huge_val;
	  __imag__ retval = __imag__ x;
	}
      else if (m81(__finite) (__imag__ x))
	{
	  float_type remainder, pi_2;
	  int quadrant;
	  __real__ retval = __imag__ retval = huge_val;

	  __asm ("fmovecr %#0,%0\n\tfscale%.w %#-1,%0" : "=f" (pi_2));
	  __asm ("fmod%.x %2,%0\n\tfmove%.l %/fpsr,%1"
		 : "=f" (remainder), "=dm" (quadrant)
		 : "f" (pi_2), "0" (__imag__ x));
	  quadrant = (quadrant >> 16) & 0x83;
	  if (quadrant & 0x80)
	    quadrant ^= 0x83;
	  switch (quadrant)
	    {
	    default:
	      break;
	    case 1:
	      __real__ retval = -__real__ retval;
	      break;
	    case 2:
	      __real__ retval = -__real__ retval;
	    case 3:
	      __imag__ retval = -__imag__ retval;
	      break;
	    }
	}
      else
	{
	  /* The subtraction raises the invalid exception.  */
	  __real__ retval = huge_val;
	  __imag__ retval = huge_val - huge_val;
	}
    }
  else if (__imag__ x == 0)
    {
      __real__ retval = 0.0/0.0;
      __imag__ retval = __imag__ x;
    }
  else
    __real__ retval = __imag__ retval = 0.0/0.0;

  return retval;
}
#define weak_aliasx(a,b) weak_alias(a,b)
weak_aliasx (s(__ccosh), s(ccosh))
