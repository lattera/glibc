/* Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson.

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


/* Use the -inf rounding mode conversion instructions to implement
   floor.  We note when the exponent is large enough that the value
   must be integral, as this avoids unpleasant integer overflows.  */

float
__floorf (float x)
{
  if (isless (fabsf (x), 16777216.0f))	/* 1 << FLT_MANT_DIG */
    {
      /* Note that Alpha S_Floating is stored in registers in a
	 restricted T_Floating format, so we don't even need to
	 convert back to S_Floating in the end.  The initial
	 conversion to T_Floating is needed to handle denormals.  */

      float tmp1, tmp2, new_x;

      __asm ("cvtst/s %3,%2\n\t"
#ifdef _IEEE_FP_INEXACT
	     "cvttq/svim %2,%1\n\t"
#else
	     "cvttq/svm %2,%1\n\t"
#endif
	     "cvtqt/m %1,%0\n\t"
	     : "=f"(new_x), "=&f"(tmp1), "=&f"(tmp2)
	     : "f"(x));

      /* floor(-0) == -0, and in general we'll always have the same
	 sign as our input.  */
      x = copysignf(new_x, x);
    }
  return x;
}

weak_alias (__floorf, floorf)
