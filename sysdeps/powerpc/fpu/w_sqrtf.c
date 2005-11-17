/* Single-precision floating point square root wrapper.
   Copyright (C) 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include "math.h"
#include "math_private.h"
#include <fenv_libc.h>

#include <sysdep.h>
#include <ldsodefs.h>

#ifdef __STDC__
float
__sqrtf (float x)		/* wrapper sqrtf */
#else
float
__sqrtf (x)			/* wrapper sqrtf */
     float x;
#endif
{
#ifdef _IEEE_LIBM
  return __ieee754_sqrtf (x);
#else
  float z;
  z = __ieee754_sqrtf (x);

  if (_LIB_VERSION == _IEEE_ || (x != x))
    return z;

  if (x < (float) 0.0)
    /* sqrtf(negative) */
    return (float) __kernel_standard ((double) x, (double) x, 126);
  else
    return z;
#endif
}

weak_alias (__sqrtf, sqrtf)
