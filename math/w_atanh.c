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

#include <math.h>
#include <math_private.h>


/* wrapper atanh */
double
__atanh (double x)
{
  if (__builtin_expect (fabs (x) >= 1.0, 0) && _LIB_VERSION != _IEEE_)
    return __kernel_standard (x, x,
			      fabs (x) > 1.0
			      ? 30		/* atanh(|x|>1) */
			      : 31);		/* atanh(|x|==1) */

  return __ieee754_atanh (x);
}
weak_alias (__atanh, atanh)
#ifdef NO_LONG_DOUBLE
strong_alias (__atanh, __atanhl)
weak_alias (__atanh, atanhl)
#endif
