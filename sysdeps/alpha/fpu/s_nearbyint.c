/* Copyright (C) 2000-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Richard Henderson.

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

#include <math_ldbl_opt.h>

#include <sysdeps/ieee754/dbl-64/wordsize-64/s_nearbyint.c>

#if LONG_DOUBLE_COMPAT (libm, GLIBC_2_1)
compat_symbol (libm, __nearbyint, nearbyintl, GLIBC_2_1);
#endif
