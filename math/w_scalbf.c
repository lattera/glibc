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

#include <errno.h>
#include <math.h>
#include <math_private.h>


static float
__attribute__ ((noinline))
sysv_scalbf (float x, float fn)
{
  float z = __ieee754_scalbf (x, fn);

  if (__glibc_unlikely (__isinff (z)))
    {
      if (__finitef (x))
	return __kernel_standard_f (x, fn, 132); /* scalb overflow */
      else
	__set_errno (ERANGE);
    }
  else if (__builtin_expect (z == 0.0f, 0) && z != x)
    return __kernel_standard_f (x, fn, 133); /* scalb underflow */

  return z;
}


/* Wrapper scalbf */
float
__scalbf (float x, float fn)
{
  if (__glibc_unlikely (_LIB_VERSION == _SVID_))
    return sysv_scalbf (x, fn);
  else
    {
      float z = __ieee754_scalbf (x, fn);

      if (__glibc_unlikely (!__finitef (z) || z == 0.0f))
	{
	  if (__isnanf (z))
	    {
	      if (!__isnanf (x) && !__isnanf (fn))
		__set_errno (EDOM);
	    }
	  else if (__isinf_nsf (z))
	    {
	      if (!__isinf_nsf (x) && !__isinf_nsf (fn))
		__set_errno (ERANGE);
	    }
	  else
	    {
	      /* z == 0.  */
	      if (x != 0.0f && !__isinf_nsf (fn))
		__set_errno (ERANGE);
	    }
	}
      return z;
    }
}
weak_alias (__scalbf, scalbf)
