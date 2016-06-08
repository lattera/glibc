/* Wrapper for __scalbln handles setting errno.
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

double
__w_scalbln (double x, long int n)
{
  if (!isfinite (x) || x == 0.0)
    return x + x;

  x = __scalbln (x, n);

  if (!isfinite (x) || x == 0.0)
    __set_errno (ERANGE);

  return x;
}
weak_alias (__w_scalbln, scalbln)

#ifdef NO_LONG_DOUBLE
weak_alias (__w_scalbln, scalblnl)
#endif
