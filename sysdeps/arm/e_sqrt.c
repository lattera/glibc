/* Compute square root for double.  ARM version.
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

/* Use architecture-indendent sqrt implementation.  */
# include <sysdeps/ieee754/dbl-64/e_sqrt.c>

#else

/* Use VFP square root instruction.  */
# include <math.h>
# include <sysdep.h>

double
__ieee754_sqrt (double x)
{
  double ret;
# if __ARM_ARCH >= 6
  asm ("vsqrt.f64 %P0, %P1" : "=w" (ret) : "w" (x));
# else
  /* As in GCC, for VFP9 Erratum 760019 avoid overwriting the
     input.  */
  asm ("vsqrt.f64 %P0, %P1" : "=&w" (ret) : "w" (x));
# endif
  return ret;
}
strong_alias (__ieee754_sqrt, __sqrt_finite)

#endif
