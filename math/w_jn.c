/* Copyright (C) 2011, 2012 Free Software Foundation, Inc.
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


/* wrapper jn */
double
jn (int n, double x)
{
  if (__builtin_expect (isgreater (fabs (x), X_TLOSS), 0)
      && _LIB_VERSION != _IEEE_ && _LIB_VERSION != _POSIX_)
    /* jn(n,|x|>X_TLOSS) */
    return __kernel_standard (n, x, 38);

  return __ieee754_jn (n, x);
}
#ifdef NO_LONG_DOUBLE
strong_alias (jn, jnl)
#endif


/* wrapper yn */
double
yn (int n, double x)
{
  if (__builtin_expect (islessequal (x, 0.0) || isgreater (x, X_TLOSS), 0)
      && _LIB_VERSION != _IEEE_)
    {
      if (x < 0.0)
	{
	  /* d = zero/(x-x) */
	  feraiseexcept (FE_INVALID);
	  return __kernel_standard (n, x, 13);
	}
      else if (x == 0.0)
	/* d = -one/(x-x) */
	return __kernel_standard (n, x, 12);
      else if (_LIB_VERSION != _POSIX_)
	/* yn(n,x>X_TLOSS) */
	return __kernel_standard (n, x, 39);
    }

  return __ieee754_yn (n, x);
}
#ifdef NO_LONG_DOUBLE
strong_alias (yn, ynl)
#endif
