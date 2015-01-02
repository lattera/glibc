/* Raise given exceptions.
   Copyright (C) 2000-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Denis Joseph Barrow (djbarrow@de.ibm.com) and
   Martin Schwidefsky (schwidefsky@de.ibm.com).

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <fenv_libc.h>
#include <float.h>
#include <math.h>


static __inline__ void
fexceptdiv (float d, float e)
{
  __asm__ __volatile__ ("debr %0,%1" : : "f" (d), "f" (e) );
}

static __inline__ void
fexceptadd (float d, float e)
{
  __asm__ __volatile__ ("aebr %0,%1" : : "f" (d), "f" (e) );
}


int
__feraiseexcept (int excepts)
{
  /* Raise exceptions represented by EXPECTS.  But we must raise only
     one signal at a time.  It is important that if the overflow/underflow
     exception and the inexact exception are given at the same time,
     the overflow/underflow exception follows the inexact exception.  */

  /* First: invalid exception.  */
  if (FE_INVALID & excepts)
    fexceptdiv (0.0, 0.0);

  /* Next: division by zero.  */
  if (FE_DIVBYZERO & excepts)
    fexceptdiv (1.0, 0.0);

  /* Next: overflow.  */
  if (FE_OVERFLOW & excepts)
    /* I don't think we can do the same trick as intel so we will have
       to live with inexact coming also.  */
    fexceptadd (FLT_MAX, 1.0e32);

  /* Next: underflow.  */
  if (FE_UNDERFLOW & excepts)
    fexceptdiv (FLT_MIN, 3.0);

  /* Last: inexact.  */
  if (FE_INEXACT & excepts)
    fexceptdiv (2.0, 3.0);

  /* Success.  */
  return 0;
}
libm_hidden_def (__feraiseexcept)
weak_alias (__feraiseexcept, feraiseexcept)
libm_hidden_weak (feraiseexcept)
