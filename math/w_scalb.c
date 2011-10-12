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

#include <errno.h>
#include <math.h>
#include <math_private.h>


static double
__attribute__ ((noinline))
sysv_scalb (double x, double fn)
{
  double z = __ieee754_scalb (x, fn);

  if (__builtin_expect (__isinf (z), 0))
    {
      if (__finite (x))
	return __kernel_standard (x, fn, 32); /* scalb overflow */
      else
	__set_errno (ERANGE);
    }
  else if (__builtin_expect (z == 0.0, 0) && z != x)
    return __kernel_standard (x, fn, 33); /* scalb underflow */

  return z;
}


/* Wrapper scalb */
double
__scalb (double x, double fn)
{
  return (__builtin_expect (_LIB_VERSION == _SVID_, 0)
	  ? sysv_scalb (x, fn)
	  : __ieee754_scalb (x, fn));
}
weak_alias (__scalb, scalb)
#ifdef NO_LONG_DOUBLE
strong_alias (__scalb, __scalbl)
weak_alias (__scalb, scalbl)
#endif
