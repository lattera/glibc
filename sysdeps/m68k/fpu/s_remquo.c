/* Compute remainder and a congruent to the quotient.  m68k fpu version
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

  /* FIXME: Which of frem and fmod is correct?  */
#if 1
  __asm ("frem%.x %2,%0\n\tfmove%.l %/fpsr,%1"
	 : "=f" (result), "=dm" (fpsr) : "f" (y), "0" (x));
  cquo = (fpsr >> 16) & 0x7f;
  if ((result > 0) != (x > 0))
    cquo--;
#else
  __asm ("fmod%.x %2,%0\n\tfmove%.l %/fpsr,%1"
	 : "=f" (result), "=dm" (fpsr) : "f" (y), "0" (x));
  cquo = (fpsr >> 16) & 0x7f;
#endif
  if (fpsr & (1 << 23))
    cquo = -cquo;
  *quo = cquo;
  return result;
}
#define weak_aliasx(a,b) weak_alias(a,b)
weak_aliasx (s(__remquo), s(remquo))
