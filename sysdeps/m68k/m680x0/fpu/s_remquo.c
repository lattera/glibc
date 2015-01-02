/* Compute remainder and a congruent to the quotient.  m68k fpu version
   Copyright (C) 1997-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <math.h>

#ifndef SUFF
#define SUFF
#endif
#ifndef float_type
#define float_type double
#endif

#define CONCATX(a,b) __CONCAT(a,b)
#define s(name) CONCATX(name,SUFF)

float_type
s(__remquo) (float_type x, float_type y, int *quo)
{
  float_type result;
  int cquo, fpsr;

  __asm ("frem%.x %2,%0\n\tfmove%.l %/fpsr,%1"
	 : "=f" (result), "=dm" (fpsr) : "f" (y), "0" (x));
  cquo = (fpsr >> 16) & 0x7f;
  if (fpsr & (1 << 23))
    cquo = -cquo;
  *quo = cquo;
  return result;
}
weak_alias (s(__remquo), s(remquo))
