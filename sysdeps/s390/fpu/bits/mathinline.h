/* Inline math functions for s390.
   Copyright (C) 2004-2016 Free Software Foundation, Inc.
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

#ifndef _MATH_H
# error "Never use <bits/mathinline.h> directly; include <math.h> instead."
#endif

#ifndef __extern_inline
# define __MATH_INLINE __inline
#else
# define __MATH_INLINE __extern_inline
#endif

#if (!defined __NO_MATH_INLINES || defined __LIBC_INTERNAL_MATH_INLINES) \
    && defined __OPTIMIZE__

#ifdef __USE_ISOC99

/* Test for negative number.  Used in the signbit() macro.  */
__MATH_INLINE int
__NTH (__signbitf (float __x))
{
  __extension__ union { float __f; int __i; } __u = { __f: __x };
  return __u.__i < 0;
}

__MATH_INLINE int
__NTH (__signbit (double __x))
{
  __extension__ union { double __d; long __i; } __u = { __d: __x };
  return __u.__i < 0;
}

# ifndef __NO_LONG_DOUBLE_MATH
__MATH_INLINE int
__NTH (__signbitl (long double __x))
{
  __extension__ union { long double __l; int __i[4]; } __u = { __l: __x };
  return __u.__i[0] < 0;
}
# else
__MATH_INLINE int
__NTH (__signbitl (long double __x))
{
  return __signbit ((double) __x);
}
# endif

#endif /* C99 */

/* This code is used internally in the GNU libc.  */
#ifdef __LIBC_INTERNAL_MATH_INLINES

__MATH_INLINE double
__NTH (__ieee754_sqrt (double x))
{
  double res;

  __asm__ ( "sqdbr %0,%1" : "=f" (res) : "f" (x) );
  return res;
}

__MATH_INLINE float
__NTH (__ieee754_sqrtf (float x))
{
  float res;

  __asm__ ( "sqebr %0,%1" : "=f" (res) : "f" (x) );
  return res;
}

# if !defined __NO_LONG_DOUBLE_MATH
__MATH_INLINE long double
__NTH (sqrtl (long double __x))
{
  long double res;

  __asm__ ( "sqxbr %0,%1" : "=f" (res) : "f" (__x) );
  return res;
}
# endif /* !__NO_LONG_DOUBLE_MATH */

#endif /* __LIBC_INTERNAL_MATH_INLINES */

#endif /* __NO_MATH_INLINES */
