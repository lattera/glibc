/* Copyright (C) 2007-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#define __llround	not___llround
#define llround		not_llround
#include <math.h>
#include <math_ldbl_opt.h>
#undef __llround
#undef llround

long int
__lround (double x)
{
  double adj, y;

  adj = copysign (0.5, x);
  asm("addt/suc %1,%2,%0" : "=&f"(y) : "f"(x), "f"(adj));
  return y;
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
