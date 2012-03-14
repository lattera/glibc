/* Single-precision floating point square root wrapper.
   Copyright (C) 2004, 2007, 2012 Free Software Foundation, Inc.
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

#include <math.h>
#include <math_private.h>
#include <fenv_libc.h>

#include <sysdep.h>
#include <ldsodefs.h>

float
__sqrtf (float x)		/* wrapper sqrtf */
{
#ifdef _IEEE_LIBM
  return __ieee754_sqrtf (x);
#else
  float z;
/* Power4 (ISA V2.0) and above implement sqrtf in hardware.  */
   __asm __volatile (
	"	fsqrts	%0,%1\n"
		: "=f" (z)
		: "f" (x));

  if (__builtin_expect (_LIB_VERSION == _IEEE_, 0))
    return z;
    
  if (__builtin_expect (x != x, 0))
    return z;
    
  if  (__builtin_expect (x < 0.0, 0))
    /* sqrtf(negative) */
    return (float) __kernel_standard ((double) x, (double) x, 126);
  else
    return z;
#endif
}

weak_alias (__sqrtf, sqrtf)
