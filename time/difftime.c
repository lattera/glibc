/* Copyright (C) 1991, 1994, 1996, 2004 Free Software Foundation, Inc.
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

/* Written by Paul Eggert <eggert@cs.ucla.edu>.  */

#include <time.h>

#include <limits.h>
#include <float.h>
#include <stdint.h>

#define TYPE_BITS(type) (sizeof (type) * CHAR_BIT)
#define TYPE_FLOATING(type) ((type) 0.5 == 0.5)
#define TYPE_SIGNED(type) ((type) -1 < 0)

/* Return the difference between TIME1 and TIME0, where TIME0 <= TIME1.
   time_t is known to be an integer type.  */

static double
subtract (time_t time1, time_t time0)
{
  if (! TYPE_SIGNED (time_t))
    return time1 - time0;
  else
    {
      /* Optimize the common special cases where time_t
	 can be converted to uintmax_t without losing information.  */
      uintmax_t dt = (uintmax_t) time1 - (uintmax_t) time0;
      double delta = dt;

      if (UINTMAX_MAX / 2 < INTMAX_MAX)
	{
	  /* This is a rare host where uintmax_t has padding bits, and possibly
	     information was lost when converting time_t to uintmax_t.
	     Check for overflow by comparing dt/2 to (time1/2 - time0/2).
	     Overflow occurred if they differ by more than a small slop.
	     Thanks to Clive D.W. Feather for detailed technical advice about
	     hosts with padding bits.

	     In the following code the "h" prefix means half.  By range
	     analysis, we have:

                  -0.5 <= ht1 - 0.5*time1 <= 0.5
                  -0.5 <= ht0 - 0.5*time0 <= 0.5
                  -1.0 <= dht - 0.5*(time1 - time0) <= 1.0

             If overflow has not occurred, we also have:

                  -0.5 <= hdt - 0.5*(time1 - time0) <= 0
                  -1.0 <= dht - hdt <= 1.5

             and since dht - hdt is an integer, we also have:

                  -1 <= dht - hdt <= 1

             or equivalently:

                  0 <= dht - hdt + 1 <= 2

             In the above analysis, all the operators have their exact
             mathematical semantics, not C semantics.  However, dht - hdt +
             1 is unsigned in C, so it need not be compared to zero.  */

	  uintmax_t hdt = dt / 2;
	  time_t ht1 = time1 / 2;
	  time_t ht0 = time0 / 2;
	  time_t dht = ht1 - ht0;

	  if (2 < dht - hdt + 1)
	    {
	      /* Repair delta overflow.

		 The following expression contains a second rounding,
		 so the result may not be the closest to the true answer.
		 This problem occurs only with very large differences.
		 It's too painful to fix this portably.  */

	      delta = dt + 2.0L * (UINTMAX_MAX - UINTMAX_MAX / 2);
	    }
	}

      return delta;
    }
}

/* Return the difference between TIME1 and TIME0.  */
double
__difftime (time_t time1, time_t time0)
{
  /* Convert to double and then subtract if no double-rounding error could
     result.  */

  if (TYPE_BITS (time_t) <= DBL_MANT_DIG
      || (TYPE_FLOATING (time_t) && sizeof (time_t) < sizeof (long double)))
    return (double) time1 - (double) time0;

  /* Likewise for long double.  */

  if (TYPE_BITS (time_t) <= LDBL_MANT_DIG || TYPE_FLOATING (time_t))
    return (long double) time1 - (long double) time0;

  /* Subtract the smaller integer from the larger, convert the difference to
     double, and then negate if needed.  */

  return time1 < time0 ? - subtract (time0, time1) : subtract (time1, time0);
}
strong_alias (__difftime, difftime)
