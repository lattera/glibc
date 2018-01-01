/* Overridable constants and operations.
   Copyright (C) 2013-2018 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as published by
   the Free Software Foundation; either version 2.1 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License
   along with this program; if not, see <http://www.gnu.org/licenses/>.  */

#include <stdint.h>

typedef long mantissa_t;
typedef int64_t mantissa_store_t;

#define TWOPOW(i) (1L << i)

#define RADIX_EXP 24
#define  RADIX TWOPOW (RADIX_EXP)               /* 2^24    */

/* Divide D by RADIX and put the remainder in R.  D must be a non-negative
   integral value.  */
#define DIV_RADIX(d, r) \
  ({                                                                         \
     r = d & (RADIX - 1);                                                    \
     d >>= RADIX_EXP;                                                        \
   })

/* Put the integer component of a double X in R and retain the fraction in
   X.  This is used in extracting mantissa digits for MP_NO by using the
   integer portion of the current value of the number as the current mantissa
   digit and then scaling by RADIX to get the next mantissa digit in the same
   manner.  */
#define INTEGER_OF(x, i) \
  ({                                                                          \
     i = (mantissa_t) x;                                                       \
     x -= i;                                                                   \
   })

/* Align IN down to F.  The code assumes that F is a power of two.  */
#define ALIGN_DOWN_TO(in, f) ((in) & - (f))
