/* Return cosine of complex double value.
   Copyright (C) 1997, 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <complex.h>
#include <fenv.h>
#include <math.h>
#include <math_private.h>


__complex__ double
__ccos (__complex__ double x)
{
  __complex__ double y;

  __real__ y = -__imag__ x;
  __imag__ y = __real__ x;

  return __ccosh (y);
}
weak_alias (__ccos, ccos)
#ifdef NO_LONG_DOUBLE
strong_alias (__ccos, __ccosl)
weak_alias (__ccos, ccosl)
#endif
