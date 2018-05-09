/* Private inline math functions for powerpc.
   Copyright (C) 2006-2018 Free Software Foundation, Inc.
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
#include <fenv_private.h>

#include_next <math_private.h>

#if defined _ARCH_PWR9 && __HAVE_DISTINCT_FLOAT128
extern __always_inline _Float128
__ieee754_sqrtf128 (_Float128 __x)
{
  _Float128 __z;
  asm ("xssqrtqp %0,%1" : "=v" (__z) : "v" (__x));
  return __z;
}
#endif

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

#endif /* _PPC_MATH_PRIVATE_H_ */
