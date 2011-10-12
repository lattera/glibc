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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <math.h>
#include <math_private.h>


/* wrapper remainderf */
float
__remainderf (float x, float y)
{
  if (((__builtin_expect (y == 0.0f, 0) && ! __isnanf (x))
       || (__builtin_expect (__isinf_nsf (x), 0) && ! __isnanf (y)))
      && _LIB_VERSION != _IEEE_)
    return __kernel_standard_f (x, y, 128); /* remainder domain */

  return __ieee754_remainderf (x, y);
}
weak_alias (__remainderf, remainderf)
