/* Complex sine hyperbole function.  m68k fpu version
   Copyright (C) 1997, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Schwab <schwab@issan.informatik.uni-dortmund.de>.

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
s(__csinh) (__complex__ float_type x)
{
  __complex__ float_type retval;
  unsigned long ix_cond;

  ix_cond = __m81_test (__imag__ x);

  if ((ix_cond & (__M81_COND_INF|__M81_COND_NAN)) == 0)
    {
      /* Imaginary part is finite.  */
      float_type sin_ix, cos_ix;

      __asm ("fsincos%.x %2,%1:%0" : "=f" (sin_ix), "=f" (cos_ix)
	     : "f" (__imag__ x));
      __real__ retval = cos_ix * m81(__ieee754_sinh) (__real__ x);
      if (ix_cond & __M81_COND_ZERO)
	__imag__ retval = __imag__ x;
      else
	__imag__ retval = sin_ix * m81(__ieee754_cosh) (__real__ x);
    }
  else
    {
      unsigned long rx_cond = __m81_test (__real__ x);

      __imag__ retval = __imag__ x - __imag__ x;
      if (rx_cond & (__M81_COND_ZERO|__M81_COND_INF|__M81_COND_NAN))
	__real__ retval = __real__ x;
      else
	__real__ retval = __imag__ retval;
    }

  return retval;
}
#define weak_aliasx(a,b) weak_alias(a,b)
weak_aliasx (s(__csinh), s(csinh))
