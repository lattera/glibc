/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Hartvig Ekner <hartvige@mips.com>, 2002.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */


#include <sgidefs.h>


#if (_MIPS_ISA >= _MIPS_ISA_MIPS2)

double __attribute__ ((nomips16))
__ieee754_sqrt (double x)
{
  double z;
  __asm__ ("sqrt.d %0,%1" : "=f" (z) : "f" (x));
  return z;
}
strong_alias (__ieee754_sqrt, __sqrt_finite)

#else

#include <sysdeps/ieee754/dbl-64/e_sqrt.c>

#endif
