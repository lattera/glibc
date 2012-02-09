/* Compute x * y + z as ternary operation.
   Copyright (C) 2010, 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2010.

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

#include <math.h>
#include <fenv.h>
#include <ieee754.h>
#include <math_private.h>

/* This implementation relies on double being more than twice as
   precise as float and uses rounding to odd in order to avoid problems
   with double rounding.
   See a paper by Boldo and Melquiond:
   http://www.lri.fr/~melquion/doc/08-tc.pdf  */

float
__fmaf (float x, float y, float z)
{
  fenv_t env;
  /* Multiplication is always exact.  */
  double temp = (double) x * (double) y;
  union ieee754_double u;
  libc_feholdexcept_setroundf (&env, FE_TOWARDZERO);
  /* Perform addition with round to odd.  */
  u.d = temp + (double) z;
  if ((u.ieee.mantissa1 & 1) == 0 && u.ieee.exponent != 0x7ff)
    u.ieee.mantissa1 |= libc_fetestexcept (FE_INEXACT) != 0;
  libc_feupdateenv (&env);
  /* And finally truncation with round to nearest.  */
  return (float) u.d;
}
#ifndef __fmaf
weak_alias (__fmaf, fmaf)
#endif
