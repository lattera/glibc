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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <math.h>
#include <math_ldbl_opt.h>


double
__round (double x)
{
  const double almost_half = 0x1.fffffffffffffp-2;
  const double two52 = 0x1.0p52;
  double tmp, r;

  __asm (
#ifdef _IEEE_FP_INEXACT
	 "addt/suic %2, %3, %1\n\tsubt/suic %1, %3, %0"
#else
	 "addt/suc %2, %3, %1\n\tsubt/suc %1, %3, %0"
#endif
	 : "=&f"(r), "=&f"(tmp)
	 : "f"(fabs (x) + almost_half), "f"(two52));

  return copysign (r, x);
}

weak_alias (__round, round)
#ifdef NO_LONG_DOUBLE
strong_alias (__round, __roundl)
weak_alias (__roundl, roundl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __round, roundl, GLIBC_2_1);
#endif
