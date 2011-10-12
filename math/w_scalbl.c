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


static long double
__attribute__ ((noinline))
sysv_scalbl (long double x, long double fn)
{
  long double z = __ieee754_scalbl (x, fn);

  if (__builtin_expect (__isinfl (z), 0))
    {
      if (__finitel (x))
	return __kernel_standard (x, fn, 232); /* scalb overflow */
      else
	__set_errno (ERANGE);
    }
  else if (__builtin_expect (z == 0.0L, 0) && z != x)
    return __kernel_standard (x, fn, 233); /* scalb underflow */

  return z;
}


/* Wrapper scalbl */
long double
__scalbl (long double x, long double fn)
{
  return (__builtin_expect (_LIB_VERSION == _SVID_, 0)
	  ? sysv_scalbl (x, fn)
	  : __ieee754_scalbl (x, fn));
}
weak_alias (__scalbl, scalbl)
