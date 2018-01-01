/* Copyright (C) 2011-2018 Free Software Foundation, Inc.
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


static long double
__attribute__ ((noinline))
invalid_fn (long double x, long double fn)
{
  if (__rintl (fn) != fn)
    return (fn - fn) / (fn - fn);
  else if (fn > 65000.0L)
    return __scalbnl (x, 65000);
  else
    return __scalbnl (x,-65000);
}


long double
__ieee754_scalbl (long double x, long double fn)
{
  if (__glibc_unlikely (isnan (x)))
    return x * fn;
  if (__glibc_unlikely (!isfinite (fn)))
    {
      if (isnan (fn) || fn > 0.0L)
	return x * fn;
      if (x == 0.0L)
	return x;
      return x / -fn;
    }
  if (__glibc_unlikely (fabsl (fn) >= 0x1p31L || (long double) (int) fn != fn))
    return invalid_fn (x, fn);

  return __scalbnl (x, (int) fn);
}
strong_alias (__ieee754_scalbl, __scalbl_finite)
