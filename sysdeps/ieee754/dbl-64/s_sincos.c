/* Compute sine and cosine of argument.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.
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
#include <libm-alias-double.h>

#define __sin __sin_local
#define __cos __cos_local
#define IN_SINCOS 1
#include "s_sin.c"

/* Consolidated version of reduce_and_compute in s_sin.c that does range
   reduction only once and computes sin and cos together.  */
static inline void
__always_inline
reduce_and_compute_sincos (double x, double *sinx, double *cosx)
{
  double a, da;
  unsigned int n = __branred (x, &a, &da);

  n = n & 3;

  if (n == 1 || n == 2)
    {
      a = -a;
      da = -da;
    }

  if (n & 1)
    {
      double *temp = cosx;
      cosx = sinx;
      sinx = temp;
    }

  if (a * a < 0.01588)
    *sinx = bsloww (a, da, x, n);
  else
    *sinx = bsloww1 (a, da, x, n);
  *cosx = bsloww2 (a, da, x, n);
}

void
__sincos (double x, double *sinx, double *cosx)
{
  mynumber u;
  int k;

  SET_RESTORE_ROUND_53BIT (FE_TONEAREST);

  u.x = x;
  k = 0x7fffffff & u.i[HIGH_HALF];

  if (k < 0x400368fd)
    {
      *sinx = __sin_local (x);
      *cosx = __cos_local (x);
      return;
    }
  if (k < 0x419921FB)
    {
      double a, da;
      int4 n = reduce_sincos_1 (x, &a, &da);

      *sinx = do_sincos_1 (a, da, x, n, false);
      *cosx = do_sincos_1 (a, da, x, n, true);

      return;
    }
  if (k < 0x42F00000)
    {
      double a, da;
      int4 n = reduce_sincos_2 (x, &a, &da);

      *sinx = do_sincos_2 (a, da, x, n, false);
      *cosx = do_sincos_2 (a, da, x, n, true);

      return;
    }
  if (k < 0x7ff00000)
    {
      reduce_and_compute_sincos (x, sinx, cosx);
      return;
    }

  if (isinf (x))
    __set_errno (EDOM);

  *sinx = *cosx = x / x;
}
libm_alias_double (__sincos, sincos)
