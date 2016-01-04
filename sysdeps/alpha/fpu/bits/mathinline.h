/* Inline math functions for Alpha.
   Copyright (C) 1996-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#ifndef _MATH_H
# error "Never use <bits/mathinline.h> directly; include <math.h> instead."
#endif

#ifndef __extern_inline
# define __MATH_INLINE __inline
#else
# define __MATH_INLINE __extern_inline
#endif

#if defined __USE_ISOC99 && defined __GNUC__ && !__GNUC_PREREQ(3,0)
# undef isgreater
# undef isgreaterequal
# undef isless
# undef islessequal
# undef islessgreater
# undef isunordered
# define isunordered(u, v)				\
  (__extension__					\
   ({ double __r, __u = (u), __v = (v);			\
      __asm ("cmptun/su %1,%2,%0\n\ttrapb"		\
	     : "=&f" (__r) : "f" (__u), "f"(__v));	\
      __r != 0; }))
#endif /* ISO C99 */

#if (!defined __NO_MATH_INLINES || defined __LIBC_INTERNAL_MATH_INLINES) \
    && defined __OPTIMIZE__

#if !__GNUC_PREREQ (4, 0)
# define __inline_copysign(NAME, TYPE)					\
__MATH_INLINE TYPE							\
__NTH (NAME (TYPE __x, TYPE __y))					\
{									\
  TYPE __z;								\
  __asm ("cpys %1, %2, %0" : "=f" (__z) : "f" (__y), "f" (__x));	\
  return __z;								\
}

__inline_copysign (__copysignf, float)
__inline_copysign (copysignf, float)
__inline_copysign (__copysign, double)
__inline_copysign (copysign, double)

# undef __inline_copysign
#endif


#if !__GNUC_PREREQ (2, 8)
# define __inline_fabs(NAME, TYPE)			\
__MATH_INLINE TYPE					\
__NTH (NAME (TYPE __x))					\
{							\
  TYPE __z;						\
  __asm ("cpys $f31, %1, %0" : "=f" (__z) : "f" (__x));	\
  return __z;						\
}

__inline_fabs (__fabsf, float)
__inline_fabs (fabsf, float)
__inline_fabs (__fabs, double)
__inline_fabs (fabs, double)

# undef __inline_fabs
#endif

#ifdef __USE_ISOC99

/* Test for negative number.  Used in the signbit() macro.  */
__MATH_INLINE int
__NTH (__signbitf (float __x))
{
#if !__GNUC_PREREQ (4, 0)
  __extension__ union { float __f; int __i; } __u = { __f: __x };
  return __u.__i < 0;
#else
  return __builtin_signbitf (__x);
#endif
}

__MATH_INLINE int
__NTH (__signbit (double __x))
{
#if !__GNUC_PREREQ (4, 0)
  __extension__ union { double __d; long __i; } __u = { __d: __x };
  return __u.__i < 0;
#else
  return __builtin_signbit (__x);
#endif
}

__MATH_INLINE int
__NTH (__signbitl (long double __x))
{
#if !__GNUC_PREREQ (4, 0)
  __extension__ union {
    long double __d;
    long __i[sizeof(long double)/sizeof(long)];
  } __u = { __d: __x };
  return __u.__i[sizeof(long double)/sizeof(long) - 1] < 0;
#else
  return __builtin_signbitl (__x);
#endif
}
#endif /* C99 */

#endif /* __NO_MATH_INLINES */
