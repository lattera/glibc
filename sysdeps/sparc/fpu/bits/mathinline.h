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
