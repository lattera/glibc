/* Copyright (C) 1995, 1996, 1997, 1998, 1999, 2002, 2003, 2004, 2006
	Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include "gmp.h"
#include "gmp-impl.h"
#include <ieee754.h>
#include <float.h>
#include <math.h>

/* Convert a multi-precision integer of the needed number of bits (106
   for long double) and an integral power of two to a `long double' in
   IBM extended format.  */

long double
__mpn_construct_long_double (mp_srcptr frac_ptr, int expt, int sign)
{
  union ibm_extended_long_double u;
  unsigned long hidden2, lzcount;
  unsigned long long hi, lo;

  u.ieee.negative = sign;
  u.ieee.negative2 = sign;
  u.ieee.exponent = expt + IBM_EXTENDED_LONG_DOUBLE_BIAS;
  u.ieee.exponent2 = expt - 53 + IBM_EXTENDED_LONG_DOUBLE_BIAS;

#if BITS_PER_MP_LIMB == 32
  /* The low order 53 bits (52 + hidden) go into the lower double */
  lo = frac_ptr[0];
  lo |= (frac_ptr[1] & ((1LL << (53 - 32)) - 1)) << 32;
  hidden2 = (frac_ptr[1] >> (52 - 32)) & ((mp_limb_t) 1);
  /* The high order 53 bits (52 + hidden) go into the upper double */
  hi = (frac_ptr[1] >> (53 - 32)) & ((1 << 11) - 1);
  hi |= ((unsigned long long) frac_ptr[2]) << 11;
  hi |= ((unsigned long long) frac_ptr[3]) << (32 + 11);
#elif BITS_PER_MP_LIMB == 64
  /* The low order 53 bits (52 + hidden) go into the lower double */
  lo = frac_ptr[0] & (((mp_limb_t) 1 << 53) - 1);
  hidden2 = (frac_ptr[0] >> 52) & ((mp_limb_t) 1);
  /* The high order 53 bits (52 + hidden) go into the upper double */
  hi = (frac_ptr[0] >> 53) & (((mp_limb_t) 1 << 11) - 1);
  hi |= (frac_ptr[1] << 11);
#else
  #error "mp_limb size " BITS_PER_MP_LIMB "not accounted for"
#endif

  if (lo != 0L)
    {
      /* hidden2 bit of low double controls rounding of the high double.
	 If hidden2 is '1' then round up hi and adjust lo (2nd mantissa)
	 plus change the sign of the low double to compensate.  */
      if (hidden2)
	{
	  hi++;
	  u.ieee.negative2 = !sign;
	  lo = (1LL << 53) - lo;
	}

      /* The hidden bit of the lo mantissa is zero so we need to normalize
	 it for the low double.  Shift it left until the hidden bit is '1'
	 then adjust the 2nd exponent accordingly.  */

      if (sizeof (lo) == sizeof (long))
	lzcount = __builtin_clzl (lo);
      else if ((lo >> 32) != 0)
	lzcount = __builtin_clzl ((long) (lo >> 32));
      else
	lzcount = __builtin_clzl ((long) lo) + 32;
      lzcount = lzcount - 11;
      if (lzcount > 0)
	{
	  lo = lo << lzcount;
	  u.ieee.exponent2 = u.ieee.exponent2 - lzcount;
	}
    }
  else
    {
      u.ieee.negative2 = 0;
      u.ieee.exponent2 = 0;
    }

  u.ieee.mantissa3 = lo & 0xffffffffLL;
  u.ieee.mantissa2 = (lo >> 32) & 0xffffff;
  u.ieee.mantissa1 = hi & 0xffffffffLL;
  u.ieee.mantissa0 = (hi >> 32) & ((1LL << (LDBL_MANT_DIG - 86)) - 1);

  return u.d;
}
