/* Raise given exceptions.
   Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson <rth@tamu.edu>, 1997.

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

#include <fenv.h>
#include <math.h>

void
feraiseexcept (int excepts)
{
  double tmp, dummy;

  /* Raise exceptions represented by EXPECTS.  But we must raise only
     one signal at a time.  It is important the if the overflow/underflow
     exception and the inexact exception are given at the same time,
     the overflow/underflow exception precedes the inexact exception.  */

  /* We do these bits in assembly to be certain GCC doesn't optimize
     away something important.  */

  /* First: invalid exception.  */
  if (FE_INVALID & excepts)
    {
      /* One example of a invalid operation is 0 * Infinity.  */
      __asm__ __volatile__("mult/sui $f31,%1,%0; trapb"
			   : "=&f"(tmp) : "f"(HUGE_VAL));
    }

  /* Next: division by zero.  */
  if (FE_DIVBYZERO & excepts)
    {
      __asm__ __volatile__("cmpteq $f31,$f31,%1; divt/sui %1,$f31,%0; trapb"
			   : "=&f"(tmp), "=f"(dummy));
    }

  /* Next: overflow.  */
  if (FE_OVERFLOW & excepts)
    {
      __asm__ __volatile__("mult/sui %1,%1,%0; trapb"
			   : "=&f"(tmp) : "f"(DBL_MAX));
    }

  /* Next: underflow.  */
  if (FE_UNDERFLOW & excepts)
    {
      __asm__ __volatile__("divt/sui %1,%2,%0; trapb"
			   : "=&f"(tmp) : "f"(DBL_MIN),
					 "f"((double) (1UL << 60)));
    }

  /* Last: inexact.  */
  if (FE_INEXACT & excepts)
    {
      __asm__ __volatile__("divt/sui %1,%2,%0; trapb"
			   : "=&f"(tmp) : "f"(1.0), "f"(M_PI));
    }
}
