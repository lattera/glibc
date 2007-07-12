/* Return nonzero value if number is negative.
   Copyright (C) 1997,1999,2004,2006 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

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

#include <math.h>
#include "math_private.h"
#include <math_ldbl_opt.h>

int
___signbitl (long double x)
{
  int64_t e;

  GET_LDOUBLE_MSW64 (e, x);
  return e < 0;
}
#ifdef IS_IN_libm
long_double_symbol (libm, ___signbitl, __signbitl);
#else
long_double_symbol (libc, ___signbitl, __signbitl);
#endif
