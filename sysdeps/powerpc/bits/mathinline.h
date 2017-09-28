/* Inline math functions for powerpc.
   Copyright (C) 1995-2017 Free Software Foundation, Inc.
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
#endif  /* __cplusplus */

#if defined __GNUC__ && !defined _SOFT_FLOAT && !defined __NO_FPRS__

#ifdef __USE_ISOC99
/* The gcc, version 2.7 or below, has problems with all this inlining
   code.  So disable it for this version of the compiler.  */
# if __GNUC_PREREQ (2, 8)
/* Test for negative number.  Used in the signbit() macro.  */
__MATH_INLINE int
__NTH (__signbitf (float __x))
{
#if __GNUC_PREREQ (4, 0)
  return __builtin_signbitf (__x);
#else
  __extension__ union { float __f; int __i; } __u = { __f: __x };
  return __u.__i < 0;
#endif
}
__MATH_INLINE int
__NTH (__signbit (double __x))
{
#if __GNUC_PREREQ (4, 0)
  return __builtin_signbit (__x);
#else
  __extension__ union { double __d; long long __i; } __u = { __d: __x };
  return __u.__i < 0;
#endif
}
#  ifdef __LONG_DOUBLE_128__
__MATH_INLINE int
__NTH (__signbitl (long double __x))
{
  return __signbit ((double) __x);
}
#  endif
# endif
#endif /* __USE_ISOC99 */

#if !defined __NO_MATH_INLINES && defined __OPTIMIZE__

#ifdef __USE_ISOC99

# ifndef __powerpc64__
__MATH_INLINE long int lrint (double __x) __THROW;
__MATH_INLINE long int
__NTH (lrint (double __x))
{
  union {
    double __d;
    long long __ll;
  } __u;
  __asm__ ("fctiw %0,%1" : "=f"(__u.__d) : "f"(__x));
  return __u.__ll;
}

__MATH_INLINE long int lrintf (float __x) __THROW;
__MATH_INLINE long int
__NTH (lrintf (float __x))
{
  return lrint ((double) __x);
}
# endif

__MATH_INLINE double fdim (double __x, double __y) __THROW;
__MATH_INLINE double
__NTH (fdim (double __x, double __y))
{
  return __x <= __y ? 0 : __x - __y;
}

__MATH_INLINE float fdimf (float __x, float __y) __THROW;
__MATH_INLINE float
__NTH (fdimf (float __x, float __y))
{
  return __x <= __y ? 0 : __x - __y;
}

#endif /* __USE_ISOC99 */
#endif /* !__NO_MATH_INLINES && __OPTIMIZE__ */
#endif /* __GNUC__ && !_SOFT_FLOAT && !__NO_FPRS__ */
