/* Wrapper for __scalblnf handles setting errno.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

#include <errno.h>
#include <math.h>
#include <math_private.h>

float
__w_scalblnf (float x, long int n)
{
  if (!isfinite (x) || x == 0.0f)
    return x + x;

  x = __scalblnf (x, n);

  if (!isfinite (x) || x == 0.0f)
    __set_errno (ERANGE);

  return x;
}
weak_alias (__w_scalblnf, scalblnf)
