/* Copyright (C) 2011-2015 Free Software Foundation, Inc.
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

#include <fenv.h>
#include <math.h>
#include <math_private.h>


static float
__attribute__ ((noinline))
invalid_fn (float x, float fn)
{
  if (__rintf (fn) != fn)
    {
      feraiseexcept (FE_INVALID);
      return __nan ("");
    }
  else if (fn > 65000.0f)
    return __scalbnf (x, 65000);
  else
    return __scalbnf (x,-65000);
}


float
__ieee754_scalbf (float x, float fn)
{
  if (__glibc_unlikely (__isnanf (x)))
    return x * fn;
  if (__glibc_unlikely (!__finitef (fn)))
    {
      if (__isnanf (fn) || fn > 0.0f)
	return x * fn;
      if (x == 0.0f)
	return x;
      return x / -fn;
    }
  if (__glibc_unlikely (fabsf (fn) >= 0x1p31f || (float) (int) fn != fn))
    return invalid_fn (x, fn);

  return __scalbnf (x, (int) fn);
}
strong_alias (__ieee754_scalbf, __scalbf_finite)
