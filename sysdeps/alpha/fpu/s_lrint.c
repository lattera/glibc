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

#define __llrint	not___llrint
#define llrint		not_llrint
#include <math.h>
#include <math_ldbl_opt.h>
#undef __llrint
#undef llrint

long int
__lrint (double x)
{
  long ret;

  __asm ("cvttq/svd %1,%0" : "=&f"(ret) : "f"(x));

  return ret;
}

strong_alias (__lrint, __llrint)
weak_alias (__lrint, lrint)
weak_alias (__llrint, llrint)
#ifdef NO_LONG_DOUBLE
strong_alias (__lrint, __lrintl)
strong_alias (__lrint, __llrintl)
weak_alias (__lrintl, lrintl)
weak_alias (__llrintl, llrintl)
#endif
#if LONG_DOUBLE_COMPAT(libm, GLIBC_2_1)
compat_symbol (libm, __lrint, lrintl, GLIBC_2_1);
compat_symbol (libm, __llrint, llrintl, GLIBC_2_1);
#endif
