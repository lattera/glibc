/* Raise given exceptions.
   Copyright (C) 1997, 1999, 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Huggins-Daines <dhd@debian.org>

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

#include <fenv.h>
#include <float.h>
#include <math.h>

int
feraiseexcept (int excepts)
{
  /* Raise exceptions represented by EXCEPTS.  But we must raise only one
     signal at a time.  It is important that if the overflow/underflow
     exception and the divide by zero exception are given at the same
     time, the overflow/underflow exception follows the divide by zero
     exception.  */

  /* We do these bits in assembly to be certain GCC doesn't optimize
     away something important, and so we can force delayed traps to
     occur.  */

  /* FIXME: These all need verification! */

  /* First: invalid exception.  */
  if (excepts & FE_INVALID)
    {
      /* One example of a invalid operation is 0 * Infinity.  */
      double d = HUGE_VAL;
      __asm__ __volatile__ ("fmpy,dbl %1,%%fr0,%0\n\t"
			    /* FIXME: is this a proper trap barrier? */
			    "fcpy,dbl %%fr0,%%fr0" : "=f" (d) : "0" (d));
    }

  /* Next: division by zero.  */
  if (excepts & FE_DIVBYZERO)
    {
      double d = 1.0;
      __asm__ __volatile__ ("fdiv,dbl %1,%%fr0,%0\n\t"
			    "fcpy,dbl %%fr0,%%fr0" : "=f" (d) : "0" (d));
    }

  /* Next: overflow.  */
  /* FIXME: Compare with IA-64 - do we have the same problem? */
  if (excepts & FE_OVERFLOW)
    {
      double d = DBL_MAX;

      __asm__ __volatile__ ("fmpy,dbl %1,%1,%0\n\t"
			    "fcpy,dbl %%fr0,%%fr0" : "=f" (d) : "0" (d));
    }

  /* Next: underflow.  */
  if (excepts & FE_UNDERFLOW)
    {
      double d = DBL_MIN;
      double e = 69.69;

      __asm__ __volatile__ ("fdiv,dbl %1,%2,%0\n\t"
			    "fcpy,dbl %%fr0,%%fr0" : "=f" (d) : "0" (d), "f" (e));
    }

  /* Last: inexact.  */
  if (excepts & FE_INEXACT)
    {
      double d = 1.0;
      double e = M_PI;

      __asm__ __volatile__ ("fdiv,dbl %1,%2,%0\n\t"
			    "fcpy,dbl %%fr0,%%fr0" : "=f" (d) : "0" (d), "f" (e));
    }

  /* Success.  */
  return 0;
}
