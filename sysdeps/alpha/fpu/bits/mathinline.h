/* Inline math functions for Alpha.
   Copyright (C) 1996, 1997, 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by David Mosberger-Tang.

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

#ifdef __cplusplus
# define __MATH_INLINE __inline
#else
# define __MATH_INLINE extern __inline
#endif

#ifdef __USE_ISOC99
# define isunordered(x, y)				\
  (__extension__					\
   ({ double __r;					\
      __asm ("cmptun/su %1,%2,%0\n\ttrapb"		\
	     : "=&f" (__r) : "f" (x), "f"(y));		\
      __r != 0; }))

# define isgreater(x, y)				\
  (__extension__					\
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);	\
      !isunordered(__x, __y) && __x > __y; }))
# define isgreaterequal(x, y)				\
  (__extension__					\
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);	\
      !isunordered(__x, __y) && __x >= __y; }))
# define isless(x, y)					\
  (__extension__					\
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);	\
      !isunordered(__x, __y) && __x < __y; }))
# define islessequal(x, y)				\
  (__extension__					\
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);	\
      !isunordered(__x, __y) && __x <= __y; }))
# define islessgreater(x, y)				\
  (__extension__					\
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);	\
      !isunordered(__x, __y) && __x != __y; }))
#endif /* ISO C99 */

#if !defined __NO_MATH_INLINES && defined __OPTIMIZE__

#define __inline_copysign(NAME, TYPE)					\
__MATH_INLINE TYPE							\
NAME (TYPE __x, TYPE __y) __THROW					\
{									\
  TYPE __z;								\
  __asm ("cpys %1, %2, %0" : "=f" (__z) : "f" (__y), "f" (__x));	\
  return __z;								\
}

__inline_copysign(__copysignf, float)
__inline_copysign(copysignf, float)
__inline_copysign(__copysign, double)
__inline_copysign(copysign, double)

#undef __MATH_INLINE_copysign


#if __GNUC_PREREQ (2, 8)
__MATH_INLINE float __fabsf (float __x) __THROW { return __builtin_fabsf (__x); }
__MATH_INLINE float fabsf (float __x) __THROW { return __builtin_fabsf (__x); }
__MATH_INLINE double __fabs (double __x) __THROW { return __builtin_fabs (__x); }
__MATH_INLINE double fabs (double __x) __THROW { return __builtin_fabs (__x); }
#else
#define __inline_fabs(NAME, TYPE)			\
__MATH_INLINE TYPE					\
NAME (TYPE __x) __THROW					\
{							\
  TYPE __z;						\
  __asm ("cpys $f31, %1, %0" : "=f" (__z) : "f" (__x));	\
  return __z;						\
}

__inline_fabs(__fabsf, float)
__inline_fabs(fabsf, float)
__inline_fabs(__fabs, double)
__inline_fabs(fabs, double)

#undef __inline_fabs
#endif


/* Use the -inf rounding mode conversion instructions to implement
   floor.  We note when the exponent is large enough that the value
   must be integral, as this avoids unpleasant integer overflows.  */

__MATH_INLINE float
__floorf (float __x) __THROW
{
  /* Check not zero since floor(-0) == -0.  */
  if (__x != 0 && fabsf (__x) < 16777216.0f)  /* 1 << FLT_MANT_DIG */
    {
      /* Note that Alpha S_Floating is stored in registers in a
	 restricted T_Floating format, so we don't even need to
	 convert back to S_Floating in the end.  The initial
	 conversion to T_Floating is needed to handle denormals.  */

      float __tmp1, __tmp2;

      __asm ("cvtst/s %3,%2\n\t"
#ifdef _IEEE_FP_INEXACT
	     "cvttq/svim %2,%1\n\t"
#else
	     "cvttq/svm %2,%1\n\t"
#endif
	     "cvtqt/m %1,%0\n\t"
	     : "=f"(__x), "=&f"(__tmp1), "=&f"(__tmp2)
	     : "f"(__x));
    }
  return __x;
}

__MATH_INLINE double
__floor (double __x) __THROW
{
  if (__x != 0 && fabs (__x) < 9007199254740992.0)  /* 1 << DBL_MANT_DIG */
    {
      double __tmp1;
      __asm (
#ifdef _IEEE_FP_INEXACT
	     "cvttq/svim %2,%1\n\t"
#else
	     "cvttq/svm %2,%1\n\t"
#endif
	     "cvtqt/m %1,%0\n\t"
	     : "=f"(__x), "=&f"(__tmp1)
	     : "f"(__x));
    }
  return __x;
}

__MATH_INLINE float floorf (float __x) __THROW { return __floorf(__x); }
__MATH_INLINE double floor (double __x) __THROW { return __floor(__x); }


#ifdef __USE_ISOC99

__MATH_INLINE float __fdimf (float __x, float __y) __THROW
{
  return __x < __y ? 0.0f : __x - __y;
}

__MATH_INLINE float fdimf (float __x, float __y) __THROW
{
  return __x < __y ? 0.0f : __x - __y;
}

__MATH_INLINE double __fdim (double __x, double __y) __THROW
{
  return __x < __y ? 0.0 : __x - __y;
}

__MATH_INLINE double fdim (double __x, double __y) __THROW
{
  return __x < __y ? 0.0 : __x - __y;
}

#endif /* C99 */

#endif /* __NO_MATH_INLINES */
