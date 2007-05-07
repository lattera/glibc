/* Copyright (C) 2007 Free Software Foundation, Inc.
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

#define __llround	not___llround
#define llround		not_llround
#include <math.h>
#include <math_ldbl_opt.h>
#undef __llround
#undef llround

long int
__lround (double x)
{
  double adj;

  adj = 0x1.fffffffffffffp-2;	/* nextafter (0.5, 0.0) */
  adj = copysign (adj, x);
  return x + adj;
}

strong_alias (__lround, __llround)
weak_alias (__lround, lround)
weak_alias (__llround, llround)
#ifdef NO_LONG_DOUBLE
strong_alias (__lround, __lroundl)
strong_alias (__lround, __llroundl)
weak_alias (__lroundl, lroundl)
weak_alias (__llroundl, llroundl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __lround, lroundl, GLIBC_2_1);
compat_symbol (libm, __llround, llroundl, GLIBC_2_1);
#endif
