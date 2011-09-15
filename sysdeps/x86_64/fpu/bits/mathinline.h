/* Inline math functions for x86-64.
   Copyright (C) 2002-2004, 2007, 2009, 2011 Free Software Foundation, Inc.
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

#ifndef _MATH_H
# error "Never use <bits/mathinline.h> directly; include <math.h> instead."
#endif

#include <bits/wordsize.h>

#ifndef __extern_always_inline
# define __MATH_INLINE __inline
#else
# define __MATH_INLINE __extern_always_inline
#endif


#if defined __USE_ISOC99 && defined __GNUC__ && __GNUC__ >= 2

/* Test for negative number.  Used in the signbit() macro.  */
__MATH_INLINE int
__NTH (__signbitf (float __x))
{
# if __WORDSIZE == 32
  __extension__ union { float __f; int __i; } __u = { __f: __x };
  return __u.__i < 0;
# else
  int __m;
  __asm ("pmovmskb %1, %0" : "=r" (__m) : "x" (__x));
  return __m & 0x8;
# endif
}
__MATH_INLINE int
__NTH (__signbit (double __x))
{
# if __WORDSIZE == 32
  __extension__ union { double __d; int __i[2]; } __u = { __d: __x };
  return __u.__i[1] < 0;
# else
  int __m;
  __asm ("pmovmskb %1, %0" : "=r" (__m) : "x" (__x));
  return __m & 0x80;
# endif
}
__MATH_INLINE int
__NTH (__signbitl (long double __x))
{
  __extension__ union { long double __l; int __i[3]; } __u = { __l: __x };
  return (__u.__i[2] & 0x8000) != 0;
}

#ifdef __USE_ISOC99
__BEGIN_NAMESPACE_C99

/* Round to nearest integer.  */
#  if __WORDSIZE == 64 || defined __SSE_MATH__
__MATH_INLINE long int
__NTH (lrintf (float __x))
{
  long int __res;
  asm ("cvtss2si %1, %0" : "=r" (__res) : "xm" (__x));
  return __res;
}
#  endif
#  if __WORDSIZE == 64 || defined __SSE2_MATH__
__MATH_INLINE long int
__NTH (lrint (double __x))
{
  long int __res;
  asm ("cvtsd2si %1, %0" : "=r" (__res) : "xm" (__x));
  return __res;
}
#  endif
#  if __WORDSIZE == 64
__MATH_INLINE long long int
__NTH (llrintf (float __x))
{
  long long int __res;
  asm ("cvtss2si %1, %0" : "=r" (__res) : "xm" (__x));
  return __res;
}
__MATH_INLINE long long int
__NTH (llrint (double __x))
{
  long long int __res;
  asm ("cvtsd2si %1, %0" : "=r" (__res) : "xm" (__x));
  return __res;
}
#  endif

#  if __FINITE_MATH_ONLY__ == 1 && (__WORDSIZE == 64 || defined __SSE2_MATH__)
/* Determine maximum of two values.  */
__MATH_INLINE float
__NTH (fmaxf (float __x, float __y))
{
  asm ("maxss %1, %0" : "+x" (__x) : "xm" (__y));
  return __x;
}
__MATH_INLINE double
__NTH (fmax (double __x, double __y))
{
  asm ("maxsd %1, %0" : "+x" (__x) : "xm" (__y));
  return __x;
}

/* Determine minimum of two values.  */
__MATH_INLINE float
__NTH (fminf (float __x, float __y))
{
  asm ("minss %1, %0" : "+x" (__x) : "xm" (__y));
  return __x;
}
__MATH_INLINE double
__NTH (fmin (double __x, double __y))
{
  asm ("minsd %1, %0" : "+x" (__x) : "xm" (__y));
  return __x;
}
#  endif

__END_NAMESPACE_C99
# endif

#endif
