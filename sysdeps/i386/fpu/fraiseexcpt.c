/* Raise given exceptions.
   Copyright (C) 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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
  /* Raise exceptions represented by EXPECTS.  But we must raise only
     one signal at a time.  It is important the if the overflow/underflow
     exception and the inexact exception are given at the same time,
     the overflow/underflow exception follows the inexact exception.  */

  /* First: invalid exception.  */
  if ((FE_INVALID & excepts) != 0)
    {
      /* One example of a invalid operation is 0 * Infinity.  */
      double d = 0.0 * HUGE_VAL;
      (void) &d;
      /* Now force the exception.  */
      __asm__ ("fwait");
    }

  /* Next: division by zero.  */
  if ((FE_DIVBYZERO & excepts) != 0)
    {
      double d;
      __asm__ ("fld1; fldz; fdivp %%st, %%st(1); fwait" : "=t" (d));
      (void) &d;
    }

  /* Next: overflow.  */
  if ((FE_OVERFLOW & excepts) != 0)
    {
      long double d = LDBL_MAX * LDBL_MAX;
      (void) &d;
      /* Now force the exception.  */
      __asm__ ("fwait");
    }

  /* Next: underflow.  */
  if ((FE_UNDERFLOW & excepts) != 0)
    {
      long double d = LDBL_MIN / 16.0;
      (void) &d;
      /* Now force the exception.  */
      __asm__ ("fwait");
    }

  /* Last: inexact.  */
  if ((FE_INEXACT & excepts) != 0)
    {
      long double d;
      __asm__ ("fld1; fldpi; fdivp %%st, %%st(1); fwait" : "=t" (d));
      (void) &d;
    }
}
