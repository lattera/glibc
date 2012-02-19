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

#include <math.h>
#include <math_private.h>

static const double
o_threshold=  7.09782712893383973096e+02,  /* 0x40862E42, 0xFEFA39EF */
u_threshold= -7.45133219101941108420e+02;  /* 0xc0874910, 0xD52D3051 */


/* wrapper exp */
double
__exp (double x)
{
  if (__builtin_expect (isgreater (x, o_threshold), 0))
    {
      if (_LIB_VERSION != _IEEE_)
	return __kernel_standard_f (x, x, 6);
    }
  else if (__builtin_expect (isless (x, u_threshold), 0))
    {
      if (_LIB_VERSION != _IEEE_)
	return __kernel_standard_f (x, x, 7);
    }

  return __ieee754_exp (x);
}
hidden_def (__exp)
weak_alias (__exp, exp)
#ifdef NO_LONG_DOUBLE
strong_alias (__exp, __expl)
weak_alias (__exp, expl)
#endif
