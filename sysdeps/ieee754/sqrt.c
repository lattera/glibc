/*
 * Copyright (c) 1985 Regents of the University of California.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted provided
 * that: (1) source distributions retain this entire copyright notice and
 * comment, and (2) distributions including binaries display the following
 * acknowledgement:  ``This product includes software developed by the
 * University of California, Berkeley and its contributors'' in the
 * documentation or other materials provided with the distribution and in
 * all advertising materials mentioning features or use of this software.
 * Neither the name of the University nor the names of its contributors may
 * be used to endorse or promote products derived from this software without
 * specific prior written permission.
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

#include <ansidecl.h>
#include <errno.h>
#include <math.h>

/* Return the square root of X.  */
double
DEFUN (sqrt, (x), double x)
{
  double q, s, b, r, t;
  CONST double zero = 0.0;
  int m, n, i;

  /* sqrt (NaN) is NaN; sqrt (+-0) is +-0.  */
  if (__isnan (x) || x == zero)
    return x;

  if (x < zero)
    return zero / zero;

  /* sqrt (Inf) is Inf.  */
  if (__isinf (x))
    return x;

  /* Scale X to [1,4).  */
  n = __logb (x);
  x = __scalb (x, -n);
  m = __logb (x);
  if (m != 0)
    /* Subnormal number.  */
    x = __scalb (x, -m);

  m += n;
  n = m / 2;

  if ((n + n) != m)
    {
      x *= 2;
      --m;
      n = m / 2;
    }

  /* Generate sqrt (X) bit by bit (accumulating in Q).  */
  q = 1.0;
  s = 4.0;
  x -= 1.0;
  r = 1;
  for (i = 1; i <= 51; i++)
    {
      t = s + 1;
      x *= 4;
      r /= 2;
      if (t <= x)
	{
	  s = t + t + 2, x -= t;
	  q += r;
	}
      else
	s *= 2;
    }

  /* Generate the last bit and determine the final rounding.  */
  r /= 2;
  x *= 4;
  if (x == zero)
    goto end;
  (void) (100 + r);		/* Trigger inexact flag.  */
  if (s < x)
    {
      q += r;
      x -= s;
      s += 2;
      s *= 2;
      x *= 4;
      t = (x - s) - 5;
      b = 1.0 + 3 * r / 4;
      if (b == 1.0)
	goto end;		/* B == 1: Round to zero.  */
      b = 1.0 + r / 4;
      if (b > 1.0)
	t = 1;			/* B > 1: Round to +Inf.  */
      if (t >= 0)
	q += r;
    }				/* Else round to nearest.  */
  else
    {
      s *= 2;
      x *= 4;
      t = (x - s) - 1;
      b = 1.0 + 3 * r / 4;
      if (b == 1.0)
	goto end;
      b = 1.0 + r / 4;
      if (b > 1.0)
	t = 1;
      if (t >= 0)
	q += r;
    }

end:
  return __scalb (q, n);
}
