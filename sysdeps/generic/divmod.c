/* __mpn_divmod -- Divide natural numbers, producing both remainder and
   quotient.

Copyright (C) 1993, 1994 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.

You should have received a copy of the GNU Library General Public License
along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

#include "gmp.h"
#include "gmp-impl.h"
#include "longlong.h"

/* Divide num (NUM_PTR/NUM_SIZE) by den (DEN_PTR/DEN_SIZE) and write
   the NUM_SIZE-DEN_SIZE least significant quotient limbs at QUOT_PTR
   and the DEN_SIZE long remainder at NUM_PTR.
   Return the most significant limb of the quotient, this is always 0 or 1.

   Argument constraints:
   1. The most significant bit of the divisor must be set.
   2. QUOT_PTR must either not overlap with the input operands at all, or
      QUOT_PTR + DEN_SIZE >= NUM_PTR must hold true.  (This means that it's
      possible to put the quotient in the high part of NUM, right after the
      remainder in NUM.  */

mp_limb
#if __STDC__
__mpn_divmod (mp_ptr quot_ptr,
	      mp_ptr num_ptr, mp_size_t num_size,
	      mp_srcptr den_ptr, mp_size_t den_size)
#else
__mpn_divmod (quot_ptr, num_ptr, num_size, den_ptr, den_size)
     mp_ptr quot_ptr;
     mp_ptr num_ptr;
     mp_size_t num_size;
     mp_srcptr den_ptr;
     mp_size_t den_size;
#endif
{
  mp_limb most_significant_q_limb = 0;

  switch (den_size)
    {
    case 0:
      /* We are asked to divide by zero, so go ahead and do it!  (To make
	 the compiler not remove this statement, return the value.)  */
      return 1 / den_size;

    case 1:
      {
	mp_size_t i;
	mp_limb n1, n0;
	mp_limb d;

	d = den_ptr[0];
	n1 = num_ptr[num_size - 1];

	if (n1 >= d)
	  {
	    most_significant_q_limb = 1;
	    n1 -= d;
	  }

	for (i = num_size - 2; i >= 0; i--)
	  {
	    n0 = num_ptr[i];
	    udiv_qrnnd (quot_ptr[i], n1, n1, n0, d);
	  }

	num_ptr[0] = n1;
      }
      break;

    case 2:
      {
	mp_size_t i;
	mp_limb n1, n0, n2;
	mp_limb d1, d0;

	num_ptr += num_size - 2;
	d1 = den_ptr[1];
	d0 = den_ptr[0];
	n1 = num_ptr[1];
	n0 = num_ptr[0];

	if (n1 >= d1 && (n1 > d1 || n0 >= d0))
	  {
	    most_significant_q_limb = 1;
	    sub_ddmmss (n1, n0, n1, n0, d1, d0);
	  }

	for (i = num_size - den_size - 1; i >= 0; i--)
	  {
	    mp_limb q;
	    mp_limb r;

	    num_ptr--;
	    if (n1 == d1)
	      {
		/* Q should be either 111..111 or 111..110.  Need special
		   treatment of this rare case as normal division would
		   give overflow.  */
		q = ~(mp_limb) 0;

		r = n0 + d1;
		if (r < d1)	/* Carry in the addition? */
		  {
		    add_ssaaaa (n1, n0, r - d0, num_ptr[0], 0, d0);
		    quot_ptr[i] = q;
		    continue;
		  }
		n1 = d0 - (d0 != 0);
		n0 = -d0;
	      }
	    else
	      {
		udiv_qrnnd (q, r, n1, n0, d1);
		umul_ppmm (n1, n0, d0, q);
	      }

	    n2 = num_ptr[0];
	  q_test:
	    if (n1 > r || (n1 == r && n0 > n2))
	      {
		/* The estimated Q was too large.  */
		q--;

		sub_ddmmss (n1, n0, n1, n0, 0, d0);
		r += d1;
		if (r >= d1)	/* If not carry, test Q again.  */
		  goto q_test;
	      }

	    quot_ptr[i] = q;
	    sub_ddmmss (n1, n0, r, n2, n1, n0);
	  }
	num_ptr[1] = n1;
	num_ptr[0] = n0;
      }
      break;

    default:
      {
	mp_size_t i;
	mp_limb dX, d1, n0;

	num_ptr += num_size;
	den_ptr += den_size;
	dX = den_ptr[-1];
	d1 = den_ptr[-2];
	n0 = num_ptr[-1];

	if (n0 >= dX)
	  {
	    if (n0 > dX
		|| __mpn_cmp (num_ptr - den_size, den_ptr - den_size,
			      den_size - 1) >= 0)
	      {
		__mpn_sub_n (num_ptr - den_size,
			     num_ptr - den_size, den_ptr - den_size,
			     den_size);
		most_significant_q_limb = 1;
	      }

	    n0 = num_ptr[-1];
	  }

	for (i = num_size - den_size - 1; i >= 0; i--)
	  {
	    mp_limb q;
	    mp_limb n1;
	    mp_limb cy_limb;

	    num_ptr--;
	    if (n0 == dX)
	      /* This might over-estimate q, but it's probably not worth
		 the extra code here to find out.  */
	      q = ~(mp_limb) 0;
	    else
	      {
		mp_limb r;

		udiv_qrnnd (q, r, n0, num_ptr[-1], dX);
		umul_ppmm (n1, n0, d1, q);

		while (n1 > r || (n1 == r && n0 > num_ptr[-2]))
		  {
		    q--;
		    r += dX;
		    if (r < dX)	/* I.e. "carry in previous addition?"  */
		      break;
		    n1 -= n0 < d1;
		    n0 -= d1;
		  }
	      }

	    /* Possible optimization: We already have (q * n0) and (1 * n1)
	       after the calculation of q.  Taking advantage of that, we
	       could make this loop make two iterations less.  */

	    cy_limb = __mpn_submul_1 (num_ptr - den_size,
				      den_ptr - den_size, den_size, q);

	    if (num_ptr[0] != cy_limb)
	      {
		mp_limb cy;
		cy = __mpn_add_n (num_ptr - den_size,
				  num_ptr - den_size,
				  den_ptr - den_size, den_size);
		if (cy == 0)
		  abort ();
		q--;
	      }

	    quot_ptr[i] = q;
	    n0 = num_ptr[-1];
	  }
      }
    }

  return most_significant_q_limb;
}
