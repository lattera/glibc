/* Miscellaneous tests which don't fit anywhere else.
   Copyright (C) 2000 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#include <fenv.h>
#include <math.h>
#include <stdio.h>
#include <string.h>


int
main (void)
{
  int result = 0;

#ifndef NO_LONG_DOUBLE
  {
    long double x = 0x100000001ll + (long double) 0.5;
    long double q;
    long double r;

    r = modfl (x, &q);
    if (q != (long double) 0x100000001ll || r != 0.5)
      {
	printf ("modfl (%Lg, ...) failed\n", x);
	result = 1;
      }
  }

# if __GNUC__ >= 3 || __GNUC_MINOR__ >= 96
  {
    long double x;
    long double m;
    long double r;
    int e;
    int i;

#  if LDBL_MANT_DIG == 64
    m = 0xf.fffffffffffffffp-4L;
#  else
#   error "Please adjust"
#  endif

    for (i = LDBL_MAX_EXP, x = LDBL_MAX; i >= LDBL_MIN_EXP; --i, x /= 2.0L)
      {
	printf ("2^%d: ", i);

	r = frexpl (x, &e);
	if (r != m)
	  {
	    printf ("mantissa incorrect: %.20La\n", r);
	    result = 1;
	    continue;
	  }
	if (e != i)
	  {
	    printf ("exponent wrong %d (%.20Lg)\n", e, x);
	    result = 1;
	    continue;
	  }
	puts ("ok");
      }
  }
# endif

#if 0
  {
    int e;
    long double r = frexpl (LDBL_MIN * LDBL_EPSILON, &e);

    if (r != 0.5)
      {
	printf ("frexpl (LDBL_MIN * LDBL_EPSILON, ...): mantissa wrong: %Lg\n",
		r);
	result = 1;
      }
    else if (e != -16444)
      {
	printf ("frexpl (LDBL_MIN * LDBL_EPSILON, ...): exponent wrong: %d\n",
		e);
	result = 1;
      }
  }
#endif
#endif

  {
    double x = 0x100000001ll + (double) 0.5;
    double q;
    double r;

    r = modf (x, &q);
    if (q != (double) 0x100000001ll || r != 0.5)
      {
	printf ("modf (%g, ...) failed\n", x);
	result = 1;
      }
  }

  if (fpclassify (FLT_MIN) != FP_NORMAL)
    {
      printf ("fpclassify (FLT_MIN) failed: %d\n", fpclassify (FLT_MIN));
      result = 1;
    }
  if (fpclassify (nextafterf (FLT_MIN, FLT_MIN / 2.0f)) != FP_SUBNORMAL)
    {
      printf ("fpclassify (FLT_MIN-epsilon) failed: %d\n",
	      fpclassify (nextafterf (FLT_MIN, FLT_MIN / 2.0f)));
      result = 1;
    }
  if (fpclassify (DBL_MIN) != FP_NORMAL)
    {
      printf ("fpclassify (DBL_MIN) failed: %d\n", fpclassify (DBL_MIN));
      result = 1;
    }
  if (fpclassify (nextafter (DBL_MIN, DBL_MIN / 2.0)) != FP_SUBNORMAL)
    {
      printf ("fpclassify (DBL_MIN-epsilon) failed: %d\n",
	      fpclassify (nextafter (DBL_MIN, DBL_MIN / 2.0)));
      result = 1;
    }
#ifndef NO_LONG_DOUBLE
  if (fpclassify (LDBL_MIN) != FP_NORMAL)
    {
      printf ("fpclassify (LDBL_MIN) failed: %d\n", fpclassify (LDBL_MIN));
      result = 1;
    }
  if (fpclassify (nextafterl (LDBL_MIN, LDBL_MIN / 2.0)) != FP_SUBNORMAL)
    {
      printf ("fpclassify (LDBL_MIN-epsilon) failed: %d (%Lg)\n",
	      fpclassify (nextafterl (LDBL_MIN, LDBL_MIN / 2.0)),
	      nextafterl (LDBL_MIN, LDBL_MIN / 2.0));
      result = 1;
    }
#endif

  if (! isnormal (FLT_MIN))
    {
      puts ("isnormal (FLT_MIN) failed");
      result = 1;
    }
  if (! isnormal (DBL_MIN))
    {
      puts ("isnormal (DBL_MIN) failed");
      result = 1;
    }
#ifndef NO_LONG_DOUBLE
  if (! isnormal (LDBL_MIN))
    {
      puts ("isnormal (LDBL_MIN) failed");
      result = 1;
    }
#endif

#ifdef __i386__
  /* This is a test for the strange long doubles in x86 FPUs.  */
  {
    union
    {
      char b[10];
      long double d;
    } u =
      { .b = { 0, 0, 0, 0, 0, 0, 0, 0x80, 0, 0 } };

    if (fpclassify (u.d) != FP_NORMAL)
      {
	printf ("fpclassify (0x00008000000000000000) failed: %d (%Lg)\n",
		fpclassify (u.d), u.d);
	result = 1;
      }
  }

  /* Special NaNs in x86 long double.  Test for scalbl.  */
  {
    union
    {
      char b[10];
      long double d;
    } u =
      { .b = { 0, 1, 0, 0, 0, 0, 0, 0xc0, 0xff, 0x7f } };
    long double r;

    r = scalbl (u.d, 0.0);
    if (!isnan (r))
      {
	puts ("scalbl(NaN, 0) does not return NaN");
	result = 1;
      }
    else if (memcmp (&r, &u.d, sizeof (double)) != 0)
      {
	puts ("scalbl(NaN, 0) does not return the same NaN");
	result = 1;
      }
  }
#endif

#ifndef NO_LONG_DOUBLE
  {
    long double r;

    feclearexcept (FE_ALL_EXCEPT);
    r = scalbl (LDBL_MIN, 2147483647);
    if (! isinf (r))
      {
	puts ("scalbl (LDBL_MIN, 2147483647) does not return Inf");
	result = 1;
      }
    else if (signbit (r) != 0)
      {
	puts ("scalbl (LDBL_MIN, 2147483647) returns -Inf");
	result = 1;
      }
    else if (fetestexcept (FE_UNDERFLOW))
      {
	puts ("scalbl(NaN, 0) raises underflow exception");
	result = 1;
      }

    feclearexcept (FE_ALL_EXCEPT);
    r = scalbl (LDBL_MAX, -2147483647);
    if (r != 0.0)
      {
	puts ("scalbl (LDBL_MAX, -2147483647) does not return 0");
	result = 1;
      }
    else if (signbit (r) != 0)
      {
	puts ("scalbl (LDBL_MAX, -2147483647) returns -Inf");
	result = 1;
      }
    else if (fetestexcept (FE_OVERFLOW))
      {
	puts ("scalbl(NaN, 0) raises overflow exception");
	result = 1;
      }
  }
#endif

  return result;
}
