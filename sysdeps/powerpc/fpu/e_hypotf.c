/* Pythagorean addition using floats
   Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Adhemerval Zanella <azanella@br.ibm.com>, 2011

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include "math.h"
#include "math_private.h"


static const float two30  = 1.0737418e09;
static const float two50  = 1.1259000e15;
static const float two60  = 1.1529221e18;
static const float two126 = 8.5070592e+37;
static const float twoM50 = 8.8817842e-16;
static const float twoM60 = 6.7762644e-21;
static const float pdnum  = 1.1754939e-38;


/* __ieee754_hypotf(x,y)
 *
 * This a FP only version without any FP->INT conversion.
 * It is similar to default C version, making appropriates
 * overflow and underflows checks as well scaling when it
 * is needed.
 */

#ifdef _ARCH_PWR7
/* POWER7 isinf and isnan optimizations are fast. */
# define TEST_INF_NAN(x, y)                                      \
   if (isinff(x) || isinff(y))                                   \
     return INFINITY;                                            \
   if (isnanf(x) || isnanf(y))                                   \
     return NAN;
# else
/* For POWER6 and below isinf/isnan triggers LHS and PLT calls are
 * costly (especially for POWER6). */
# define GET_TWO_FLOAT_WORD(f1,f2,i1,i2)                         \
 do {                                                            \
   ieee_float_shape_type gf_u1;                                  \
   ieee_float_shape_type gf_u2;                                  \
   gf_u1.value = (f1);                                           \
   gf_u2.value = (f2);                                           \
   (i1) = gf_u1.word;                                            \
   (i2) = gf_u2.word;                                            \
 } while (0)

# define TEST_INF_NAN(x, y)                                      \
 do {                                                            \
   int32_t hx, hy;                                               \
   GET_TWO_FLOAT_WORD(x, y, hx, hy);                             \
   if (hy > hx) {                                                \
     uint32_t ht = hx; hx = hy; hy = ht;                         \
   }                                                             \
   if (hx >= 0x7f800000) {                                       \
     if (hx == 0x7f800000 || hy == 0x7f800000)                   \
       return INFINITY;                                          \
     return NAN;                                                 \
   }                                                             \
 } while (0)
#endif


float
__ieee754_hypotf (float x, float y)
{
  x = fabsf (x);
  y = fabsf (y);

  TEST_INF_NAN (x, y);

  if (y > x)
    {
      float t = y;
      y = x;
      x = t;
    }
  if (y == 0.0 || (x / y) > two30)
    {
      return x + y;
    }
  if (x > two50)
    {
      x *= twoM60;
      y *= twoM60;
      return __ieee754_sqrtf (x * x + y * y) / twoM60;
    }
  if (y < twoM50)
    {
      if (y <= pdnum)
	{
	  x *= two126;
	  y *= two126;
	  return __ieee754_sqrtf (x * x + y * y) / two126;
	}
      else
	{
	  x *= two60;
	  y *= two60;
	  return __ieee754_sqrtf (x * x + y * y) / two60;
	}
    }
  return __ieee754_sqrtf (x * x + y * y);
}
strong_alias (__ieee754_hypotf, __hypotf_finite)
