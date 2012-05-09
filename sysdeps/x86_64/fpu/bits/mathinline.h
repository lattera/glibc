/* Inline math functions for x86-64.
   Copyright (C) 2002-2004,2007,2009,2011,2012 Free Software Foundation, Inc.
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

#ifndef __extern_always_inline
# define __MATH_INLINE __inline
#else
# define __MATH_INLINE __extern_always_inline
#endif


/* The gcc, version 2.7 or below, has problems with all this inlining
   code.  So disable it for this version of the compiler.  */
#if __GNUC_PREREQ (2, 8) && defined __USE_ISOC99
__BEGIN_NAMESPACE_C99

/* Test for negative number.  Used in the signbit() macro.  */
__MATH_INLINE int
__NTH (__signbitf (float __x))
{
# ifndef __x86_64__
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
# ifndef __x86_64__
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
  return __u.__i[2] & 0x8000;
}

__END_NAMESPACE_C99
#endif


#if __GNUC_PREREQ (2, 8) && !defined __NO_MATH_INLINES && defined __OPTIMIZE__

# ifdef __USE_ISOC99
__BEGIN_NAMESPACE_C99

/* Round to nearest integer.  */
#  ifdef __SSE_MATH__
__MATH_INLINE long int
__NTH (lrintf (float __x))
{
  long int __res;
  /* Mark as volatile since the result is dependend on the state of
     the SSE control register (the rounding mode). Otherwise GCC might
     remove these assembler instructions since it does not know about
     the rounding mode change and cannot currently be told.  */
  __asm __volatile__ ("cvtss2si %1, %0" : "=r" (__res) : "xm" (__x));
  return __res;
}
#  endif
#  ifdef __SSE2_MATH__
__MATH_INLINE long int
__NTH (lrint (double __x))
{
  long int __res;
  /* Mark as volatile since the result is dependend on the state of
     the SSE control register (the rounding mode). Otherwise GCC might
     remove these assembler instructions since it does not know about
     the rounding mode change and cannot currently be told.  */
  __asm __volatile__ ("cvtsd2si %1, %0" : "=r" (__res) : "xm" (__x));
  return __res;
}
#  endif
#  ifdef __x86_64__
__MATH_INLINE long long int
__NTH (llrintf (float __x))
{
  long long int __res;
  /* Mark as volatile since the result is dependend on the state of
     the SSE control register (the rounding mode). Otherwise GCC might
     remove these assembler instructions since it does not know about
     the rounding mode change and cannot currently be told.  */
  __asm __volatile__ ("cvtss2si %1, %0" : "=r" (__res) : "xm" (__x));
  return __res;
}
__MATH_INLINE long long int
__NTH (llrint (double __x))
{
  long long int __res;
  /* Mark as volatile since the result is dependend on the state of
     the SSE control register (the rounding mode). Otherwise GCC might
     remove these assembler instructions since it does not know about
     the rounding mode change and cannot currently be told.  */
  __asm __volatile__ ("cvtsd2si %1, %0" : "=r" (__res) : "xm" (__x));
  return __res;
}
#  endif

#  if defined __FINITE_MATH_ONLY__ && __FINITE_MATH_ONLY__ > 0 \
      && defined __SSE2_MATH__
/* Determine maximum of two values.  */
__MATH_INLINE float
__NTH (fmaxf (float __x, float __y))
{
#   ifdef __AVX__
  float __res;
  __asm ("vmaxss %2, %1, %0" : "=x" (__res) : "x" (x), "xm" (__y));
  return __res;
#   else
  __asm ("maxss %1, %0" : "+x" (__x) : "xm" (__y));
  return __x;
#   endif
}
__MATH_INLINE double
__NTH (fmax (double __x, double __y))
{
#   ifdef __AVX__
  float __res;
  __asm ("vmaxsd %2, %1, %0" : "=x" (__res) : "x" (x), "xm" (__y));
  return __res;
#   else
  __asm ("maxsd %1, %0" : "+x" (__x) : "xm" (__y));
  return __x;
#   endif
}

/* Determine minimum of two values.  */
__MATH_INLINE float
__NTH (fminf (float __x, float __y))
{
#   ifdef __AVX__
  float __res;
  __asm ("vminss %2, %1, %0" : "=x" (__res) : "x" (x), "xm" (__y));
  return __res;
#   else
  __asm ("minss %1, %0" : "+x" (__x) : "xm" (__y));
  return __x;
#   endif
}
__MATH_INLINE double
__NTH (fmin (double __x, double __y))
{
#   ifdef __AVX__
  float __res;
  __asm ("vminsd %2, %1, %0" : "=x" (__res) : "x" (x), "xm" (__y));
  return __res;
#   else
  __asm ("minsd %1, %0" : "+x" (__x) : "xm" (__y));
  return __x;
#   endif
}
#  endif

__END_NAMESPACE_C99
# endif

# if defined __SSE4_1__ && defined __SSE2_MATH__
#  if defined __USE_MISC || defined __USE_XOPEN_EXTENDED || defined __USE_ISOC99
__BEGIN_NAMESPACE_C99

/* Round to nearest integer.  */
__MATH_INLINE double
__NTH (rint (double __x))
{
  double __res;
  /* Mark as volatile since the result is dependend on the state of
     the SSE control register (the rounding mode). Otherwise GCC might
     remove these assembler instructions since it does not know about
     the rounding mode change and cannot currently be told.  */
  __asm __volatile__ ("roundsd $4, %1, %0" : "=x" (__res) : "xm" (__x));
  return __res;
}
__MATH_INLINE float
__NTH (rintf (float __x))
{
  float __res;
  /* Mark as volatile since the result is dependend on the state of
     the SSE control register (the rounding mode). Otherwise GCC might
     remove these assembler instructions since it does not know about
     the rounding mode change and cannot currently be told.  */
  __asm __volatile__ ("roundss $4, %1, %0" : "=x" (__res) : "xm" (__x));
  return __res;
}

#   ifdef __USE_ISOC99
/* Round to nearest integer without raising inexact exception.  */
__MATH_INLINE double
__NTH (nearbyint (double __x))
{
  double __res;
  /* Mark as volatile since the result is dependend on the state of
     the SSE control register (the rounding mode). Otherwise GCC might
     remove these assembler instructions since it does not know about
     the rounding mode change and cannot currently be told.  */
  __asm __volatile__ ("roundsd $0xc, %1, %0" : "=x" (__res) : "xm" (__x));
  return __res;
}
__MATH_INLINE float
__NTH (nearbyintf (float __x))
{
  float __res;
  /* Mark as volatile since the result is dependend on the state of
     the SSE control register (the rounding mode). Otherwise GCC might
     remove these assembler instructions since it does not know about
     the rounding mode change and cannot currently be told.  */
  __asm __volatile__ ("roundss $0xc, %1, %0" : "=x" (__res) : "xm" (__x));
  return __res;
}
#   endif

__END_NAMESPACE_C99
#  endif

__BEGIN_NAMESPACE_STD
/* Smallest integral value not less than X.  */
__MATH_INLINE double
__NTH (ceil (double __x))
{
  double __res;
  __asm ("roundsd $2, %1, %0" : "=x" (__res) : "xm" (__x));
  return __res;
}
__END_NAMESPACE_STD

__BEGIN_NAMESPACE_C99
__MATH_INLINE float
__NTH (ceilf (float __x))
{
  float __res;
  __asm ("roundss $2, %1, %0" : "=x" (__res) : "xm" (__x));
  return __res;
}
__END_NAMESPACE_C99

__BEGIN_NAMESPACE_STD
/* Largest integer not greater than X.  */
__MATH_INLINE double
__NTH (floor (double __x))
{
  double __res;
  __asm ("roundsd $1, %1, %0" : "=x" (__res) : "xm" (__x));
  return __res;
}
__END_NAMESPACE_STD

__BEGIN_NAMESPACE_C99
__MATH_INLINE float
__NTH (floorf (float __x))
{
  float __res;
  __asm ("roundss $1, %1, %0" : "=x" (__res) : "xm" (__x));
  return __res;
}
__END_NAMESPACE_C99
# endif

#endif
