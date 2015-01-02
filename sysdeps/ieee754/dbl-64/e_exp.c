/*
 * IBM Accurate Mathematical Library
 * written by International Business Machines Corp.
 * Copyright (C) 2001-2015 Free Software Foundation, Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 */
/***************************************************************************/
/*  MODULE_NAME:uexp.c                                                     */
/*                                                                         */
/*  FUNCTION:uexp                                                          */
/*           exp1                                                          */
/*                                                                         */
/* FILES NEEDED:dla.h endian.h mpa.h mydefs.h uexp.h                       */
/*              mpa.c mpexp.x slowexp.c                                    */
/*                                                                         */
/* An ultimate exp routine. Given an IEEE double machine number x          */
/* it computes the correctly rounded (to nearest) value of e^x             */
/* Assumption: Machine arithmetic operations are performed in              */
/* round to nearest mode of IEEE 754 standard.                             */
/*                                                                         */
/***************************************************************************/

#include <math.h>
#include "endian.h"
#include "uexp.h"
#include "mydefs.h"
#include "MathLib.h"
#include "uexp.tbl"
#include <math_private.h>
#include <fenv.h>
#include <float.h>

#ifndef SECTION
# define SECTION
#endif

double __slowexp (double);

/* An ultimate exp routine. Given an IEEE double machine number x it computes
   the correctly rounded (to nearest) value of e^x.  */
double
SECTION
__ieee754_exp (double x)
{
  double bexp, t, eps, del, base, y, al, bet, res, rem, cor;
  mynumber junk1, junk2, binexp = {{0, 0}};
  int4 i, j, m, n, ex;
  double retval;

  {
    SET_RESTORE_ROUND (FE_TONEAREST);

    junk1.x = x;
    m = junk1.i[HIGH_HALF];
    n = m & hugeint;

    if (n > smallint && n < bigint)
      {
	y = x * log2e.x + three51.x;
	bexp = y - three51.x;	/*  multiply the result by 2**bexp        */

	junk1.x = y;

	eps = bexp * ln_two2.x;	/* x = bexp*ln(2) + t - eps               */
	t = x - bexp * ln_two1.x;

	y = t + three33.x;
	base = y - three33.x;	/* t rounded to a multiple of 2**-18      */
	junk2.x = y;
	del = (t - base) - eps;	/*  x = bexp*ln(2) + base + del           */
	eps = del + del * del * (p3.x * del + p2.x);

	binexp.i[HIGH_HALF] = (junk1.i[LOW_HALF] + 1023) << 20;

	i = ((junk2.i[LOW_HALF] >> 8) & 0xfffffffe) + 356;
	j = (junk2.i[LOW_HALF] & 511) << 1;

	al = coar.x[i] * fine.x[j];
	bet = ((coar.x[i] * fine.x[j + 1] + coar.x[i + 1] * fine.x[j])
	       + coar.x[i + 1] * fine.x[j + 1]);

	rem = (bet + bet * eps) + al * eps;
	res = al + rem;
	cor = (al - res) + rem;
	if (res == (res + cor * err_0))
	  {
	    retval = res * binexp.x;
	    goto ret;
	  }
	else
	  {
	    retval = __slowexp (x);
	    goto ret;
	  }			/*if error is over bound */
      }

    if (n <= smallint)
      {
	retval = 1.0;
	goto ret;
      }

    if (n >= badint)
      {
	if (n > infint)
	  {
	    retval = x + x;
	    goto ret;
	  }			/* x is NaN */
	if (n < infint)
	  {
	    if (x > 0)
	      goto ret_huge;
	    else
	      goto ret_tiny;
	  }
	/* x is finite,  cause either overflow or underflow  */
	if (junk1.i[LOW_HALF] != 0)
	  {
	    retval = x + x;
	    goto ret;
	  }			/*  x is NaN  */
	retval = (x > 0) ? inf.x : zero;	/* |x| = inf;  return either inf or 0 */
	goto ret;
      }

    y = x * log2e.x + three51.x;
    bexp = y - three51.x;
    junk1.x = y;
    eps = bexp * ln_two2.x;
    t = x - bexp * ln_two1.x;
    y = t + three33.x;
    base = y - three33.x;
    junk2.x = y;
    del = (t - base) - eps;
    eps = del + del * del * (p3.x * del + p2.x);
    i = ((junk2.i[LOW_HALF] >> 8) & 0xfffffffe) + 356;
    j = (junk2.i[LOW_HALF] & 511) << 1;
    al = coar.x[i] * fine.x[j];
    bet = ((coar.x[i] * fine.x[j + 1] + coar.x[i + 1] * fine.x[j])
	   + coar.x[i + 1] * fine.x[j + 1]);
    rem = (bet + bet * eps) + al * eps;
    res = al + rem;
    cor = (al - res) + rem;
    if (m >> 31)
      {
	ex = junk1.i[LOW_HALF];
	if (res < 1.0)
	  {
	    res += res;
	    cor += cor;
	    ex -= 1;
	  }
	if (ex >= -1022)
	  {
	    binexp.i[HIGH_HALF] = (1023 + ex) << 20;
	    if (res == (res + cor * err_0))
	      {
		retval = res * binexp.x;
		goto ret;
	      }
	    else
	      {
		retval = __slowexp (x);
		goto check_uflow_ret;
	      }			/*if error is over bound */
	  }
	ex = -(1022 + ex);
	binexp.i[HIGH_HALF] = (1023 - ex) << 20;
	res *= binexp.x;
	cor *= binexp.x;
	eps = 1.0000000001 + err_0 * binexp.x;
	t = 1.0 + res;
	y = ((1.0 - t) + res) + cor;
	res = t + y;
	cor = (t - res) + y;
	if (res == (res + eps * cor))
	  {
	    binexp.i[HIGH_HALF] = 0x00100000;
	    retval = (res - 1.0) * binexp.x;
	    goto check_uflow_ret;
	  }
	else
	  {
	    retval = __slowexp (x);
	    goto check_uflow_ret;
	  }			/*   if error is over bound    */
      check_uflow_ret:
	if (retval < DBL_MIN)
	  {
#if FLT_EVAL_METHOD != 0
	    volatile
#endif
	      double force_underflow = tiny * tiny;
	    math_force_eval (force_underflow);
	  }
	if (retval == 0)
	  goto ret_tiny;
	goto ret;
      }
    else
      {
	binexp.i[HIGH_HALF] = (junk1.i[LOW_HALF] + 767) << 20;
	if (res == (res + cor * err_0))
	  retval = res * binexp.x * t256.x;
	else
	  retval = __slowexp (x);
	if (__isinf (retval))
	  goto ret_huge;
	else
	  goto ret;
      }
  }
ret:
  return retval;

 ret_huge:
  return hhuge * hhuge;

 ret_tiny:
  return tiny * tiny;
}
#ifndef __ieee754_exp
strong_alias (__ieee754_exp, __exp_finite)
#endif

/* Compute e^(x+xx).  The routine also receives bound of error of previous
   calculation.  If after computing exp the error exceeds the allowed bounds,
   the routine returns a non-positive number.  Otherwise it returns the
   computed result, which is always positive.  */
double
SECTION
__exp1 (double x, double xx, double error)
{
  double bexp, t, eps, del, base, y, al, bet, res, rem, cor;
  mynumber junk1, junk2, binexp = {{0, 0}};
  int4 i, j, m, n, ex;

  junk1.x = x;
  m = junk1.i[HIGH_HALF];
  n = m & hugeint;		/* no sign */

  if (n > smallint && n < bigint)
    {
      y = x * log2e.x + three51.x;
      bexp = y - three51.x;	/*  multiply the result by 2**bexp        */

      junk1.x = y;

      eps = bexp * ln_two2.x;	/* x = bexp*ln(2) + t - eps               */
      t = x - bexp * ln_two1.x;

      y = t + three33.x;
      base = y - three33.x;	/* t rounded to a multiple of 2**-18      */
      junk2.x = y;
      del = (t - base) + (xx - eps);	/*  x = bexp*ln(2) + base + del      */
      eps = del + del * del * (p3.x * del + p2.x);

      binexp.i[HIGH_HALF] = (junk1.i[LOW_HALF] + 1023) << 20;

      i = ((junk2.i[LOW_HALF] >> 8) & 0xfffffffe) + 356;
      j = (junk2.i[LOW_HALF] & 511) << 1;

      al = coar.x[i] * fine.x[j];
      bet = ((coar.x[i] * fine.x[j + 1] + coar.x[i + 1] * fine.x[j])
	     + coar.x[i + 1] * fine.x[j + 1]);

      rem = (bet + bet * eps) + al * eps;
      res = al + rem;
      cor = (al - res) + rem;
      if (res == (res + cor * (1.0 + error + err_1)))
	return res * binexp.x;
      else
	return -10.0;
    }

  if (n <= smallint)
    return 1.0;			/*  if x->0 e^x=1 */

  if (n >= badint)
    {
      if (n > infint)
	return (zero / zero);	/* x is NaN,  return invalid */
      if (n < infint)
	return ((x > 0) ? (hhuge * hhuge) : (tiny * tiny));
      /* x is finite,  cause either overflow or underflow  */
      if (junk1.i[LOW_HALF] != 0)
	return (zero / zero);	/*  x is NaN  */
      return ((x > 0) ? inf.x : zero);	/* |x| = inf;  return either inf or 0 */
    }

  y = x * log2e.x + three51.x;
  bexp = y - three51.x;
  junk1.x = y;
  eps = bexp * ln_two2.x;
  t = x - bexp * ln_two1.x;
  y = t + three33.x;
  base = y - three33.x;
  junk2.x = y;
  del = (t - base) + (xx - eps);
  eps = del + del * del * (p3.x * del + p2.x);
  i = ((junk2.i[LOW_HALF] >> 8) & 0xfffffffe) + 356;
  j = (junk2.i[LOW_HALF] & 511) << 1;
  al = coar.x[i] * fine.x[j];
  bet = ((coar.x[i] * fine.x[j + 1] + coar.x[i + 1] * fine.x[j])
	 + coar.x[i + 1] * fine.x[j + 1]);
  rem = (bet + bet * eps) + al * eps;
  res = al + rem;
  cor = (al - res) + rem;
  if (m >> 31)
    {
      ex = junk1.i[LOW_HALF];
      if (res < 1.0)
	{
	  res += res;
	  cor += cor;
	  ex -= 1;
	}
      if (ex >= -1022)
	{
	  binexp.i[HIGH_HALF] = (1023 + ex) << 20;
	  if (res == (res + cor * (1.0 + error + err_1)))
	    return res * binexp.x;
	  else
	    return -10.0;
	}
      ex = -(1022 + ex);
      binexp.i[HIGH_HALF] = (1023 - ex) << 20;
      res *= binexp.x;
      cor *= binexp.x;
      eps = 1.00000000001 + (error + err_1) * binexp.x;
      t = 1.0 + res;
      y = ((1.0 - t) + res) + cor;
      res = t + y;
      cor = (t - res) + y;
      if (res == (res + eps * cor))
	{
	  binexp.i[HIGH_HALF] = 0x00100000;
	  return (res - 1.0) * binexp.x;
	}
      else
	return -10.0;
    }
  else
    {
      binexp.i[HIGH_HALF] = (junk1.i[LOW_HALF] + 767) << 20;
      if (res == (res + cor * (1.0 + error + err_1)))
	return res * binexp.x * t256.x;
      else
	return -10.0;
    }
}
