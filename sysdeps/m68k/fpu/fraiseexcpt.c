/* Raise given exceptions.
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

#include <fenv.h>
#include <math.h>

void
feraiseexcept (int excepts)
{
  /* Raise exceptions represented by EXCEPTS.  But we must raise only one
     signal at a time.  It is important that if the overflow/underflow
     exception and the divide by zero exception are given at the same
     time, the overflow/underflow exception follows the divide by zero
     exception.  */

  /* First: invalid exception.  */
  if (excepts & FE_INVALID)
    {
      /* One example of a invalid operation is 0 * Infinity.  */
      double d = HUGE_VAL;
      __asm__ __volatile__ ("fmul%.s %#0r0,%0; fnop" : "=f" (d) : "0" (d));
    }

  /* Next: division by zero.  */
  if (excepts & FE_DIVBYZERO)
    {
      double d = 1.0;
      __asm__ __volatile__ ("fdiv%.s %#0r0,%0; fnop" : "=f" (d) : "0" (d));
    }

  /* Next: overflow.  */
  if (excepts & FE_OVERFLOW)
    {
      /* We cannot raise the overflow exception without also setting the
	 inexact flag.  Restore it after the operation, unless it should
	 be set anyway.  */
      long double d = LDBL_MAX;
      fexcept_t fpsr;

      __asm__ ("fmove%.l %/fpsr,%0" : "=dm" (fpsr));
      __asm__ __volatile__ ("fmul%.x %0,%0" : "=f" (d) : "0" (d));
      if (!((excepts | fpsr) & FE_INEXACT))
	{
	  __asm__ ("fmove%.l %/fpsr,%0" : "=dm" (fpsr));
	  fpsr &= ~FE_INEXACT;
	  __asm__ __volatile__ ("fmove%.l %0,%/fpsr" : : "dm" (fpsr));
	}
      else
	__asm__ ("fnop");
    }

  /* Next: underflow.  */
  if (excepts & FE_UNDERFLOW)
    {
      /* We cannot raise the underflow exception without also setting the
	 inexact flag.  Restore it after the operation, unless it should
	 be set anyway.  */
      long double d = -LDBL_MAX;
      fexcept_t fpsr;

      __asm__ ("fmove%.l %/fpsr,%0" : "=dm" (fpsr));
      __asm__ __volatile__ ("fetox%.x %0" : "=f" (d) : "0" (d));
      if (!((excepts | fpsr) & FE_INEXACT))
	{
	  __asm__ ("fmove%.l %/fpsr,%0" : "=dm" (fpsr));
	  fpsr &= ~FE_INEXACT;
	  __asm__ __volatile__ ("fmove%.l %0,%/fpsr" : : "dm" (fpsr));
	}
      else
	__asm__ ("fnop");
    }

  /* Last: inexact.  */
  if (excepts & FE_INEXACT)
    {
      long double d = 1.0;
      __asm__ __volatile__ ("fdiv%.s %#0r3,%0; fnop" : "=f" (d) : "0" (d));
    }
}
