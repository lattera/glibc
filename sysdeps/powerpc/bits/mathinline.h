/* Inline math functions for powerpc.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef __MATH_H
#define __MATH_H

#ifdef __GNUC__
#ifndef __NO_MATH_INLINES

#ifdef __cplusplus
#define        __MATH_INLINE __inline
#else
#define        __MATH_INLINE extern __inline
#endif

__MATH_INLINE double __sgn1 (double __x);
__MATH_INLINE double
__sgn1 (double __x)
{
  return __x >= 0.0 ? 1.0 : -1.0;
}

/* We'd want to use this if it was implemented in hardware, but
   how can we tell? */
#if 0
__MATH_INLINE double sqrt (double __x);
__MATH_INLINE double
sqrt (double __x)
{
  register double __value;
  __asm
    ("fsqrt %0,%1"
     : "=f" (__value) : "f" (__x));

  return __value;
}
#endif

__MATH_INLINE double fabs (double __x);
__MATH_INLINE double
fabs (double __x)
{
  register double __value;
  __asm
    ("fabs %0,%1"
     : "=f" (__value) : "f" (__x));

  return __value;
}

#endif /* __NO_MATH_INLINES  */
#endif /* __GNUC__  */

#endif /* __MATH_H  */
