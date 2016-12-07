/* Pythagorean addition using doubles
   Copyright (C) 2011-2016 Free Software Foundation, Inc.
   This file is part of the GNU C Library
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
   License along with the GNU C Library; see the file COPYING.LIB.  If
   not, see <http://www.gnu.org/licenses/>.  */

#include <math.h>
#include <math_private.h>
#include <stdint.h>

static const double two60   = 1.152921504606847e+18;
static const double two500  = 3.2733906078961419e+150;
static const double two600  = 4.149515568880993e+180;
static const double two1022 = 4.49423283715579e+307;
static const double twoM500 = 3.054936363499605e-151;
static const double twoM600 = 2.4099198651028841e-181;
static const double two60factor = 1.5592502418239997e+290;
static const double pdnum   = 2.225073858507201e-308;

/* __ieee754_hypot(x,y)
 *
 * This a FP only version without any FP->INT conversion.
 * It is similar to default C version, making appropriates
 * overflow and underflows checks as well scaling when it
 * is needed.
 */

#ifdef _ARCH_PWR7
/* POWER7 isinf and isnan optimization are fast. */
# define TEST_INF_NAN(x, y)                                       \
   if ((isinf(x) || isinf(y))					  \
       && !issignaling (x) && !issignaling (y))			  \
       return INFINITY;                                           \
   if (isnan(x) || isnan(y))                                      \
       return x + y;
# else
/* For POWER6 and below isinf/isnan triggers LHS and PLT calls are
 * costly (especially for POWER6). */
# define GET_TW0_HIGH_WORD(d1,d2,i1,i2)                           \
 do {                                                             \
   ieee_double_shape_type gh_u1;                                  \
   ieee_double_shape_type gh_u2;                                  \
   gh_u1.value = (d1);                                            \
   gh_u2.value = (d2);                                            \
   (i1) = gh_u1.parts.msw & 0x7fffffff;                           \
   (i2) = gh_u2.parts.msw & 0x7fffffff;                           \
 } while (0)

# define TEST_INF_NAN(x, y)                                      \
 do {                                                            \
   uint32_t hx, hy;                                              \
   GET_TW0_HIGH_WORD(x, y, hx, hy);                              \
   if (hy > hx) {                                                \
     uint32_t ht = hx; hx = hy; hy = ht;                         \
   }                                                             \
   if (hx >= 0x7ff00000) {                                       \
     if ((hx == 0x7ff00000 || hy == 0x7ff00000)			 \
	 && !issignaling (x) && !issignaling (y))		 \
       return INFINITY;                                          \
     return x + y;						 \
   }                                                             \
 } while (0)

#endif


double
__ieee754_hypot (double x, double y)
{
  x = fabs (x);
  y = fabs (y);

  TEST_INF_NAN (x, y);

  if (y > x)
    {
      double t = x;
      x = y;
      y = t;
    }
  if (y == 0.0)
    return x;
  /* if y is higher enough, y * 2^60 might overflow. The tests if
     y >= 1.7976931348623157e+308/2^60 (two60factor) and uses the
     appropriate check to avoid the overflow exception generation.  */
  if (y > two60factor)
    {
      if ((x / y) > two60)
	return x + y;
    }
  else
    {
      if (x > (y * two60))
	return x + y;
    }
  if (x > two500)
    {
      x *= twoM600;
      y *= twoM600;
      return __ieee754_sqrt (x * x + y * y) / twoM600;
    }
  if (y < twoM500)
    {
      if (y <= pdnum)
	{
	  x *= two1022;
	  y *= two1022;
	  double ret = __ieee754_sqrt (x * x + y * y) / two1022;
	  math_check_force_underflow_nonneg (ret);
	  return ret;
	}
      else
	{
	  x *= two600;
	  y *= two600;
	  return __ieee754_sqrt (x * x + y * y) / two600;
	}
    }
  return __ieee754_sqrt (x * x + y * y);
}
strong_alias (__ieee754_hypot, __hypot_finite)
