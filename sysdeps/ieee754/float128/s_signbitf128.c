/* Return nonzero value if number is negative.
   Copyright (C) 2017-2018 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <float128_private.h>
#include <math.h>
#include <math_private.h>

/* Once GCC >= 6.0 is required for building glibc, this implementation can
   be removed and replaced with an inclusion of ldbl-128/s_signbitl.c.  */
int
__signbitf128 (_Float128 x)
{
#if __GNUC_PREREQ (6, 0)
  return __builtin_signbit (x);
#else
  int64_t e;

  GET_FLOAT128_MSW64 (e, x);
  return e < 0;
#endif
}
hidden_def (__signbitf128)
