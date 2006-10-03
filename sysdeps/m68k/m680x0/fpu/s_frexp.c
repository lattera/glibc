/* Copyright (C) 1996, 1997, 2003 Free Software Foundation, Inc.
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

#ifndef FUNC
#define FUNC frexp
#endif
#ifndef float_type
#define float_type double
#endif

#define __CONCATX(a,b) __CONCAT(a,b)

float_type
__CONCATX(__,FUNC) (float_type value, int *expptr)
{
  float_type mantissa, exponent;
  int iexponent;
  unsigned long fpsr;

  __asm ("ftst%.x %1\n"
	 "fmove%.l %/fpsr, %0"
	 : "=dm" (fpsr) : "f" (value));
  if (fpsr & (7 << 24))
    {
      /* Not finite or zero.  */
      *expptr = 0;
      return value;
    }
  __asm ("fgetexp%.x %1, %0" : "=f" (exponent) : "f" (value));
  iexponent = (int) exponent + 1;
  *expptr = iexponent;
  __asm ("fscale%.l %2, %0"
	 : "=f" (mantissa)
	 : "0" (value), "dmi" (-iexponent));
  return mantissa;
}

#define weak_aliasx(a,b) weak_alias(a,b)
weak_aliasx (__CONCATX(__,FUNC), FUNC)
