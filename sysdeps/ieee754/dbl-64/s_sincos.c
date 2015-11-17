/* Compute sine and cosine of argument.
   Copyright (C) 1997-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <math.h>

#include <math_private.h>

#define __sin __sin_local
#define __cos __cos_local
#define IN_SINCOS 1
#include "s_sin.c"

void
__sincos (double x, double *sinx, double *cosx)
{
  SET_RESTORE_ROUND_53BIT (FE_TONEAREST);

  *sinx = __sin (x);
  *cosx = __cos (x);
}
weak_alias (__sincos, sincos)
#ifdef NO_LONG_DOUBLE
strong_alias (__sincos, __sincosl)
weak_alias (__sincos, sincosl)
#endif
