/* Test compilation of tgmath macros.
   Copyright (C) 2001, 2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com> and
   Ulrich Drepper <drepper@redhat.com>, 2001.

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

#ifndef HAVE_MAIN
#undef __NO_MATH_INLINES
#define __NO_MATH_INLINES 1
#include <math.h>
#include <stdio.h>
#include <tgmath.h>

//#define DEBUG

static void compile_test (void);
static void compile_testf (void);
#ifndef NO_LONG_DOUBLE
static void compile_testl (void);
#endif

float fx;
double dx;
long double lx;

int count_double;
int count_float;
int count_ldouble;

#define NCALLS     115
#define NCALLS_INT 4

int
main (void)
{
  int result = 0;

  count_float = count_double = count_ldouble = 0;
  compile_test ();
  if (count_float != 0)
    {
      puts ("float function called for double test");
      result = 1;
    }
  if (count_ldouble != 0)
    {
      puts ("long double function called for double test");
      result = 1;
    }
  if (count_double < NCALLS + NCALLS_INT)
    {
      printf ("double functions not called often enough (%d)\n",
	      count_double);
      result = 1;
    }
  else if (count_double > NCALLS + NCALLS_INT)
    {
      printf ("double functions called too often (%d)\n",
	      count_double);
      result = 1;
    }

  count_float = count_double = count_ldouble = 0;
  compile_testf ();
  if (count_double != 0)
    {
      puts ("double function called for float test");
      result = 1;
    }
  if (count_ldouble != 0)
    {
      puts ("long double function called for float test");
      result = 1;
    }
  if (count_float < NCALLS)
    {
      printf ("float functions not called often enough (%d)\n", count_float);
      result = 1;
    }
  else if (count_float > NCALLS)
    {
      printf ("float functions called too often (%d)\n",
	      count_double);
      result = 1;
    }

#ifndef NO_LONG_DOUBLE
  count_float = count_double = count_ldouble = 0;
  compile_testl ();
  if (count_float != 0)
    {
      puts ("float function called for long double test");
      result = 1;
    }
  if (count_double != 0)
    {
      puts ("double function called for long double test");
      result = 1;
    }
  if (count_ldouble < NCALLS)
    {
      printf ("long double functions not called often enough (%d)\n",
	      count_ldouble);
      result = 1;
    }
  else if (count_ldouble > NCALLS)
    {
      printf ("long double functions called too often (%d)\n",
	      count_double);
      result = 1;
    }
#endif

  return result;
}

/* Now generate the three functions.  */
#define HAVE_MAIN

#define F(name) name
#define TYPE double
#define TEST_INT 1
#define x dx
#define count count_double
#include "test-tgmath.c"

#define F(name) name##f
#define TYPE float
#define x fx
#define count count_float
#include "test-tgmath.c"

#ifndef NO_LONG_DOUBLE
#define F(name) name##l
#define TYPE long double
#define x lx
#define count count_ldouble
#include "test-tgmath.c"
#endif

#else

#ifdef DEBUG
#define P() puts (__FUNCTION__)
#else
#define P()
#endif

static void
F(compile_test) (void)
{
  TYPE a, b, c = 1.0;
  int i;
  long int j;
  long long int k;

  a = cos (cos (x));
  b = acos (acos (a));
  a = sin (sin (x));
  b = asin (asin (a));
  a = tan (tan (x));
  b = atan (atan (a));
  c = atan2 (atan2 (a, c), atan2 (b, x));
  a = cosh (cosh (x));
  b = acosh (acosh (a));
  a = sinh (sinh (x));
  b = asinh (asinh (a));
  a = tanh (tanh (x));
  b = atanh (atanh (a));
  a = exp (exp (x));
  b = log (log (a));
  a = log10 (log10 (x));
  b = ldexp (ldexp (a, 1), 5);
  a = frexp (frexp (x, &i), &i);
  b = expm1 (expm1 (a));
  a = log1p (log1p (x));
  b = logb (logb (a));
  a = exp2 (exp2 (x));
  b = log2 (log2 (a));
  a = pow (pow (x, a), pow (c, b));
  b = sqrt (sqrt (a));
  a = hypot (hypot (x, b), hypot (c, a));
  b = cbrt (cbrt (a));
  a = ceil (ceil (x));
  b = fabs (fabs (a));
  a = floor (floor (x));
  b = fmod (fmod (a, b), fmod (c, x));
  a = nearbyint (nearbyint (x));
  b = round (round (a));
  a = trunc (trunc (x));
  b = remquo (remquo (a, b, &i), remquo (c, x, &i), &i);
  j = lrint (x) + lround (a);
  k = llrint (b) + llround (c);
  a = erf (erf (x));
  b = erfc (erfc (a));
  a = tgamma (tgamma (x));
  b = lgamma (lgamma (a));
  a = rint (rint (x));
  b = nextafter (nextafter (a, b), nextafter (c, x));
  a = nexttoward (nexttoward (x, a), c);
  b = remainder (remainder (a, b), remainder (c, x));
  a = scalb (scalb (x, a), (TYPE) (6));
  k = scalbn (a, 7) + scalbln (c, 10l);
  i = ilogb (x);
  a = fdim (fdim (x, a), fdim (c, b));
  b = fmax (fmax (a, x), fmax (c, b));
  a = fmin (fmin (x, a), fmin (c, b));
  b = fma (sin (a), sin (x), sin (c));

#ifdef TEST_INT
  a = atan2 (i, b);
  b = remquo (i, a, &i);
  c = fma (i, b, i);
  a = pow (i, c);
#endif
}
#undef x


TYPE
(F(cos)) (TYPE x)
{
  ++count;
  return x;
}

TYPE
(F(acos)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(sin)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(asin)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(tan)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(atan)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(atan2)) (TYPE x, TYPE y)
{
  ++count;
  P();
  return x + y;
}

TYPE
(F(cosh)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(acosh)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(sinh)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(asinh)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(tanh)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(atanh)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(exp)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(log)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(log10)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(ldexp)) (TYPE x, int y)
{
  ++count;
  P();
  return x;
}

TYPE
(F(frexp)) (TYPE x, int *y)
{
  ++count;
  P();
  return x;
}

TYPE
(F(expm1)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(log1p)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(logb)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(exp2)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(log2)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(pow)) (TYPE x, TYPE y)
{
  ++count;
  P();
  return x + y;
}

TYPE
(F(sqrt)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(hypot)) (TYPE x, TYPE y)
{
  ++count;
  P();
  return x + y;
}

TYPE
(F(cbrt)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(ceil)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(fabs)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(floor)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(fmod)) (TYPE x, TYPE y)
{
  ++count;
  P();
  return x + y;
}

TYPE
(F(nearbyint)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(round)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(trunc)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(remquo)) (TYPE x, TYPE y, int *i)
{
  ++count;
  P();
  return x + y;
}

long int
(F(lrint)) (TYPE x)
{
  ++count;
  P();
  return x;
}

long int
(F(lround)) (TYPE x)
{
  ++count;
  P();
  return x;
}

long long int
(F(llrint)) (TYPE x)
{
  ++count;
  P();
  return x;
}

long long int
(F(llround)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(erf)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(erfc)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(tgamma)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(lgamma)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(rint)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(nextafter)) (TYPE x, TYPE y)
{
  ++count;
  P();
  return x + y;
}

TYPE
(F(nexttoward)) (TYPE x, long double y)
{
  ++count;
  P();
  return x;
}

TYPE
(F(remainder)) (TYPE x, TYPE y)
{
  ++count;
  P();
  return x + y;
}

TYPE
(F(scalb)) (TYPE x, TYPE y)
{
  ++count;
  P();
  return x + y;
}

TYPE
(F(scalbn)) (TYPE x, int y)
{
  ++count;
  P();
  return x;
}

TYPE
(F(scalbln)) (TYPE x, long int y)
{
  ++count;
  P();
  return x;
}

int
(F(ilogb)) (TYPE x)
{
  ++count;
  P();
  return x;
}

TYPE
(F(fdim)) (TYPE x, TYPE y)
{
  ++count;
  P();
  return x + y;
}

TYPE
(F(fmin)) (TYPE x, TYPE y)
{
  ++count;
  P();
  return x + y;
}

TYPE
(F(fmax)) (TYPE x, TYPE y)
{
  ++count;
  P();
  return x + y;
}

TYPE
(F(fma)) (TYPE x, TYPE y, TYPE z)
{
  ++count;
  P();
  return x + y + z;
}

#undef F
#undef TYPE
#undef count
#undef TEST_INT
#endif
