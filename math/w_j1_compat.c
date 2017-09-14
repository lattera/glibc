/* Copyright (C) 2011-2017 Free Software Foundation, Inc.
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
#include <math-svid-compat.h>


#if LIBM_SVID_COMPAT
/* wrapper j1 */
double
__j1 (double x)
{
  if (__builtin_expect (isgreater (fabs (x), X_TLOSS), 0)
      && _LIB_VERSION != _IEEE_ && _LIB_VERSION != _POSIX_)
    /* j1(|x|>X_TLOSS) */
    return __kernel_standard (x, x, 36);

  return __ieee754_j1 (x);
}
weak_alias (__j1, j1)
# ifdef NO_LONG_DOUBLE
strong_alias (__j1, __j1l)
weak_alias (__j1, j1l)
# endif


/* wrapper y1 */
double
__y1 (double x)
{
  if (__builtin_expect (islessequal (x, 0.0) || isgreater (x, X_TLOSS), 0)
      && _LIB_VERSION != _IEEE_)
    {
      if (x < 0.0)
	{
	  /* d = zero/(x-x) */
	  __feraiseexcept (FE_INVALID);
	  return __kernel_standard (x, x, 11);
	}
      else if (x == 0.0)
	{
	  /* d = -one/(x-x) */
	  __feraiseexcept (FE_DIVBYZERO);
	  return __kernel_standard (x, x, 10);
	}
      else if (_LIB_VERSION != _POSIX_)
	/* y1(x>X_TLOSS) */
	return __kernel_standard (x, x, 37);
    }

  return __ieee754_y1 (x);
}
weak_alias (__y1, y1)
# ifdef NO_LONG_DOUBLE
strong_alias (__y1, __y1l)
weak_alias (__y1, y1l)
# endif
#endif
