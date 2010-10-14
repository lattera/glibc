/* Compute x * y + z as ternary operation.
   Copyright (C) 2010 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <math.h>
#include <fenv.h>
#include <ieee754.h>

/* This implementation relies on long double being more than twice as
   precise as double and uses rounding to odd in order to avoid problems
   with double rounding.
   See a paper by Boldo and Melquiond:
   http://www.lri.fr/~melquion/doc/08-tc.pdf  */

double
__fma (double x, double y, double z)
{
  fenv_t env;
  /* Multiplication is always exact.  */
  long double temp = (long double) x * (long double) y;
  union ieee854_long_double u;
  feholdexcept (&env);
  fesetround (FE_TOWARDZERO);
  /* Perform addition with round to odd.  */
  u.d = temp + (long double) z;
  if ((u.ieee.mantissa3 & 1) == 0 && u.ieee.exponent != 0x7fff)
    u.ieee.mantissa3 |= fetestexcept (FE_INEXACT) != 0;
  feupdateenv (&env);
  /* And finally truncation with round to nearest.  */
  return (double) u.d;
}
#ifndef __fma
weak_alias (__fma, fma)
#endif
