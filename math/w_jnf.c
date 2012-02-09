/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@gmail.com>, 2011.

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

#include <fenv.h>
#include <math.h>
#include <math_private.h>


/* wrapper jnf */
float
jnf (int n, float x)
{
  if (__builtin_expect (fabsf (x) > (float) X_TLOSS, 0)
      && _LIB_VERSION != _IEEE_)
    /* jn(n,|x|>X_TLOSS) */
    return __kernel_standard_f (n, x, 138);

  return __ieee754_jnf (n, x);
}


/* wrapper ynf */
float
ynf (int n, float x)
{
  if (__builtin_expect (x <= 0.0f || x > (float) X_TLOSS, 0)
      && _LIB_VERSION != _IEEE_)
    {
      if (x < 0.0f)
	{
	  /* d = zero/(x-x) */
	  feraiseexcept (FE_INVALID);
	  return __kernel_standard_f (n, x, 113);
	}
      else if (x == 0.0)
	/* d = -one/(x-x) */
	return __kernel_standard_f (n, x, 112);
      else
	/* yn(n,x>X_TLOSS) */
	return __kernel_standard_f (n, x, 139);
    }

  return __ieee754_ynf (n, x);
}
