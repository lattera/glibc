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

typedef double mantissa_t;
typedef double mantissa_store_t;

#define TWOPOW(i) (0x1.0p##i)

#define RADIX TWOPOW (24)		/* 2^24    */
#define CUTTER TWOPOW (76)		/* 2^76    */
#define RADIXI 0x1.0p-24		/* 2^-24 */
#define TWO52 TWOPOW (52)		/* 2^52 */

/* Divide D by RADIX and put the remainder in R.  */
#define DIV_RADIX(d,r) \
  ({									      \
    double u = ((d) + CUTTER) - CUTTER;					      \
    if (u > (d))							      \
      u -= RADIX;							      \
    r = (d) - u;							      \
    (d) = u * RADIXI;							      \
  })

/* Put the integer component of a double X in R and retain the fraction in
   X.  */
#define INTEGER_OF(x, r) \
  ({									      \
    double u = ((x) + TWO52) - TWO52;					      \
    if (u > (x))							      \
      u -= 1;								      \
    (r) = u;								      \
    (x) -= u;								      \
  })

/* Align IN down to a multiple of F, where F is a power of two.  */
#define ALIGN_DOWN_TO(in, f) \
  ({									      \
    double factor = f * TWO52;						      \
    double u = (in + factor) - factor;					      \
    if (u > in)								      \
      u -= f;								      \
    u;									      \
  })
