/* Double-precision floating point square root wrapper.
   Copyright (C) 2004, 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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
#include <fenv_libc.h>

double
__sqrt (double x)		/* wrapper sqrt */
{
#ifdef _IEEE_LIBM
  return __ieee754_sqrt (x);
#else
  double z;
  z = __ieee754_sqrt (x);
  if (_LIB_VERSION == _IEEE_ || (x != x))
    return z;

  if (x < 0.0)
    return __kernel_standard (x, x, 26);	/* sqrt(negative) */
  else
    return z;
#endif
}

weak_alias (__sqrt, sqrt)
#ifdef NO_LONG_DOUBLE
  strong_alias (__sqrt, __sqrtl) weak_alias (__sqrt, sqrtl)
#endif
