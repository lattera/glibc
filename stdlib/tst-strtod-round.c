/* Test for correct rounding of results of strtod and related
   functions.
   Copyright (C) 2012-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* Defining _LIBC_TEST ensures long double math functions are
   declared in the headers.  */
#define _LIBC_TEST 1
#include <fenv.h>
#include <float.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math-tests.h>

struct exactness
{
  bool f;
  bool d;
  bool ld;
};

struct test_results {
  float f;
  double d;
  long double ld;
};

struct test {
  const char *s;
  struct exactness exact;
  struct test_results rd, rn, rz, ru;
};

#if LDBL_MANT_DIG == 53 && LDBL_MAX_EXP == 1024
# define TEST(s, fexact, fd, fn, fz, fu, dexact, dd, dn, dz, du,	\
	      ld53exact, ld53d, ld53n, ld53z, ld53u,			\
	      ld64iexact, ld64id, ld64in, ld64iz, ld64iu,		\
	      ld64mexact, ld64md, ld64mn, ld64mz, ld64mu,		\
	      ld106exact, ld106d, ld106n, ld106z, ld106u,		\
	      ld113exact, ld113d, ld113n, ld113z, ld113u)		\
  {									\
    s,									\
    { fexact, dexact, ld53exact },					\
    { fd, dd, ld53d },							\
    { fn, dn, ld53n },							\
    { fz, dz, ld53z },							\
    { fu, du, ld53u }							\
  }
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384 && LDBL_MIN_EXP == -16381
/* This is for the Intel extended float format.  */
# define TEST(s, fexact, fd, fn, fz, fu, dexact, dd, dn, dz, du,	\
	      ld53exact, ld53d, ld53n, ld53z, ld53u,			\
	      ld64iexact, ld64id, ld64in, ld64iz, ld64iu,		\
	      ld64mexact, ld64md, ld64mn, ld64mz, ld64mu,		\
	      ld106exact, ld106d, ld106n, ld106z, ld106u,		\
	      ld113exact, ld113d, ld113n, ld113z, ld113u)		\
  {									\
    s,									\
    { fexact, dexact, ld64iexact },					\
    { fd, dd, ld64id },							\
    { fn, dn, ld64in },							\
    { fz, dz, ld64iz },							\
    { fu, du, ld64iu }							\
  }
#elif LDBL_MANT_DIG == 64 && LDBL_MAX_EXP == 16384 && LDBL_MIN_EXP == -16382
/* This is for the Motorola extended float format.  */
# define TEST(s, fexact, fd, fn, fz, fu, dexact, dd, dn, dz, du,	\
	      ld53exact, ld53d, ld53n, ld53z, ld53u,			\
	      ld64iexact, ld64id, ld64in, ld64iz, ld64iu,		\
	      ld64mexact, ld64md, ld64mn, ld64mz, ld64mu,		\
	      ld106exact, ld106d, ld106n, ld106z, ld106u,		\
	      ld113exact, ld113d, ld113n, ld113z, ld113u)		\
  {									\
    s,									\
    { fexact, dexact, ld64mexact },					\
    { fd, dd, ld64md },							\
    { fn, dn, ld64mn },							\
    { fz, dz, ld64mz },							\
    { fu, du, ld64mu }							\
  }
#elif LDBL_MANT_DIG == 106 && LDBL_MAX_EXP == 1024
# define TEST(s, fexact, fd, fn, fz, fu, dexact, dd, dn, dz, du,	\
	      ld53exact, ld53d, ld53n, ld53z, ld53u,			\
	      ld64iexact, ld64id, ld64in, ld64iz, ld64iu,		\
	      ld64mexact, ld64md, ld64mn, ld64mz, ld64mu,		\
	      ld106exact, ld106d, ld106n, ld106z, ld106u,		\
	      ld113exact, ld113d, ld113n, ld113z, ld113u)		\
  {									\
    s,									\
    { fexact, dexact, ld106exact },					\
    { fd, dd, ld106d },							\
    { fn, dn, ld106n },							\
    { fz, dz, ld106z },							\
    { fu, du, ld106u }							\
  }
#elif LDBL_MANT_DIG == 113 && LDBL_MAX_EXP == 16384
# define TEST(s, fexact, fd, fn, fz, fu, dexact, dd, dn, dz, du,	\
	      ld53exact, ld53d, ld53n, ld53z, ld53u,			\
	      ld64iexact, ld64id, ld64in, ld64iz, ld64iu,		\
	      ld64mexact, ld64md, ld64mn, ld64mz, ld64mu,		\
	      ld106exact, ld106d, ld106n, ld106z, ld106u,		\
	      ld113exact, ld113d, ld113n, ld113z, ld113u)		\
  {									\
    s,									\
    { fexact, dexact, ld113exact },					\
    { fd, dd, ld113d },							\
    { fn, dn, ld113n },							\
    { fz, dz, ld113z },							\
    { fu, du, ld113u }							\
  }
#else
# error "unknown long double format"
#endif

/* Include the generated test data.  */
#include "tst-strtod-round-data.h"

static int
test_in_one_mode (const char *s, const struct test_results *expected,
		  const struct exactness *exact, const char *mode_name,
		  bool float_round_ok, bool double_round_ok,
		  bool long_double_round_ok)
{
  int result = 0;
  float f = strtof (s, NULL);
  double d = strtod (s, NULL);
  long double ld = strtold (s, NULL);
  if (f != expected->f
      || copysignf (1.0f, f) != copysignf (1.0f, expected->f))
    {
      printf ("strtof (%s) returned %a not %a (%s)\n", s, f,
	      expected->f, mode_name);
      if (float_round_ok || exact->f)
	result = 1;
      else
	printf ("ignoring this inexact result\n");
    }
  if (d != expected->d
      || copysign (1.0, d) != copysign (1.0, expected->d))
    {
      printf ("strtod (%s) returned %a not %a (%s)\n", s, d,
	      expected->d, mode_name);
      if (double_round_ok || exact->d)
	result = 1;
      else
	printf ("ignoring this inexact result\n");
    }
  if (ld != expected->ld
      || copysignl (1.0L, ld) != copysignl (1.0L, expected->ld))
    {
      printf ("strtold (%s) returned %La not %La (%s)\n", s, ld,
	      expected->ld, mode_name);
      if ((long_double_round_ok && LDBL_MANT_DIG != 106) || exact->ld)
	result = 1;
      else
	printf ("ignoring this inexact result\n");
    }
  return result;
}

static int
do_test (void)
{
  int save_round_mode __attribute__ ((unused)) = fegetround ();
  int result = 0;
  for (size_t i = 0; i < sizeof (tests) / sizeof (tests[0]); i++)
    {
      result |= test_in_one_mode (tests[i].s, &tests[i].rn, &tests[i].exact,
				  "default rounding mode",
				  true, true, true);
#ifdef FE_DOWNWARD
      if (!fesetround (FE_DOWNWARD))
	{
	  result |= test_in_one_mode (tests[i].s, &tests[i].rd,
				      &tests[i].exact, "FE_DOWNWARD",
				      ROUNDING_TESTS (float, FE_DOWNWARD),
				      ROUNDING_TESTS (double, FE_DOWNWARD),
				      ROUNDING_TESTS (long double,
						      FE_DOWNWARD));
	  fesetround (save_round_mode);
	}
#endif
#ifdef FE_TOWARDZERO
      if (!fesetround (FE_TOWARDZERO))
	{
	  result |= test_in_one_mode (tests[i].s, &tests[i].rz,
				      &tests[i].exact, "FE_TOWARDZERO",
				      ROUNDING_TESTS (float, FE_TOWARDZERO),
				      ROUNDING_TESTS (double, FE_TOWARDZERO),
				      ROUNDING_TESTS (long double,
						      FE_TOWARDZERO));
	  fesetround (save_round_mode);
	}
#endif
#ifdef FE_UPWARD
      if (!fesetround (FE_UPWARD))
	{
	  result |= test_in_one_mode (tests[i].s, &tests[i].ru,
				      &tests[i].exact, "FE_UPWARD",
				      ROUNDING_TESTS (float, FE_UPWARD),
				      ROUNDING_TESTS (double, FE_UPWARD),
				      ROUNDING_TESTS (long double, FE_UPWARD));
	  fesetround (save_round_mode);
	}
#endif
    }
  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
