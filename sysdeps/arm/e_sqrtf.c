/* Compute square root for float.  ARM version.
   Copyright (C) 2016-2018 Free Software Foundation, Inc.
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

#ifdef __SOFTFP__

/* Use architecture-indendent sqrtf implementation.  */
# include <sysdeps/ieee754/flt-32/e_sqrtf.c>

#else

/* Use VFP square root instruction.  */
# include <math.h>
# include <sysdep.h>

float
__ieee754_sqrtf (float x)
{
  float ret;
# if __ARM_ARCH >= 6
  asm ("vsqrt.f32 %0, %1" : "=t" (ret) : "t" (x));
# else
  /* As in GCC, for VFP9 Erratum 760019 avoid overwriting the
     input.  */
  asm ("vsqrt.f32 %0, %1" : "=&t" (ret) : "t" (x));
# endif
  return ret;
}
strong_alias (__ieee754_sqrtf, __sqrtf_finite)

#endif
