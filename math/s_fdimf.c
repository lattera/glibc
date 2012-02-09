/* Return positive difference between arguments.
   Copyright (C) 1997, 2004, 2009 Free Software Foundation, Inc.
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

#include <errno.h>
#include <math.h>

float
__fdimf (float x, float y)
{
  int clsx = fpclassify (x);
  int clsy = fpclassify (y);

  if (clsx == FP_NAN || clsy == FP_NAN
      || (y < 0 && clsx == FP_INFINITE && clsy == FP_INFINITE))
    /* Raise invalid flag.  */
    return x - y;

  if (x <= y)
    return 0.0f;

  float r = x - y;
  if (fpclassify (r) == FP_INFINITE)
    __set_errno (ERANGE);

  return r;
}
weak_alias (__fdimf, fdimf)
