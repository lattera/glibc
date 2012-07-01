/* Raise given exceptions.
   Copyright (C) 2000, 2002, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>
   based on corresponding file in the M68K port.

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

#include <fenv.h>
#include <float.h>
#include <math.h>
#include <shlib-compat.h>

int
__feraiseexcept (int excepts)
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
      float x = HUGE_VALF, y = 0.0f;
      __asm__ __volatile__ ("fmul %1,%0" : "+f" (x) : "f" (y));
    }

  /* Next: division by zero.  */
  if (excepts & FE_DIVBYZERO)
    {
      float x = 1.0f, y = 0.0f;
      __asm__ __volatile__ ("fdiv %1,%0" : "+f" (x) : "f" (y));
    }

  /* Next: overflow.  */
  if (excepts & FE_OVERFLOW)
    {
      float x = FLT_MAX;

      __asm__ __volatile__ ("fmul %0,%0" : "+f" (x));
    }

  /* Next: underflow.  */
  if (excepts & FE_UNDERFLOW)
    {
      float x = -FLT_MIN;

      __asm__ __volatile__ ("fmul %0,%0" : "+f" (x));
    }

  /* Last: inexact.  */
  if (excepts & FE_INEXACT)
    {
      float x = 1.0f, y = 3.0f;
      __asm__ __volatile__ ("fdiv %1,%0" : "=f" (x) : "f" (y));
    }

  /* Success.  */
  return 0;
}

libm_hidden_ver (__feraiseexcept, feraiseexcept)
versioned_symbol (libm, __feraiseexcept, feraiseexcept, GLIBC_2_2);
