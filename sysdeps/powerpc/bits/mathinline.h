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

#ifdef __GNUC__
#if !defined __NO_MATH_INLINES && defined __OPTIMIZE__

#ifdef __cplusplus
# define __MATH_INLINE __inline
#else
# define __MATH_INLINE extern __inline
#endif

__MATH_INLINE double __sgn1 (double __x);
__MATH_INLINE double
__sgn1 (double __x)
{
  return __x >= 0.0 ? 1.0 : -1.0;
}
#endif /* __NO_MATH_INLINES && __OPTIMZE__ */

#if __USE_ISOC9X
# define __unordered_cmp(x, y) \
  (__extension__							      \
   ({ __typeof__(x) __x = (x); __typeof__(y) __y = (y);			      \
      unsigned __r;							      \
      __asm__("fcmpu 7,%1,%2 ; mfcr %0" : "=r" (__r) : "f" (__x), "f"(__y));  \
      __r; }))

# define isgreater(x, y) (__unordered_cmp (x, y) >> 2 & 1)
# define isgreaterequal(x, y) ((__unordered_cmp (x, y) & 6) != 0)
# define isless(x, y) (__unordered_cmp (x, y) >> 3 & 1)
# define islessequal(x, y) ((__unordered_cmp (x, y) & 0xA) != 0)
# define islessgreater(x, y) ((__unordered_cmp (x, y) & 0xC) != 0)
# define isunordered(x, y) (__unordered_cmp (x, y) & 1)
#endif /* __USE_ISOC9X */

#endif /* __GNUC__  */
