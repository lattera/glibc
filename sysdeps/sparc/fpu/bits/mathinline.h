/* Inline math functions for SPARC.
   Copyright (C) 1999-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>.

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

#include <bits/wordsize.h>

#ifdef __GNUC__

#if (!defined __NO_MATH_INLINES || defined __LIBC_INTERNAL_MATH_INLINES) && defined __OPTIMIZE__

# ifndef __extern_inline
#  define __MATH_INLINE __inline
# else
#  define __MATH_INLINE __extern_inline
# endif  /* __cplusplus */

/* The gcc, version 2.7 or below, has problems with all this inlining
   code.  So disable it for this version of the compiler.  */
# if __GNUC_PREREQ (2, 8)

#  if !defined __NO_MATH_INLINES && !__GNUC_PREREQ (3, 2)

__MATH_INLINE double
__NTH (sqrt (double __x))
{
  register double __r;
  __asm ("fsqrtd %1,%0" : "=f" (__r) : "f" (__x));
  return __r;
}

__MATH_INLINE float
__NTH (sqrtf (float __x))
{
  register float __r;
  __asm ("fsqrts %1,%0" : "=f" (__r) : "f" (__x));
  return __r;
}

#   if __WORDSIZE == 64
__MATH_INLINE long double
__NTH (sqrtl (long double __x))
{
  long double __r;
  extern void _Qp_sqrt (long double *, const long double *);
  _Qp_sqrt (&__r, &__x);
  return __r;
}
#   elif !defined __NO_LONG_DOUBLE_MATH
__MATH_INLINE long double
sqrtl (long double __x) __THROW
{
  extern long double _Q_sqrt (const long double);
  return _Q_sqrt (__x);
}
#   endif /* sparc64 */

#  endif /* !__NO_MATH_INLINES && !GCC 3.2+ */

/* This code is used internally in the GNU libc.  */
#  ifdef __LIBC_INTERNAL_MATH_INLINES
__MATH_INLINE double
__ieee754_sqrt (double __x)
{
  register double __r;
  __asm ("fsqrtd %1,%0" : "=f" (__r) : "f" (__x));
  return __r;
}

__MATH_INLINE float
__ieee754_sqrtf (float __x)
{
  register float __r;
  __asm ("fsqrts %1,%0" : "=f" (__r) : "f" (__x));
  return __r;
}

#   if __WORDSIZE == 64
__MATH_INLINE long double
__ieee754_sqrtl (long double __x)
{
  long double __r;
  extern void _Qp_sqrt (long double *, const long double *);
  _Qp_sqrt(&__r, &__x);
  return __r;
}
#   elif !defined __NO_LONG_DOUBLE_MATH
__MATH_INLINE long double
__ieee754_sqrtl (long double __x)
{
  extern long double _Q_sqrt (const long double);
  return _Q_sqrt (__x);
}
#   endif /* sparc64 */
#  endif /* __LIBC_INTERNAL_MATH_INLINES */
# endif /* gcc 2.8+ */

# ifdef __USE_ISOC99

#  ifndef __NO_MATH_INLINES

__MATH_INLINE double __NTH (fdim (double __x, double __y));
__MATH_INLINE double
__NTH (fdim (double __x, double __y))
{
  return __x <= __y ? 0 : __x - __y;
}

__MATH_INLINE float __NTH (fdimf (float __x, float __y));
__MATH_INLINE float
__NTH (fdimf (float __x, float __y))
{
  return __x <= __y ? 0 : __x - __y;
}

#  endif /* !__NO_MATH_INLINES */
# endif /* __USE_ISOC99 */
#endif /* !__NO_MATH_INLINES && __OPTIMIZE__ */
#endif /* __GNUC__ */
