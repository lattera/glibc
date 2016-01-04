/* Copyright (C) 2011-2016 Free Software Foundation, Inc.
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


/* wrapper acosh */
double
__acosh (double x)
{
  if (__builtin_expect (isless (x,  1.0), 0) && _LIB_VERSION != _IEEE_)
    /* acosh(x<1) */
    return __kernel_standard (x, x, 29);

  return __ieee754_acosh (x);
}
weak_alias (__acosh, acosh)
#ifdef NO_LONG_DOUBLE
strong_alias (__acosh, __acoshl)
weak_alias (__acosh, acoshl)
#endif
