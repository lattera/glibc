/* Private inline math functions for powerpc.
   Copyright (C) 2006, 2011
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _PPC_MATH_PRIVATE_H_
#define _PPC_MATH_PRIVATE_H_

#include <sysdep.h>
#include <ldsodefs.h>
#include <dl-procinfo.h>

#include <math/math_private.h>

# if __WORDSIZE == 64 || defined _ARCH_PWR4
#  define __CPU_HAS_FSQRT 1

#ifndef __ieee754_sqrt
# define __ieee754_sqrt(x)		\
  ({ double __z;			\
     __asm __volatile (			\
	"	fsqrt %0,%1\n"		\
		: "=f" (__z)		\
		: "f"(x));		\
     __z; })
#endif
#ifndef __ieee754_sqrtf
# define __ieee754_sqrtf(x)		\
  ({ float __z;				\
     __asm __volatile (			\
	"	fsqrts %0,%1\n"		\
		: "=f" (__z)		\
		: "f"(x));		\
     __z; })
#endif

# else
#  define __CPU_HAS_FSQRT ((GLRO(dl_hwcap) & PPC_FEATURE_64) != 0)
# endif	// __WORDSIZE == 64 || defined _ARCH_PWR4


#if defined _ARCH_PWR5X

# ifndef __round
#  define __round(x)			\
    ({ double __z;			\
      __asm __volatile (		\
	"	frin %0,%1\n"		\
		: "=f" (__z)		\
		: "f" (x));		\
     __z; })
# endif
# ifndef __roundf
#  define __roundf(x)			\
    ({ float __z;			\
     __asm __volatile (			\
	"	frin %0,%1\n"		\
	"	frsp %0,%0\n"		\
		: "=f" (__z)		\
		: "f" (x));		\
     __z; })
# endif

# ifndef __trunc
#  define __trunc(x)			\
    ({ double __z;			\
     __asm __volatile (			\
	"	friz %0,%1\n"		\
		: "=f" (__z)		\
		: "f" (x));		\
     __z; })
# endif
# ifndef __truncf
#  define __truncf(x)			\
    ({ float __z;			\
     __asm __volatile (			\
	"	friz %0,%1\n"		\
	"	frsp %0,%0\n"		\
		: "=f" (__z)		\
		: "f" (x));		\
     __z; })
# endif

# ifndef __ceil
#  define __ceil(x)			\
    ({ double __z;			\
     __asm __volatile (			\
	"	frip %0,%1\n"		\
		: "=f" (__z)		\
		: "f" (x));		\
     __z; })
# endif
# ifndef __ceilf
#  define __ceilf(x)			\
    ({ float __z;			\
     __asm __volatile (			\
	"	frip %0,%1\n"		\
	"	frsp %0,%0\n"		\
		: "=f" (__z)		\
		: "f" (x));		\
     __z; })
# endif

# ifndef __floor
#  define __floor(x)			\
    ({ double __z;			\
     __asm __volatile (			\
	"	frim %0,%1\n"		\
		: "=f" (__z)		\
		: "f" (x));		\
     __z; })
# endif
# ifndef __floorf
#  define __floorf(x)			\
    ({ float __z;			\
     __asm __volatile (			\
	"	frim %0,%1\n"		\
	"	frsp %0,%0\n"		\
		: "=f" (__z)		\
		: "f" (x));		\
     __z; })
# endif

#endif	/* defined _ARCH_PWR5X */


#if defined _ARCH_PWR6

# ifndef __copysign
#  define __copysign(x, y)		\
    ({ double __z;			\
     __asm __volatile (			\
	"	fcpsgn %0,%1,%2\n"	\
		: "=f" (__z)		\
		: "f" (y), "f" (x));	\
     __z; })
# endif
# ifndef __copysignf
#  define __copysignf(x, y)		\
    ({ float __z;			\
     __asm __volatile (			\
	"	fcpsgn %0,%1,%2\n"	\
	"	frsp %0,%0\n"		\
		: "=f" (__z)		\
		: "f" (y), "f" (x));	\
     __z; })
# endif

#endif /* defined _ARCH_PWR6 */


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

#endif /* _PPC_MATH_PRIVATE_H_ */
