/* Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.
Contributed by Ulrich Drepper <drepper@gnu.ai.mit.edu>, August 1995.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */

#include <stdlib.h>
#include "gmp.h"
#include "gmp-mparam.h"
#include <float.h>


/* Function to construct a floating point number from an MP integer
   containing the fraction bits, a base 2 exponent, and a sign flag.  */
extern double __mpn_construct_double (mp_srcptr mpn, int exponent, int neg);

int
erand48_r (xsubi, buffer, result)
     unsigned short int xsubi[3];
     struct drand48_data *buffer;
     double *result;
{
  mp_limb mpn[(3 * sizeof (unsigned short int) + sizeof (mp_limb) - 1)
	      / sizeof (mp_limb)];

  /* Compute next state.  */
  if (__drand48_iterate (xsubi, buffer) < 0)
    return -1;

  /* Build a 48-bit mpn containing the 48 random bits.  */

#if BITS_PER_MP_LIMB == 64
  mpn[0] = (xsubi[0] << 32) | (xsubi[1] << 16) | xsubi[2];
#elif BITS_PER_MP_LIMB == 32
  mpn[0] = (xsubi[1] << 16) | xsubi[2];
  mpn[1] = xsubi[0];
#else
 #error "BITS_PER_MP_LIMB value not handled"
#endif

  /* Shift them up so they are most significant bits of the fraction.  */
  __mpn_lshift (mpn, mpn, sizeof mpn / sizeof mpn[0], DBL_MANT_DIG - 48);

  /* Construct a positive double using those bits for the fractional part,
     and a zero exponent so the resulting FP number is [0.0,1.0).  */
  *result = __mpn_construct_double (mpn, 0, 0);

  return 0;
}
