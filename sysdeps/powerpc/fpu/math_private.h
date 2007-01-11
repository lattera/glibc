/* Private inline math functions for powerpc.
   Copyright (C) 2006
   Free Software Foundation, Inc.
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
   Software Foundation, Inc., 51 Franklin St - Fifth Floor, Boston,
   MA 02110-1301 USA  */

#ifndef _PPC_MATH_PRIVATE_H_
#define _PPC_MATH_PRIVATE_H_

#include <sysdep.h>
#include <ldsodefs.h>
#include <dl-procinfo.h>

# if __WORDSIZE == 64 || defined _ARCH_PWR4
#  define __CPU_HAS_FSQRT 1
# else
#  define __CPU_HAS_FSQRT ((GLRO(dl_hwcap) & PPC_FEATURE_64) != 0)
# endif

# ifndef __LIBC_INTERNAL_MATH_INLINES
extern double __slow_ieee754_sqrt (double);
__inline double
__ieee754_sqrt (double __x)
{
  double __z;

  /* If the CPU is 64-bit we can use the optional FP instructions.  */
  if (__CPU_HAS_FSQRT)
  {
    /* Volatile is required to prevent the compiler from moving the
       fsqrt instruction above the branch.  */
     __asm __volatile (
	"	fsqrt	%0,%1\n"
		: "=f" (__z)
		: "f" (__x));
  }
  else
     __z = __slow_ieee754_sqrt(__x);

  return __z;
}

extern float __slow_ieee754_sqrtf (float);

__inline float
__ieee754_sqrtf (float __x)
{
  float __z;

  /* If the CPU is 64-bit we can use the optional FP instructions.  */
  if (__CPU_HAS_FSQRT)
  {
    /* Volatile is required to prevent the compiler from moving the
       fsqrts instruction above the branch.  */
     __asm __volatile (
	"	fsqrts	%0,%1\n"
		: "=f" (__z)
		: "f" (__x));
  }
  else
     __z = __slow_ieee754_sqrtf(__x);

  return __z;
}
#endif /* __LIBC_INTERNAL_MATH_INLINES */

#include <math/math_private.h>

#endif /* _PPC_MATH_PRIVATE_H_ */
