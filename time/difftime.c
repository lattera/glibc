/* Copyright (C) 1991, 1994, 1996 Free Software Foundation, Inc.
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

#include <time.h>
#include <values.h>


/* Return the difference between TIME1 and TIME0.  */
double
difftime (time1, time0)
     time_t time1;
     time_t time0;
{
  /* Algorithm courtesy Paul Eggert (eggert@twinsun.com).  */

  time_t delta, hibit;

  if (sizeof (time_t) < sizeof (double))
    return (double) time1 - (double) time0;
  if (sizeof (time_t) < sizeof (long double))
    return (long double) time1 - (long double) time0;

  if (time1 < time0)
    return - difftime (time0, time1);

  /* As much as possible, avoid loss of precision by computing the
    difference before converting to double.  */
  delta = time1 - time0;
  if (delta >= 0)
    return delta;

  /* Repair delta overflow.  */
  hibit = (~ (time_t) 0) << (_TYPEBITS (time_t) - 1);

  /* The following expression rounds twice, which means the result may not
     be the closest to the true answer.  For example, suppose time_t is
     64-bit signed int, long_double is IEEE 754 double with default
     rounding, time1 = 9223372036854775807 and time0 = -1536.  Then the
     true difference is 9223372036854777343, which rounds to
     9223372036854777856 with a total error of 513.  But delta overflows to
     -9223372036854774273, which rounds to -9223372036854774784, and
     correcting this by subtracting 2 * (long_double) hibit (i.e. by adding
     2**64 = 18446744073709551616) yields 9223372036854776832, which rounds
     to 9223372036854775808 with a total error of 1535 instead.  This
     problem occurs only with very large differences.  It's too painful to
     fix this portably.  We are not alone in this problem; many C compilers
     round twice when converting large unsigned types to small floating
     types, so if time_t is unsigned the "return delta" above has the same
     double-rounding problem.  */
  return delta - 2 * (long double) hibit;
}
