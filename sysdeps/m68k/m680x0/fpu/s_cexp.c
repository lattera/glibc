/* Complex exponential function.  m68k fpu version
   Copyright (C) 1997, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@issan.informatik.uni-dortmund.de>

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

#include <complex.h>
#include <math.h>
#include "mathimpl.h"

#ifndef SUFF
#define SUFF
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
  unsigned long ix_cond;

  ix_cond = __m81_test (__imag__ x);

  if ((ix_cond & (__M81_COND_NAN|__M81_COND_INF)) == 0)
    {
      /* Imaginary part is finite.  */
      float_type exp_val = m81(__ieee754_exp) (__real__ x);

      __real__ retval = __imag__ retval = exp_val;
      if (m81(__finite) (exp_val))
	{
	  float_type sin_ix, cos_ix;
	  __asm ("fsincos%.x %2,%1:%0" : "=f" (sin_ix), "=f" (cos_ix)
		 : "f" (__imag__ x));
	  __real__ retval *= cos_ix;
	  if (ix_cond & __M81_COND_ZERO)
	    __imag__ retval = __imag__ x;
	  else
	    __imag__ retval *= sin_ix;
	}
      else
	{
	  /* Compute the sign of the result.  */
	  float_type remainder, pi_2;
	  int quadrant;

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
	  if (ix_cond & __M81_COND_ZERO && !m81(__isnan) (exp_val))
	    __imag__ retval = __imag__ x;
	}
    }
  else
    {
      unsigned long rx_cond = __m81_test (__real__ x);

      if (rx_cond & __M81_COND_INF)
	{
	  /* Real part is infinite.  */
	  if (rx_cond & __M81_COND_NEG)
	    {
	      __real__ retval = __imag__ retval = 0.0;
	      if (ix_cond & __M81_COND_NEG)
		__imag__ retval = -__imag__ retval;
	    }
	  else
	    {
	      __real__ retval = __real__ x;
	      __imag__ retval = __imag__ x - __imag__ x;
	    }
	}
      else
	__real__ retval = __imag__ retval = __imag__ x - __imag__ x;
    }

  return retval;
}
#define weak_aliasx(a,b) weak_alias(a,b)
weak_aliasx (s(__cexp), s(cexp))
