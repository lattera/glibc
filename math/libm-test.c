/* Copyright (C) 1997, 1998, 1999 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@arthur.rhein-neckar.de>, 1997.

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

/* Part of testsuite for libm.

   This file has to be included by a master file that defines:

   Makros:
   FUNC(function): converts general function name (like cos) to
   name with correct suffix (e.g. cosl or cosf)
   MATHCONST(x):   like FUNC but for constants (e.g convert 0.0 to 0.0L)
   MATHTYPE:	   floating point type to test
   TEST_MSG:	   informal message to be displayed
   CHOOSE(Clongdouble,Cdouble,Cfloat):
   chooses one of the parameters as epsilon for testing
   equality
   PRINTF_EXPR	   Floating point conversion specification to print a variable
   of type MATHTYPE with printf.  PRINTF_EXPR just contains
   the specifier, not the percent and width arguments,
   e.g. "f".
   PRINTF_XEXPR	   Like PRINTF_EXPR, but print in hexadecimal format.
 */

/* This program isn't finished yet.
   It has tests for:
   acos, acosh, asin, asinh, atan, atan2, atanh,
   cbrt, ceil, copysign, cos, cosh, erf, erfc, exp, exp10, exp2, expm1,
   fabs, fdim, floor, fma, fmax, fmin, fmod, fpclassify,
   frexp, gamma, hypot,
   ilogb, isfinite, isinf, isnan, isnormal,
   isless, islessequal, isgreater, isgreaterequal, islessgreater, isunordered,
   j0, j1, jn,
   ldexp, lgamma, log, log10, log1p, log2, logb,
   modf, nearbyint, nextafter,
   pow, remainder, remquo, rint, lrint, llrint,
   round, lround, llround,
   scalb, scalbn, signbit, sin, sincos, sinh, sqrt, tan, tanh, tgamma, trunc,
   y0, y1, yn

   and for the following complex math functions:
   cabs, cacos, cacosh, carg, casin, casinh, catan, catanh,
   ccos, ccosh, cexp, clog, cpow, csin, csinh, csqrt, ctan, ctanh.

   At the moment the following functions aren't tested:
   conj, cproj, cimag, creal, drem,
   significand,
   nan

   The routines using random variables are still under construction.  I don't
   like it the way it's working now and will change it.

   Parameter handling is primitive in the moment:
   --verbose=[0..4] for different levels of output:
   0: only error count
   1: basic report on failed tests (default)
   2: full report on failed tests
   3: full report on failed and passed tests
   4: additional report on exceptions
   -v for full output (equals --verbose=4)
   -s,--silent outputs only the error count (equals --verbose=0)
 */

/* "Philosophy":

   This suite tests some aspects of the correct implementation of
   mathematical functions in libm.  Some simple, specific parameters
   are tested for correctness but there's no exhaustive
   testing.  Handling of specific inputs (e.g. infinity, not-a-number)
   is also tested.  Correct handling of exceptions is checked
   against.  These implemented tests should check all cases that are
   specified in ISO C 9X.

   Exception testing: At the moment only divide-by-zero and invalid
   exceptions are tested.  Overflow/underflow and inexact exceptions
   aren't checked at the moment.

   NaN values: There exist signalling and quiet NaNs.  This implementation
   only uses signalling NaN as parameter but does not differenciate
   between the two kinds of NaNs as result.

   Inline functions: Inlining functions should give an improvement in
   speed - but not in precission.  The inlined functions return
   reasonable values for a reasonable range of input values.  The
   result is not necessarily correct for all values and exceptions are
   not correctly raised in all cases.  Problematic input and return
   values are infinity, not-a-number and minus zero.  This suite
   therefore does not check these specific inputs and the exception
   handling for inlined mathematical functions - just the "reasonable"
   values are checked.

   Beware: The tests might fail for any of the following reasons:
   - Tests are wrong
   - Functions are wrong
   - Floating Point Unit not working properly
   - Compiler has errors

   With e.g. gcc 2.7.2.2 the test for cexp fails because of a compiler error.
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <complex.h>
#include <math.h>
#include <float.h>
#include <fenv.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>

/* Possible exceptions */
#define NO_EXCEPTION		 0x0
#define INVALID_EXCEPTION	 0x1
#define DIVIDE_BY_ZERO_EXCEPTION 0x2

#define PRINT 1
#define NO_PRINT 0

/* Various constants (we must supply them precalculated for accuracy).  */
#define M_PI_6l	 .52359877559829887308L
#define M_E2l	7.389056098930650227230L
#define M_E3l	20.08553692318766774093L

static int noErrors;	/* number of errors */
static int noTests;	/* number of tests (without testing exceptions) */
static int noExcTests;	/* number of tests for exception flags */

static int verbose = 3;
static MATHTYPE minus_zero, plus_zero;
static MATHTYPE plus_infty, minus_infty, nan_value;

typedef MATHTYPE (*mathfunc) (MATHTYPE);

#define BUILD_COMPLEX(real, imag) \
  ({ __complex__ MATHTYPE __retval;					      \
     __real__ __retval = (real);					      \
     __imag__ __retval = (imag);					      \
     __retval; })

/* Test if Floating-Point stack hasn't changed */
static void
fpstack_test (const char *test_name)
{
#ifdef i386
  static int old_stack;
  int sw;

  asm ("fnstsw" : "=a" (sw));
  sw >>= 11;
  sw &= 7;

  if (sw != old_stack)
    {
      printf ("FP-Stack wrong after test %s (%d, should be %d)\n",
	      test_name, sw, old_stack);
      ++noErrors;
      old_stack = sw;
    }
#endif
}


/* Get a random value x with min_value < x < max_value
   and min_value, max_value finite,
   max_value and min_value shouldn't be too close together */
static MATHTYPE
random_value (MATHTYPE min_value, MATHTYPE max_value)
{
  int r;
  MATHTYPE x;

  r = rand ();

  x = (max_value - min_value) / RAND_MAX * (MATHTYPE) r + min_value;

  if ((x <= min_value) || (x >= max_value) || !isfinite (x))
    x = (max_value - min_value) / 2 + min_value;

  /* Make sure the RNG has no influence on the exceptions.  */
  feclearexcept (FE_ALL_EXCEPT);

  return x;
}


/* Get a random value x with x > min_value.  */
static MATHTYPE
random_greater (MATHTYPE min_value)
{
  return random_value (min_value, 1e6);		/* CHOOSE (LDBL_MAX, DBL_MAX, FLT_MAX) */
}


/* Get a random value x with x < max_value.  */
static MATHTYPE
random_less (MATHTYPE max_value)
{
  return random_value (-1e6, max_value);
}


static void
output_new_test (const char *test_name)
{
  if (verbose > 2)
    printf ("\nTesting: %s\n", test_name);
}


static void
output_pass_value (void)
{
  if (verbose > 2)
    printf ("Pass: Value Ok.\n");
}


static void
output_fail_value (const char *test_name)
{
  if (verbose > 0 && verbose < 3)
    printf ("Fail: %s\n", test_name);
  if (verbose >= 3)
    printf ("Fail:\n");
}


/* Test whether a given exception was raised.  */
static void
test_single_exception (const char *test_name,
		       short int exception,
		       short int exc_flag,
		       int fe_flag,
		       const char *flag_name)
{
#ifndef TEST_INLINE
  if (exception & exc_flag)
    {
      if (fetestexcept (fe_flag))
	{
	  if (verbose > 3)
	    printf ("Pass: Exception \"%s\" set\n", flag_name);
	}
      else
	{
	  if (verbose && verbose < 3)
	    printf ("Fail: %s: Exception \"%s\" not set\n",
		    test_name, flag_name);
	  if (verbose >= 3)
	    printf ("Fail:  Exception \"%s\" not set\n",
		    flag_name);
	  ++noErrors;
	}
    }
  else
    {
      if (fetestexcept (fe_flag))
	{
	  if (verbose && verbose < 3)
	    printf ("Fail: %s: Exception \"%s\" set\n",
		    test_name, flag_name);
	  if (verbose >= 3)
	    printf ("Fail:  Exception \"%s\" set\n",
		    flag_name);
	  ++noErrors;
	}
      else
	{
	  if (verbose > 3)
	    printf ("Pass: Exception \"%s\" not set\n",
		    flag_name);
	}
    }
#endif
}


/* Test whether exception given by EXCEPTION are raised.  */
static void
test_not_exception (const char *test_name, short int exception)
{
  ++noExcTests;
#ifdef FE_DIVBYZERO
  if ((exception & DIVIDE_BY_ZERO_EXCEPTION) == 0)
    test_single_exception (test_name, exception,
			   DIVIDE_BY_ZERO_EXCEPTION, FE_DIVBYZERO,
			   "Divide by zero");
#endif
#ifdef FE_INVALID
  if ((exception & INVALID_EXCEPTION) == 0)
    test_single_exception (test_name, exception, INVALID_EXCEPTION, FE_INVALID,
			   "Invalid operation");
#endif
  feclearexcept (FE_ALL_EXCEPT);
}


/* Test whether exceptions given by EXCEPTION are raised.  */
static void
test_exceptions (const char *test_name, short int exception)
{
  ++noExcTests;
#ifdef FE_DIVBYZERO
  test_single_exception (test_name, exception,
			 DIVIDE_BY_ZERO_EXCEPTION, FE_DIVBYZERO,
			 "Divide by zero");
#endif
#ifdef FE_INVALID
  test_single_exception (test_name, exception, INVALID_EXCEPTION, FE_INVALID,
			 "Invalid operation");
#endif
  feclearexcept (FE_ALL_EXCEPT);
}


/* Test if two floating point numbers are equal.  */
static int
check_equal (MATHTYPE computed, MATHTYPE supplied, MATHTYPE eps, MATHTYPE * diff)
{
  int ret_value;

  /* Both plus Infinity or both minus infinity.  */
  if (isinf (computed) && (isinf (computed) == isinf (supplied)))
    return 1;

  if (isnan (computed) && isnan (supplied))	/* isnan works for all types */
    return 1;

  *diff = FUNC(fabs) (computed - supplied);


  ret_value = (*diff <= eps &&
	       (signbit (computed) == signbit (supplied) || eps != 0.0));

  /* Make sure the subtraction/comparison
     have no influence on the exceptions.  */
  feclearexcept (FE_ALL_EXCEPT);

  return ret_value;
}


static void
output_result_bool (const char *test_name, int result)
{
  ++noTests;
  if (result)
    {
      output_pass_value ();
    }
  else
    {
      output_fail_value (test_name);
      if (verbose > 1)
	printf (" Value: %d\n", result);
      ++noErrors;
    }

  fpstack_test (test_name);
}


static void
output_isvalue (const char *test_name, int result,
		MATHTYPE value)
{
  ++noTests;
  if (result)
    {
      output_pass_value ();
    }
  else
    {
      output_fail_value (test_name);
      if (verbose > 1)
	printf (" Value: % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
		value, value);
      ++noErrors;
    }

  fpstack_test (test_name);
}


static void
output_isvalue_ext (const char *test_name, int result,
		    MATHTYPE value, MATHTYPE parameter)
{
  ++noTests;
  if (result)
    {
      output_pass_value ();
    }
  else
    {
      output_fail_value (test_name);
      if (verbose > 1)
	{
	  printf (" Value:     % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
		  value, value);
	  printf (" Parameter: % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
		  parameter, parameter);
	}
      noErrors++;
    }

  fpstack_test (test_name);
}


static void
output_result (const char *test_name, int result,
	       MATHTYPE computed, MATHTYPE expected,
	       MATHTYPE difference,
	       int print_values, int print_diff)
{
  ++noTests;
  if (result)
    {
      output_pass_value ();
    }
  else
    {
      output_fail_value (test_name);
      if (verbose > 1 && print_values)
	{
	  printf ("Result:\n");
	  printf (" is:         % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
		  computed, computed);
	  printf (" should be:  % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
		  expected, expected);
	  if (print_diff)
	    printf (" difference: % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR
		    "\n", difference, difference);
	}
      ++noErrors;
    }

  fpstack_test (test_name);
}


static void
output_result_ext (const char *test_name, int result,
		   MATHTYPE computed, MATHTYPE expected,
		   MATHTYPE difference,
		   MATHTYPE parameter,
		   int print_values, int print_diff)
{
  ++noTests;
  if (result)
    {
      output_pass_value ();
    }
  else
    {
      output_fail_value (test_name);
      if (verbose > 1 && print_values)
	{
	  printf ("Result:\n");
	  printf (" is:         % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
		  computed, computed);
	  printf (" should be:  % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
		  expected, expected);
	  if (print_diff)
	    printf (" difference: % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR
		    "\n", difference, difference);
	  printf ("Parameter:   % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
		  parameter, parameter);
	}
      ++noErrors;
    }

  fpstack_test (test_name);
}


/* check that computed and expected values are the same */
static void
check (const char *test_name, MATHTYPE computed, MATHTYPE expected)
{
  MATHTYPE diff;
  int result;

  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);
  result = check_equal (computed, expected, 0, &diff);
  output_result (test_name, result,
		 computed, expected, diff, PRINT, PRINT);
}


/* check that computed and expected values are the same,
   outputs the parameter to the function */
static void
check_ext (const char *test_name, MATHTYPE computed, MATHTYPE expected,
	   MATHTYPE parameter)
{
  MATHTYPE diff;
  int result;

  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);
  result = check_equal (computed, expected, 0, &diff);
  output_result_ext (test_name, result,
		     computed, expected, diff, parameter, PRINT, PRINT);
}


/* check that computed and expected values are the same and
   checks also for exception flags */
static void
check_exc (const char *test_name, MATHTYPE computed, MATHTYPE expected,
	   short exception)
{
  MATHTYPE diff;
  int result;

  output_new_test (test_name);
  test_exceptions (test_name, exception);
  result = check_equal (computed, expected, 0, &diff);
  output_result (test_name, result,
		 computed, expected, diff, PRINT, PRINT);
}


/* check that computed and expected values are close enough */
static void
check_eps (const char *test_name, MATHTYPE computed, MATHTYPE expected,
	   MATHTYPE epsilon)
{
  MATHTYPE diff;
  int result;

  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);
  result = check_equal (computed, expected, epsilon, &diff);
  output_result (test_name, result,
		 computed, expected, diff, PRINT, PRINT);
}


/* check a boolean condition */
static void
check_bool (const char *test_name, int computed)
{
  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);
  output_result_bool (test_name, computed);
}


/* check that computed and expected values are equal (int values) */
static void
check_int (const char *test_name, int computed, int expected)
{
  int diff = computed - expected;
  int result = diff == 0;

  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);

  if (result)
    {
      output_pass_value ();
    }
  else
    {
      output_fail_value (test_name);
      if (verbose > 1)
	{
	  printf ("Result:\n");
	  printf (" is:         %d\n", computed);
	  printf (" should be:  %d\n", expected);
	}
      noErrors++;
    }

  fpstack_test (test_name);
}


/* check that computed and expected values are equal (long int values) */
static void
check_long (const char *test_name, long int computed, long int expected)
{
  long int diff = computed - expected;
  int result = diff == 0;

  ++noTests;
  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);

  if (result)
    {
      output_pass_value ();
    }
  else
    {
      output_fail_value (test_name);
      if (verbose > 1)
	{
	  printf ("Result:\n");
	  printf (" is:         %ld\n", computed);
	  printf (" should be:  %ld\n", expected);
	}
      noErrors++;
    }

  fpstack_test (test_name);
}


/* check that computed and expected values are equal (long long int values) */
static void
check_longlong (const char *test_name, long long int computed,
		long long int expected)
{
  long long int diff = computed - expected;
  int result = diff == 0;

  ++noTests;
  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);

  if (result)
    {
      output_pass_value ();
    }
  else
    {
      output_fail_value (test_name);
      if (verbose > 1)
	{
	  printf ("Result:\n");
	  printf (" is:         %lld\n", computed);
	  printf (" should be:  %lld\n", expected);
	}
      noErrors++;
    }

  fpstack_test (test_name);
}


/* check that computed value is not-a-number */
static void
check_isnan (const char *test_name, MATHTYPE computed)
{
  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);
  output_isvalue (test_name, isnan (computed), computed);
}


/* check that computed value is not-a-number and test for exceptions */
static void
check_isnan_exc (const char *test_name, MATHTYPE computed,
		 short exception)
{
  output_new_test (test_name);
  test_exceptions (test_name, exception);
  output_isvalue (test_name, isnan (computed), computed);
}


/* check that computed value is not-a-number and test for exceptions */
static void
check_isnan_maybe_exc (const char *test_name, MATHTYPE computed,
		       short exception)
{
  output_new_test (test_name);
  test_not_exception (test_name, exception);
  output_isvalue (test_name, isnan (computed), computed);
}


/* check that computed value is not-a-number and supply parameter */
#ifndef TEST_INLINE
static void
check_isnan_ext (const char *test_name, MATHTYPE computed,
		 MATHTYPE parameter)
{
  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);
  output_isvalue_ext (test_name, isnan (computed), computed, parameter);
}
#endif

/* check that computed value is not-a-number, test for exceptions
   and supply parameter */
static void
check_isnan_exc_ext (const char *test_name, MATHTYPE computed,
		     short exception, MATHTYPE parameter)
{
  output_new_test (test_name);
  test_exceptions (test_name, exception);
  output_isvalue_ext (test_name, isnan (computed), computed, parameter);
}


/* Tests if computed is +Inf */
static void
check_isinfp (const char *test_name, MATHTYPE computed)
{
  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);
  output_isvalue (test_name, (isinf (computed) == +1), computed);
}


/* Tests if computed is +Inf and supply parameter */
static void
check_isinfp_ext (const char *test_name, MATHTYPE computed,
		  MATHTYPE parameter)
{
  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);
  output_isvalue_ext (test_name, (isinf (computed) == +1), computed, parameter);
}


/* Tests if computed is +Inf and check exceptions */
static void
check_isinfp_exc (const char *test_name, MATHTYPE computed,
		  int exception)
{
  output_new_test (test_name);
  test_exceptions (test_name, exception);
  output_isvalue (test_name, (isinf (computed) == +1), computed);
}


/* Tests if computed is -Inf */
static void
check_isinfn (const char *test_name, MATHTYPE computed)
{
  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);
  output_isvalue (test_name, (isinf (computed) == -1), computed);
}


/* Tests if computed is -Inf and supply parameter */
#ifndef TEST_INLINE
static void
check_isinfn_ext (const char *test_name, MATHTYPE computed,
		  MATHTYPE parameter)
{
  output_new_test (test_name);
  test_exceptions (test_name, NO_EXCEPTION);
  output_isvalue_ext (test_name, (isinf (computed) == -1), computed, parameter);
}
#endif

/* Tests if computed is -Inf and check exceptions */
static void
check_isinfn_exc (const char *test_name, MATHTYPE computed,
		  int exception)
{
  output_new_test (test_name);
  test_exceptions (test_name, exception);
  output_isvalue (test_name, (isinf (computed) == -1), computed);
}


/* This is to prevent messages from the SVID libm emulation.  */
int
matherr (struct exception *x __attribute__ ((unused)))
{
  return 1;
}


/****************************************************************************
  Test for single functions of libm
****************************************************************************/

static void
acos_test (void)
{
#ifndef TEST_INLINE
  MATHTYPE x;

  x = random_greater (1);
  check_isnan_exc ("acos (x) == NaN plus invalid exception for |x| > 1",
		   FUNC(acos) (x),
		   INVALID_EXCEPTION);

  x = random_less (1);
  check_isnan_exc ("acos (x) == NaN plus invalid exception for |x| > 1",
		   FUNC(acos) (x),
		   INVALID_EXCEPTION);
#endif
  check ("acos (0) == pi/2", FUNC(acos) (0), M_PI_2l);
  check ("acos (-0) == pi/2", FUNC(acos) (minus_zero), M_PI_2l);

  check ("acos (1) == 0", FUNC(acos) (1), 0);
  check ("acos (-1) == pi", FUNC(acos) (-1), M_PIl);

  check_eps ("acos (0.5) == pi/3", FUNC(acos) (0.5), M_PI_6l * 2.0,
	     CHOOSE (1e-18, 0, 0));
  check_eps ("acos (-0.5) == 2*pi/3", FUNC(acos) (-0.5), M_PI_6l * 4.0,
	     CHOOSE (1e-17, 0, 0));

  check_eps ("acos (0.7) == 0.795398830...", FUNC(acos) (0.7),
	     0.7953988301841435554L, CHOOSE (7e-17L, 0, 0));
}


static void
acosh_test (void)
{
#ifndef TEST_INLINE
  MATHTYPE x;

  check_isinfp ("acosh(+inf) == +inf", FUNC(acosh) (plus_infty));

  x = random_less (1);
  check_isnan_exc ("acosh(x) == NaN plus invalid exception if x < 1",
		   FUNC(acosh) (x), INVALID_EXCEPTION);
#endif

  check ("acosh(1) == 0", FUNC(acosh) (1), 0);
  check_eps ("acosh(7) == 2.633915793...", FUNC(acosh) (7),
	     2.6339157938496334172L, CHOOSE (3e-19, 0, 0));
}


static void
asin_test (void)
{
#ifndef TEST_INLINE
  MATHTYPE x;

  x = random_greater (1);
  check_isnan_exc ("asin x == NaN plus invalid exception for |x| > 1",
		   FUNC(asin) (x),
		   INVALID_EXCEPTION);

  x = random_less (1);
  check_isnan_exc ("asin x == NaN plus invalid exception for |x| > 1",
		   FUNC(asin) (x),
		   INVALID_EXCEPTION);
#endif

  check ("asin (0) == 0", FUNC(asin) (0), 0);
  check ("asin (-0) == -0", FUNC(asin) (minus_zero), minus_zero);
  check_eps ("asin (0.5) ==  pi/6", FUNC(asin) (0.5), M_PI_6l,
	     CHOOSE (3.5e-18, 0, 2e-7));
  check_eps ("asin (-0.5) ==  -pi/6", FUNC(asin) (-0.5), -M_PI_6l,
	     CHOOSE (3.5e-18, 0, 2e-7));
  check ("asin (1.0) ==  pi/2", FUNC(asin) (1.0), M_PI_2l);
  check ("asin (-1.0) ==  -pi/2", FUNC(asin) (-1.0), -M_PI_2l);
  check_eps ("asin (0.7) ==  0.775397496...", FUNC(asin) (0.7),
	     0.7753974966107530637L, CHOOSE (7e-17L, 2e-16, 2e-7));
}


static void
asinh_test (void)
{
  check ("asinh(+0) == +0", FUNC(asinh) (0), 0);
#ifndef TEST_INLINE
  check ("asinh(-0) == -0", FUNC(asinh) (minus_zero), minus_zero);
  check_isinfp ("asinh(+inf) == +inf", FUNC(asinh) (plus_infty));
  check_isinfn ("asinh(-inf) == -inf", FUNC(asinh) (minus_infty));
#endif
  check_eps ("asinh(0.7) == 0.652666566...", FUNC(asinh) (0.7),
	     0.652666566082355786L, CHOOSE (4e-17L, 0, 6e-8));
}


static void
atan_test (void)
{
  check ("atan (0) == 0", FUNC(atan) (0), 0);
  check ("atan (-0) == -0", FUNC(atan) (minus_zero), minus_zero);

  check ("atan (+inf) == pi/2", FUNC(atan) (plus_infty), M_PI_2l);
  check ("atan (-inf) == -pi/2", FUNC(atan) (minus_infty), -M_PI_2l);

  check_eps ("atan (1) == pi/4", FUNC(atan) (1), M_PI_4l,
	     CHOOSE (1e-18, 0, 0));
  check_eps ("atan (-1) == -pi/4", FUNC(atan) (1), M_PI_4l,
	     CHOOSE (1e-18, 0, 0));

  check_eps ("atan (0.7) == 0.610725964...", FUNC(atan) (0.7),
	     0.6107259643892086165L, CHOOSE (3e-17L, 0, 0));
}


static void
atan2_test (void)
{
  MATHTYPE x;

  x = random_greater (0);
  check ("atan2 (0,x) == 0 for x > 0",
	 FUNC(atan2) (0, x), 0);
  x = random_greater (0);
  check ("atan2 (-0,x) == -0 for x > 0",
	 FUNC(atan2) (minus_zero, x), minus_zero);

  check ("atan2 (+0,+0) == +0", FUNC(atan2) (0, 0), 0);
  check ("atan2 (-0,+0) == -0", FUNC(atan2) (minus_zero, 0), minus_zero);

  x = -random_greater (0);
  check ("atan2 (+0,x) == +pi for x < 0", FUNC(atan2) (0, x), M_PIl);

  x = -random_greater (0);
  check ("atan2 (-0,x) == -pi for x < 0", FUNC(atan2) (minus_zero, x), -M_PIl);

  check ("atan2 (+0,-0) == +pi", FUNC(atan2) (0, minus_zero), M_PIl);
  check ("atan2 (-0,-0) == -pi", FUNC(atan2) (minus_zero, minus_zero), -M_PIl);

  x = random_greater (0);
  check ("atan2 (y,+0) == pi/2 for y > 0", FUNC(atan2) (x, 0), M_PI_2l);

  x = random_greater (0);
  check ("atan2 (y,-0) == pi/2 for y > 0", FUNC(atan2) (x, minus_zero),
	 M_PI_2l);

  x = random_less (0);
  check ("atan2 (y,+0) == -pi/2 for y < 0", FUNC(atan2) (x, 0), -M_PI_2l);

  x = random_less (0);
  check ("atan2 (y,-0) == -pi/2 for y < 0", FUNC(atan2) (x, minus_zero),
	 -M_PI_2l);

  x = random_greater (0);
  check ("atan2 (y,inf) == +0 for finite y > 0",
	 FUNC(atan2) (x, plus_infty), 0);

  x = -random_greater (0);
  check ("atan2 (y,inf) == -0 for finite y < 0",
	 FUNC(atan2) (x, plus_infty), minus_zero);

  x = random_value (-1e4, 1e4);
  check ("atan2(+inf, x) == pi/2 for finite x",
	 FUNC(atan2) (plus_infty, x), M_PI_2l);

  x = random_value (-1e4, 1e4);
  check ("atan2(-inf, x) == -pi/2 for finite x",
	 FUNC(atan2) (minus_infty, x), -M_PI_2l);

  x = random_greater (0);
  check ("atan2 (y,-inf) == +pi for finite y > 0",
	 FUNC(atan2) (x, minus_infty), M_PIl);

  x = -random_greater (0);
  check ("atan2 (y,-inf) == -pi for finite y < 0",
	 FUNC(atan2) (x, minus_infty), -M_PIl);

  check ("atan2 (+inf,+inf) == +pi/4",
	 FUNC(atan2) (plus_infty, plus_infty), M_PI_4l);

  check ("atan2 (-inf,+inf) == -pi/4",
	 FUNC(atan2) (minus_infty, plus_infty), -M_PI_4l);

  check ("atan2 (+inf,-inf) == +3*pi/4",
	 FUNC(atan2) (plus_infty, minus_infty), 3 * M_PI_4l);

  check ("atan2 (-inf,-inf) == -3*pi/4",
	 FUNC(atan2) (minus_infty, minus_infty), -3 * M_PI_4l);

  /* FIXME: Add some specific tests */
  check_eps ("atan2 (0.7,1) == 0.61072...", FUNC(atan2) (0.7, 1),
	     0.6107259643892086165L, CHOOSE (3e-17L, 0, 0));
  check_eps ("atan2 (0.4,0.0003) == 1.57004...", FUNC(atan2) (0.4, 0.0003),
	     1.5700463269355215718L, CHOOSE (2e-19L, 0, 1.2e-7));
}


static void
atanh_test (void)
{
#ifndef TEST_INLINE
  MATHTYPE x;
#endif

  check ("atanh(+0) == +0", FUNC(atanh) (0), 0);
#ifndef TEST_INLINE
  check ("atanh(-0) == -0", FUNC(atanh) (minus_zero), minus_zero);

  check_isinfp_exc ("atanh(+1) == +inf plus divide-by-zero exception",
		    FUNC(atanh) (1), DIVIDE_BY_ZERO_EXCEPTION);
  check_isinfn_exc ("atanh(-1) == -inf plus divide-by-zero exception",
		    FUNC(atanh) (-1), DIVIDE_BY_ZERO_EXCEPTION);

  x = random_greater (1.0);
  check_isnan_exc_ext ("atanh (x) == NaN plus invalid exception if |x| > 1",
		       FUNC(atanh) (x), INVALID_EXCEPTION, x);

  x = random_less (1.0);
  check_isnan_exc_ext ("atanh (x) == NaN plus invalid exception if |x| > 1",
		       FUNC(atanh) (x), INVALID_EXCEPTION, x);

#endif
  check_eps ("atanh(0.7) == 0.867300527...", FUNC(atanh) (0.7),
	     0.8673005276940531944L, CHOOSE (9e-17L, 2e-16, 0));
}


static void
cbrt_test (void)
{
  check ("cbrt (+0) == +0", FUNC(cbrt) (0.0), 0.0);
  check ("cbrt (-0) == -0", FUNC(cbrt) (minus_zero), minus_zero);

#ifndef TEST_INLINE
  check_isinfp ("cbrt (+inf) == +inf", FUNC(cbrt) (plus_infty));
  check_isinfn ("cbrt (-inf) == -inf", FUNC(cbrt) (minus_infty));
  check_isnan ("cbrt (NaN) == NaN", FUNC(cbrt) (nan_value));
#endif
  check_eps ("cbrt (-0.001) == -0.1", FUNC(cbrt) (-0.001), -0.1,
	     CHOOSE (5e-18L, 0, 0));
  check_eps ("cbrt (8) == 2", FUNC(cbrt) (8), 2, CHOOSE (5e-17L, 0, 0));
  check_eps ("cbrt (-27) == -3", FUNC(cbrt) (-27.0), -3.0,
	     CHOOSE (3e-16L, 5e-16, 0));
  check_eps ("cbrt (0.970299) == 0.99", FUNC(cbrt) (0.970299), 0.99,
	     CHOOSE (2e-17L, 2e-16, 0));
  check_eps ("cbrt (0.7) == .8879040017...", FUNC(cbrt) (0.7),
	     0.8879040017426007084L, CHOOSE (2e-17L, 6e-16, 0));
}


static void
ceil_test (void)
{
  check ("ceil (+0) == +0", FUNC(ceil) (0.0), 0.0);
  check ("ceil (-0) == -0", FUNC(ceil) (minus_zero), minus_zero);
  check_isinfp ("ceil (+inf) == +inf", FUNC(ceil) (plus_infty));
  check_isinfn ("ceil (-inf) == -inf", FUNC(ceil) (minus_infty));

  check ("ceil (pi) == 4", FUNC(ceil) (M_PIl), 4.0);
  check ("ceil (-pi) == -3", FUNC(ceil) (-M_PIl), -3.0);
}


static void
cos_test (void)
{
  check ("cos (+0) == 1", FUNC(cos) (0), 1);
  check ("cos (-0) == 1", FUNC(cos) (minus_zero), 1);
  check_isnan_exc ("cos (+inf) == NaN plus invalid exception",
		   FUNC(cos) (plus_infty),
		   INVALID_EXCEPTION);
  check_isnan_exc ("cos (-inf) == NaN plus invalid exception",
		   FUNC(cos) (minus_infty),
		   INVALID_EXCEPTION);

  check_eps ("cos (pi/3) == 0.5", FUNC(cos) (M_PI_6l * 2.0),
	     0.5, CHOOSE (4e-18L, 1e-15L, 1e-7L));
  check_eps ("cos (2*pi/3) == -0.5", FUNC(cos) (M_PI_6l * 4.0),
	     -0.5, CHOOSE (4e-18L, 1e-15L, 1e-7L));
  check_eps ("cos (pi/2) == 0", FUNC(cos) (M_PI_2l),
	     0, CHOOSE (1e-19L, 1e-16L, 1e-7L));

  check_eps ("cos (0.7) == 0.7648421872...", FUNC(cos) (0.7),
	     0.7648421872844884262L, CHOOSE (3e-17, 2e-16, 6e-8));
}


static void
cosh_test (void)
{
  check ("cosh (+0) == 1", FUNC(cosh) (0), 1);
  check ("cosh (-0) == 1", FUNC(cosh) (minus_zero), 1);

#ifndef TEST_INLINE
  check_isinfp ("cosh (+inf) == +inf", FUNC(cosh) (plus_infty));
  check_isinfp ("cosh (-inf) == +inf", FUNC(cosh) (minus_infty));
#endif

  check_eps ("cosh (0.7) ==  1.2551690056...", FUNC(cosh) (0.7),
	     1.255169005630943018L, CHOOSE (4e-17L, 2.3e-16, 0));
}


static void
erf_test (void)
{
  errno = 0;
  FUNC(erf) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  check ("erf (+0) == +0", FUNC(erf) (0), 0);
  check ("erf (-0) == -0", FUNC(erf) (minus_zero), minus_zero);
  check ("erf (+inf) == +1", FUNC(erf) (plus_infty), 1);
  check ("erf (-inf) == -1", FUNC(erf) (minus_infty), -1);

  check_eps ("erf (0.7) == 0.6778011938...", FUNC(erf) (0.7),
	     0.67780119383741847297L, CHOOSE (0, 2e-16, 0));
}


static void
erfc_test (void)
{
  errno = 0;
  FUNC(erfc) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  check ("erfc (+inf) == 0", FUNC(erfc) (plus_infty), 0.0);
  check ("erfc (-inf) == 2", FUNC(erfc) (minus_infty), 2.0);
  check ("erfc (+0) == 1", FUNC(erfc) (0.0), 1.0);
  check ("erfc (-0) == 1", FUNC(erfc) (minus_zero), 1.0);

  check_eps ("erfc (0.7) == 0.3221988061...", FUNC(erfc) (0.7),
	     0.32219880616258152702L, CHOOSE (0, 6e-17, 0));
}


static void
exp_test (void)
{
  check ("exp (+0) == 1", FUNC(exp) (0), 1);
  check ("exp (-0) == 1", FUNC(exp) (minus_zero), 1);

#ifndef TEST_INLINE
  check_isinfp ("exp (+inf) == +inf", FUNC(exp) (plus_infty));
  check ("exp (-inf) == 0", FUNC(exp) (minus_infty), 0);
#endif
  check_eps ("exp (1) == e", FUNC(exp) (1), M_El, CHOOSE (4e-18L, 0, 0));

  check_eps ("exp (2) == e^2", FUNC(exp) (2), M_E2l,
	     CHOOSE (1e-18, 8.9e-16, 0));
  check_eps ("exp (3) == e^3", FUNC(exp) (3), M_E3l,
	     CHOOSE (1.5e-17, 3.6e-15, 0));
  check_eps ("exp (0.7) == 2.0137527074...", FUNC(exp) (0.7),
	     2.0137527074704765216L, CHOOSE (9e-17L, 4.5e-16, 0));
}


static void
exp10_test (void)
{
  errno = 0;
  FUNC(exp10) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  check ("exp10 (+0) == 1", FUNC(exp10) (0), 1);
  check ("exp10 (-0) == 1", FUNC(exp10) (minus_zero), 1);

  check_isinfp ("exp10 (+inf) == +inf", FUNC(exp10) (plus_infty));
  check ("exp10 (-inf) == 0", FUNC(exp10) (minus_infty), 0);
  check_eps ("exp10 (3) == 1000", FUNC(exp10) (3), 1000,
	     CHOOSE (5e-16, 7e-13, 2e-4));
  check_eps ("exp10 (-1) == 0.1", FUNC(exp10) (-1), 0.1,
	     CHOOSE (6e-18, 3e-17, 8e-09));
  check_isinfp ("exp10 (1e6) == +inf", FUNC(exp10) (1e6));
  check ("exp10 (-1e6) == 0", FUNC(exp10) (-1e6), 0);
  check_eps ("exp10 (0.7) == 5.0118723...", FUNC(exp10) (0.7),
	     5.0118723362727228500L, CHOOSE (6e-16, 9e-16, 5e-7));
}


static void
exp2_test (void)
{
  errno = 0;
  FUNC(exp2) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  check ("exp2 (+0) == 1", FUNC(exp2) (0), 1);
  check ("exp2 (-0) == 1", FUNC(exp2) (minus_zero), 1);

  check_isinfp ("exp2 (+inf) == +inf", FUNC(exp2) (plus_infty));
  check ("exp2 (-inf) == 0", FUNC(exp2) (minus_infty), 0);
  check ("exp2 (10) == 1024", FUNC(exp2) (10), 1024);
  check ("exp2 (-1) == 0.5", FUNC(exp2) (-1), 0.5);
  check_isinfp ("exp2 (1e6) == +inf", FUNC(exp2) (1e6));
  check ("exp2 (-1e6) == 0", FUNC(exp2) (-1e6), 0);
  check_eps ("exp2 (0.7) == 1.6245047927...", FUNC(exp2) (0.7),
	     1.6245047927124710452L, CHOOSE (6e-17L, 0, 1.2e-7));
}


static void
expm1_test (void)
{
  check ("expm1 (+0) == 0", FUNC(expm1) (0), 0);
#ifndef TEST_INLINE
  check ("expm1 (-0) == -0", FUNC(expm1) (minus_zero), minus_zero);

  check_isinfp ("expm1 (+inf) == +inf", FUNC(expm1) (plus_infty));
  check ("expm1 (-inf) == -1", FUNC(expm1) (minus_infty), -1);
#endif

  check_eps ("expm1 (1) == e-1", FUNC(expm1) (1), M_El - 1.0,
	     CHOOSE (4e-18L, 0, 2e-7));

  check_eps ("expm1 (0.7) == 1.01375...", FUNC(expm1) (0.7),
	     1.0137527074704765216L, CHOOSE (9e-17L, 0, 0));
}


static void
check_frexp (const char *test_name, MATHTYPE computed, MATHTYPE expected,
	     int comp_int, int exp_int)
{
  MATHTYPE diff;
  int result;

  result = (check_equal (computed, expected, 0, &diff)
	    && (comp_int == exp_int));

  if (result)
    {
      if (verbose > 2)
	printf ("Pass: %s\n", test_name);
    }
  else
    {
      if (verbose)
	printf ("Fail: %s\n", test_name);
      if (verbose > 1)
	{
	  printf ("Result:\n");
	  printf (" is:         %.20" PRINTF_EXPR " *2^%d  %.20"
		  PRINTF_XEXPR "*2^%d\n",
		  computed, comp_int, computed, comp_int);
	  printf (" should be:  %.20" PRINTF_EXPR " *2^%d  %.20"
		  PRINTF_XEXPR "*2^%d\n",
		  expected, exp_int, expected, exp_int);
	  printf (" difference: %.20" PRINTF_EXPR "  %.20" PRINTF_XEXPR "\n",
		  diff, diff);
	}
      noErrors++;
    }
  fpstack_test (test_name);
  output_result (test_name, result,
		 computed, expected, diff, PRINT, PRINT);
}


static void
frexp_test (void)
{
  int x_int;
  MATHTYPE result;

  result = FUNC(frexp) (plus_infty, &x_int);
  check_isinfp ("frexp (+inf, expr) == +inf", result);

  result = FUNC(frexp) (minus_infty, &x_int);
  check_isinfn ("frexp (-inf, expr) == -inf", result);

  result = FUNC(frexp) (nan_value, &x_int);
  check_isnan ("frexp (Nan, expr) == NaN", result);

  result = FUNC(frexp) (0, &x_int);
  check_frexp ("frexp: +0 == 0 * 2^0", result, 0, x_int, 0);

  result = FUNC(frexp) (minus_zero, &x_int);
  check_frexp ("frexp: -0 == -0 * 2^0", result, minus_zero, x_int, 0);

  result = FUNC(frexp) (12.8L, &x_int);
  check_frexp ("frexp: 12.8 == 0.8 * 2^4", result, 0.8L, x_int, 4);

  result = FUNC(frexp) (-27.34L, &x_int);
  check_frexp ("frexp: -27.34 == -0.854375 * 2^5",
	       result, -0.854375L, x_int, 5);
}


static void
fpclassify_test (void)
{
  MATHTYPE x;

  /* fpclassify is a macro, don't give it constants as parameter */
  check_bool ("fpclassify (NaN) == FP_NAN", fpclassify (nan_value) == FP_NAN);
  check_bool ("fpclassify (+inf) == FP_INFINITE",
	      fpclassify (plus_infty) == FP_INFINITE);
  check_bool ("fpclassify (-inf) == FP_INFINITE",
	      fpclassify (minus_infty) == FP_INFINITE);
  check_bool ("fpclassify (+0) == FP_ZERO",
	      fpclassify (plus_zero) == FP_ZERO);
  check_bool ("fpclassify (-0) == FP_ZERO",
	      fpclassify (minus_zero) == FP_ZERO);

  x = 1000.0;
  check_bool ("fpclassify (1000) == FP_NORMAL",
	      fpclassify (x) == FP_NORMAL);
}


static void
isfinite_test (void)
{
  check_bool ("isfinite (0) != 0", isfinite (0));
  check_bool ("isfinite (-0) != 0", isfinite (minus_zero));
  check_bool ("isfinite (10) != 0", isfinite (10));
  check_bool ("isfinite (+inf) == 0", isfinite (plus_infty) == 0);
  check_bool ("isfinite (-inf) == 0", isfinite (minus_infty) == 0);
  check_bool ("isfinite (NaN) == 0", isfinite (nan_value) == 0);
}


static void
isnormal_test (void)
{
  check_bool ("isnormal (0) == 0", isnormal (0) == 0);
  check_bool ("isnormal (-0) == 0", isnormal (minus_zero) == 0);
  check_bool ("isnormal (10) != 0", isnormal (10));
  check_bool ("isnormal (+inf) == 0", isnormal (plus_infty) == 0);
  check_bool ("isnormal (-inf) == 0", isnormal (minus_infty) == 0);
  check_bool ("isnormal (NaN) == 0", isnormal (nan_value) == 0);
}


static void
signbit_test (void)
{
  MATHTYPE x;

  check_bool ("signbit (+0) == 0", signbit (0) == 0);
  check_bool ("signbit (-0) != 0", signbit (minus_zero));
  check_bool ("signbit (+inf) == 0", signbit (plus_infty) == 0);
  check_bool ("signbit (-inf) != 0", signbit (minus_infty));

  x = random_less (0);
  check_bool ("signbit (x) != 0 for x < 0", signbit (x));

  x = random_greater (0);
  check_bool ("signbit (x) == 0 for x > 0", signbit (x) == 0);
}


static void
gamma_test (void)
{
  errno = 0;
  FUNC(gamma) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;
  feclearexcept (FE_ALL_EXCEPT);


  check_isinfp ("gamma (+inf) == +inf", FUNC(gamma) (plus_infty));
  check_isinfp_exc ("gamma (0) == +inf plus divide by zero exception",
		    FUNC(gamma) (0), DIVIDE_BY_ZERO_EXCEPTION);

  check_isinfp_exc ("gamma (x) == +inf plus divide by zero exception for integer x <= 0",
		    FUNC(gamma) (-3), DIVIDE_BY_ZERO_EXCEPTION);
  check_isnan_exc ("gamma (-inf) == NaN plus invalid exception",
		   FUNC(gamma) (minus_infty), INVALID_EXCEPTION);

  signgam = 0;
  check ("gamma (1) == 0", FUNC(gamma) (1), 0);
  check_int ("gamma (1) sets signgam to 1", signgam, 1);

  signgam = 0;
  check ("gamma (3) == M_LN2", FUNC(gamma) (3), M_LN2l);
  check_int ("gamma (3) sets signgam to 1", signgam, 1);

  signgam = 0;
  check_eps ("gamma (0.5) == log(sqrt(pi))", FUNC(gamma) (0.5),
	     FUNC(log) (FUNC(sqrt) (M_PIl)), CHOOSE (0, 1e-15, 1e-7));
  check_int ("gamma (0.5) sets signgam to 1", signgam, 1);

  signgam = 0;
  check_eps ("gamma (-0.5) == log(2*sqrt(pi))", FUNC(gamma) (-0.5),
	     FUNC(log) (2*FUNC(sqrt) (M_PIl)), CHOOSE (0, 1e-15, 0));

  check_int ("gamma (-0.5) sets signgam to -1", signgam, -1);
}


static void
tgamma_test (void)
{
  errno = 0;
  FUNC(tgamma) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;
  feclearexcept (FE_ALL_EXCEPT);

  check_isinfp ("tgamma (+inf) == +inf", FUNC(tgamma) (plus_infty));
  check_isnan_exc ("tgamma (0) == NaN plus invalid exception",
		   FUNC(tgamma) (0), INVALID_EXCEPTION);

  check_isnan_exc_ext ("tgamma (x) == NaN plus invalid exception for integer x <= 0",
		       FUNC(tgamma) (-2), INVALID_EXCEPTION, -2);
  check_isnan_exc ("tgamma (-inf) == NaN plus invalid exception",
		   FUNC(tgamma) (minus_infty), INVALID_EXCEPTION);

  check_eps ("tgamma (0.5) == sqrt(pi)", FUNC(tgamma) (0.5),
	     FUNC(sqrt) (M_PIl), CHOOSE (0, 5e-16, 2e-7));
  check_eps ("tgamma (-0.5) == -2*sqrt(pi)", FUNC(tgamma) (-0.5),
	     -2*FUNC(sqrt) (M_PIl), CHOOSE (0, 5e-16, 3e-7));

  check ("tgamma (1) == 1", FUNC(tgamma) (1), 1);
  check_eps ("tgamma (4) == 6", FUNC(tgamma) (4), 6, CHOOSE (0, 8.9e-16, 0));

  check_eps ("tgamma (0.7) == 1.29805...", FUNC(tgamma) (0.7),
	     1.29805533264755778568L, CHOOSE (0, 3e-16, 2e-7));
  check_eps ("tgamma (1.2) == 0.91816...", FUNC(tgamma) (1.2),
	     0.91816874239976061064L, CHOOSE (0, 1.2e-16, 0));
}


static void
lgamma_test (void)
{
  errno = 0;
  FUNC(lgamma) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;
  feclearexcept (FE_ALL_EXCEPT);

  check_isinfp ("lgamma (+inf) == +inf", FUNC(lgamma) (plus_infty));
  check_isinfp_exc ("lgamma (0) == +inf plus divide by zero exception",
		    FUNC(lgamma) (0), DIVIDE_BY_ZERO_EXCEPTION);

  check_isinfp_exc ("lgamma (x) == +inf plus divide by zero exception for integer x <= 0",
		    FUNC(lgamma) (-3), DIVIDE_BY_ZERO_EXCEPTION);
  check_isnan_exc ("lgamma (-inf) == NaN plus invalid exception",
		   FUNC(lgamma) (minus_infty), INVALID_EXCEPTION);

  signgam = 0;
  check ("lgamma (1) == 0", FUNC(lgamma) (1), 0);
  check_int ("lgamma (0) sets signgam to 1", signgam, 1);

  signgam = 0;
  check ("lgamma (3) == M_LN2", FUNC(lgamma) (3), M_LN2l);
  check_int ("lgamma (3) sets signgam to 1", signgam, 1);

  signgam = 0;
  check_eps ("lgamma (0.5) == log(sqrt(pi))", FUNC(lgamma) (0.5),
	     FUNC(log) (FUNC(sqrt) (M_PIl)), CHOOSE (0, 1e-15, 1e-7));
  check_int ("lgamma (0.5) sets signgam to 1", signgam, 1);

  signgam = 0;
  check_eps ("lgamma (-0.5) == log(2*sqrt(pi))", FUNC(lgamma) (-0.5),
	     FUNC(log) (2*FUNC(sqrt) (M_PIl)), CHOOSE (0, 1e-15, 0));

  check_int ("lgamma (-0.5) sets signgam to -1", signgam, -1);

  signgam = 0;
  check_eps ("lgamma (0.7) == 0.26086...", FUNC(lgamma) (0.7),
	     0.26086724653166651439L, CHOOSE (0, 6e-17, 3e-8));
  check_int ("lgamma (0.7) sets signgam to 1", signgam, 1);

  signgam = 0;
  check_eps ("lgamma (1.2) == -0.08537...", FUNC(lgamma) (1.2),
	     -0.853740900033158497197e-1L, CHOOSE (0, 2e-17, 2e-8));
  check_int ("lgamma (1.2) sets signgam to 1", signgam, 1);
}


static void
ilogb_test (void)
{
  int i;

  check_int ("ilogb (1) == 0", FUNC(ilogb) (1), 0);
  check_int ("ilogb (e) == 1", FUNC(ilogb) (M_El), 1);
  check_int ("ilogb (1024) == 10", FUNC(ilogb) (1024), 10);
  check_int ("ilogb (-2000) == 10", FUNC(ilogb) (-2000), 10);

  /* XXX We have a problem here: the standard does not tell us whether
     exceptions are allowed/required.  ignore them for now.  */
  i = FUNC(ilogb) (0.0);
  feclearexcept (FE_ALL_EXCEPT);
  check_int ("ilogb (0) == FP_ILOGB0", i, FP_ILOGB0);
  i = FUNC(ilogb) (nan_value);
  feclearexcept (FE_ALL_EXCEPT);
  check_int ("ilogb (NaN) == FP_ILOGBNAN", i, FP_ILOGBNAN);
}


static void
ldexp_test (void)
{
  MATHTYPE x;

  check ("ldexp (0, 0) == 0", FUNC(ldexp) (0, 0), 0);

  check_isinfp ("ldexp (+inf, 1) == +inf", FUNC(ldexp) (plus_infty, 1));
  check_isinfn ("ldexp (-inf, 1) == -inf", FUNC(ldexp) (minus_infty, 1));
  check_isnan ("ldexp (NaN, 1) == NaN", FUNC(ldexp) (nan_value, 1));

  check ("ldexp (0.8, 4) == 12.8", FUNC(ldexp) (0.8L, 4), 12.8L);
  check ("ldexp (-0.854375, 5) == -27.34", FUNC(ldexp) (-0.854375L, 5), -27.34L);

  x = random_greater (0.0);
  check_ext ("ldexp (x, 0) == x", FUNC(ldexp) (x, 0L), x, x);
}


static void
log_test (void)
{
  check_isinfn_exc ("log (+0) == -inf plus divide-by-zero exception",
		    FUNC(log) (0), DIVIDE_BY_ZERO_EXCEPTION);
  check_isinfn_exc ("log (-0) == -inf plus divide-by-zero exception",
		    FUNC(log) (minus_zero), DIVIDE_BY_ZERO_EXCEPTION);

  check ("log (1) == 0", FUNC(log) (1), 0);

  check_isnan_exc ("log (x) == NaN plus invalid exception if x < 0",
		   FUNC(log) (-1), INVALID_EXCEPTION);
  check_isinfp ("log (+inf) == +inf", FUNC(log) (plus_infty));

  check_eps ("log (e) == 1", FUNC(log) (M_El), 1, CHOOSE (1e-18L, 0, 9e-8L));
  check_eps ("log (1/e) == -1", FUNC(log) (1.0 / M_El), -1,
	     CHOOSE (2e-18L, 0, 0));
  check_eps ("log (2) == M_LN2", FUNC(log) (2), M_LN2l,
	     CHOOSE (6e-20L, 0, 0));
  check_eps ("log (10) == M_LN10", FUNC(log) (10), M_LN10l,
	     CHOOSE (1e-18L, 0, 0));
  check_eps ("log (0.7) == -0.3566749439...", FUNC(log) (0.7),
	     -0.35667494393873237891L, CHOOSE (7e-17L, 6e-17, 3e-8));
}


static void
log10_test (void)
{
  check_isinfn_exc ("log10 (+0) == -inf plus divide-by-zero exception",
		    FUNC(log10) (0), DIVIDE_BY_ZERO_EXCEPTION);
  check_isinfn_exc ("log10 (-0) == -inf plus divide-by-zero exception",
		    FUNC(log10) (minus_zero), DIVIDE_BY_ZERO_EXCEPTION);

  check ("log10 (1) == +0", FUNC(log10) (1), 0);

  check_isnan_exc ("log10 (x) == NaN plus invalid exception if x < 0",
		   FUNC(log10) (-1), INVALID_EXCEPTION);

  check_isinfp ("log10 (+inf) == +inf", FUNC(log10) (plus_infty));

  check_eps ("log10 (0.1) == -1", FUNC(log10) (0.1L), -1,
	     CHOOSE (1e-18L, 0, 0));
  check_eps ("log10 (10) == 1", FUNC(log10) (10.0), 1,
	     CHOOSE (1e-18L, 0, 0));
  check_eps ("log10 (100) == 2", FUNC(log10) (100.0), 2,
	     CHOOSE (1e-18L, 0, 0));
  check ("log10 (10000) == 4", FUNC(log10) (10000.0), 4);
  check_eps ("log10 (e) == M_LOG10E", FUNC(log10) (M_El), M_LOG10El,
	     CHOOSE (1e-18, 0, 9e-8));
  check_eps ("log10 (0.7) == -0.1549019599...", FUNC(log10) (0.7),
	     -0.15490195998574316929L, CHOOSE (3e-17L, 3e-17, 2e-8));
}


static void
log1p_test (void)
{
  check ("log1p (+0) == +0", FUNC(log1p) (0), 0);
  check ("log1p (-0) == -0", FUNC(log1p) (minus_zero), minus_zero);

  check_isinfn_exc ("log1p (-1) == -inf plus divide-by-zero exception",
		    FUNC(log1p) (-1), DIVIDE_BY_ZERO_EXCEPTION);
  check_isnan_exc ("log1p (x) == NaN plus invalid exception if x < -1",
		   FUNC(log1p) (-2), INVALID_EXCEPTION);

  check_isinfp ("log1p (+inf) == +inf", FUNC(log1p) (plus_infty));

  check_eps ("log1p (e-1) == 1", FUNC(log1p) (M_El - 1.0), 1,
	     CHOOSE (1e-18L, 0, 6e-8));

  check_eps ("log1p (-0.3) == -0.35667...", FUNC(log1p) (-0.3),
	     -0.35667494393873237891L, CHOOSE (2e-17L, 6e-17, 3e-8));
}


static void
log2_test (void)
{
  check_isinfn_exc ("log2 (+0) == -inf plus divide-by-zero exception",
		    FUNC(log2) (0), DIVIDE_BY_ZERO_EXCEPTION);
  check_isinfn_exc ("log2 (-0) == -inf plus divide-by-zero exception",
		    FUNC(log2) (minus_zero), DIVIDE_BY_ZERO_EXCEPTION);

  check ("log2 (1) == +0", FUNC(log2) (1), 0);

  check_isnan_exc ("log2 (x) == NaN plus invalid exception if x < 0",
		   FUNC(log2) (-1), INVALID_EXCEPTION);

  check_isinfp ("log2 (+inf) == +inf", FUNC(log2) (plus_infty));

  check_eps ("log2 (e) == M_LOG2E", FUNC(log2) (M_El), M_LOG2El,
	     CHOOSE (1e-18L, 0, 0));
  check ("log2 (2) == 1", FUNC(log2) (2.0), 1);
  check_eps ("log2 (16) == 4", FUNC(log2) (16.0), 4, CHOOSE (1e-18L, 0, 0));
  check ("log2 (256) == 8", FUNC(log2) (256.0), 8);
  check_eps ("log2 (0.7) == -0.5145731728...", FUNC(log2) (0.7),
	     -0.51457317282975824043L, CHOOSE (1e-16L, 2e-16, 6e-8));
}


static void
logb_test (void)
{
  check_isinfp ("logb (+inf) == +inf", FUNC(logb) (plus_infty));
  check_isinfp ("logb (-inf) == +inf", FUNC(logb) (minus_infty));

  check_isinfn_exc ("logb (+0) == -inf plus divide-by-zero exception",
		    FUNC(logb) (0), DIVIDE_BY_ZERO_EXCEPTION);

  check_isinfn_exc ("logb (-0) == -inf plus divide-by-zero exception",
		    FUNC(logb) (minus_zero), DIVIDE_BY_ZERO_EXCEPTION);

  check ("logb (1) == 0", FUNC(logb) (1), 0);
  check ("logb (e) == 1", FUNC(logb) (M_El), 1);
  check ("logb (1024) == 10", FUNC(logb) (1024), 10);
  check ("logb (-2000) == 10", FUNC(logb) (-2000), 10);
}


static void
modf_test (void)
{
  MATHTYPE result, intpart;

  result = FUNC(modf) (plus_infty, &intpart);
  check ("modf (+inf, &x) returns +0", result, 0);
  check_isinfp ("modf (+inf, &x) set x to +inf", intpart);

  result = FUNC(modf) (minus_infty, &intpart);
  check ("modf (-inf, &x) returns -0", result, minus_zero);
  check_isinfn ("modf (-inf, &x) sets x to -inf", intpart);

  result = FUNC(modf) (nan_value, &intpart);
  check_isnan ("modf (NaN, &x) returns NaN", result);
  check_isnan ("modf (NaN, &x) sets x to NaN", intpart);

  result = FUNC(modf) (0, &intpart);
  check ("modf (0, &x) returns 0", result, 0);
  check ("modf (0, &x) sets x to 0", intpart, 0);

  result = FUNC(modf) (minus_zero, &intpart);
  check ("modf (-0, &x) returns -0", result, minus_zero);
  check ("modf (-0, &x) sets x to -0", intpart, minus_zero);

  result = FUNC(modf) (1.5, &intpart);
  check ("modf (1.5, &x) returns 0.5", result, 0.5);
  check ("modf (1.5, &x) sets x to 1", intpart, 1);

  result = FUNC(modf) (2.5, &intpart);
  check ("modf (2.5, &x) returns 0.5", result, 0.5);
  check ("modf (2.5, &x) sets x to 2", intpart, 2);

  result = FUNC(modf) (-2.5, &intpart);
  check ("modf (-2.5, &x) returns -0.5", result, -0.5);
  check ("modf (-2.5, &x) sets x to -2", intpart, -2);

  result = FUNC(modf) (20, &intpart);
  check ("modf (20, &x) returns 0", result, 0);
  check ("modf (20, &x) sets x to 20", intpart, 20);

  result = FUNC(modf) (21, &intpart);
  check ("modf (21, &x) returns 0", result, 0);
  check ("modf (21, &x) sets x to 21", intpart, 21);

  result = FUNC(modf) (89.6, &intpart);
  check_eps ("modf (89.6, &x) returns 0.6", result, 0.6,
	     CHOOSE (6e-15L, 6e-15, 2e-6));
  check ("modf (89.6, &x) sets x to 89", intpart, 89);
}


static void
scalb_test (void)
{
  MATHTYPE x;

  check_isnan ("scalb (2, 0.5) == NaN", FUNC(scalb) (2, 0.5));
  check_isnan ("scalb (3, -2.5) == NaN", FUNC(scalb) (3, -2.5));

  check_isnan ("scalb (0, NaN) == NaN", FUNC(scalb) (0, nan_value));
  check_isnan ("scalb (1, NaN) == NaN", FUNC(scalb) (1, nan_value));

  x = random_greater (0.0);
  check ("scalb (x, 0) == 0", FUNC(scalb) (x, 0), x);
  x = random_greater (0.0);
  check ("scalb (-x, 0) == 0", FUNC(scalb) (-x, 0), -x);

  check_isnan_exc ("scalb (+0, +inf) == NaN plus invalid exception",
		   FUNC(scalb) (0, plus_infty), INVALID_EXCEPTION);
  check_isnan_exc ("scalb (-0, +inf) == NaN plus invalid exception",
		   FUNC(scalb) (minus_zero, plus_infty), INVALID_EXCEPTION);

  check ("scalb (+0, 2) == +0", FUNC(scalb) (0, 2), 0);
  check ("scalb (-0, 4) == -0", FUNC(scalb) (minus_zero, -4), minus_zero);
  check ("scalb (+0, 0) == +0", FUNC(scalb) (0, 0), 0);
  check ("scalb (-0, 0) == -0", FUNC(scalb) (minus_zero, 0), minus_zero);
  check ("scalb (+0, -1) == +0", FUNC(scalb) (0, -1), 0);
  check ("scalb (-0, -10) == -0", FUNC(scalb) (minus_zero, -10), minus_zero);
  check ("scalb (+0, -inf) == +0", FUNC(scalb) (0, minus_infty), 0);
  check ("scalb (-0, -inf) == -0", FUNC(scalb) (minus_zero, minus_infty),
	 minus_zero);

  check_isinfp ("scalb (+inf, -1) == +inf", FUNC(scalb) (plus_infty, -1));
  check_isinfn ("scalb (-inf, -10) == -inf", FUNC(scalb) (minus_infty, -10));
  check_isinfp ("scalb (+inf, 0) == +inf", FUNC(scalb) (plus_infty, 0));
  check_isinfn ("scalb (-inf, 0) == -inf", FUNC(scalb) (minus_infty, 0));
  check_isinfp ("scalb (+inf, 2) == +inf", FUNC(scalb) (plus_infty, 2));
  check_isinfn ("scalb (-inf, 100) == -inf", FUNC(scalb) (minus_infty, 100));

  x = random_greater (0.0);
  check ("scalb (x, -inf) == 0", FUNC(scalb) (x, minus_infty), 0.0);
  check ("scalb (-x, -inf) == -0", FUNC(scalb) (-x, minus_infty), minus_zero);

  x = random_greater (0.0);
  check_isinfp ("scalb (x, +inf) == +inf", FUNC(scalb) (x, plus_infty));
  x = random_greater (0.0);
  check_isinfn ("scalb (-x, +inf) == -inf", FUNC(scalb) (-x, plus_infty));
  check_isinfp ("scalb (+inf, +inf) == +inf",
		FUNC(scalb) (plus_infty, plus_infty));
  check_isinfn ("scalb (-inf, +inf) == -inf",
		FUNC(scalb) (minus_infty, plus_infty));

  check_isnan ("scalb (+inf, -inf) == NaN",
	       FUNC(scalb) (plus_infty, minus_infty));
  check_isnan ("scalb (-inf, -inf) == NaN",
	       FUNC(scalb) (minus_infty, minus_infty));

  check_isnan ("scalb (NaN, 1) == NaN", FUNC(scalb) (nan_value, 1));
  check_isnan ("scalb (1, NaN) == NaN", FUNC(scalb) (1, nan_value));
  check_isnan ("scalb (NaN, 0) == NaN", FUNC(scalb) (nan_value, 0));
  check_isnan ("scalb (0, NaN) == NaN", FUNC(scalb) (0, nan_value));
  check_isnan ("scalb (NaN, +inf) == NaN",
	       FUNC(scalb) (nan_value, plus_infty));
  check_isnan ("scalb (+inf, NaN) == NaN",
	       FUNC(scalb) (plus_infty, nan_value));
  check_isnan ("scalb (NaN, NaN) == NaN", FUNC(scalb) (nan_value, nan_value));

  check ("scalb (0.8, 4) == 12.8", FUNC(scalb) (0.8L, 4), 12.8L);
  check ("scalb (-0.854375, 5) == -27.34", FUNC(scalb) (-0.854375L, 5), -27.34L);
}


static void
scalbn_test (void)
{
  MATHTYPE x;

  check ("scalbn (0, 0) == 0", FUNC(scalbn) (0, 0), 0);

  check_isinfp ("scalbn (+inf, 1) == +inf", FUNC(scalbn) (plus_infty, 1));
  check_isinfn ("scalbn (-inf, 1) == -inf", FUNC(scalbn) (minus_infty, 1));
  check_isnan ("scalbn (NaN, 1) == NaN", FUNC(scalbn) (nan_value, 1));

  check ("scalbn (0.8, 4) == 12.8", FUNC(scalbn) (0.8L, 4), 12.8L);
  check ("scalbn (-0.854375, 5) == -27.34", FUNC(scalbn) (-0.854375L, 5), -27.34L);

  x = random_greater (0.0);
  check_ext ("scalbn (x, 0) == x", FUNC(scalbn) (x, 0L), x, x);
}


static void
sin_test (void)
{
  check ("sin (+0) == +0", FUNC(sin) (0), 0);
  check ("sin (-0) == -0", FUNC(sin) (minus_zero), minus_zero);
  check_isnan_exc ("sin (+inf) == NaN plus invalid exception",
		   FUNC(sin) (plus_infty),
		   INVALID_EXCEPTION);
  check_isnan_exc ("sin (-inf) == NaN plus invalid exception",
		   FUNC(sin) (minus_infty),
		   INVALID_EXCEPTION);

  check_eps ("sin (pi/6) == 0.5", FUNC(sin) (M_PI_6l),
	     0.5, CHOOSE (4e-18L, 0, 0));
  check_eps ("sin (-pi/6) == -0.5", FUNC(sin) (-M_PI_6l),
	     -0.5, CHOOSE (4e-18L, 0, 0));
  check ("sin (pi/2) == 1", FUNC(sin) (M_PI_2l), 1);
  check ("sin (-pi/2) == -1", FUNC(sin) (-M_PI_2l), -1);
  check_eps ("sin (0.7) == 0.6442176872...", FUNC(sin) (0.7),
	     0.64421768723769105367L, CHOOSE (4e-17L, 0, 0));
}


static void
sinh_test (void)
{
  check ("sinh (+0) == +0", FUNC(sinh) (0), 0);

#ifndef TEST_INLINE
  check ("sinh (-0) == -0", FUNC(sinh) (minus_zero), minus_zero);

  check_isinfp ("sinh (+inf) == +inf", FUNC(sinh) (plus_infty));
  check_isinfn ("sinh (-inf) == -inf", FUNC(sinh) (minus_infty));
#endif

  check_eps ("sinh (0.7) == 0.7585837018...", FUNC(sinh) (0.7),
	     0.75858370183953350346L, CHOOSE (6e-17L, 2e-16, 6e-8));
}


static void
sincos_test (void)
{
  MATHTYPE sin_res, cos_res;
  fenv_t fenv;

  FUNC(sincos) (0, &sin_res, &cos_res);
  fegetenv (&fenv);
  check ("sincos (+0, &sin, &cos) puts +0 in sin", sin_res, 0);
  fesetenv (&fenv);
  check ("sincos (+0, &sin, &cos) puts 1 in cos", cos_res, 1);

  FUNC(sincos) (minus_zero, &sin_res, &cos_res);
  fegetenv (&fenv);
  check ("sincos (-0, &sin, &cos) puts -0 in sin", sin_res, minus_zero);
  fesetenv (&fenv);
  check ("sincos (-0, &sin, &cos) puts 1 in cos", cos_res, 1);

  FUNC(sincos) (plus_infty, &sin_res, &cos_res);
  fegetenv (&fenv);
  check_isnan_exc ("sincos (+inf, &sin, &cos) puts NaN in sin plus invalid exception",
		   sin_res, INVALID_EXCEPTION);
  fesetenv (&fenv);
  check_isnan_exc ("sincos (+inf, &sin, &cos) puts NaN in cos plus invalid exception",
		   cos_res, INVALID_EXCEPTION);

  FUNC(sincos) (minus_infty, &sin_res, &cos_res);
  fegetenv (&fenv);
  check_isnan_exc ("sincos (-inf,&sin, &cos) puts NaN in sin plus invalid exception",
		   sin_res, INVALID_EXCEPTION);
  fesetenv (&fenv);
  check_isnan_exc ("sincos (-inf,&sin, &cos) puts NaN in cos plus invalid exception",
		   cos_res, INVALID_EXCEPTION);

  FUNC(sincos) (M_PI_2l, &sin_res, &cos_res);
  fegetenv (&fenv);
  check ("sincos (pi/2, &sin, &cos) puts 1 in sin", sin_res, 1);
  fesetenv (&fenv);
  check_eps ("sincos (pi/2, &sin, &cos) puts 0 in cos", cos_res, 0,
	     CHOOSE (1e-18L, 1e-16, 1e-7));

  FUNC(sincos) (M_PI_6l, &sin_res, &cos_res);
  check_eps ("sincos (pi/6, &sin, &cos) puts 0.5 in sin", sin_res, 0.5,
	     CHOOSE (5e-18L, 0, 0));

  FUNC(sincos) (M_PI_6l*2.0, &sin_res, &cos_res);
  check_eps ("sincos (pi/3, &sin, &cos) puts 0.5 in cos", cos_res, 0.5,
	     CHOOSE (5e-18L, 1e-15, 1e-7));

  FUNC(sincos) (0.7, &sin_res, &cos_res);
  check_eps ("sincos (0.7, &sin, &cos) puts 0.6442176872... in sin", sin_res,
	     0.64421768723769105367L, CHOOSE (4e-17L, 0, 0));
  check_eps ("sincos (0.7, &sin, &cos) puts 0.7648421872... in cos", cos_res,
	     0.76484218728448842626L, CHOOSE (3e-17L, 2e-16, 6e-8));
}


static void
tan_test (void)
{
  check ("tan (+0) == +0", FUNC(tan) (0), 0);
  check ("tan (-0) == -0", FUNC(tan) (minus_zero), minus_zero);
  check_isnan_exc ("tan (+inf) == NaN plus invalid exception",
		   FUNC(tan) (plus_infty), INVALID_EXCEPTION);
  check_isnan_exc ("tan (-inf) == NaN plus invalid exception",
		   FUNC(tan) (minus_infty), INVALID_EXCEPTION);

  check_eps ("tan (pi/4) == 1", FUNC(tan) (M_PI_4l), 1,
	     CHOOSE (2e-18L, 1e-15L, 2e-7));
  check_eps ("tan (0.7) == 0.8422883804...", FUNC(tan) (0.7),
	     0.84228838046307944813L, CHOOSE (8e-17L, 0, 0));
}


static void
tanh_test (void)
{
  check ("tanh (+0) == +0", FUNC(tanh) (0), 0);
#ifndef TEST_INLINE
  check ("tanh (-0) == -0", FUNC(tanh) (minus_zero), minus_zero);

  check ("tanh (+inf) == +1", FUNC(tanh) (plus_infty), 1);
  check ("tanh (-inf) == -1", FUNC(tanh) (minus_infty), -1);
#endif
  check_eps ("tanh (0.7) == 0.6043677771...", FUNC(tanh) (0.7),
	     0.60436777711716349631L, CHOOSE (3e-17L, 2e-16, 6e-8));
}


static void
fabs_test (void)
{
  check ("fabs (+0) == +0", FUNC(fabs) (0), 0);
  check ("fabs (-0) == +0", FUNC(fabs) (minus_zero), 0);

  check_isinfp ("fabs (+inf) == +inf", FUNC(fabs) (plus_infty));
  check_isinfp ("fabs (-inf) == +inf", FUNC(fabs) (minus_infty));

  check ("fabs (+38) == 38", FUNC(fabs) (38.0), 38.0);
  check ("fabs (-e) == e", FUNC(fabs) (-M_El), M_El);
}


static void
floor_test (void)
{
  check ("floor (+0) == +0", FUNC(floor) (0.0), 0.0);
  check ("floor (-0) == -0", FUNC(floor) (minus_zero), minus_zero);
  check_isinfp ("floor (+inf) == +inf", FUNC(floor) (plus_infty));
  check_isinfn ("floor (-inf) == -inf", FUNC(floor) (minus_infty));

  check ("floor (pi) == 3", FUNC(floor) (M_PIl), 3.0);
  check ("floor (-pi) == -4", FUNC(floor) (-M_PIl), -4.0);
}


static void
hypot_test (void)
{
  MATHTYPE a;

  a = random_greater (0);
  check_isinfp_ext ("hypot (+inf, x) == +inf", FUNC(hypot) (plus_infty, a), a);
  check_isinfp_ext ("hypot (-inf, x) == +inf", FUNC(hypot) (minus_infty, a), a);

#ifndef TEST_INLINE
  check_isinfp ("hypot (+inf, NaN) == +inf", FUNC(hypot) (plus_infty, nan_value));
  check_isinfp ("hypot (-inf, NaN) == +inf", FUNC(hypot) (minus_infty, nan_value));
#endif

  check_isnan ("hypot (NaN, NaN) == NaN", FUNC(hypot) (nan_value, nan_value));

  a = FUNC(hypot) (12.4L, 0.7L);
  check ("hypot (x,y) == hypot (y,x)", FUNC(hypot) (0.7L, 12.4L), a);
  check ("hypot (x,y) == hypot (-x,y)", FUNC(hypot) (-12.4L, 0.7L), a);
  check ("hypot (x,y) == hypot (-y,x)", FUNC(hypot) (-0.7L, 12.4L), a);
  check ("hypot (x,y) == hypot (-x,-y)", FUNC(hypot) (-12.4L, -0.7L), a);
  check ("hypot (x,y) == hypot (-y,-x)", FUNC(hypot) (-0.7L, -12.4L), a);
  check ("hypot (x,0) == fabs (x)", FUNC(hypot) (-0.7L, 0), 0.7L);
  check ("hypot (x,0) == fabs (x)", FUNC(hypot) (0.7L, 0), 0.7L);
  check ("hypot (x,0) == fabs (x)", FUNC(hypot) (-1.0L, 0), 1.0L);
  check ("hypot (x,0) == fabs (x)", FUNC(hypot) (1.0L, 0), 1.0L);
  check ("hypot (x,0) == fabs (x)", FUNC(hypot) (-5.7e7L, 0), 5.7e7L);
  check ("hypot (x,0) == fabs (x)", FUNC(hypot) (5.7e7L, 0), 5.7e7L);

  check_eps ("hypot (0.7,1.2) == 1.38924...", FUNC(hypot) (0.7, 1.2),
	     1.3892443989449804508L, CHOOSE (7e-17L, 3e-16, 0));
}


static void
pow_test (void)
{
  MATHTYPE x;

  check ("pow (+0, +0) == 1", FUNC(pow) (0, 0), 1);
  check ("pow (+0, -0) == 1", FUNC(pow) (0, minus_zero), 1);
  check ("pow (-0, +0) == 1", FUNC(pow) (minus_zero, 0), 1);
  check ("pow (-0, -0) == 1", FUNC(pow) (minus_zero, minus_zero), 1);

  check ("pow (+10, +0) == 1", FUNC(pow) (10, 0), 1);
  check ("pow (+10, -0) == 1", FUNC(pow) (10, minus_zero), 1);
  check ("pow (-10, +0) == 1", FUNC(pow) (-10, 0), 1);
  check ("pow (-10, -0) == 1", FUNC(pow) (-10, minus_zero), 1);

  check ("pow (NaN, +0) == 1", FUNC(pow) (nan_value, 0), 1);
  check ("pow (NaN, -0) == 1", FUNC(pow) (nan_value, minus_zero), 1);

#ifndef TEST_INLINE
  check_isinfp ("pow (+1.1, +inf) == +inf", FUNC(pow) (1.1, plus_infty));
  check_isinfp ("pow (+inf, +inf) == +inf", FUNC(pow) (plus_infty, plus_infty));
  check_isinfp ("pow (-1.1, +inf) == +inf", FUNC(pow) (-1.1, plus_infty));
  check_isinfp ("pow (-inf, +inf) == +inf", FUNC(pow) (minus_infty, plus_infty));

  check ("pow (0.9, +inf) == +0", FUNC(pow) (0.9L, plus_infty), 0);
  check ("pow (1e-7, +inf) == +0", FUNC(pow) (1e-7L, plus_infty), 0);
  check ("pow (-0.9, +inf) == +0", FUNC(pow) (-0.9L, plus_infty), 0);
  check ("pow (-1e-7, +inf) == +0", FUNC(pow) (-1e-7L, plus_infty), 0);

  check ("pow (+1.1, -inf) == 0", FUNC(pow) (1.1, minus_infty), 0);
  check ("pow (+inf, -inf) == 0", FUNC(pow) (plus_infty, minus_infty), 0);
  check ("pow (-1.1, -inf) == 0", FUNC(pow) (-1.1, minus_infty), 0);
  check ("pow (-inf, -inf) == 0", FUNC(pow) (minus_infty, minus_infty), 0);

  check_isinfp ("pow (0.9, -inf) == +inf", FUNC(pow) (0.9L, minus_infty));
  check_isinfp ("pow (1e-7, -inf) == +inf", FUNC(pow) (1e-7L, minus_infty));
  check_isinfp ("pow (-0.9, -inf) == +inf", FUNC(pow) (-0.9L, minus_infty));
  check_isinfp ("pow (-1e-7, -inf) == +inf", FUNC(pow) (-1e-7L, minus_infty));

  check_isinfp ("pow (+inf, 1e-7) == +inf", FUNC(pow) (plus_infty, 1e-7L));
  check_isinfp ("pow (+inf, 1) == +inf", FUNC(pow) (plus_infty, 1));
  check_isinfp ("pow (+inf, 1e7) == +inf", FUNC(pow) (plus_infty, 1e7L));

  check ("pow (+inf, -1e-7) == 0", FUNC(pow) (plus_infty, -1e-7L), 0);
  check ("pow (+inf, -1) == 0", FUNC(pow) (plus_infty, -1), 0);
  check ("pow (+inf, -1e7) == 0", FUNC(pow) (plus_infty, -1e7L), 0);

  check_isinfn ("pow (-inf, 1) == -inf", FUNC(pow) (minus_infty, 1));
  check_isinfn ("pow (-inf, 11) == -inf", FUNC(pow) (minus_infty, 11));
  check_isinfn ("pow (-inf, 1001) == -inf", FUNC(pow) (minus_infty, 1001));

  check_isinfp ("pow (-inf, 2) == +inf", FUNC(pow) (minus_infty, 2));
  check_isinfp ("pow (-inf, 12) == +inf", FUNC(pow) (minus_infty, 12));
  check_isinfp ("pow (-inf, 1002) == +inf", FUNC(pow) (minus_infty, 1002));
  check_isinfp ("pow (-inf, 0.1) == +inf", FUNC(pow) (minus_infty, 0.1));
  check_isinfp ("pow (-inf, 1.1) == +inf", FUNC(pow) (minus_infty, 1.1));
  check_isinfp ("pow (-inf, 11.1) == +inf", FUNC(pow) (minus_infty, 11.1));
  check_isinfp ("pow (-inf, 1001.1) == +inf", FUNC(pow) (minus_infty, 1001.1));

  check ("pow (-inf, -1) == -0", FUNC(pow) (minus_infty, -1), minus_zero);
  check ("pow (-inf, -11) == -0", FUNC(pow) (minus_infty, -11), minus_zero);
  check ("pow (-inf, -1001) == -0", FUNC(pow) (minus_infty, -1001), minus_zero);

  check ("pow (-inf, -2) == +0", FUNC(pow) (minus_infty, -2), 0);
  check ("pow (-inf, -12) == +0", FUNC(pow) (minus_infty, -12), 0);
  check ("pow (-inf, -1002) == +0", FUNC(pow) (minus_infty, -1002), 0);
  check ("pow (-inf, -0.1) == +0", FUNC(pow) (minus_infty, -0.1), 0);
  check ("pow (-inf, -1.1) == +0", FUNC(pow) (minus_infty, -1.1), 0);
  check ("pow (-inf, -11.1) == +0", FUNC(pow) (minus_infty, -11.1), 0);
  check ("pow (-inf, -1001.1) == +0", FUNC(pow) (minus_infty, -1001.1), 0);

  check_isnan ("pow (NaN, NaN) == NaN", FUNC(pow) (nan_value, nan_value));
  check_isnan ("pow (0, NaN) == NaN", FUNC(pow) (0, nan_value));
  check_isnan ("pow (1, NaN) == NaN", FUNC(pow) (1, nan_value));
  check_isnan ("pow (-1, NaN) == NaN", FUNC(pow) (-1, nan_value));
  check_isnan ("pow (NaN, 1) == NaN", FUNC(pow) (nan_value, 1));
  check_isnan ("pow (NaN, -1) == NaN", FUNC(pow) (nan_value, -1));

  x = random_greater (0.0);
  check_isnan_ext ("pow (x, NaN) == NaN", FUNC(pow) (x, nan_value), x);

  check_isnan_exc ("pow (+1, +inf) == NaN plus invalid exception",
		   FUNC(pow) (1, plus_infty), INVALID_EXCEPTION);
  check_isnan_exc ("pow (-1, +inf) == NaN plus invalid exception",
		   FUNC(pow) (-1, plus_infty), INVALID_EXCEPTION);
  check_isnan_exc ("pow (+1, -inf) == NaN plus invalid exception",
		   FUNC(pow) (1, minus_infty), INVALID_EXCEPTION);
  check_isnan_exc ("pow (-1, -inf) == NaN plus invalid exception",
		   FUNC(pow) (-1, minus_infty), INVALID_EXCEPTION);

  check_isnan_exc ("pow (-0.1, 1.1) == NaN plus invalid exception",
		   FUNC(pow) (-0.1, 1.1), INVALID_EXCEPTION);
  check_isnan_exc ("pow (-0.1, -1.1) == NaN plus invalid exception",
		   FUNC(pow) (-0.1, -1.1), INVALID_EXCEPTION);
  check_isnan_exc ("pow (-10.1, 1.1) == NaN plus invalid exception",
		   FUNC(pow) (-10.1, 1.1), INVALID_EXCEPTION);
  check_isnan_exc ("pow (-10.1, -1.1) == NaN plus invalid exception",
		   FUNC(pow) (-10.1, -1.1), INVALID_EXCEPTION);

  check_isinfp_exc ("pow (+0, -1) == +inf plus divide-by-zero exception",
		    FUNC(pow) (0, -1), DIVIDE_BY_ZERO_EXCEPTION);
  check_isinfp_exc ("pow (+0, -11) == +inf plus divide-by-zero exception",
		    FUNC(pow) (0, -11), DIVIDE_BY_ZERO_EXCEPTION);
  check_isinfn_exc ("pow (-0, -1) == -inf plus divide-by-zero exception",
		    FUNC(pow) (minus_zero, -1), DIVIDE_BY_ZERO_EXCEPTION);
  check_isinfn_exc ("pow (-0, -11) == -inf plus divide-by-zero exception",
		    FUNC(pow) (minus_zero, -11), DIVIDE_BY_ZERO_EXCEPTION);

  check_isinfp_exc ("pow (+0, -2) == +inf plus divide-by-zero exception",
		    FUNC(pow) (0, -2), DIVIDE_BY_ZERO_EXCEPTION);
  check_isinfp_exc ("pow (+0, -11.1) == +inf plus divide-by-zero exception",
		    FUNC(pow) (0, -11.1), DIVIDE_BY_ZERO_EXCEPTION);
  check_isinfp_exc ("pow (-0, -2) == +inf plus divide-by-zero exception",
		    FUNC(pow) (minus_zero, -2), DIVIDE_BY_ZERO_EXCEPTION);
  check_isinfp_exc ("pow (-0, -11.1) == +inf plus divide-by-zero exception",
		    FUNC(pow) (minus_zero, -11.1), DIVIDE_BY_ZERO_EXCEPTION);
#endif

  check ("pow (+0, 1) == +0", FUNC(pow) (0, 1), 0);
  check ("pow (+0, 11) == +0", FUNC(pow) (0, 11), 0);
#ifndef TEST_INLINE
  check ("pow (-0, 1) == -0", FUNC(pow) (minus_zero, 1), minus_zero);
  check ("pow (-0, 11) == -0", FUNC(pow) (minus_zero, 11), minus_zero);
#endif

  check ("pow (+0, 2) == +0", FUNC(pow) (0, 2), 0);
  check ("pow (+0, 11.1) == +0", FUNC(pow) (0, 11.1), 0);

#ifndef TEST_INLINE
  check ("pow (-0, 2) == +0", FUNC(pow) (minus_zero, 2), 0);
  check ("pow (-0, 11.1) == +0", FUNC(pow) (minus_zero, 11.1), 0);

  x = random_greater (1.0);
  check_isinfp_ext ("pow (x, +inf) == +inf for |x| > 1",
		    FUNC(pow) (x, plus_infty), x);

  x = random_value (-1.0, 1.0);
  check_ext ("pow (x, +inf) == +0 for |x| < 1",
	     FUNC(pow) (x, plus_infty), 0.0, x);

  x = random_greater (1.0);
  check_ext ("pow (x, -inf) == +0 for |x| > 1",
	     FUNC(pow) (x, minus_infty), 0.0, x);

  x = random_value (-1.0, 1.0);
  check_isinfp_ext ("pow (x, -inf) == +inf for |x| < 1",
		    FUNC(pow) (x, minus_infty), x);

  x = random_greater (0.0);
  check_isinfp_ext ("pow (+inf, y) == +inf for y > 0",
		    FUNC(pow) (plus_infty, x), x);

  x = random_less (0.0);
  check_ext ("pow (+inf, y) == +0 for y < 0",
	     FUNC(pow) (plus_infty, x), 0.0, x);

  x = (rand () % 1000000) * 2.0 + 1;	/* Get random odd integer > 0 */
  check_isinfn_ext ("pow (-inf, y) == -inf for y an odd integer > 0",
		    FUNC(pow) (minus_infty, x), x);

  x = ((rand () % 1000000) + 1) * 2.0;	/* Get random even integer > 1 */
  check_isinfp_ext ("pow (-inf, y) == +inf for y > 0 and not an odd integer",
		    FUNC(pow) (minus_infty, x), x);

  x = -((rand () % 1000000) * 2.0 + 1);	/* Get random odd integer < 0 */
  check_ext ("pow (-inf, y) == -0 for y an odd integer < 0",
	     FUNC(pow) (minus_infty, x), minus_zero, x);

  x = ((rand () % 1000000) + 1) * -2.0;	/* Get random even integer < 0 */
  check_ext ("pow (-inf, y) == +0 for y < 0 and not an odd integer",
	     FUNC(pow) (minus_infty, x), 0.0, x);
#endif

  x = (rand () % 1000000) * 2.0 + 1;	/* Get random odd integer > 0 */
  check_ext ("pow (+0, y) == +0 for y an odd integer > 0",
	     FUNC(pow) (0.0, x), 0.0, x);
#ifndef TEST_INLINE
  x = (rand () % 1000000) * 2.0 + 1;	/* Get random odd integer > 0 */
  check_ext ("pow (-0, y) == -0 for y an odd integer > 0",
	     FUNC(pow) (minus_zero, x), minus_zero, x);
#endif

  x = ((rand () % 1000000) + 1) * 2.0;	/* Get random even integer > 1 */
  check_ext ("pow (+0, y) == +0 for y > 0 and not an odd integer",
	     FUNC(pow) (0.0, x), 0.0, x);

  x = ((rand () % 1000000) + 1) * 2.0;	/* Get random even integer > 1 */
  check_ext ("pow (-0, y) == +0 for y > 0 and not an odd integer",
	     FUNC(pow) (minus_zero, x), 0.0, x);

  check_eps ("pow (0.7, 1.2) == 0.65180...", FUNC(pow) (0.7, 1.2),
	     0.65180494056638638188L, CHOOSE (4e-17L, 0, 0));

#ifdef TEST_DOUBLE
  check ("pow (-7.49321e+133, -9.80818e+16) == 0",
	 FUNC(pow) (-7.49321e+133, -9.80818e+16), 0);
#endif
}


static void
fdim_test (void)
{
  check ("fdim (+0, +0) = +0", FUNC(fdim) (0, 0), 0);
  check ("fdim (9, 0) = 9", FUNC(fdim) (9, 0), 9);
  check ("fdim (0, 9) = 0", FUNC(fdim) (0, 9), 0);
  check ("fdim (-9, 0) = 0", FUNC(fdim) (-9, 0), 0);
  check ("fdim (0, -9) = 9", FUNC(fdim) (0, -9), 9);

  check_isinfp ("fdim (+inf, 9) = +inf", FUNC(fdim) (plus_infty, 9));
  check_isinfp ("fdim (+inf, -9) = +inf", FUNC(fdim) (plus_infty, -9));
  check ("fdim (-inf, 9) = 0", FUNC(fdim) (minus_infty, 9), 0);
  check ("fdim (-inf, -9) = 0", FUNC(fdim) (minus_infty, -9), 0);
  check_isinfp ("fdim (+9, -inf) = +inf", FUNC(fdim) (9, minus_infty));
  check_isinfp ("fdim (-9, -inf) = +inf", FUNC(fdim) (-9, minus_infty));
  check ("fdim (9, inf) = 0", FUNC(fdim) (9, plus_infty), 0);
  check ("fdim (-9, inf) = 0", FUNC(fdim) (-9, plus_infty), 0);

  check_isnan ("fdim (0, NaN) = NaN", FUNC(fdim) (0, nan_value));
  check_isnan ("fdim (9, NaN) = NaN", FUNC(fdim) (9, nan_value));
  check_isnan ("fdim (-9, NaN) = NaN", FUNC(fdim) (-9, nan_value));
  check_isnan ("fdim (NaN, 9) = NaN", FUNC(fdim) (nan_value, 9));
  check_isnan ("fdim (NaN, -9) = NaN", FUNC(fdim) (nan_value, -9));
  check_isnan ("fdim (+inf, NaN) = NaN", FUNC(fdim) (plus_infty, nan_value));
  check_isnan ("fdim (-inf, NaN) = NaN", FUNC(fdim) (minus_infty, nan_value));
  check_isnan ("fdim (NaN, +inf) = NaN", FUNC(fdim) (nan_value, plus_infty));
  check_isnan ("fdim (NaN, -inf) = NaN", FUNC(fdim) (nan_value, minus_infty));
  check_isnan ("fdim (NaN, NaN) = NaN", FUNC(fdim) (nan_value, nan_value));
}


static void
fmin_test (void)
{
  check ("fmin (+0, +0) = +0", FUNC(fmin) (0, 0), 0);
  check ("fmin (9, 0) = 0", FUNC(fmin) (9, 0), 0);
  check ("fmin (0, 9) = 0", FUNC(fmin) (0, 9), 0);
  check ("fmin (-9, 0) = -9", FUNC(fmin) (-9, 0), -9);
  check ("fmin (0, -9) = -9", FUNC(fmin) (0, -9), -9);

  check ("fmin (+inf, 9) = 9", FUNC(fmin) (plus_infty, 9), 9);
  check ("fmin (9, +inf) = 9", FUNC(fmin) (9, plus_infty), 9);
  check ("fmin (+inf, -9) = -9", FUNC(fmin) (plus_infty, -9), -9);
  check ("fmin (-9, +inf) = -9", FUNC(fmin) (-9, plus_infty), -9);
  check_isinfn ("fmin (-inf, 9) = -inf", FUNC(fmin) (minus_infty, 9));
  check_isinfn ("fmin (-inf, -9) = -inf", FUNC(fmin) (minus_infty, -9));
  check_isinfn ("fmin (+9, -inf) = -inf", FUNC(fmin) (9, minus_infty));
  check_isinfn ("fmin (-9, -inf) = -inf", FUNC(fmin) (-9, minus_infty));

  check ("fmin (0, NaN) = 0", FUNC(fmin) (0, nan_value), 0);
  check ("fmin (9, NaN) = 9", FUNC(fmin) (9, nan_value), 9);
  check ("fmin (-9, NaN) = 9", FUNC(fmin) (-9, nan_value), -9);
  check ("fmin (NaN, 0) = 0", FUNC(fmin) (nan_value, 0), 0);
  check ("fmin (NaN, 9) = NaN", FUNC(fmin) (nan_value, 9), 9);
  check ("fmin (NaN, -9) = NaN", FUNC(fmin) (nan_value, -9), -9);
  check_isinfp ("fmin (+inf, NaN) = +inf", FUNC(fmin) (plus_infty, nan_value));
  check_isinfn ("fmin (-inf, NaN) = -inf", FUNC(fmin) (minus_infty, nan_value));
  check_isinfp ("fmin (NaN, +inf) = +inf", FUNC(fmin) (nan_value, plus_infty));
  check_isinfn ("fmin (NaN, -inf) = -inf", FUNC(fmin) (nan_value, minus_infty));
  check_isnan ("fmin (NaN, NaN) = NaN", FUNC(fmin) (nan_value, nan_value));
}


static void
fmax_test (void)
{
  check ("fmax (+0, +0) = +0", FUNC(fmax) (0, 0), 0);
  check ("fmax (9, 0) = 9", FUNC(fmax) (9, 0), 9);
  check ("fmax (0, 9) = 9", FUNC(fmax) (0, 9), 9);
  check ("fmax (-9, 0) = 0", FUNC(fmax) (-9, 0), 0);
  check ("fmax (0, -9) = 0", FUNC(fmax) (0, -9), 0);

  check_isinfp ("fmax (+inf, 9) = +inf", FUNC(fmax) (plus_infty, 9));
  check_isinfp ("fmax (9, +inf) = +inf", FUNC(fmax) (0, plus_infty));
  check_isinfp ("fmax (-9, +inf) = +inf", FUNC(fmax) (-9, plus_infty));
  check_isinfp ("fmax (+inf, -9) = +inf", FUNC(fmax) (plus_infty, -9));
  check ("fmax (-inf, 9) = 9", FUNC(fmax) (minus_infty, 9), 9);
  check ("fmax (-inf, -9) = -9", FUNC(fmax) (minus_infty, -9), -9);
  check ("fmax (+9, -inf) = 9", FUNC(fmax) (9, minus_infty), 9);
  check ("fmax (-9, -inf) = -9", FUNC(fmax) (-9, minus_infty), -9);

  check ("fmax (0, NaN) = 0", FUNC(fmax) (0, nan_value), 0);
  check ("fmax (9, NaN) = 9", FUNC(fmax) (9, nan_value), 9);
  check ("fmax (-9, NaN) = 9", FUNC(fmax) (-9, nan_value), -9);
  check ("fmax (NaN, 0) = 0", FUNC(fmax) (nan_value, 0), 0);
  check ("fmax (NaN, 9) = NaN", FUNC(fmax) (nan_value, 9), 9);
  check ("fmax (NaN, -9) = NaN", FUNC(fmax) (nan_value, -9), -9);
  check_isinfp ("fmax (+inf, NaN) = +inf", FUNC(fmax) (plus_infty, nan_value));
  check_isinfn ("fmax (-inf, NaN) = -inf", FUNC(fmax) (minus_infty, nan_value));
  check_isinfp ("fmax (NaN, +inf) = +inf", FUNC(fmax) (nan_value, plus_infty));
  check_isinfn ("fmax (NaN, -inf) = -inf", FUNC(fmax) (nan_value, minus_infty));
  check_isnan ("fmax (NaN, NaN) = NaN", FUNC(fmax) (nan_value, nan_value));
}


static void
fmod_test (void)
{
  MATHTYPE x;

  x = random_greater (0);
  check_ext ("fmod (+0, y) == +0 for y != 0", FUNC(fmod) (0, x), 0, x);

  x = random_greater (0);
  check_ext ("fmod (-0, y) == -0 for y != 0", FUNC(fmod) (minus_zero, x),
	     minus_zero, x);

  check_isnan_exc_ext ("fmod (+inf, y) == NaN plus invalid exception",
		       FUNC(fmod) (plus_infty, x), INVALID_EXCEPTION, x);
  check_isnan_exc_ext ("fmod (-inf, y) == NaN plus invalid exception",
		       FUNC(fmod) (minus_infty, x), INVALID_EXCEPTION, x);
  check_isnan_exc_ext ("fmod (x, +0) == NaN plus invalid exception",
		       FUNC(fmod) (x, 0), INVALID_EXCEPTION, x);
  check_isnan_exc_ext ("fmod (x, -0) == NaN plus invalid exception",
		       FUNC(fmod) (x, minus_zero), INVALID_EXCEPTION, x);

  x = random_greater (0);
  check_ext ("fmod (x, +inf) == x for x not infinite",
	     FUNC(fmod) (x, plus_infty), x, x);
  x = random_greater (0);
  check_ext ("fmod (x, -inf) == x for x not infinite",
	     FUNC(fmod) (x, minus_infty), x, x);

  check_eps ("fmod (6.5, 2.3) == 1.9", FUNC(fmod) (6.5, 2.3), 1.9,
	     CHOOSE (5e-16, 1e-15, 2e-7));
  check_eps ("fmod (-6.5, 2.3) == -1.9", FUNC(fmod) (-6.5, 2.3), -1.9,
	     CHOOSE (5e-16, 1e-15, 2e-7));
  check_eps ("fmod (6.5, -2.3) == 1.9", FUNC(fmod) (6.5, -2.3), 1.9,
	     CHOOSE (5e-16, 1e-15, 2e-7));
  check_eps ("fmod (-6.5, -2.3) == -1.9", FUNC(fmod) (-6.5, -2.3), -1.9,
	     CHOOSE (5e-16, 1e-15, 2e-7));
}


static void
nextafter_test (void)
{
  MATHTYPE x;

  check ("nextafter (+0, +0) = +0", FUNC(nextafter) (0, 0), 0);
  check ("nextafter (-0, +0) = +0", FUNC(nextafter) (minus_zero, 0), 0);
  check ("nextafter (+0, -0) = -0", FUNC(nextafter) (0, minus_zero),
	 minus_zero);
  check ("nextafter (-0, -0) = -0", FUNC(nextafter) (minus_zero, minus_zero),
	 minus_zero);

  check ("nextafter (9, 9) = 9", FUNC(nextafter) (9, 9), 9);
  check ("nextafter (-9, -9) = -9", FUNC(nextafter) (-9, -9), -9);
  check_isinfp ("nextafter (+inf, +inf) = +inf",
		FUNC(nextafter) (plus_infty, plus_infty));
  check_isinfn ("nextafter (-inf, -inf) = -inf",
		FUNC(nextafter) (minus_infty, minus_infty));

  x = rand () * 1.1;
  check_isnan ("nextafter (NaN, x) = NaN", FUNC(nextafter) (nan_value, x));
  check_isnan ("nextafter (x, NaN) = NaN", FUNC(nextafter) (x, nan_value));
  check_isnan ("nextafter (NaN, NaN) = NaN", FUNC(nextafter) (nan_value,
							      nan_value));

  /* XXX We need the hexadecimal FP number representation here for further
     tests.  */
}


static void
copysign_test (void)
{
  check ("copysign (0, 4) = 0", FUNC(copysign) (0, 4), 0);
  check ("copysign (0, -4) = -0", FUNC(copysign) (0, -4), minus_zero);
  check ("copysign (-0, 4) = 0", FUNC(copysign) (minus_zero, 4), 0);
  check ("copysign (-0, -4) = -0", FUNC(copysign) (minus_zero, -4),
	 minus_zero);

  check_isinfp ("copysign (+inf, 0) = +inf", FUNC(copysign) (plus_infty, 0));
  check_isinfn ("copysign (+inf, -0) = -inf", FUNC(copysign) (plus_infty,
							      minus_zero));
  check_isinfp ("copysign (-inf, 0) = +inf", FUNC(copysign) (minus_infty, 0));
  check_isinfn ("copysign (-inf, -0) = -inf", FUNC(copysign) (minus_infty,
							      minus_zero));

  check ("copysign (0, +inf) = 0", FUNC(copysign) (0, plus_infty), 0);
  check ("copysign (0, -inf) = -0", FUNC(copysign) (0, minus_zero),
	 minus_zero);
  check ("copysign (-0, +inf) = 0", FUNC(copysign) (minus_zero, plus_infty),
	 0);
  check ("copysign (-0, -inf) = -0", FUNC(copysign) (minus_zero, minus_zero),
	 minus_zero);

  /* XXX More correctly we would have to check the sign of the NaN.  */
  check_isnan ("copysign (+NaN, 0) = +NaN", FUNC(copysign) (nan_value, 0));
  check_isnan ("copysign (+NaN, -0) = -NaN", FUNC(copysign) (nan_value,
							     minus_zero));
  check_isnan ("copysign (-NaN, 0) = +NaN", FUNC(copysign) (-nan_value, 0));
  check_isnan ("copysign (-NaN, -0) = -NaN", FUNC(copysign) (-nan_value,
							     minus_zero));
}


static void
trunc_test (void)
{
  check ("trunc(0) = 0", FUNC(trunc) (0), 0);
  check ("trunc(-0) = -0", FUNC(trunc) (minus_zero), minus_zero);
  check ("trunc(0.625) = 0", FUNC(trunc) (0.625), 0);
  check ("trunc(-0.625) = -0", FUNC(trunc) (-0.625), minus_zero);
  check ("trunc(1) = 1", FUNC(trunc) (1), 1);
  check ("trunc(-1) = -1", FUNC(trunc) (-1), -1);
  check ("trunc(1.625) = 1", FUNC(trunc) (1.625), 1);
  check ("trunc(-1.625) = -1", FUNC(trunc) (-1.625), -1);

  check ("trunc(1048580.625) = 1048580", FUNC(trunc) (1048580.625L),
	 1048580L);
  check ("trunc(-1048580.625) = -1048580", FUNC(trunc) (-1048580.625L),
	 -1048580L);

  check ("trunc(8388610.125) = 8388610", FUNC(trunc) (8388610.125L),
	 8388610.0L);
  check ("trunc(-8388610.125) = -8388610", FUNC(trunc) (-8388610.125L),
	 -8388610.0L);

  check ("trunc(4294967296.625) = 4294967296", FUNC(trunc) (4294967296.625L),
	 4294967296.0L);
  check ("trunc(-4294967296.625) = -4294967296",
	 FUNC(trunc) (-4294967296.625L), -4294967296.0L);

  check_isinfp ("trunc(+inf) = +inf", FUNC(trunc) (plus_infty));
  check_isinfn ("trunc(-inf) = -inf", FUNC(trunc) (minus_infty));
  check_isnan ("trunc(NaN) = NaN", FUNC(trunc) (nan_value));
}


static void
sqrt_test (void)
{
  MATHTYPE x;


  /* XXX Tests fuer negative x are missing */
  check ("sqrt (0) == 0", FUNC(sqrt) (0), 0);
  check_isnan ("sqrt (NaN) == NaN", FUNC(sqrt) (nan_value));
  check_isinfp ("sqrt (+inf) == +inf", FUNC(sqrt) (plus_infty));

  check ("sqrt (-0) == -0", FUNC(sqrt) (0), 0);

  x = random_less (0.0);
  check_isnan_exc_ext ("sqrt (x) == NaN plus invalid exception for x < 0",
		       FUNC(sqrt) (x), INVALID_EXCEPTION, x);

  x = random_value (0, 10000);
  check_ext ("sqrt (x*x) == x", FUNC(sqrt) (x*x), x, x);
  check ("sqrt (4) == 2", FUNC(sqrt) (4), 2);
  check ("sqrt (2) == 1.14142...", FUNC(sqrt) (2), M_SQRT2l);
  check ("sqrt (0.25) == 0.5", FUNC(sqrt) (0.25), 0.5);
  check ("sqrt (6642.25) == 81.5", FUNC(sqrt) (6642.25), 81.5);
  check_eps ("sqrt (15239.903) == 123.45", FUNC(sqrt) (15239.903), 123.45,
	     CHOOSE (3e-6L, 3e-6, 8e-6));
  check_eps ("sqrt (0.7) == 0.8366600265", FUNC(sqrt) (0.7),
	     0.83666002653407554798L, CHOOSE (3e-17L, 0, 0));
}


static void
remainder_test (void)
{
  MATHTYPE result;

  result = FUNC(remainder) (1, 0);
  check_isnan_exc ("remainder(1, +0) == NaN plus invalid exception",
		   result, INVALID_EXCEPTION);

  result = FUNC(remainder) (1, minus_zero);
  check_isnan_exc ("remainder(1, -0) == NaN plus invalid exception",
		   result, INVALID_EXCEPTION);

  result = FUNC(remainder) (plus_infty, 1);
  check_isnan_exc ("remainder(+inf, 1) == NaN plus invalid exception",
		   result, INVALID_EXCEPTION);

  result = FUNC(remainder) (minus_infty, 1);
  check_isnan_exc ("remainder(-inf, 1) == NaN plus invalid exception",
		   result, INVALID_EXCEPTION);

  result = FUNC(remainder) (1.625, 1.0);
  check ("remainder(1.625, 1.0) == -0.375", result, -0.375);

  result = FUNC(remainder) (-1.625, 1.0);
  check ("remainder(-1.625, 1.0) == 0.375", result, 0.375);

  result = FUNC(remainder) (1.625, -1.0);
  check ("remainder(1.625, -1.0) == -0.375", result, -0.375);

  result = FUNC(remainder) (-1.625, -1.0);
  check ("remainder(-1.625, -1.0) == 0.375", result, 0.375);

  result = FUNC(remainder) (5.0, 2.0);
  check ("remainder(5.0, 2.0) == 1.0", result, 1.0);

  result = FUNC(remainder) (3.0, 2.0);
  check ("remainder(3.0, 2.0) == -1.0", result, -1.0);
}


static void
remquo_test (void)
{
  int quo;
  MATHTYPE result;

  result = FUNC(remquo) (1, 0, &quo);
  check_isnan_exc ("remquo(1, +0, &x) == NaN plus invalid exception",
		   result, INVALID_EXCEPTION);

  result = FUNC(remquo) (1, minus_zero, &quo);
  check_isnan_exc ("remquo(1, -0, &x) == NaN plus invalid exception",
		   result, INVALID_EXCEPTION);

  result = FUNC(remquo) (plus_infty, 1, &quo);
  check_isnan_exc ("remquo(+inf, 1, &x) == NaN plus invalid exception",
		   result, INVALID_EXCEPTION);

  result = FUNC(remquo) (minus_infty, 1, &quo);
  check_isnan_exc ("remquo(-inf, 1, &x) == NaN plus invalid exception",
		   result, INVALID_EXCEPTION);

  result = FUNC(remquo) (1.625, 1.0, &quo);
  check ("remquo(1.625, 1.0, &x) == -0.375", result, -0.375);
  check_long ("remquo(1.625, 1.0, &x) puts 2 in x", quo, 2);

  result = FUNC(remquo) (-1.625, 1.0, &quo);
  check ("remquo(-1.625, 1.0, &x) == 0.375", result, 0.375);
  check_long ("remquo(-1.625, 1.0, &x) puts -2 in x", quo, -2);

  result = FUNC(remquo) (1.625, -1.0, &quo);
  check ("remquo(1.625, -1.0, &x) == -0.375", result, -0.375);
  check_long ("remquo(1.625, -1.0, &x) puts -2 in x", quo, -2);

  result = FUNC(remquo) (-1.625, -1.0, &quo);
  check ("remquo(-1.625, -1.0, &x) == 0.375", result, 0.375);
  check_long ("remquo(-1.625, -1.0, &x) puts 2 in x", quo, 2);

  result = FUNC(remquo) (5.0, 2.0, &quo);
  check ("remquo(5.0, 2.0, &x) == 1.0", result, 1.0);
  check_long ("remquo (5.0, 2.0, &x) puts 2 in x", quo, 2);

  result = FUNC(remquo) (3.0, 2.0, &quo);
  check ("remquo(3.0, 2.0, &x) == -1.0", result, -1.0);
  check_long ("remquo (3.0, 2.0, &x) puts 2 in x", quo, 2);
}


static void
cexp_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(cexp) (BUILD_COMPLEX (plus_zero, plus_zero));
  check ("real(cexp(0 + 0i)) = 1", __real__ result, 1);
  check ("imag(cexp(0 + 0i)) = 0", __imag__ result, 0);
  result = FUNC(cexp) (BUILD_COMPLEX (minus_zero, plus_zero));
  check ("real(cexp(-0 + 0i)) = 1", __real__ result, 1);
  check ("imag(cexp(-0 + 0i)) = 0", __imag__ result, 0);
  result = FUNC(cexp) (BUILD_COMPLEX (plus_zero, minus_zero));
  check ("real(cexp(0 - 0i)) = 1", __real__ result, 1);
  check ("imag(cexp(0 - 0i)) = -0", __imag__ result, minus_zero);
  result = FUNC(cexp) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(cexp(-0 - 0i)) = 1", __real__ result, 1);
  check ("imag(cexp(-0 - 0i)) = -0", __imag__ result, minus_zero);

  result = FUNC(cexp) (BUILD_COMPLEX (plus_infty, plus_zero));
  check_isinfp ("real(cexp(+inf + 0i)) = +inf", __real__ result);
  check ("imag(cexp(+inf + 0i)) = 0", __imag__ result, 0);
  result = FUNC(cexp) (BUILD_COMPLEX (plus_infty, minus_zero));
  check_isinfp ("real(cexp(+inf - 0i)) = +inf", __real__ result);
  check ("imag(cexp(+inf - 0i)) = -0", __imag__ result, minus_zero);

  result = FUNC(cexp) (BUILD_COMPLEX (minus_infty, plus_zero));
  check ("real(cexp(-inf + 0i)) = 0", __real__ result, 0);
  check ("imag(cexp(-inf + 0i)) = 0", __imag__ result, 0);
  result = FUNC(cexp) (BUILD_COMPLEX (minus_infty, minus_zero));
  check ("real(cexp(-inf - 0i)) = 0", __real__ result, 0);
  check ("imag(cexp(-inf - 0i)) = -0", __imag__ result, minus_zero);


  result = FUNC(cexp) (BUILD_COMPLEX (0.0, plus_infty));
  check_isnan_exc ("real(cexp(0 + i inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(0 + i inf)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(cexp) (BUILD_COMPLEX (minus_zero, plus_infty));
  check_isnan_exc ("real(cexp(-0 + i inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(-0 + i inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(cexp) (BUILD_COMPLEX (0.0, minus_infty));
  check_isnan_exc ("real(cexp(0 - i inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(0 - i inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(cexp) (BUILD_COMPLEX (minus_zero, minus_infty));
  check_isnan_exc ("real(cexp(-0 - i inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(-0 - i inf)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(cexp) (BUILD_COMPLEX (100.0, plus_infty));
  check_isnan_exc ("real(cexp(100.0 + i inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(100.0 + i inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(cexp) (BUILD_COMPLEX (-100.0, plus_infty));
  check_isnan_exc ("real(cexp(-100.0 + i inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(-100.0 + i inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(cexp) (BUILD_COMPLEX (100.0, minus_infty));
  check_isnan_exc ("real(cexp(100.0 - i inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(100.0 - i inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(cexp) (BUILD_COMPLEX (-100.0, minus_infty));
  check_isnan_exc ("real(cexp(-100.0 - i inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(-100.0 - i inf)) = NaN", __imag__ result);

  result = FUNC(cexp) (BUILD_COMPLEX (minus_infty, 2.0));
  check ("real(cexp(-inf + 2.0i)) = -0", __real__ result, minus_zero);
  check ("imag(cexp(-inf + 2.0i)) = 0", __imag__ result, 0);
  result = FUNC(cexp) (BUILD_COMPLEX (minus_infty, 4.0));
  check ("real(cexp(-inf + 4.0i)) = -0", __real__ result, minus_zero);
  check ("imag(cexp(-inf + 4.0i)) = -0", __imag__ result, minus_zero);

  result = FUNC(cexp) (BUILD_COMPLEX (plus_infty, 2.0));
  check_isinfn ("real(cexp(+inf + 2.0i)) = -inf", __real__ result);
  check_isinfp ("imag(cexp(+inf + 2.0i)) = +inf", __imag__ result);
  result = FUNC(cexp) (BUILD_COMPLEX (plus_infty, 4.0));
  check_isinfn ("real(cexp(+inf + 4.0i)) = -inf", __real__ result);
  check_isinfn ("imag(cexp(+inf + 4.0i)) = -inf", __imag__ result);

  result = FUNC(cexp) (BUILD_COMPLEX (plus_infty, plus_infty));
  check_isinfp_exc ("real(cexp(+inf + i inf)) = +inf plus invalid exception",
		    __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(+inf + i inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(cexp) (BUILD_COMPLEX (plus_infty, minus_infty));
  check_isinfp_exc ("real(cexp(+inf - i inf)) = +inf plus invalid exception",
		    __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(+inf - i inf)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(cexp) (BUILD_COMPLEX (minus_infty, plus_infty));
  check ("real(cexp(-inf + i inf)) = 0", __real__ result, 0);
  check ("imag(cexp(-inf + i inf)) = 0", __imag__ result, 0);
  result = FUNC(cexp) (BUILD_COMPLEX (minus_infty, minus_infty));
  check ("real(cexp(-inf - i inf)) = 0", __real__ result, 0);
  check ("imag(cexp(-inf - i inf)) = -0", __imag__ result, minus_zero);

  result = FUNC(cexp) (BUILD_COMPLEX (minus_infty, nan_value));
  check ("real(cexp(-inf + i NaN)) = 0", __real__ result, 0);
  check ("imag(cexp(-inf + i NaN)) = 0", fabs (__imag__ result), 0);

  result = FUNC(cexp) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isinfp ("real(cexp(+inf + i NaN)) = +inf", __real__ result);
  check_isnan ("imag(cexp(+inf + i NaN)) = NaN", __imag__ result);

  result = FUNC(cexp) (BUILD_COMPLEX (nan_value, 0.0));
  check_isnan_maybe_exc ("real(cexp(NaN + i0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(NaN + i0)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(cexp) (BUILD_COMPLEX (nan_value, 1.0));
  check_isnan_maybe_exc ("real(cexp(NaN + 1i)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(NaN + 1i)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(cexp) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isnan_maybe_exc ("real(cexp(NaN + i inf)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(NaN + i inf)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(cexp) (BUILD_COMPLEX (0, nan_value));
  check_isnan_maybe_exc ("real(cexp(0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(cexp) (BUILD_COMPLEX (1, nan_value));
  check_isnan_maybe_exc ("real(cexp(1 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cexp(1 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(cexp) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(cexp(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(cexp(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(cexp) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(cexp(0.7 + i 1.2)) == 0.72969...", __real__ result,
	     0.7296989091503236012L, CHOOSE (6e-17L, 2e-16, 2e-7));
  check_eps ("imag(cexp(0.7 + i 1.2)) == 1.87689...", __imag__ result,
	     1.8768962328348102821L, CHOOSE (2e-16L, 4.5e-16, 3e-7));

  result = FUNC(cexp) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(cexp(-2 - i 3)) == -0.13398...", __real__ result,
	     -0.1339809149295426134L, CHOOSE (6.8e-20L, 2.8e-17, 2e-8));
  check_eps ("imag(cexp(-2 - i 3)) == -0.01909...", __imag__ result,
	     -0.0190985162611351964L, CHOOSE (4e-20L, 3.5e-18, 2e-9));
}


static void
csin_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(csin) (BUILD_COMPLEX (0.0, 0.0));
  check ("real(csin(0 + 0i)) = 0", __real__ result, 0);
  check ("imag(csin(0 + 0i)) = 0", __imag__ result, 0);
  result = FUNC(csin) (BUILD_COMPLEX (minus_zero, 0.0));
  check ("real(csin(-0 + 0i)) = -0", __real__ result, minus_zero);
  check ("imag(csin(-0 + 0i)) = 0", __imag__ result, 0);
  result = FUNC(csin) (BUILD_COMPLEX (0.0, minus_zero));
  check ("real(csin(0 - 0i)) = 0", __real__ result, 0);
  check ("imag(csin(0 - 0i)) = -0", __imag__ result, minus_zero);
  result = FUNC(csin) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(csin(-0 - 0i)) = -0", __real__ result, minus_zero);
  check ("imag(csin(-0 - 0i)) = -0", __imag__ result, minus_zero);

  result = FUNC(csin) (BUILD_COMPLEX (0.0, plus_infty));
  check ("real(csin(0 + i Inf)) = 0", __real__ result, 0);
  check_isinfp ("imag(csin(0 + i Inf)) = +Inf", __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (minus_zero, plus_infty));
  check ("real(csin(-0 + i Inf)) = -0", __real__ result, minus_zero);
  check_isinfp ("imag(csin(-0 + i Inf)) = +Inf", __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (0.0, minus_infty));
  check ("real(csin(0 - i Inf)) = 0", __real__ result, 0);
  check_isinfn ("imag(csin(0 - i Inf)) = -Inf", __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (minus_zero, minus_infty));
  check ("real(csin(-0 - i Inf)) = -0", __real__ result, minus_zero);
  check_isinfn ("imag(csin(-0 - i Inf)) = -Inf", __imag__ result);

  result = FUNC(csin) (BUILD_COMPLEX (plus_infty, 0.0));
  check_isnan_exc ("real(csin(+Inf + 0i)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(csin(+Inf + 0i)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0);
  result = FUNC(csin) (BUILD_COMPLEX (minus_infty, 0.0));
  check_isnan_exc ("real(csin(-Inf + 0i)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(csin(-Inf + 0i)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0);
  result = FUNC(csin) (BUILD_COMPLEX (plus_infty, minus_zero));
  check_isnan_exc ("real(csin(+Inf - 0i)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(csin(+Inf - 0i)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0.0);
  result = FUNC(csin) (BUILD_COMPLEX (minus_infty, minus_zero));
  check_isnan_exc ("real(csin(-Inf - 0i)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(csin(-Inf - 0i)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0.0);

  result = FUNC(csin) (BUILD_COMPLEX (plus_infty, plus_infty));
  check_isnan_exc ("real(csin(+Inf + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isinfp ("imag(csin(+Inf + i Inf)) = +-Inf plus invalid exception",
		FUNC(fabs) (__imag__ result));
  result = FUNC(csin) (BUILD_COMPLEX (minus_infty, plus_infty));
  check_isnan_exc ("real(csin(-Inf + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isinfp ("imag(csin(-Inf + i Inf)) = +-Inf plus invalid exception",
		FUNC(fabs) (__imag__ result));
  result = FUNC(csin) (BUILD_COMPLEX (plus_infty, minus_infty));
  check_isnan_exc ("real(csin(Inf - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isinfp ("imag(csin(Inf - i Inf)) = +-Inf plus invalid exception",
		FUNC(fabs) (__imag__ result));
  result = FUNC(csin) (BUILD_COMPLEX (minus_infty, minus_infty));
  check_isnan_exc ("real(csin(-Inf - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isinfp ("imag(csin(-Inf - i Inf)) = +-Inf plus invalid exception",
		FUNC(fabs) (__imag__ result));

  result = FUNC(csin) (BUILD_COMPLEX (plus_infty, 6.75));
  check_isnan_exc ("real(csin(+Inf + i 6.75)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csin(+Inf + i6.75)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (plus_infty, -6.75));
  check_isnan_exc ("real(csin(+Inf - i 6.75)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csin(+Inf - i6.75)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (minus_infty, 6.75));
  check_isnan_exc ("real(csin(-Inf + i6.75)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csin(-Inf + i6.75)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (minus_infty, -6.75));
  check_isnan_exc ("real(csin(-Inf - i6.75)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csin(-Inf - i6.75)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(csin) (BUILD_COMPLEX (4.625, plus_infty));
  check_isinfn ("real(csin(4.625 + i Inf)) = -Inf", __real__ result);
  check_isinfn ("imag(csin(4.625 + i Inf)) = -Inf", __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (4.625, minus_infty));
  check_isinfn ("real(csin(4.625 - i Inf)) = -Inf", __real__ result);
  check_isinfp ("imag(csin(4.625 - i Inf)) = +Inf", __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (-4.625, plus_infty));
  check_isinfp ("real(csin(-4.625 + i Inf)) = +Inf", __real__ result);
  check_isinfn ("imag(csin(-4.625 + i Inf)) = -Inf", __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (-4.625, minus_infty));
  check_isinfp ("real(csin(-4.625 - i Inf)) = +Inf", __real__ result);
  check_isinfp ("imag(csin(-4.625 - i Inf)) = +Inf", __imag__ result);

  result = FUNC(csin) (BUILD_COMPLEX (nan_value, 0.0));
  check_isnan ("real(csin(NaN + i0)) = NaN", __real__ result);
  check ("imag(csin(NaN + i0)) = +-0", FUNC(fabs) (__imag__ result), 0);
  result = FUNC(csin) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan ("real(csin(NaN - i0)) = NaN", __real__ result);
  check ("imag(csin(NaN - i0)) = +-0", FUNC(fabs) (__imag__ result), 0);

  result = FUNC(csin) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isnan ("real(csin(NaN + i Inf)) = NaN", __real__ result);
  check_isinfp ("imag(csin(NaN + i Inf)) = +-Inf",
		FUNC(fabs) (__imag__ result));
  result = FUNC(csin) (BUILD_COMPLEX (nan_value, minus_infty));
  check_isnan ("real(csin(NaN - i Inf)) = NaN", __real__ result);
  check_isinfp ("real(csin(NaN - i Inf)) = +-Inf",
		FUNC(fabs) (__imag__ result));

  result = FUNC(csin) (BUILD_COMPLEX (nan_value, 9.0));
  check_isnan_maybe_exc ("real(csin(NaN + i9.0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csin(NaN + i9.0)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (nan_value, -9.0));
  check_isnan_maybe_exc ("real(csin(NaN - i9.0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csin(NaN - i9.0)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(csin) (BUILD_COMPLEX (0.0, nan_value));
  check ("real(csin(0 + i NaN))", __real__ result, 0.0);
  check_isnan ("imag(csin(0 + i NaN)) = NaN", __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (minus_zero, nan_value));
  check ("real(csin(-0 + i NaN)) = -0", __real__ result, minus_zero);
  check_isnan ("imag(csin(-0 + NaN)) = NaN", __imag__ result);

  result = FUNC(csin) (BUILD_COMPLEX (10.0, nan_value));
  check_isnan_maybe_exc ("real(csin(10 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csin(10 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (nan_value, -10.0));
  check_isnan_maybe_exc ("real(csin(-10 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csin(-10 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(csin) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isnan_maybe_exc ("real(csin(+Inf + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csin(+Inf + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csin) (BUILD_COMPLEX (minus_infty, nan_value));
  check_isnan_maybe_exc ("real(csin(-Inf + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csin(-Inf + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(csin) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(csin(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(csin(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(csin) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(csin(0.7 + i 1.2)) = 1.166456341...", __real__ result,
	     1.1664563419657581376L, CHOOSE (2e-16L, 2.3e-16, 0));
  check_eps ("imag(csin(0.7 + i 1.2)) = 1.154499724...", __imag__ result,
	     1.1544997246948547371L, CHOOSE (2e-17L, 0, 2e-7));

  result = FUNC(csin) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(csin(-2 - i 3)) == -9.15449...", __real__ result,
	     -9.1544991469114295734L, CHOOSE (4e-18L, 1.8e-15, 1e-6));
  check_eps ("imag(csin(-2 - i 3)) == -4.16890...", __imag__ result,
	     4.1689069599665643507L, CHOOSE (2e-17L, 0, 5e-7));
}


static void
csinh_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(csinh) (BUILD_COMPLEX (0.0, 0.0));
  check ("real(csinh(0 + 0i)) = 0", __real__ result, 0);
  check ("imag(csinh(0 + 0i)) = 0", __imag__ result, 0);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_zero, 0.0));
  check ("real(csinh(-0 + 0i)) = -0", __real__ result, minus_zero);
  check ("imag(csinh(-0 + 0i)) = 0", __imag__ result, 0);
  result = FUNC(csinh) (BUILD_COMPLEX (0.0, minus_zero));
  check ("real(csinh(0 - 0i)) = 0", __real__ result, 0);
  check ("imag(csinh(0 - 0i)) = -0", __imag__ result, minus_zero);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(csinh(-0 - 0i)) = -0", __real__ result, minus_zero);
  check ("imag(csinh(-0 - 0i)) = -0", __imag__ result, minus_zero);

  result = FUNC(csinh) (BUILD_COMPLEX (0.0, plus_infty));
  check_exc ("real(csinh(0 + i Inf)) = +-0 plus invalid exception",
	     FUNC(fabs) (__real__ result), 0, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(0 + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_zero, plus_infty));
  check_exc ("real(csinh(-0 + i Inf)) = +-0 plus invalid exception",
	     FUNC(fabs) (__real__ result), 0, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(-0 + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (0.0, minus_infty));
  check_exc ("real(csinh(0 - i Inf)) = +-0 plus invalid exception",
	     FUNC(fabs) (__real__ result), 0, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(0 - i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_zero, minus_infty));
  check_exc ("real(csinh(-0 - i Inf)) = +-0 plus invalid exception",
	     FUNC(fabs) (__real__ result), 0, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(-0 - i Inf)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(csinh) (BUILD_COMPLEX (plus_infty, 0.0));
  check_isinfp ("real(csinh(+Inf + 0i)) = +Inf", __real__ result);
  check ("imag(csinh(+Inf + 0i)) = 0", __imag__ result, 0);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_infty, 0.0));
  check_isinfn ("real(csinh(-Inf + 0i)) = -Inf", __real__ result);
  check ("imag(csinh(-Inf + 0i)) = 0", __imag__ result, 0);
  result = FUNC(csinh) (BUILD_COMPLEX (plus_infty, minus_zero));
  check_isinfp ("real(csinh(+Inf - 0i)) = +Inf", __real__ result);
  check ("imag(csinh(+Inf - 0i)) = -0", __imag__ result, minus_zero);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_infty, minus_zero));
  check_isinfn ("real(csinh(-Inf - 0i)) = -Inf", __real__ result);
  check ("imag(csinh(-Inf - 0i)) = -0", __imag__ result, minus_zero);

  result = FUNC(csinh) (BUILD_COMPLEX (plus_infty, plus_infty));
  check_isinfp_exc ("real(csinh(+Inf + i Inf)) = +-Inf plus invalid exception",
		    FUNC(fabs) (__real__ result), INVALID_EXCEPTION);
  check_isnan ("imag(csinh(+Inf + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_infty, plus_infty));
  check_isinfp_exc ("real(csinh(-Inf + i Inf)) = +-Inf plus invalid exception",
		    FUNC(fabs) (__real__ result), INVALID_EXCEPTION);
  check_isnan ("imag(csinh(-Inf + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (plus_infty, minus_infty));
  check_isinfp_exc ("real(csinh(Inf - i Inf)) = +-Inf plus invalid exception",
		    FUNC(fabs) (__real__ result), INVALID_EXCEPTION);
  check_isnan ("imag(csinh(Inf - i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_infty, minus_infty));
  check_isinfp_exc ("real(csinh(-Inf - i Inf)) = +-Inf plus invalid exception",
		    FUNC(fabs) (__real__ result), INVALID_EXCEPTION);
  check_isnan ("imag(csinh(-Inf - i Inf)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(csinh) (BUILD_COMPLEX (plus_infty, 4.625));
  check_isinfn ("real(csinh(+Inf + i4.625)) = -Inf", __real__ result);
  check_isinfn ("imag(csinh(+Inf + i4.625)) = -Inf", __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_infty, 4.625));
  check_isinfp ("real(csinh(-Inf + i4.625)) = +Inf", __real__ result);
  check_isinfn ("imag(csinh(-Inf + i4.625)) = -Inf", __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (plus_infty, -4.625));
  check_isinfn ("real(csinh(+Inf - i4.625)) = -Inf", __real__ result);
  check_isinfp ("imag(csinh(+Inf - i4.625)) = +Inf", __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_infty, -4.625));
  check_isinfp ("real(csinh(-Inf - i4.625)) = +Inf", __real__ result);
  check_isinfp ("imag(csinh(-Inf - i4.625)) = +Inf", __imag__ result);

  result = FUNC(csinh) (BUILD_COMPLEX (6.75, plus_infty));
  check_isnan_exc ("real(csinh(6.75 + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(6.75 + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (-6.75, plus_infty));
  check_isnan_exc ("real(csinh(-6.75 + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(-6.75 + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (6.75, minus_infty));
  check_isnan_exc ("real(csinh(6.75 - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(6.75 - i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (-6.75, minus_infty));
  check_isnan_exc ("real(csinh(-6.75 - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(-6.75 - i Inf)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(csinh) (BUILD_COMPLEX (0.0, nan_value));
  check ("real(csinh(0 + i NaN)) = +-0", FUNC(fabs) (__real__ result), 0);
  check_isnan ("imag(csinh(0 + i NaN)) = NaN", __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_zero, nan_value));
  check ("real(csinh(-0 + i NaN)) = +-0", FUNC(fabs) (__real__ result), 0);
  check_isnan ("imag(csinh(-0 + i NaN)) = NaN", __imag__ result);

  result = FUNC(csinh) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isinfp ("real(csinh(+Inf + i NaN)) = +-Inf",
		FUNC(fabs) (__real__ result));
  check_isnan ("imag(csinh(+Inf + i NaN)) = NaN", __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (minus_infty, nan_value));
  check_isinfp ("real(csinh(-Inf + i NaN)) = +-Inf",
		FUNC(fabs) (__real__ result));
  check_isnan ("imag(csinh(-Inf + i NaN)) = NaN", __imag__ result);

  result = FUNC(csinh) (BUILD_COMPLEX (9.0, nan_value));
  check_isnan_maybe_exc ("real(csinh(9.0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(9.0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (-9.0, nan_value));
  check_isnan_maybe_exc ("real(csinh(-9.0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(-9.0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(csinh) (BUILD_COMPLEX (nan_value, 0.0));
  check_isnan ("real(csinh(NaN + i0)) = NaN", __real__ result);
  check ("imag(csinh(NaN + i0)) = 0", __imag__ result, 0.0);
  result = FUNC(csinh) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan ("real(csinh(NaN - i0)) = NaN", __real__ result);
  check ("imag(csinh(NaN - i0)) = -0", __imag__ result, minus_zero);

  result = FUNC(csinh) (BUILD_COMPLEX (nan_value, 10.0));
  check_isnan_maybe_exc ("real(csinh(NaN + i10)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(NaN + i10)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (nan_value, -10.0));
  check_isnan_maybe_exc ("real(csinh(NaN - i10)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(NaN - i10)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(csinh) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isnan_maybe_exc ("real(csinh(NaN + i Inf)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(NaN + i Inf)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csinh) (BUILD_COMPLEX (nan_value, minus_infty));
  check_isnan_maybe_exc ("real(csinh(NaN - i Inf)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csinh(NaN - i Inf)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(csinh) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(csinh(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(csinh(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(csinh) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(csinh(0.7 + i 1.2)) = 0.274878686...", __real__ result,
	     0.27487868678117583582L, CHOOSE (2e-17L, 6e-17, 3e-8));
  check_eps ("imag(csinh(0.7 + i 1.2)) = 1.169866572...", __imag__ result,
	     1.1698665727426565139L, CHOOSE (6e-17L, 2.3e-16, 2e-7));

  result = FUNC(csinh) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(csinh(-2 - i 3)) == -3.59056...", __real__ result,
	     3.5905645899857799520L, CHOOSE (7e-19L, 5e-16, 3e-7));
  check_eps ("imag(csinh(-2 - i 3)) == -0.53092...", __imag__ result,
	     -0.5309210862485198052L, CHOOSE (3e-19L, 2e-16, 6e-8));
}


static void
ccos_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(ccos) (BUILD_COMPLEX (0.0, 0.0));
  check ("real(ccos(0 + 0i)) = 1.0", __real__ result, 1.0);
  check ("imag(ccos(0 + 0i)) = -0", __imag__ result, minus_zero);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_zero, 0.0));
  check ("real(ccos(-0 + 0i)) = 1.0", __real__ result, 1.0);
  check ("imag(ccos(-0 + 0i)) = 0", __imag__ result, 0.0);
  result = FUNC(ccos) (BUILD_COMPLEX (0.0, minus_zero));
  check ("real(ccos(0 - 0i)) = 1.0", __real__ result, 1.0);
  check ("imag(ccos(0 - 0i)) = 0", __imag__ result, 0.0);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(ccos(-0 - 0i)) = 1.0", __real__ result, 1.0);
  check ("imag(ccos(-0 - 0i)) = -0", __imag__ result, minus_zero);

  result = FUNC(ccos) (BUILD_COMPLEX (plus_infty, 0.0));
  check_isnan_exc ("real(ccos(+Inf + i0)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(ccos(Inf + i0)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0);
  result = FUNC(ccos) (BUILD_COMPLEX (plus_infty, minus_zero));
  check_isnan_exc ("real(ccos(Inf - i0)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(ccos(Inf - i0)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_infty, 0.0));
  check_isnan_exc ("real(ccos(-Inf + i0)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(ccos(-Inf + i0)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_infty, minus_zero));
  check_isnan_exc ("real(ccos(-Inf - i0)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(ccos(-Inf - i0)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0);

  result = FUNC(ccos) (BUILD_COMPLEX (0.0, plus_infty));
  check_isinfp ("real(ccos(0 + i Inf)) = +Inf", __real__ result);
  check ("imag(ccos(0 + i Inf)) = -0", __imag__ result, minus_zero);
  result = FUNC(ccos) (BUILD_COMPLEX (0.0, minus_infty));
  check_isinfp ("real(ccos(0 - i Inf)) = +Inf", __real__ result);
  check ("imag(ccos(0 - i Inf)) = 0", __imag__ result, 0);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_zero, plus_infty));
  check_isinfp ("real(ccos(-0 + i Inf)) = +Inf", __real__ result);
  check ("imag(ccos(-0 + i Inf)) = 0", __imag__ result, 0.0);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_zero, minus_infty));
  check_isinfp ("real(ccos(-0 - i Inf)) = +Inf", __real__ result);
  check ("imag(ccos(-0 - i Inf)) = -0", __imag__ result, minus_zero);

  result = FUNC(ccos) (BUILD_COMPLEX (plus_infty, plus_infty));
  check_isinfp_exc ("real(ccos(+Inf + i Inf)) = +Inf plus invalid exception",
		    __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(+Inf + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_infty, plus_infty));
  check_isinfp_exc ("real(ccos(-Inf + i Inf)) = +Inf plus invalid exception",
		    __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(-Inf + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (plus_infty, minus_infty));
  check_isinfp_exc ("real(ccos(Inf - i Inf)) = +Inf plus invalid exception",
		    __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(Inf - i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_infty, minus_infty));
  check_isinfp_exc ("real(ccos(-Inf - i Inf)) = +Inf plus invalid exception",
		    __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(-Inf - i Inf)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(ccos) (BUILD_COMPLEX (4.625, plus_infty));
  check_isinfn ("real(ccos(4.625 + i Inf)) = -Inf", __real__ result);
  check_isinfp ("imag(ccos(4.625 + i Inf)) = +Inf", __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (4.625, minus_infty));
  check_isinfn ("real(ccos(4.625 - i Inf)) = -Inf", __real__ result);
  check_isinfn ("imag(ccos(4.625 - i Inf)) = -Inf", __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (-4.625, plus_infty));
  check_isinfn ("real(ccos(-4.625 + i Inf)) = -Inf", __real__ result);
  check_isinfn ("imag(ccos(-4.625 + i Inf)) = -Inf", __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (-4.625, minus_infty));
  check_isinfn ("real(ccos(-4.625 - i Inf)) = -Inf", __real__ result);
  check_isinfp ("imag(ccos(-4.625 - i Inf)) = +Inf", __imag__ result);

  result = FUNC(ccos) (BUILD_COMPLEX (plus_infty, 6.75));
  check_isnan_exc ("real(ccos(+Inf + i6.75)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(+Inf + i6.75)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (plus_infty, -6.75));
  check_isnan_exc ("real(ccos(+Inf - i6.75)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(+Inf - i6.75)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_infty, 6.75));
  check_isnan_exc ("real(ccos(-Inf + i6.75)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(-Inf + i6.75)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_infty, -6.75));
  check_isnan_exc ("real(ccos(-Inf - i6.75)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(-Inf - i6.75)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(ccos) (BUILD_COMPLEX (nan_value, 0.0));
  check_isnan ("real(ccos(NaN + i0)) = NaN", __real__ result);
  check ("imag(ccos(NaN + i0)) = +-0", FUNC(fabs) (__imag__ result), 0);
  result = FUNC(ccos) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan ("real(ccos(NaN - i0)) = NaN", __real__ result);
  check ("imag(ccos(NaN - i0)) = +-0", FUNC(fabs) (__imag__ result), 0);

  result = FUNC(ccos) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isinfp ("real(ccos(NaN + i Inf)) = +Inf", __real__ result);
  check_isnan ("imag(ccos(NaN + i Inf)) = NaN", __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (nan_value, minus_infty));
  check_isinfp ("real(ccos(NaN - i Inf)) = +Inf", __real__ result);
  check_isnan ("imag(ccos(NaN - i Inf)) = NaN", __imag__ result);

  result = FUNC(ccos) (BUILD_COMPLEX (nan_value, 9.0));
  check_isnan_maybe_exc ("real(ccos(NaN + i9.0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(NaN + i9.0)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (nan_value, -9.0));
  check_isnan_maybe_exc ("real(ccos(NaN - i9.0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(NaN - i9.0)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(ccos) (BUILD_COMPLEX (0.0, nan_value));
  check_isnan ("real(ccos(0 + i NaN)) = NaN", __real__ result);
  check ("imag(ccos(0 + i NaN)) = +-0", FUNC(fabs) (__imag__ result), 0.0);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_zero, nan_value));
  check_isnan ("real(ccos(-0 + i NaN)) = NaN", __real__ result);
  check ("imag(ccos(-0 + i NaN)) = +-0", FUNC(fabs) (__imag__ result), 0.0);

  result = FUNC(ccos) (BUILD_COMPLEX (10.0, nan_value));
  check_isnan_maybe_exc ("real(ccos(10 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(10 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (-10.0, nan_value));
  check_isnan_maybe_exc ("real(ccos(-10 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(-10 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(ccos) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isnan_maybe_exc ("real(ccos(+Inf + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(+Inf + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ccos) (BUILD_COMPLEX (minus_infty, nan_value));
  check_isnan_maybe_exc ("real(ccos(-Inf + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccos(-Inf + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(ccos) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(ccos(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(ccos(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(ccos) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(ccos(0.7 + i 1.2)) = 1.384865764...", __real__ result,
	     1.3848657645312111080L, CHOOSE (4e-18L, 3e-16, 2e-7));
  check_eps ("imag(ccos(0.7 + i 1.2)) = -0.972421703...", __imag__ result,
	     -0.97242170335830028619L, CHOOSE (2e-16L, 2e-16, 0));

  result = FUNC(ccos) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(ccos(-2 - i 3)) == -4.18962...", __real__ result,
	     -4.1896256909688072301L, CHOOSE (2e-17L, 8.9e-16, 5e-7));
  check_eps ("imag(ccos(-2 - i 3)) == -9.10922...", __imag__ result,
	     -9.1092278937553365979L, CHOOSE (3e-18L, 0, 1e-6));
}


static void
ccosh_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(ccosh) (BUILD_COMPLEX (0.0, 0.0));
  check ("real(ccosh(0 + 0i)) = 1.0", __real__ result, 1.0);
  check ("imag(ccosh(0 + 0i)) = 0", __imag__ result, 0);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_zero, 0.0));
  check ("real(ccosh(-0 + 0i)) = 1.0", __real__ result, 1.0);
  check ("imag(ccosh(-0 + 0i)) = -0", __imag__ result, minus_zero);
  result = FUNC(ccosh) (BUILD_COMPLEX (0.0, minus_zero));
  check ("real(ccosh(0 - 0i)) = 1.0", __real__ result, 1.0);
  check ("imag(ccosh(0 - 0i)) = -0", __imag__ result, minus_zero);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(ccosh(-0 - 0i)) = 1.0", __real__ result, 1.0);
  check ("imag(ccosh(-0 - 0i)) = 0", __imag__ result, 0.0);

  result = FUNC(ccosh) (BUILD_COMPLEX (0.0, plus_infty));
  check_isnan_exc ("real(ccosh(0 + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(ccosh(0 + i Inf)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_zero, plus_infty));
  check_isnan_exc ("real(ccosh(-0 + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(ccosh(-0 + i Inf)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0);
  result = FUNC(ccosh) (BUILD_COMPLEX (0.0, minus_infty));
  check_isnan_exc ("real(ccosh(0 - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(ccosh(0 - i Inf)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_zero, minus_infty));
  check_isnan_exc ("real(ccosh(-0 - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check ("imag(ccosh(-0 - i Inf)) = +-0 plus invalid exception",
	 FUNC(fabs) (__imag__ result), 0);

  result = FUNC(ccosh) (BUILD_COMPLEX (plus_infty, 0.0));
  check_isinfp ("real(ccosh(+Inf + 0i)) = +Inf", __real__ result);
  check ("imag(ccosh(+Inf + 0i)) = 0", __imag__ result, 0);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_infty, 0.0));
  check_isinfp ("real(ccosh(-Inf + 0i)) = +Inf", __real__ result);
  check ("imag(ccosh(-Inf + 0i)) = -0", __imag__ result, minus_zero);
  result = FUNC(ccosh) (BUILD_COMPLEX (plus_infty, minus_zero));
  check_isinfp ("real(ccosh(+Inf - 0i)) = +Inf", __real__ result);
  check ("imag(ccosh(+Inf - 0i)) = -0", __imag__ result, minus_zero);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_infty, minus_zero));
  check_isinfp ("real(ccosh(-Inf - 0i)) = +Inf", __real__ result);
  check ("imag(ccosh(-Inf - 0i)) = 0", __imag__ result, 0.0);

  result = FUNC(ccosh) (BUILD_COMPLEX (plus_infty, plus_infty));
  check_isinfp_exc ("real(ccosh(+Inf + i Inf)) = +Inf plus invalid exception",
		    __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(+Inf + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_infty, plus_infty));
  check_isinfp_exc ("real(ccosh(-Inf + i Inf)) = +Inf plus invalid exception",
		    __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(-Inf + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (plus_infty, minus_infty));
  check_isinfp_exc ("real(ccosh(Inf - i Inf)) = +Inf plus invalid exception",
		    __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(Inf - i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_infty, minus_infty));
  check_isinfp_exc ("real(ccosh(-Inf - i Inf)) = +Inf plus invalid exception",
		    __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(-Inf - i Inf)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(ccosh) (BUILD_COMPLEX (plus_infty, 4.625));
  check_isinfn ("real(ccosh(+Inf + i4.625)) = -Inf", __real__ result);
  check_isinfn ("imag(ccosh(+Inf + i4.625)) = -Inf", __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_infty, 4.625));
  check_isinfn ("real(ccosh(-Inf + i4.625)) = -Inf", __real__ result);
  check_isinfp ("imag(ccosh(-Inf + i4.625)) = Inf", __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (plus_infty, -4.625));
  check_isinfn ("real(ccosh(+Inf - i4.625)) = -Inf", __real__ result);
  check_isinfp ("imag(ccosh(+Inf - i4.625)) = +Inf", __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_infty, -4.625));
  check_isinfn ("real(ccosh(-Inf - i4.625)) = -Inf", __real__ result);
  check_isinfn ("imag(ccosh(-Inf - i4.625)) = -Inf", __imag__ result);

  result = FUNC(ccosh) (BUILD_COMPLEX (6.75, plus_infty));
  check_isnan_exc ("real(ccosh(6.75 + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(6.75 + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (-6.75, plus_infty));
  check_isnan_exc ("real(ccosh(-6.75 + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(-6.75 + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (6.75, minus_infty));
  check_isnan_exc ("real(ccosh(6.75 - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(6.75 - i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (-6.75, minus_infty));
  check_isnan_exc ("real(ccosh(-6.75 - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(-6.75 - i Inf)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(ccosh) (BUILD_COMPLEX (0.0, nan_value));
  check_isnan ("real(ccosh(0 + i NaN)) = NaN", __real__ result);
  check ("imag(ccosh(0 + i NaN)) = +-0", FUNC(fabs) (__imag__ result), 0);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_zero, nan_value));
  check_isnan ("real(ccosh(-0 + i NaN)) = NaN", __real__ result);
  check ("imag(ccosh(-0 + i NaN)) = +-0", FUNC(fabs) (__imag__ result), 0);

  result = FUNC(ccosh) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isinfp ("real(ccosh(+Inf + i NaN)) = +Inf", __real__ result);
  check_isnan ("imag(ccosh(+Inf + i NaN)) = NaN", __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (minus_infty, nan_value));
  check_isinfp ("real(ccosh(-Inf + i NaN)) = +Inf", __real__ result);
  check_isnan ("imag(ccosh(-Inf + i NaN)) = NaN", __imag__ result);

  result = FUNC(ccosh) (BUILD_COMPLEX (9.0, nan_value));
  check_isnan_maybe_exc ("real(ccosh(9.0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(9.0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (-9.0, nan_value));
  check_isnan_maybe_exc ("real(ccosh(-9.0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(-9.0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(ccosh) (BUILD_COMPLEX (nan_value, 0.0));
  check_isnan ("real(ccosh(NaN + i0)) = NaN", __real__ result);
  check ("imag(ccosh(NaN + i0)) = +-0", FUNC(fabs) (__imag__ result), 0.0);
  result = FUNC(ccosh) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan ("real(ccosh(NaN - i0)) = NaN", __real__ result);
  check ("imag(ccosh(NaN - i0)) = +-0", FUNC(fabs) (__imag__ result), 0.0);

  result = FUNC(ccosh) (BUILD_COMPLEX (nan_value, 10.0));
  check_isnan_maybe_exc ("real(ccosh(NaN + i10)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(NaN + i10)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (nan_value, -10.0));
  check_isnan_maybe_exc ("real(ccosh(NaN - i10)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(NaN - i10)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(ccosh) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isnan_maybe_exc ("real(ccosh(NaN + i Inf)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(NaN + i Inf)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ccosh) (BUILD_COMPLEX (nan_value, minus_infty));
  check_isnan_maybe_exc ("real(ccosh(NaN - i Inf)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ccosh(NaN - i Inf)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(ccosh) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(ccosh(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(ccosh(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(ccosh) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(ccosh(0.7 + i 1.2)) == 0.45482...", __real__ result,
	     0.4548202223691477654L, CHOOSE (5e-17L, 6e-17, 9e-8));
  check_eps ("imag(ccosh(0.7 + i 1.2)) == 0.70702...", __imag__ result,
	     0.7070296600921537682L, CHOOSE (7e-17L, 2e-16, 0));

  result = FUNC(ccosh) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(ccosh(-2 - i 3)) == -3.72454...", __real__ result,
	     -3.7245455049153225654L, CHOOSE (7e-19L, 4.5e-16, 3e-7));
  check_eps ("imag(ccosh(-2 - i 3)) == -0.51182...", __imag__ result,
	     0.5118225699873846088L, CHOOSE (3e-19L, 2e-16, 6e-8));
}


static void
cacos_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(cacos) (BUILD_COMPLEX (0, 0));
  check ("real(cacos(0 + i0)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(cacos(0 + i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(cacos) (BUILD_COMPLEX (minus_zero, 0));
  check ("real(cacos(-0 + i0)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(cacos(-0 + i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(cacos) (BUILD_COMPLEX (0, minus_zero));
  check ("real(cacos(0 - i0)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(cacos(0 - i0)) = 0", __imag__ result, 0);
  result = FUNC(cacos) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(cacos(-0 - i0)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(cacos(-0 - i0)) = 0", __imag__ result, 0);

  result = FUNC(cacos) (BUILD_COMPLEX (minus_infty, plus_infty));
  check ("real(cacos(-Inf + i Inf)) = 3*pi/4", __real__ result,
	 M_PIl - M_PI_4l);
  check_isinfn ("imag(cacos(-Inf + i Inf)) = -Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (minus_infty, minus_infty));
  check ("real(cacos(-Inf - i Inf)) = 3*pi/4", __real__ result,
	 M_PIl - M_PI_4l);
  check_isinfp ("imag(cacos(-Inf - i Inf)) = +Inf", __imag__ result);

  result = FUNC(cacos) (BUILD_COMPLEX (plus_infty, plus_infty));
  check ("real(cacos(+Inf + i Inf)) = pi/4", __real__ result, M_PI_4l);
  check_isinfn ("imag(cacos(+Inf + i Inf)) = -Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (plus_infty, minus_infty));
  check ("real(cacos(+Inf - i Inf)) = pi/4", __real__ result, M_PI_4l);
  check_isinfp ("imag(cacos(+Inf - i Inf)) = +Inf", __imag__ result);

  result = FUNC(cacos) (BUILD_COMPLEX (-10.0, plus_infty));
  check ("real(cacos(-10.0 + i Inf)) = pi/2", __real__ result, M_PI_2l);
  check_isinfn ("imag(cacos(-10.0 + i Inf)) = -Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (-10.0, minus_infty));
  check ("real(cacos(-10.0 - i Inf)) = pi/2", __real__ result, M_PI_2l);
  check_isinfp ("imag(cacos(-10.0 - i Inf)) = +Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (0, plus_infty));
  check ("real(cacos(0 + i Inf)) = pi/2", __real__ result, M_PI_2l);
  check_isinfn ("imag(cacos(0 + i Inf)) = -Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (0, minus_infty));
  check ("real(cacos(0 - i Inf)) = pi/2", __real__ result, M_PI_2l);
  check_isinfp ("imag(cacos(0 - i Inf)) = +Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (0.1, plus_infty));
  check ("real(cacos(0.1 + i Inf)) = pi/2", __real__ result, M_PI_2l);
  check_isinfn ("imag(cacos(0.1 + i Inf)) = -Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (0.1, minus_infty));
  check ("real(cacos(0.1 - i Inf)) = pi/2", __real__ result, M_PI_2l);
  check_isinfp ("imag(cacos(0.1 - i Inf)) = +Inf", __imag__ result);

  result = FUNC(cacos) (BUILD_COMPLEX (minus_infty, 0));
  check ("real(cacos(-Inf + i0)) = pi", __real__ result, M_PIl);
  check_isinfn ("imag(cacos(-Inf + i0)) = -Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (minus_infty, minus_zero));
  check ("real(cacos(-Inf - i0)) = pi", __real__ result, M_PIl);
  check_isinfp ("imag(cacos(-Inf - i0)) = +Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (minus_infty, 100));
  check ("real(cacos(-Inf + i100)) = pi", __real__ result, M_PIl);
  check_isinfn ("imag(cacos(-Inf + i100)) = -Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (minus_infty, -100));
  check ("real(cacos(-Inf - i100)) = pi", __real__ result, M_PIl);
  check_isinfp ("imag(cacos(-Inf - i100)) = +Inf", __imag__ result);

  result = FUNC(cacos) (BUILD_COMPLEX (plus_infty, 0));
  check ("real(cacos(+Inf + i0)) = 0", __real__ result, 0);
  check_isinfn ("imag(cacos(+Inf + i0)) = -Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (plus_infty, minus_zero));
  check ("real(cacos(+Inf - i0)) = 0", __real__ result, 0);
  check_isinfp ("imag(cacos(+Inf - i0)) = +Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (plus_infty, 0.5));
  check ("real(cacos(+Inf + i0.5)) = 0", __real__ result, 0);
  check_isinfn ("imag(cacos(+Inf + i0.5)) = -Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (plus_infty, -0.5));
  check ("real(cacos(+Inf - i0.5)) = 0", __real__ result, 0);
  check_isinfp ("imag(cacos(+Inf - i0.5)) = +Inf", __imag__ result);

  result = FUNC(cacos) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isnan ("real(cacos(+Inf + i NaN)) = NaN", __real__ result);
  check_isinfp ("imag(cacos(+Inf + i NaN)) = +-Inf",
		FUNC(fabs) (__imag__ result));
  result = FUNC(cacos) (BUILD_COMPLEX (minus_infty, nan_value));
  check_isnan ("real(cacos(-Inf + i NaN)) = NaN", __real__ result);
  check_isinfp ("imag(cacos(-Inf + i NaN)) = +-Inf",
		FUNC(fabs) (__imag__ result));

  result = FUNC(cacos) (BUILD_COMPLEX (0, nan_value));
  check ("real(cacos(0 + i NaN)) = pi/2", __real__ result, M_PI_2l);
  check_isnan ("imag(cacos(0 + i NaN)) = NaN", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (minus_zero, nan_value));
  check ("real(cacos(-0 + i NaN)) = pi/2", __real__ result, M_PI_2l);
  check_isnan ("imag(cacos(-0 + i NaN)) = NaN", __imag__ result);

  result = FUNC(cacos) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isnan ("real(cacos(NaN + i Inf)) = NaN", __real__ result);
  check_isinfn ("imag(cacos(NaN + i Inf)) = -Inf", __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (nan_value, minus_infty));
  check_isnan ("real(cacos(NaN - i Inf)) = NaN", __real__ result);
  check_isinfp ("imag(cacos(NaN - i Inf)) = +Inf", __imag__ result);

  result = FUNC(cacos) (BUILD_COMPLEX (10.5, nan_value));
  check_isnan_maybe_exc ("real(cacos(10.5 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cacos(10.5 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (-10.5, nan_value));
  check_isnan_maybe_exc ("real(cacos(-10.5 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cacos(-10.5 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(cacos) (BUILD_COMPLEX (nan_value, 0.75));
  check_isnan_maybe_exc ("real(cacos(NaN + i0.75)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cacos(NaN + i0.75)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(cacos) (BUILD_COMPLEX (-10.5, nan_value));
  check_isnan_maybe_exc ("real(cacos(NaN - i0.75)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cacos(NaN - i0.75)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(cacos) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(cacos(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(cacos(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(cacos) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(cacos(0.7 + i 1.2)) == 1.13518...", __real__ result,
	     1.1351827477151551089L, CHOOSE (2e-17L, 3e-16, 2e-7));
  check_eps ("imag(cacos(0.7 + i 1.2)) == -1.09276...", __imag__ result,
	     -1.0927647857577371459L, CHOOSE (4e-17L, 3e-16, 3e-7));

  result = FUNC(cacos) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(cacos(-2 - i 3)) == 2.14144...", __real__ result,
	     2.1414491111159960199L, CHOOSE (3e-19L, 0, 0));
  check_eps ("imag(cacos(-2 - i 3)) == -1.98338...", __imag__ result,
	     1.9833870299165354323L, CHOOSE (3e-19L, 0, 0));
}


static void
cacosh_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(cacosh) (BUILD_COMPLEX (0, 0));
  check ("real(cacosh(0 + i0)) = 0", __real__ result, 0);
  check ("imag(cacosh(0 + i0)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(cacosh) (BUILD_COMPLEX (minus_zero, 0));
  check ("real(cacosh(-0 + i0)) = 0", __real__ result, 0);
  check ("imag(cacosh(-0 + i0)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(cacosh) (BUILD_COMPLEX (0, minus_zero));
  check ("real(cacosh(0 - i0)) = 0", __real__ result, 0);
  check ("imag(cacosh(0 - i0)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(cacosh) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(cacosh(-0 - i0)) = 0", __real__ result, 0);
  check ("imag(cacosh(-0 - i0)) = -pi/2", __imag__ result, -M_PI_2l);

  result = FUNC(cacosh) (BUILD_COMPLEX (minus_infty, plus_infty));
  check_isinfp ("real(cacosh(-Inf + i Inf)) = +Inf", __real__ result);
  check ("imag(cacosh(-Inf + i Inf)) = 3*pi/4", __imag__ result,
	 M_PIl - M_PI_4l);
  result = FUNC(cacosh) (BUILD_COMPLEX (minus_infty, minus_infty));
  check_isinfp ("real(cacosh(-Inf - i Inf)) = +Inf", __real__ result);
  check ("imag(cacosh(-Inf - i Inf)) = -3*pi/4", __imag__ result,
	 M_PI_4l - M_PIl);

  result = FUNC(cacosh) (BUILD_COMPLEX (plus_infty, plus_infty));
  check_isinfp ("real(cacosh(+Inf + i Inf)) = +Inf", __real__ result);
  check ("imag(cacosh(+Inf + i Inf)) = pi/4", __imag__ result, M_PI_4l);
  result = FUNC(cacosh) (BUILD_COMPLEX (plus_infty, minus_infty));
  check_isinfp ("real(cacosh(+Inf - i Inf)) = +Inf", __real__ result);
  check ("imag(cacosh(+Inf - i Inf)) = -pi/4", __imag__ result, -M_PI_4l);

  result = FUNC(cacosh) (BUILD_COMPLEX (-10.0, plus_infty));
  check_isinfp ("real(cacosh(-10.0 + i Inf)) = +Inf", __real__ result);
  check ("imag(cacosh(-10.0 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(cacosh) (BUILD_COMPLEX (-10.0, minus_infty));
  check_isinfp ("real(cacosh(-10.0 - i Inf)) = +Inf", __real__ result);
  check ("imag(cacosh(-10.0 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(cacosh) (BUILD_COMPLEX (0, plus_infty));
  check_isinfp ("real(cacosh(0 + i Inf)) = +Inf", __real__ result);
  check ("imag(cacosh(0 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(cacosh) (BUILD_COMPLEX (0, minus_infty));
  check_isinfp ("real(cacosh(0 - i Inf)) = +Inf", __real__ result);
  check ("imag(cacosh(0 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(cacosh) (BUILD_COMPLEX (0.1, plus_infty));
  check_isinfp ("real(cacosh(0.1 + i Inf)) = +Inf", __real__ result);
  check ("imag(cacosh(0.1 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(cacosh) (BUILD_COMPLEX (0.1, minus_infty));
  check_isinfp ("real(cacosh(0.1 - i Inf)) = +Inf", __real__ result);
  check ("imag(cacosh(0.1 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);

  result = FUNC(cacosh) (BUILD_COMPLEX (minus_infty, 0));
  check_isinfp ("real(cacosh(-Inf + i0)) = +Inf", __real__ result);
  check ("imag(cacosh(-Inf + i0)) = pi", __imag__ result, M_PIl);
  result = FUNC(cacosh) (BUILD_COMPLEX (minus_infty, minus_zero));
  check_isinfp ("real(cacosh(-Inf - i0)) = +Inf", __real__ result);
  check ("imag(cacosh(-Inf - i0)) = -pi", __imag__ result, -M_PIl);
  result = FUNC(cacosh) (BUILD_COMPLEX (minus_infty, 100));
  check_isinfp ("real(cacosh(-Inf + i100)) = +Inf", __real__ result);
  check ("imag(cacosh(-Inf + i100)) = pi", __imag__ result, M_PIl);
  result = FUNC(cacosh) (BUILD_COMPLEX (minus_infty, -100));
  check_isinfp ("real(cacosh(-Inf - i100)) = +Inf", __real__ result);
  check ("imag(cacosh(-Inf - i100)) = -pi", __imag__ result, -M_PIl);

  result = FUNC(cacosh) (BUILD_COMPLEX (plus_infty, 0));
  check_isinfp ("real(cacosh(+Inf + i0)) = +Inf", __real__ result);
  check ("imag(cacosh(+Inf + i0)) = 0", __imag__ result, 0);
  result = FUNC(cacosh) (BUILD_COMPLEX (plus_infty, minus_zero));
  check_isinfp ("real(cacosh(+Inf - i0)) = +Inf", __real__ result);
  check ("imag(cacosh(+Inf - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(cacosh) (BUILD_COMPLEX (plus_infty, 0.5));
  check_isinfp ("real(cacosh(+Inf + i0.5)) = +Inf", __real__ result);
  check ("imag(cacosh(+Inf + i0.5)) = 0", __imag__ result, 0);
  result = FUNC(cacosh) (BUILD_COMPLEX (plus_infty, -0.5));
  check_isinfp ("real(cacosh(+Inf - i0.5)) = +Inf", __real__ result);
  check ("imag(cacosh(+Inf - i0.5)) = -0", __imag__ result, minus_zero);

  result = FUNC(cacosh) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isinfp ("real(cacosh(+Inf + i NaN)) = +Inf", __real__ result);
  check_isnan ("imag(cacosh(+Inf + i NaN)) = NaN", __imag__ result);
  result = FUNC(cacosh) (BUILD_COMPLEX (minus_infty, nan_value));
  check_isinfp ("real(cacosh(-Inf + i NaN)) = +Inf", __real__ result);
  check_isnan ("imag(cacosh(-Inf + i NaN)) = NaN", __imag__ result);

  result = FUNC(cacosh) (BUILD_COMPLEX (0, nan_value));
  check_isnan ("real(cacosh(0 + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(cacosh(0 + i NaN)) = NaN", __imag__ result);
  result = FUNC(cacosh) (BUILD_COMPLEX (minus_zero, nan_value));
  check_isnan ("real(cacosh(-0 + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(cacosh(-0 + i NaN)) = NaN", __imag__ result);

  result = FUNC(cacosh) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isinfp ("real(cacosh(NaN + i Inf)) = +Inf", __real__ result);
  check_isnan ("imag(cacosh(NaN + i Inf)) = NaN", __imag__ result);
  result = FUNC(cacosh) (BUILD_COMPLEX (nan_value, minus_infty));
  check_isinfp ("real(cacosh(NaN - i Inf)) = +Inf", __real__ result);
  check_isnan ("imag(cacosh(NaN - i Inf)) = NaN", __imag__ result);

  result = FUNC(cacosh) (BUILD_COMPLEX (10.5, nan_value));
  check_isnan_maybe_exc ("real(cacosh(10.5 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cacosh(10.5 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(cacosh) (BUILD_COMPLEX (-10.5, nan_value));
  check_isnan_maybe_exc ("real(cacosh(-10.5 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cacosh(-10.5 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(cacosh) (BUILD_COMPLEX (nan_value, 0.75));
  check_isnan_maybe_exc ("real(cacosh(NaN + i0.75)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cacosh(NaN + i0.75)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(cacosh) (BUILD_COMPLEX (-10.5, nan_value));
  check_isnan_maybe_exc ("real(cacosh(NaN - i0.75)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(cacosh(NaN - i0.75)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(cacosh) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(cacosh(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(cacosh(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(cacosh) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(cacosh(0.7 + i 1.2)) == 1.09276...", __real__ result,
	     1.0927647857577371459L, CHOOSE (4e-17L, 3e-16, 2e-7));
  check_eps ("imag(cacosh(0.7 + i 1.2)) == 1.13518...", __imag__ result,
	     1.1351827477151551089L, CHOOSE (2e-17L, 0, 1.2e-7));

  result = FUNC(cacosh) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(cacosh(-2 - i 3)) == -1.98338...", __real__ result,
	     -1.9833870299165354323L, CHOOSE (2e-18L, 3e-16, 9e-7));
  check_eps ("imag(cacosh(-2 - i 3)) == 2.14144...", __imag__ result,
	     2.1414491111159960199L, CHOOSE (4.5e-19, 5e-16, 1e-6));
}


static void
casin_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(casin) (BUILD_COMPLEX (0, 0));
  check ("real(casin(0 + i0)) = 0", __real__ result, 0);
  check ("imag(casin(0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(casin) (BUILD_COMPLEX (minus_zero, 0));
  check ("real(casin(-0 + i0)) = -0", __real__ result, minus_zero);
  check ("imag(casin(-0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(casin) (BUILD_COMPLEX (0, minus_zero));
  check ("real(casin(0 - i0)) = 0", __real__ result, 0);
  check ("imag(casin(0 - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(casin) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(casin(-0 - i0)) = -0", __real__ result, minus_zero);
  check ("imag(casin(-0 - i0)) = -0", __imag__ result, minus_zero);

  result = FUNC(casin) (BUILD_COMPLEX (plus_infty, plus_infty));
  check ("real(casin(+Inf + i Inf)) = pi/4", __real__ result, M_PI_4l);
  check_isinfp ("imag(casin(+Inf + i Inf)) = +Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (plus_infty, minus_infty));
  check ("real(casin(+Inf - i Inf)) = pi/4", __real__ result, M_PI_4l);
  check_isinfn ("imag(casin(+Inf - i Inf)) = -Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (minus_infty, plus_infty));
  check ("real(casin(-Inf + i Inf)) = -pi/4", __real__ result, -M_PI_4l);
  check_isinfp ("imag(casin(-Inf + i Inf)) = +Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (minus_infty, minus_infty));
  check ("real(casin(-Inf - i Inf)) = -pi/4", __real__ result, -M_PI_4l);
  check_isinfn ("imag(casin(-Inf - i Inf)) = -Inf", __imag__ result);

  result = FUNC(casin) (BUILD_COMPLEX (-10.0, plus_infty));
  check ("real(casin(-10.0 + i Inf)) = -0", __real__ result, minus_zero);
  check_isinfp ("imag(casin(-10.0 + i Inf)) = +Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (-10.0, minus_infty));
  check ("real(casin(-10.0 - i Inf)) = -0", __real__ result, minus_zero);
  check_isinfn ("imag(casin(-10.0 - i Inf)) = -Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (0, plus_infty));
  check ("real(casin(0 + i Inf)) = 0", __real__ result, 0.0);
  check_isinfp ("imag(casin(0 + i Inf)) = +Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (0, minus_infty));
  check ("real(casin(0 - i Inf)) = 0", __real__ result, 0.0);
  check_isinfn ("imag(casin(0 - i Inf)) = -Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (minus_zero, plus_infty));
  check ("real(casin(-0 + i Inf)) = -0", __real__ result, minus_zero);
  check_isinfp ("imag(casin(-0 + i Inf)) = +Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (minus_zero, minus_infty));
  check ("real(casin(-0 - i Inf)) = -0", __real__ result, minus_zero);
  check_isinfn ("imag(casin(-0 - i Inf)) = -Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (0.1, plus_infty));
  check ("real(casin(0.1 + i Inf)) = 0", __real__ result, 0);
  check_isinfp ("imag(casin(0.1 + i Inf)) = +Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (0.1, minus_infty));
  check ("real(casin(0.1 - i Inf)) = 0", __real__ result, 0);
  check_isinfn ("imag(casin(0.1 - i Inf)) = -Inf", __imag__ result);

  result = FUNC(casin) (BUILD_COMPLEX (minus_infty, 0));
  check ("real(casin(-Inf + i0)) = -pi/2", __real__ result, -M_PI_2l);
  check_isinfp ("imag(casin(-Inf + i0)) = +Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (minus_infty, minus_zero));
  check ("real(casin(-Inf - i0)) = -pi/2", __real__ result, -M_PI_2l);
  check_isinfn ("imag(casin(-Inf - i0)) = -Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (minus_infty, 100));
  check ("real(casin(-Inf + i100)) = -pi/2", __real__ result, -M_PI_2l);
  check_isinfp ("imag(casin(-Inf + i100)) = +Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (minus_infty, -100));
  check ("real(casin(-Inf - i100)) = -pi/2", __real__ result, -M_PI_2l);
  check_isinfn ("imag(casin(-Inf - i100)) = -Inf", __imag__ result);

  result = FUNC(casin) (BUILD_COMPLEX (plus_infty, 0));
  check ("real(casin(+Inf + i0)) = pi/2", __real__ result, M_PI_2l);
  check_isinfp ("imag(casin(+Inf + i0)) = +Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (plus_infty, minus_zero));
  check ("real(casin(+Inf - i0)) = pi/2", __real__ result, M_PI_2l);
  check_isinfn ("imag(casin(+Inf - i0)) = -Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (plus_infty, 0.5));
  check ("real(casin(+Inf + i0.5)) = pi/2", __real__ result, M_PI_2l);
  check_isinfp ("imag(casin(+Inf + i0.5)) = +Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (plus_infty, -0.5));
  check ("real(casin(+Inf - i0.5)) = pi/2", __real__ result, M_PI_2l);
  check_isinfn ("imag(casin(+Inf - i0.5)) = -Inf", __imag__ result);

  result = FUNC(casin) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isnan ("real(casin(NaN + i Inf)) = NaN", __real__ result);
  check_isinfp ("imag(casin(NaN + i Inf)) = +Inf", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (nan_value, minus_infty));
  check_isnan ("real(casin(NaN - i Inf)) = NaN", __real__ result);
  check_isinfn ("imag(casin(NaN - i Inf)) = -Inf", __imag__ result);

  result = FUNC(casin) (BUILD_COMPLEX (0.0, nan_value));
  check ("real(casin(0 + i NaN)) = 0", __real__ result, 0.0);
  check_isnan ("imag(casin(0 + i NaN)) = NaN", __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (minus_zero, nan_value));
  check ("real(casin(-0 + i NaN)) = -0", __real__ result, minus_zero);
  check_isnan ("imag(casin(-0 + i NaN)) = NaN", __imag__ result);

  result = FUNC(casin) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isnan ("real(casin(+Inf + i NaN)) = NaN", __real__ result);
  check_isinfp ("imag(casin(+Inf + i NaN)) = +-Inf",
		FUNC(fabs) (__imag__ result));
  result = FUNC(casin) (BUILD_COMPLEX (minus_infty, nan_value));
  check_isnan ("real(casin(-Inf + i NaN)) = NaN", __real__ result);
  check_isinfp ("imag(casin(-Inf + NaN)) = +-Inf",
		FUNC(fabs) (__imag__ result));

  result = FUNC(casin) (BUILD_COMPLEX (nan_value, 10.5));
  check_isnan_maybe_exc ("real(casin(NaN + i10.5)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(casin(NaN + i10.5)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (nan_value, -10.5));
  check_isnan_maybe_exc ("real(casin(NaN - i10.5)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(casin(NaN - i10.5)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(casin) (BUILD_COMPLEX (0.75, nan_value));
  check_isnan_maybe_exc ("real(casin(0.75 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(casin(0.75 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(casin) (BUILD_COMPLEX (-0.75, nan_value));
  check_isnan_maybe_exc ("real(casin(-0.75 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(casin(-0.75 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(casin) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(casin(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(casin(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(casin) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(casin(0.7 + i 1.2)) == 0.43561...", __real__ result,
	     0.4356135790797415103L, CHOOSE (2e-17L, 2e-16, 2e-7));
  check_eps ("imag(casin(0.7 + i 1.2)) == 1.09276...", __imag__ result,
	     1.0927647857577371459L, CHOOSE (4e-17L, 3e-16, 3e-7));

  result = FUNC(casin) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(casin(-2 - i 3)) == -0.57065...", __real__ result,
	     -0.5706527843210994007L, CHOOSE (4e-19L, 0, 0));
  check_eps ("imag(casin(-2 - i 3)) == -1.98338...", __imag__ result,
	     -1.9833870299165354323L, CHOOSE (3e-19L, 0, 0));
}


static void
casinh_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(casinh) (BUILD_COMPLEX (0, 0));
  check ("real(casinh(0 + i0)) = 0", __real__ result, 0);
  check ("imag(casinh(0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(casinh) (BUILD_COMPLEX (minus_zero, 0));
  check ("real(casinh(-0 + i0)) = -0", __real__ result, minus_zero);
  check ("imag(casinh(-0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(casinh) (BUILD_COMPLEX (0, minus_zero));
  check ("real(casinh(0 - i0)) = 0", __real__ result, 0);
  check ("imag(casinh(0 - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(casinh) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(casinh(-0 - i0)) = -0", __real__ result, minus_zero);
  check ("imag(casinh(-0 - i0)) = -0", __imag__ result, minus_zero);

  result = FUNC(casinh) (BUILD_COMPLEX (plus_infty, plus_infty));
  check_isinfp ("real(casinh(+Inf + i Inf)) = +Inf", __real__ result);
  check ("imag(casinh(+Inf + i Inf)) = pi/4", __imag__ result, M_PI_4l);
  result = FUNC(casinh) (BUILD_COMPLEX (plus_infty, minus_infty));
  check_isinfp ("real(casinh(+Inf - i Inf)) = +Inf", __real__ result);
  check ("imag(casinh(+Inf - i Inf)) = -pi/4", __imag__ result, -M_PI_4l);
  result = FUNC(casinh) (BUILD_COMPLEX (minus_infty, plus_infty));
  check_isinfn ("real(casinh(-Inf + i Inf)) = -Inf", __real__ result);
  check ("imag(casinh(-Inf + i Inf)) = pi/4", __imag__ result, M_PI_4l);
  result = FUNC(casinh) (BUILD_COMPLEX (minus_infty, minus_infty));
  check_isinfn ("real(casinh(-Inf - i Inf)) = -Inf", __real__ result);
  check ("imag(casinh(-Inf - i Inf)) = -pi/4", __imag__ result, -M_PI_4l);

  result = FUNC(casinh) (BUILD_COMPLEX (-10.0, plus_infty));
  check_isinfn ("real(casinh(-10.0 + i Inf)) = -Inf", __real__ result);
  check ("imag(casinh(-10.0 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(casinh) (BUILD_COMPLEX (-10.0, minus_infty));
  check_isinfn ("real(casinh(-10.0 - i Inf)) = -Inf", __real__ result);
  check ("imag(casinh(-10.0 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(casinh) (BUILD_COMPLEX (0, plus_infty));
  check_isinfp ("real(casinh(0 + i Inf)) = +Inf", __real__ result);
  check ("imag(casinh(0 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(casinh) (BUILD_COMPLEX (0, minus_infty));
  check_isinfp ("real(casinh(0 - i Inf)) = +Inf", __real__ result);
  check ("imag(casinh(0 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(casinh) (BUILD_COMPLEX (minus_zero, plus_infty));
  check_isinfn ("real(casinh(-0 + i Inf)) = -Inf", __real__ result);
  check ("imag(casinh(-0 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(casinh) (BUILD_COMPLEX (minus_zero, minus_infty));
  check_isinfn ("real(casinh(-0 - i Inf)) = -Inf", __real__ result);
  check ("imag(casinh(-0 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(casinh) (BUILD_COMPLEX (0.1, plus_infty));
  check_isinfp ("real(casinh(0.1 + i Inf)) = +Inf", __real__ result);
  check ("imag(casinh(0.1 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(casinh) (BUILD_COMPLEX (0.1, minus_infty));
  check_isinfp ("real(casinh(0.1 - i Inf)) = +Inf", __real__ result);
  check ("imag(casinh(0.1 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);

  result = FUNC(casinh) (BUILD_COMPLEX (minus_infty, 0));
  check_isinfn ("real(casinh(-Inf + i0)) = -Inf", __real__ result);
  check ("imag(casinh(-Inf + i0)) = 0", __imag__ result, 0);
  result = FUNC(casinh) (BUILD_COMPLEX (minus_infty, minus_zero));
  check_isinfn ("real(casinh(-Inf - i0)) = -Inf", __real__ result);
  check ("imag(casinh(-Inf - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(casinh) (BUILD_COMPLEX (minus_infty, 100));
  check_isinfn ("real(casinh(-Inf + i100)) = -Inf", __real__ result);
  check ("imag(casinh(-Inf + i100)) = 0", __imag__ result, 0);
  result = FUNC(casinh) (BUILD_COMPLEX (minus_infty, -100));
  check_isinfn ("real(casinh(-Inf - i100)) = -Inf", __real__ result);
  check ("imag(casinh(-Inf - i100)) = -0", __imag__ result, minus_zero);

  result = FUNC(casinh) (BUILD_COMPLEX (plus_infty, 0));
  check_isinfp ("real(casinh(+Inf + i0)) = +Inf", __real__ result);
  check ("imag(casinh(+Inf + i0)) = 0", __imag__ result, 0);
  result = FUNC(casinh) (BUILD_COMPLEX (plus_infty, minus_zero));
  check_isinfp ("real(casinh(+Inf - i0)) = +Inf", __real__ result);
  check ("imag(casinh(+Inf - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(casinh) (BUILD_COMPLEX (plus_infty, 0.5));
  check_isinfp ("real(casinh(+Inf + i0.5)) = +Inf", __real__ result);
  check ("imag(casinh(+Inf + i0.5)) = 0", __imag__ result, 0);
  result = FUNC(casinh) (BUILD_COMPLEX (plus_infty, -0.5));
  check_isinfp ("real(casinh(+Inf - i0.5)) = +Inf", __real__ result);
  check ("imag(casinh(+Inf - i0.5)) = -0", __imag__ result, minus_zero);

  result = FUNC(casinh) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isinfp ("real(casinh(+Inf + i NaN)) = +Inf", __real__ result);
  check_isnan ("imag(casinh(+Inf + i NaN)) = NaN", __imag__ result);
  result = FUNC(casinh) (BUILD_COMPLEX (minus_infty, nan_value));
  check_isinfn ("real(casinh(-Inf + i NaN)) = -Inf", __real__ result);
  check_isnan ("imag(casinh(-Inf + i NaN)) = NaN", __imag__ result);

  result = FUNC(casinh) (BUILD_COMPLEX (nan_value, 0));
  check_isnan ("real(casinh(NaN + i0)) = NaN", __real__ result);
  check ("imag(casinh(NaN + i0)) = 0", __imag__ result, 0);
  result = FUNC(casinh) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan ("real(casinh(NaN - i0)) = NaN", __real__ result);
  check ("imag(casinh(NaN - i0)) = -0", __imag__ result, minus_zero);

  result = FUNC(casinh) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isinfp ("real(casinh(NaN + i Inf)) = +-Inf",
		FUNC(fabs) (__real__ result));
  check_isnan ("imag(casinh(NaN + i Inf)) = NaN", __imag__ result);
  result = FUNC(casinh) (BUILD_COMPLEX (nan_value, minus_infty));
  check_isinfp ("real(casinh(NaN - i Inf)) = +-Inf",
		FUNC(fabs) (__real__ result));
  check_isnan ("imag(casinh(NaN - i Inf)) = NaN", __imag__ result);

  result = FUNC(casinh) (BUILD_COMPLEX (10.5, nan_value));
  check_isnan_maybe_exc ("real(casinh(10.5 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(casinh(10.5 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(casinh) (BUILD_COMPLEX (-10.5, nan_value));
  check_isnan_maybe_exc ("real(casinh(-10.5 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(casinh(-10.5 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(casinh) (BUILD_COMPLEX (nan_value, 0.75));
  check_isnan_maybe_exc ("real(casinh(NaN + i0.75)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(casinh(NaN + i0.75)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(casinh) (BUILD_COMPLEX (-0.75, nan_value));
  check_isnan_maybe_exc ("real(casinh(NaN - i0.75)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(casinh(NaN - i0.75)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(casinh) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(casinh(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(casinh(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(casinh) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(casinh(0.7 + i 1.2)) == 0.97865...", __real__ result,
	     0.9786545955936738768L, CHOOSE (5e-17L, 2e-16, 0));
  check_eps ("imag(casinh(0.7 + i 1.2)) == 0.91135...", __imag__ result,
	     0.9113541895315601156L, CHOOSE (7e-19L, 2e-16, 2e-7));

  result = FUNC(casinh) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(casinh(-2 - i 3)) == -1.96863...", __real__ result,
	     -1.9686379257930962917L, CHOOSE (7e-19L, 2e-15, 3e-6));
  check_eps ("imag(casinh(-2 - i 3)) == -0.96465...", __imag__ result,
	     -0.9646585044076027920L, CHOOSE (4e-19L, 2e-15, 4.5e-7));
}


static void
catan_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(catan) (BUILD_COMPLEX (0, 0));
  check ("real(catan(0 + i0)) = 0", __real__ result, 0);
  check ("imag(catan(0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(catan) (BUILD_COMPLEX (minus_zero, 0));
  check ("real(catan(-0 + i0)) = -0", __real__ result, minus_zero);
  check ("imag(catan(-0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(catan) (BUILD_COMPLEX (0, minus_zero));
  check ("real(catan(0 - i0)) = 0", __real__ result, 0);
  check ("imag(catan(0 - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(catan) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(catan(-0 - i0)) = -0", __real__ result, minus_zero);
  check ("imag(catan(-0 - i0)) = -0", __imag__ result, minus_zero);

  result = FUNC(catan) (BUILD_COMPLEX (plus_infty, plus_infty));
  check ("real(catan(+Inf + i Inf)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(catan(+Inf + i Inf)) = 0", __imag__ result, 0);
  result = FUNC(catan) (BUILD_COMPLEX (plus_infty, minus_infty));
  check ("real(catan(+Inf - i Inf)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(catan(+Inf - i Inf)) = -0", __imag__ result, minus_zero);
  result = FUNC(catan) (BUILD_COMPLEX (minus_infty, plus_infty));
  check ("real(catan(-Inf + i Inf)) = -pi/2", __real__ result, -M_PI_2l);
  check ("imag(catan(-Inf + i Inf)) = 0", __imag__ result, 0.0);
  result = FUNC(catan) (BUILD_COMPLEX (minus_infty, minus_infty));
  check ("real(catan(-Inf - i Inf)) = -pi/2", __real__ result, -M_PI_2l);
  check ("imag(catan(-Inf - i Inf)) = -0", __imag__ result, minus_zero);

  result = FUNC(catan) (BUILD_COMPLEX (plus_infty, -10.0));
  check ("real(catan(+Inf - i10.0)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(catan(+Inf - i10.0)) = -0", __imag__ result, minus_zero);
  result = FUNC(catan) (BUILD_COMPLEX (minus_infty, -10.0));
  check ("real(catan(-Inf - i10.0)) = -pi/2", __real__ result, -M_PI_2l);
  check ("imag(catan(-Inf - i10.0)) = -0", __imag__ result, minus_zero);
  result = FUNC(catan) (BUILD_COMPLEX (plus_infty, minus_zero));
  check ("real(catan(Inf - i0)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(catan(Inf - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(catan) (BUILD_COMPLEX (minus_infty, minus_zero));
  check ("real(catan(-Inf - i0)) = -pi/2", __real__ result, -M_PI_2l);
  check ("imag(catan(-Inf - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(catan) (BUILD_COMPLEX (plus_infty, 0.0));
  check ("real(catan(Inf + i0)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(catan(Inf + i0)) = 0", __imag__ result, 0.0);
  result = FUNC(catan) (BUILD_COMPLEX (minus_infty, 0.0));
  check ("real(catan(-Inf + i0)) = -pi/2", __real__ result, -M_PI_2l);
  check ("imag(catan(-Inf + i0)) = 0", __imag__ result, 0.0);
  result = FUNC(catan) (BUILD_COMPLEX (plus_infty, 0.1));
  check ("real(catan(+Inf + i0.1)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(catan(+Inf + i0.1)) = 0", __imag__ result, 0);
  result = FUNC(catan) (BUILD_COMPLEX (minus_infty, 0.1));
  check ("real(catan(-Inf + i0.1)) = -pi/2", __real__ result, -M_PI_2l);
  check ("imag(catan(-Inf + i0.1)) = 0", __imag__ result, 0);

  result = FUNC(catan) (BUILD_COMPLEX (0.0, minus_infty));
  check ("real(catan(0 - i Inf)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(catan(0 - i Inf)) = -0", __imag__ result, minus_zero);
  result = FUNC(catan) (BUILD_COMPLEX (minus_zero, minus_infty));
  check ("real(catan(-0 - i Inf)) = -pi/2", __real__ result, -M_PI_2l);
  check ("imag(catan(-0 - i Inf)) = -0", __imag__ result, minus_zero);
  result = FUNC(catan) (BUILD_COMPLEX (100.0, minus_infty));
  check ("real(catan(100 - i Inf)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(catan(100 - i Inf)) = -0", __imag__ result, minus_zero);
  result = FUNC(catan) (BUILD_COMPLEX (-100.0, minus_infty));
  check ("real(catan(-100 - i Inf)) = -pi/2", __real__ result, -M_PI_2l);
  check ("imag(catan(-100 - i Inf)) = -0", __imag__ result, minus_zero);

  result = FUNC(catan) (BUILD_COMPLEX (0.0, plus_infty));
  check ("real(catan(0 + i Inf)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(catan(0 + i Inf)) = 0", __imag__ result, 0);
  result = FUNC(catan) (BUILD_COMPLEX (minus_zero, plus_infty));
  check ("real(catan(-0 + i Inf)) = -pi/2", __real__ result, -M_PI_2l);
  check ("imag(catan(-0 + i Inf)) = 0", __imag__ result, 0);
  result = FUNC(catan) (BUILD_COMPLEX (0.5, plus_infty));
  check ("real(catan(0.5 + i Inf)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(catan(0.5 + i Inf)) = 0", __imag__ result, 0);
  result = FUNC(catan) (BUILD_COMPLEX (-0.5, plus_infty));
  check ("real(catan(-0.5 + i Inf)) = -pi/2", __real__ result, -M_PI_2l);
  check ("imag(catan(-0.5 + i Inf)) = 0", __imag__ result, 0);

  result = FUNC(catan) (BUILD_COMPLEX (nan_value, 0.0));
  check_isnan ("real(catan(NaN + i0)) = NaN", __real__ result);
  check ("imag(catan(NaN + i0)) = 0", __imag__ result, 0.0);
  result = FUNC(catan) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan ("real(catan(NaN - i0)) = NaN", __real__ result);
  check ("imag(catan(NaN - i0)) = -0", __imag__ result, minus_zero);

  result = FUNC(catan) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isnan ("real(catan(NaN + i Inf)) = NaN", __real__ result);
  check ("imag(catan(NaN + i Inf)) = 0", __imag__ result, 0);
  result = FUNC(catan) (BUILD_COMPLEX (nan_value, minus_infty));
  check_isnan ("real(catan(NaN - i Inf)) = NaN", __real__ result);
  check ("imag(catan(NaN - i Inf)) = -0", __imag__ result, minus_zero);

  result = FUNC(catan) (BUILD_COMPLEX (0.0, nan_value));
  check_isnan ("real(catan(0 + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(catan(0 + i NaN)) = NaN", __imag__ result);
  result = FUNC(catan) (BUILD_COMPLEX (minus_zero, nan_value));
  check_isnan ("real(catan(-0 + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(catan(-0 + i NaN)) = NaN", __imag__ result);

  result = FUNC(catan) (BUILD_COMPLEX (plus_infty, nan_value));
  check ("real(catan(+Inf + i NaN)) = pi/2", __real__ result, M_PI_2l);
  check ("imag(catan(+Inf + i NaN)) = +-0", FUNC(fabs) (__imag__ result), 0);
  result = FUNC(catan) (BUILD_COMPLEX (minus_infty, nan_value));
  check ("real(catan(-Inf + i NaN)) = -pi/2", __real__ result, -M_PI_2l);
  check ("imag(catan(-Inf + i NaN)) = +-0", FUNC(fabs) (__imag__ result), 0);

  result = FUNC(catan) (BUILD_COMPLEX (nan_value, 10.5));
  check_isnan_maybe_exc ("real(catan(NaN + i10.5)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(catan(NaN + i10.5)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(catan) (BUILD_COMPLEX (nan_value, -10.5));
  check_isnan_maybe_exc ("real(catan(NaN - i10.5)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(catan(NaN - i10.5)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(catan) (BUILD_COMPLEX (0.75, nan_value));
  check_isnan_maybe_exc ("real(catan(0.75 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(catan(0.75 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(catan) (BUILD_COMPLEX (-0.75, nan_value));
  check_isnan_maybe_exc ("real(catan(-0.75 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(catan(-0.75 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(catan) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(catan(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(catan(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(catan) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(catan(0.7 + i 1.2)) == 1.07857...", __real__ result,
	     1.0785743834118921877L, CHOOSE (3e-17, 0, 5e-7));
  check_eps ("imag(catan(0.7 + i 1.2)) == 0.57705...", __imag__ result,
	     0.5770573776534306764L, CHOOSE (3e-17L, 2e-16, 6e-8));

  result = FUNC(catan) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(catan(-2 - i 3)) == -1.40992...", __real__ result,
	     -1.4099210495965755225L, CHOOSE (0, 0, 4e-7));
  check_eps ("imag(catan(-2 - i 3)) == -0.22907...", __imag__ result,
	     -0.2290726829685387662L, CHOOSE (1.1e-19L, 3e-17, 2e-8));
}


static void
catanh_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(catanh) (BUILD_COMPLEX (0, 0));
  check ("real(catanh(0 + i0)) = 0", __real__ result, 0);
  check ("imag(catanh(0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(catanh) (BUILD_COMPLEX (minus_zero, 0));
  check ("real(catanh(-0 + i0)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(catanh) (BUILD_COMPLEX (0, minus_zero));
  check ("real(catanh(0 - i0)) = 0", __real__ result, 0);
  check ("imag(catanh(0 - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(catanh) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(catanh(-0 - i0)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-0 - i0)) = -0", __imag__ result, minus_zero);

  result = FUNC(catanh) (BUILD_COMPLEX (plus_infty, plus_infty));
  check ("real(catanh(+Inf + i Inf)) = 0", __real__ result, 0);
  check ("imag(catanh(+Inf + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (plus_infty, minus_infty));
  check ("real(catanh(+Inf - i Inf)) = 0", __real__ result, 0);
  check ("imag(catanh(+Inf - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (minus_infty, plus_infty));
  check ("real(catanh(-Inf + i Inf)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-Inf + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (minus_infty, minus_infty));
  check ("real(catanh(-Inf - i Inf)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-Inf - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);

  result = FUNC(catanh) (BUILD_COMPLEX (-10.0, plus_infty));
  check ("real(catanh(-10.0 + i Inf)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-10.0 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (-10.0, minus_infty));
  check ("real(catanh(-10.0 - i Inf)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-10.0 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (minus_zero, plus_infty));
  check ("real(catanh(-0 + i Inf)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-0 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (minus_zero, minus_infty));
  check ("real(catanh(-0 - i Inf)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-0 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (0, plus_infty));
  check ("real(catanh(0 + i Inf)) = 0", __real__ result, 0);
  check ("imag(catanh(0 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (0, minus_infty));
  check ("real(catanh(0 - i Inf)) = 0", __real__ result, 0);
  check ("imag(catanh(0 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (0.1, plus_infty));
  check ("real(catanh(0.1 + i Inf)) = 0", __real__ result, 0);
  check ("imag(catanh(0.1 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (0.1, minus_infty));
  check ("real(catanh(0.1 - i Inf)) = 0", __real__ result, 0);
  check ("imag(catanh(0.1 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);

  result = FUNC(catanh) (BUILD_COMPLEX (minus_infty, 0));
  check ("real(catanh(-Inf + i0)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-Inf + i0)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (minus_infty, minus_zero));
  check ("real(catanh(-Inf - i0)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-Inf - i0)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (minus_infty, 100));
  check ("real(catanh(-Inf + i100)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-Inf + i100)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (minus_infty, -100));
  check ("real(catanh(-Inf - i100)) = -0", __real__ result, minus_zero);
  check ("imag(catanh(-Inf - i100)) = -pi/2", __imag__ result, -M_PI_2l);

  result = FUNC(catanh) (BUILD_COMPLEX (plus_infty, 0));
  check ("real(catanh(+Inf + i0)) = 0", __real__ result, 0);
  check ("imag(catanh(+Inf + i0)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (plus_infty, minus_zero));
  check ("real(catanh(+Inf - i0)) = 0", __real__ result, 0);
  check ("imag(catanh(+Inf - i0)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (plus_infty, 0.5));
  check ("real(catanh(+Inf + i0.5)) = 0", __real__ result, 0);
  check ("imag(catanh(+Inf + i0.5)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (plus_infty, -0.5));
  check ("real(catanh(+Inf - i0.5)) = 0", __real__ result, 0);
  check ("imag(catanh(+Inf - i0.5)) = -pi/2", __imag__ result, -M_PI_2l);

  result = FUNC(catanh) (BUILD_COMPLEX (0, nan_value));
  check ("real(catanh(0 + i NaN)) = 0", __real__ result, 0);
  check_isnan ("imag(catanh(0 + i NaN)) = NaN", __imag__ result);
  result = FUNC(catanh) (BUILD_COMPLEX (minus_zero, nan_value));
  check ("real(catanh(-0 + i NaN)) = -0", __real__ result, minus_zero);
  check_isnan ("imag(catanh(-0 + i NaN)) = NaN", __imag__ result);

  result = FUNC(catanh) (BUILD_COMPLEX (plus_infty, nan_value));
  check ("real(catanh(+Inf + i NaN)) = 0", __real__ result, 0);
  check_isnan ("imag(catanh(+Inf + i NaN)) = NaN", __imag__ result);
  result = FUNC(catanh) (BUILD_COMPLEX (minus_infty, nan_value));
  check ("real(catanh(-Inf + i NaN)) = -0", __real__ result, minus_zero);
  check_isnan ("imag(catanh(-Inf + i NaN)) = NaN", __imag__ result);

  result = FUNC(catanh) (BUILD_COMPLEX (nan_value, 0));
  check_isnan ("real(catanh(NaN + i0)) = NaN", __real__ result);
  check_isnan ("imag(catanh(NaN + i0)) = NaN", __imag__ result);
  result = FUNC(catanh) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan ("real(catanh(NaN - i0)) = NaN", __real__ result);
  check_isnan ("imag(catanh(NaN - i0)) = NaN", __imag__ result);

  result = FUNC(catanh) (BUILD_COMPLEX (nan_value, plus_infty));
  check ("real(catanh(NaN + i Inf)) = +-0", FUNC(fabs) (__real__ result), 0);
  check ("imag(catanh(NaN + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(catanh) (BUILD_COMPLEX (nan_value, minus_infty));
  check ("real(catanh(NaN - i Inf)) = +-0", FUNC(fabs) (__real__ result), 0);
  check ("imag(catanh(NaN - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);

  result = FUNC(catanh) (BUILD_COMPLEX (10.5, nan_value));
  check_isnan_maybe_exc ("real(catanh(10.5 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(catanh(10.5 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(catanh) (BUILD_COMPLEX (-10.5, nan_value));
  check_isnan_maybe_exc ("real(catanh(-10.5 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(catanh(-10.5 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(catanh) (BUILD_COMPLEX (nan_value, 0.75));
  check_isnan_maybe_exc ("real(catanh(NaN + i0.75)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(catanh(NaN + i0.75)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(catanh) (BUILD_COMPLEX (nan_value, -0.75));
  check_isnan_maybe_exc ("real(catanh(NaN - i0.75)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(catanh(NaN - i0.75)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(catanh) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(catanh(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(catanh(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(catanh) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(catanh(0.7 + i 1.2)) == 0.26007...", __real__ result,
	     0.2600749516525135959L, CHOOSE (2e-18, 6e-17, 3e-8));
  check_eps ("imag(catanh(0.7 + i 1.2)) == 0.97024...", __imag__ result,
	     0.9702403077950989849L, CHOOSE (3e-17, 2e-16, 4e-7));

  result = FUNC(catanh) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(catanh(-2 - i 3)) == -0.14694...", __real__ result,
	     -0.1469466662255297520L, CHOOSE (9e-20L, 2e-16, 2e-8));
  check_eps ("imag(catanh(-2 - i 3)) == -1.33897...", __imag__ result,
	     -1.3389725222944935611L, CHOOSE (7e-19L, 0, 5e-7));
}


static void
ctan_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(ctan) (BUILD_COMPLEX (0, 0));
  check ("real(ctan(0 + i0)) = 0", __real__ result, 0);
  check ("imag(ctan(0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(ctan) (BUILD_COMPLEX (0, minus_zero));
  check ("real(ctan(0 - i0)) = 0", __real__ result, 0);
  check ("imag(ctan(0 - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(ctan) (BUILD_COMPLEX (minus_zero, 0));
  check ("real(ctan(-0 + i0)) = -0", __real__ result, minus_zero);
  check ("imag(ctan(-0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(ctan) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(ctan(-0 - i0)) = -0", __real__ result, minus_zero);
  check ("imag(ctan(-0 - i0)) = -0", __imag__ result, minus_zero);


  result = FUNC(ctan) (BUILD_COMPLEX (0, plus_infty));
  check ("real(ctan(0 + i Inf)) = 0", __real__ result, 0);
  check ("imag(ctan(0 + i Inf)) = 1", __imag__ result, 1);
  result = FUNC(ctan) (BUILD_COMPLEX (1, plus_infty));
  check ("real(ctan(1 + i Inf)) = 0", __real__ result, 0);
  check ("imag(ctan(1 + i Inf)) = 1", __imag__ result, 1);
  result = FUNC(ctan) (BUILD_COMPLEX (minus_zero, plus_infty));
  check ("real(ctan(-0 + i Inf)) = -0", __real__ result, minus_zero);
  check ("imag(ctan(-0 + i Inf)) = 1", __imag__ result, 1);
  result = FUNC(ctan) (BUILD_COMPLEX (-1, plus_infty));
  check ("real(ctan(-1 + i Inf)) = -0", __real__ result, minus_zero);
  check ("imag(ctan(-1 + i Inf)) = 1", __imag__ result, 1);

  result = FUNC(ctan) (BUILD_COMPLEX (0, minus_infty));
  check ("real(ctan(0 - i Inf)) = 0", __real__ result, 0);
  check ("imag(ctan(0 - i Inf)) = -1", __imag__ result, -1);
  result = FUNC(ctan) (BUILD_COMPLEX (1, minus_infty));
  check ("real(ctan(1 - i Inf)) = 0", __real__ result, 0);
  check ("imag(ctan(1 - i Inf)) = -1", __imag__ result, -1);
  result = FUNC(ctan) (BUILD_COMPLEX (minus_zero, minus_infty));
  check ("real(ctan(-0 - i Inf)) = -0", __real__ result, minus_zero);
  check ("imag(ctan(-0 - i Inf)) = -1", __imag__ result, -1);
  result = FUNC(ctan) (BUILD_COMPLEX (-1, minus_infty));
  check ("real(ctan(-1 - i Inf)) = -0", __real__ result, minus_zero);
  check ("imag(ctan(-1 - i Inf)) = -1", __imag__ result, -1);

  result = FUNC(ctan) (BUILD_COMPLEX (plus_infty, 0));
  check_isnan_exc ("real(ctan(Inf + i 0)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(Inf + i 0)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (plus_infty, 2));
  check_isnan_exc ("real(ctan(Inf + i 2)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(Inf + i 2)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (minus_infty, 0));
  check_isnan_exc ("real(ctan(-Inf + i 0)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(-Inf + i 0)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (minus_infty, 2));
  check_isnan_exc ("real(ctan(- Inf + i 2)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(- Inf + i 2)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (plus_infty, minus_zero));
  check_isnan_exc ("real(ctan(Inf - i 0)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(Inf - i 0)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (plus_infty, -2));
  check_isnan_exc ("real(ctan(Inf - i 2)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(Inf - i 2)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (minus_infty, minus_zero));
  check_isnan_exc ("real(ctan(-Inf - i 0)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(-Inf - i 0)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (minus_infty, -2));
  check_isnan_exc ("real(ctan(-Inf - i 2)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(-Inf - i 2)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(ctan) (BUILD_COMPLEX (nan_value, plus_infty));
  check ("real(ctan(NaN + i Inf)) = +-0", FUNC(fabs) (__real__ result), 0);
  check ("imag(ctan(NaN + i Inf)) = 1", __imag__ result, 1);
  result = FUNC(ctan) (BUILD_COMPLEX (nan_value, minus_infty));
  check ("real(ctan(NaN - i Inf)) = +-0", FUNC(fabs) (__real__ result), 0);
  check ("imag(ctan(NaN - i Inf)) = -1", __imag__ result, -1);

  result = FUNC(ctan) (BUILD_COMPLEX (0, nan_value));
  check ("real(ctan(0 + i NaN)) = 0", __real__ result, 0);
  check_isnan ("imag(ctan(0 + i NaN)) = NaN", __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (minus_zero, nan_value));
  check ("real(ctan(-0 + i NaN)) = -0", __real__ result, minus_zero);
  check_isnan ("imag(ctan(-0 + i NaN)) = NaN", __imag__ result);

  result = FUNC(ctan) (BUILD_COMPLEX (0.5, nan_value));
  check_isnan_maybe_exc ("real(ctan(0.5 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(0.5 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (-4.5, nan_value));
  check_isnan_maybe_exc ("real(ctan(-4.5 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(-4.5 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(ctan) (BUILD_COMPLEX (nan_value, 0));
  check_isnan_maybe_exc ("real(ctan(NaN + i 0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(NaN + i 0)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (nan_value, 5));
  check_isnan_maybe_exc ("real(ctan(NaN + i 5)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(NaN + i 5)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan_maybe_exc ("real(ctan(NaN - i 0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(NaN - i 0)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ctan) (BUILD_COMPLEX (nan_value, -0.25));
  check_isnan_maybe_exc ("real(ctan(NaN -i 0.25)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctan(NaN -i 0.25)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(ctan) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(ctan(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(ctan(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(ctan) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(ctan(0.7 + i 1.2)) == 0.17207...", __real__ result,
	     0.1720734197630349001L, CHOOSE (1e-17L, 5.6e-17, 2e-8));
  check_eps ("imag(ctan(0.7 + i 1.2)) == 0.95448...", __imag__ result,
	     0.9544807059989405538L, CHOOSE (2e-17L, 2.3e-16, 6e-8));

  result = FUNC(ctan) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(ctan(-2 - i 3)) == -0.00376...", __real__ result,
	     0.0037640256415042482L, CHOOSE (1e-19L, 5e-19, 0));
  check_eps ("imag(ctan(-2 - i 3)) == -1.00323...", __imag__ result,
	     -1.0032386273536098014L, CHOOSE (2e-19L, 2.3e-16, 2e-7));
}


static void
ctanh_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(ctanh) (BUILD_COMPLEX (0, 0));
  check ("real(ctanh(0 + i0)) = 0", __real__ result, 0);
  check ("imag(ctanh(0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(ctanh) (BUILD_COMPLEX (0, minus_zero));
  check ("real(ctanh(0 - i0)) = 0", __real__ result, 0);
  check ("imag(ctanh(0 - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(ctanh) (BUILD_COMPLEX (minus_zero, 0));
  check ("real(ctanh(-0 + i0)) = -0", __real__ result, minus_zero);
  check ("imag(ctanh(-0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(ctanh) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(ctanh(-0 - i0)) = -0", __real__ result, minus_zero);
  check ("imag(ctanh(-0 - i0)) = -0", __imag__ result, minus_zero);

  result = FUNC(ctanh) (BUILD_COMPLEX (plus_infty, 0));
  check ("real(ctanh(+Inf + i0)) = 1", __real__ result, 1);
  check ("imag(ctanh(+Inf + i0)) = 0", __imag__ result, 0);
  result = FUNC(ctanh) (BUILD_COMPLEX (plus_infty, 1));
  check ("real(ctanh(+Inf + i1)) = 1", __real__ result, 1);
  check ("imag(ctanh(+Inf + i1)) = 0", __imag__ result, 0);
  result = FUNC(ctanh) (BUILD_COMPLEX (plus_infty, minus_zero));
  check ("real(ctanh(+Inf - i0)) = 1", __real__ result, 1);
  check ("imag(ctanh(+Inf - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(ctanh) (BUILD_COMPLEX (plus_infty, -1));
  check ("real(ctanh(+Inf - i1)) = 1", __real__ result, 1);
  check ("imag(ctanh(+Inf - i1)) = -0", __imag__ result, minus_zero);
  result = FUNC(ctanh) (BUILD_COMPLEX (minus_infty, 0));
  check ("real(ctanh(-Inf + i0)) = -1", __real__ result, -1);
  check ("imag(ctanh(-Inf + i0)) = 0", __imag__ result, 0);
  result = FUNC(ctanh) (BUILD_COMPLEX (minus_infty, 1));
  check ("real(ctanh(-Inf + i1)) = -1", __real__ result, -1);
  check ("imag(ctanh(-Inf + i1)) = 0", __imag__ result, 0);
  result = FUNC(ctanh) (BUILD_COMPLEX (minus_infty, minus_zero));
  check ("real(ctanh(-Inf - i0)) = -1", __real__ result, -1);
  check ("imag(ctanh(-Inf - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(ctanh) (BUILD_COMPLEX (minus_infty, -1));
  check ("real(ctanh(-Inf - i1)) = -1", __real__ result, -1);
  check ("imag(ctanh(-Inf - i1)) = -0", __imag__ result, minus_zero);

  result = FUNC(ctanh) (BUILD_COMPLEX (0, plus_infty));
  check_isnan_exc ("real(ctanh(0 + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(0 + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctanh) (BUILD_COMPLEX (2, plus_infty));
  check_isnan_exc ("real(ctanh(2 + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(2 + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctanh) (BUILD_COMPLEX (0, minus_infty));
  check_isnan_exc ("real(ctanh(0 - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(0 - i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctanh) (BUILD_COMPLEX (2, minus_infty));
  check_isnan_exc ("real(ctanh(2 - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(2 - i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctanh) (BUILD_COMPLEX (minus_zero, plus_infty));
  check_isnan_exc ("real(ctanh(-0 + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(-0 + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctanh) (BUILD_COMPLEX (-2, plus_infty));
  check_isnan_exc ("real(ctanh(-2 + i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(-2 + i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctanh) (BUILD_COMPLEX (minus_zero, minus_infty));
  check_isnan_exc ("real(ctanh(-0 - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(-0 - i Inf)) = NaN plus invalid exception",
	       __imag__ result);
  result = FUNC(ctanh) (BUILD_COMPLEX (-2, minus_infty));
  check_isnan_exc ("real(ctanh(-2 - i Inf)) = NaN plus invalid exception",
		   __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(-2 - i Inf)) = NaN plus invalid exception",
	       __imag__ result);

  result = FUNC(ctanh) (BUILD_COMPLEX (plus_infty, nan_value));
  check ("real(ctanh(+Inf + i NaN)) = 1", __real__ result, 1);
  check ("imag(ctanh(+Inf + i NaN)) = +-0", FUNC(fabs) (__imag__ result), 0);
  result = FUNC(ctanh) (BUILD_COMPLEX (minus_infty, nan_value));
  check ("real(ctanh(-Inf + i NaN)) = -1", __real__ result, -1);
  check ("imag(ctanh(-Inf + i NaN)) = +-0", FUNC(fabs) (__imag__ result), 0);

  result = FUNC(ctanh) (BUILD_COMPLEX (nan_value, 0));
  check_isnan ("real(ctanh(NaN + i0)) = NaN", __real__ result);
  check ("imag(ctanh(NaN + i0)) = 0", __imag__ result, 0);
  result = FUNC(ctanh) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan ("real(ctanh(NaN - i0)) = NaN", __real__ result);
  check ("imag(ctanh(NaN - i0)) = -0", __imag__ result, minus_zero);

  result = FUNC(ctanh) (BUILD_COMPLEX (nan_value, 0.5));
  check_isnan_maybe_exc ("real(ctanh(NaN + i0.5)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(NaN + i0.5)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ctanh) (BUILD_COMPLEX (nan_value, -4.5));
  check_isnan_maybe_exc ("real(ctanh(NaN - i4.5)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(NaN - i4.5)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(ctanh) (BUILD_COMPLEX (0, nan_value));
  check_isnan_maybe_exc ("real(ctanh(0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ctanh) (BUILD_COMPLEX (5, nan_value));
  check_isnan_maybe_exc ("real(ctanh(5 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(5 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ctanh) (BUILD_COMPLEX (minus_zero, nan_value));
  check_isnan_maybe_exc ("real(ctanh(-0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(-0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(ctanh) (BUILD_COMPLEX (-0.25, nan_value));
  check_isnan_maybe_exc ("real(ctanh(-0.25 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(ctanh(-0.25 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(ctanh) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(ctanh(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(ctanh(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(ctanh) (BUILD_COMPLEX (0, M_PI_4l));
  check ("real(ctanh (0 + i pi/4)) == 0", __real__ result, 0);
  check_eps ("imag(ctanh (0 + i pi/4)) == 1", __imag__ result, 1,
	     CHOOSE (0, 2e-16, 2e-7));

  result = FUNC(ctanh) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(ctanh(0.7 + i 1.2)) == 1.34721...", __real__ result,
	     1.3472197399061191630L, CHOOSE (4e-17L, 6.7e-16, 2e-7));
  check_eps ("imag(ctanh(0.7 + i 1.2)) == -0.47786...", __imag__ result,
	     0.4778641038326365540L, CHOOSE (9e-17L, 2.8e-16, 9e-8));

  result = FUNC(ctanh) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(ctanh(-2 - i 3)) == -0.96538...", __real__ result,
	     -0.9653858790221331242L, CHOOSE (2e-19L, 2e-16, 2e-7));
  check_eps ("imag(ctanh(-2 - i 3)) == 0.00988...", __imag__ result,
	     0.0098843750383224937L, CHOOSE (7e-20L, 2e-16, 1e-9));
}


static void
clog_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(clog) (BUILD_COMPLEX (minus_zero, 0));
  check_isinfn_exc ("real(clog(-0 + i0)) = -Inf plus divide-by-zero exception",
		    __real__ result, DIVIDE_BY_ZERO_EXCEPTION);
  check ("imag(clog(-0 + i0)) = pi plus divide-by-zero exception",
	 __imag__ result, M_PIl);
  result = FUNC(clog) (BUILD_COMPLEX (minus_zero, minus_zero));
  check_isinfn_exc ("real(clog(-0 - i0)) = -Inf plus divide-by-zero exception",
		    __real__ result, DIVIDE_BY_ZERO_EXCEPTION);
  check ("imag(clog(-0 - i0)) = -pi plus divide-by-zero exception",
	 __imag__ result, -M_PIl);

  result = FUNC(clog) (BUILD_COMPLEX (0, 0));
  check_isinfn_exc ("real(clog(0 + i0)) = -Inf plus divide-by-zero exception",
		    __real__ result, DIVIDE_BY_ZERO_EXCEPTION);
  check ("imag(clog(0 + i0)) = 0 plus divide-by-zero exception",
	 __imag__ result, 0);
  result = FUNC(clog) (BUILD_COMPLEX (0, minus_zero));
  check_isinfn_exc ("real(clog(0 - i0)) = -Inf plus divide-by-zero exception",
		    __real__ result, DIVIDE_BY_ZERO_EXCEPTION);
  check ("imag(clog(0 - i0)) = -0 plus divide-by-zero exception",
	 __imag__ result, minus_zero);

  result = FUNC(clog) (BUILD_COMPLEX (minus_infty, plus_infty));
  check_isinfp ("real(clog(-Inf + i Inf)) = +Inf", __real__ result);
  check ("imag(clog(-Inf + i Inf)) = 3*pi/4", __imag__ result,
	 M_PIl - M_PI_4l);
  result = FUNC(clog) (BUILD_COMPLEX (minus_infty, minus_infty));
  check_isinfp ("real(clog(-Inf - i Inf)) = +Inf", __real__ result);
  check ("imag(clog(-Inf - i Inf)) = -3*pi/4", __imag__ result,
	 M_PI_4l - M_PIl);

  result = FUNC(clog) (BUILD_COMPLEX (plus_infty, plus_infty));
  check_isinfp ("real(clog(+Inf + i Inf)) = +Inf", __real__ result);
  check ("imag(clog(+Inf + i Inf)) = pi/4", __imag__ result, M_PI_4l);
  result = FUNC(clog) (BUILD_COMPLEX (plus_infty, minus_infty));
  check_isinfp ("real(clog(+Inf - i Inf)) = +Inf", __real__ result);
  check ("imag(clog(+Inf - i Inf)) = -pi/4", __imag__ result, -M_PI_4l);

  result = FUNC(clog) (BUILD_COMPLEX (0, plus_infty));
  check_isinfp ("real(clog(0 + i Inf)) = +Inf", __real__ result);
  check ("imag(clog(0 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(clog) (BUILD_COMPLEX (3, plus_infty));
  check_isinfp ("real(clog(3 + i Inf)) = +Inf", __real__ result);
  check ("imag(clog(3 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(clog) (BUILD_COMPLEX (minus_zero, plus_infty));
  check_isinfp ("real(clog(-0 + i Inf)) = +Inf", __real__ result);
  check ("imag(clog(-0 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(clog) (BUILD_COMPLEX (-3, plus_infty));
  check_isinfp ("real(clog(-3 + i Inf)) = +Inf", __real__ result);
  check ("imag(clog(-3 + i Inf)) = pi/2", __imag__ result, M_PI_2l);
  result = FUNC(clog) (BUILD_COMPLEX (0, minus_infty));
  check_isinfp ("real(clog(0 - i Inf)) = +Inf", __real__ result);
  check ("imag(clog(0 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(clog) (BUILD_COMPLEX (3, minus_infty));
  check_isinfp ("real(clog(3 - i Inf)) = +Inf", __real__ result);
  check ("imag(clog(3 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(clog) (BUILD_COMPLEX (minus_zero, minus_infty));
  check_isinfp ("real(clog(-0 - i Inf)) = +Inf", __real__ result);
  check ("imag(clog(-0 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);
  result = FUNC(clog) (BUILD_COMPLEX (-3, minus_infty));
  check_isinfp ("real(clog(-3 - i Inf)) = +Inf", __real__ result);
  check ("imag(clog(-3 - i Inf)) = -pi/2", __imag__ result, -M_PI_2l);

  result = FUNC(clog) (BUILD_COMPLEX (minus_infty, 0));
  check_isinfp ("real(clog(-Inf + i0)) = +Inf", __real__ result);
  check ("imag(clog(-Inf + i0)) = pi", __imag__ result, M_PIl);
  result = FUNC(clog) (BUILD_COMPLEX (minus_infty, 1));
  check_isinfp ("real(clog(-Inf + i1)) = +Inf", __real__ result);
  check ("imag(clog(-Inf + i1)) = pi", __imag__ result, M_PIl);
  result = FUNC(clog) (BUILD_COMPLEX (minus_infty, minus_zero));
  check_isinfp ("real(clog(-Inf - i0)) = +Inf", __real__ result);
  check ("imag(clog(-Inf - i0)) = -pi", __imag__ result, -M_PIl);
  result = FUNC(clog) (BUILD_COMPLEX (minus_infty, -1));
  check_isinfp ("real(clog(-Inf - i1)) = +Inf", __real__ result);
  check ("imag(clog(-Inf - i1)) = -pi", __imag__ result, -M_PIl);

  result = FUNC(clog) (BUILD_COMPLEX (plus_infty, 0));
  check_isinfp ("real(clog(+Inf + i0)) = +Inf", __real__ result);
  check ("imag(clog(+Inf + i0)) = 0", __imag__ result, 0);
  result = FUNC(clog) (BUILD_COMPLEX (plus_infty, 1));
  check_isinfp ("real(clog(+Inf + i1)) = +Inf", __real__ result);
  check ("imag(clog(+Inf + i1)) = 0", __imag__ result, 0);
  result = FUNC(clog) (BUILD_COMPLEX (plus_infty, minus_zero));
  check_isinfp ("real(clog(+Inf - i0)) = +Inf", __real__ result);
  check ("imag(clog(+Inf - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(clog) (BUILD_COMPLEX (plus_infty, -1));
  check_isinfp ("real(clog(+Inf - i1)) = +Inf", __real__ result);
  check ("imag(clog(+Inf - i1)) = -0", __imag__ result, minus_zero);

  result = FUNC(clog) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isinfp ("real(clog(+Inf + i NaN)) = +Inf", __real__ result);
  check_isnan ("imag(clog(+Inf + i NaN)) = NaN", __imag__ result);
  result = FUNC(clog) (BUILD_COMPLEX (minus_infty, nan_value));
  check_isinfp ("real(clog(-Inf + i NaN)) = +Inf", __real__ result);
  check_isnan ("imag(clog(-Inf + i NaN)) = NaN", __imag__ result);

  result = FUNC(clog) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isinfp ("real(clog(NaN + i Inf)) = +Inf", __real__ result);
  check_isnan ("imag(clog(NaN + i Inf)) = NaN", __imag__ result);
  result = FUNC(clog) (BUILD_COMPLEX (nan_value, minus_infty));
  check_isinfp ("real(clog(NaN - i Inf)) = +Inf", __real__ result);
  check_isnan ("imag(clog(NaN - i Inf)) = NaN", __imag__ result);

  result = FUNC(clog) (BUILD_COMPLEX (0, nan_value));
  check_isnan_maybe_exc ("real(clog(0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog(0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog) (BUILD_COMPLEX (3, nan_value));
  check_isnan_maybe_exc ("real(clog(3 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog(3 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog) (BUILD_COMPLEX (minus_zero, nan_value));
  check_isnan_maybe_exc ("real(clog(-0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog(-0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog) (BUILD_COMPLEX (-3, nan_value));
  check_isnan_maybe_exc ("real(clog(-3 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog(-3 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(clog) (BUILD_COMPLEX (nan_value, 0));
  check_isnan_maybe_exc ("real(clog(NaN + i0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog(NaN + i0)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog) (BUILD_COMPLEX (nan_value, 5));
  check_isnan_maybe_exc ("real(clog(NaN + i5)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog(NaN + i5)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan_maybe_exc ("real(clog(NaN - i0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog(NaN - i0)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog) (BUILD_COMPLEX (nan_value, -5));
  check_isnan_maybe_exc ("real(clog(NaN - i5)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog(NaN - i5)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(clog) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(clog(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(clog(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(clog) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(clog(0.7 + i 1.2)) == 0.32876...", __real__ result,
	     0.3287600014583970919L, CHOOSE (5e-17L, 6e-17, 3e-8));
  check_eps ("imag(clog(0.7 + i 1.2)) == 1.04272...", __imag__ result,
	     1.0427218783685369524L, CHOOSE (2e-17L, 2.5e-16, 1.2e-7));

  result = FUNC(clog) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(clog(-2 - i 3)) == 1.28247...", __real__ result,
	     1.2824746787307683680L, CHOOSE (3e-19L, 0, 0));
  check_eps ("imag(clog(-2 - i 3)) == -2.15879...", __imag__ result,
	     -2.1587989303424641704L, CHOOSE (2e-18L, 5e-16, 8e-7));
}


static void
clog10_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(clog10) (BUILD_COMPLEX (minus_zero, 0));
  check_isinfn_exc ("real(clog10(-0 + i0)) = -Inf plus divide-by-zero exception",
		    __real__ result, DIVIDE_BY_ZERO_EXCEPTION);
  check ("imag(clog10(-0 + i0)) = pi plus divide-by-zero exception",
	 __imag__ result, M_PIl);
  result = FUNC(clog10) (BUILD_COMPLEX (minus_zero, minus_zero));
  check_isinfn_exc ("real(clog10(-0 - i0)) = -Inf plus divide-by-zero exception",
		    __real__ result, DIVIDE_BY_ZERO_EXCEPTION);
  check ("imag(clog10(-0 - i0)) = -pi plus divide-by-zero exception",
	 __imag__ result, -M_PIl);

  result = FUNC(clog10) (BUILD_COMPLEX (0, 0));
  check_isinfn_exc ("real(clog10(0 + i0)) = -Inf plus divide-by-zero exception",
		    __real__ result, DIVIDE_BY_ZERO_EXCEPTION);
  check ("imag(clog10(0 + i0)) = 0 plus divide-by-zero exception",
	 __imag__ result, 0);
  result = FUNC(clog10) (BUILD_COMPLEX (0, minus_zero));
  check_isinfn_exc ("real(clog10(0 - i0)) = -Inf plus divide-by-zero exception",
		    __real__ result, DIVIDE_BY_ZERO_EXCEPTION);
  check ("imag(clog10(0 - i0)) = -0 plus divide-by-zero exception",
	 __imag__ result, minus_zero);

  result = FUNC(clog10) (BUILD_COMPLEX (minus_infty, plus_infty));
  check_isinfp ("real(clog10(-Inf + i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(-Inf + i Inf)) = 3*pi/4*M_LOG10E", __imag__ result,
	     (M_PIl - M_PI_4l) * M_LOG10El, CHOOSE (0, 3e-16, 0));
  result = FUNC(clog10) (BUILD_COMPLEX (minus_infty, minus_infty));
  check_isinfp ("real(clog10(-Inf - i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(-Inf - i Inf)) = -3*pi/4*M_LOG10E", __imag__ result,
	     (M_PI_4l - M_PIl) * M_LOG10El, CHOOSE (0, 3e-16, 0));

  result = FUNC(clog10) (BUILD_COMPLEX (plus_infty, plus_infty));
  check_isinfp ("real(clog10(+Inf + i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(+Inf + i Inf)) = pi/4*M_LOG10E", __imag__ result,
	     M_PI_4l * M_LOG10El, CHOOSE (0, 6e-17, 3e-8));
  result = FUNC(clog10) (BUILD_COMPLEX (plus_infty, minus_infty));
  check_isinfp ("real(clog10(+Inf - i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(+Inf - i Inf)) = -pi/4*M_LOG10E", __imag__ result,
	     -M_PI_4l * M_LOG10El, CHOOSE (0, 6e-17, 3e-8));

  result = FUNC(clog10) (BUILD_COMPLEX (0, plus_infty));
  check_isinfp ("real(clog10(0 + i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(0 + i Inf)) = pi/2*M_LOG10E", __imag__ result,
	     M_PI_2l * M_LOG10El, CHOOSE (0, 2e-16, 6e-8));
  result = FUNC(clog10) (BUILD_COMPLEX (3, plus_infty));
  check_isinfp ("real(clog10(3 + i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(3 + i Inf)) = pi/2*M_LOG10E", __imag__ result,
	     M_PI_2l * M_LOG10El, CHOOSE (0, 2e-16, 6e-8));
  result = FUNC(clog10) (BUILD_COMPLEX (minus_zero, plus_infty));
  check_isinfp ("real(clog10(-0 + i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(-0 + i Inf)) = pi/2*M_LOG10E", __imag__ result,
	     M_PI_2l * M_LOG10El, CHOOSE (0, 2e-16, 6e-8));
  result = FUNC(clog10) (BUILD_COMPLEX (-3, plus_infty));
  check_isinfp ("real(clog10(-3 + i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(-3 + i Inf)) = pi/2*M_LOG10E", __imag__ result,
	     M_PI_2l * M_LOG10El, CHOOSE (0, 2e-16, 6e-8));
  result = FUNC(clog10) (BUILD_COMPLEX (0, minus_infty));
  check_isinfp ("real(clog10(0 - i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(0 - i Inf)) = -pi/2*M_LOG10E", __imag__ result,
	     -M_PI_2l * M_LOG10El, CHOOSE (0, 2e-16, 6e-8));
  result = FUNC(clog10) (BUILD_COMPLEX (3, minus_infty));
  check_isinfp ("real(clog10(3 - i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(3 - i Inf)) = -pi/2*M_LOG10E", __imag__ result,
	     -M_PI_2l * M_LOG10El, CHOOSE (0, 2e-16, 6e-8));
  result = FUNC(clog10) (BUILD_COMPLEX (minus_zero, minus_infty));
  check_isinfp ("real(clog10(-0 - i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(-0 - i Inf)) = -pi/2*M_LOG10E", __imag__ result,
	     -M_PI_2l * M_LOG10El, CHOOSE (0, 2e-16, 6e-8));
  result = FUNC(clog10) (BUILD_COMPLEX (-3, minus_infty));
  check_isinfp ("real(clog10(-3 - i Inf)) = +Inf", __real__ result);
  check_eps ("imag(clog10(-3 - i Inf)) = -pi/2*M_LOG10E", __imag__ result,
	     -M_PI_2l * M_LOG10El, CHOOSE (0, 2e-16, 6e-8));

  result = FUNC(clog10) (BUILD_COMPLEX (minus_infty, 0));
  check_isinfp ("real(clog10(-Inf + i0)) = +Inf", __real__ result);
  check_eps ("imag(clog10(-Inf + i0)) = pi*M_LOG10E", __imag__ result,
	     M_PIl * M_LOG10El, CHOOSE (0, 3e-16, 2e-7));
  result = FUNC(clog10) (BUILD_COMPLEX (minus_infty, 1));
  check_isinfp ("real(clog10(-Inf + i1)) = +Inf", __real__ result);
  check_eps ("imag(clog10(-Inf + i1)) = pi*M_LOG10E", __imag__ result,
	     M_PIl * M_LOG10El, CHOOSE (0, 3e-16, 2e-7));
  result = FUNC(clog10) (BUILD_COMPLEX (minus_infty, minus_zero));
  check_isinfp ("real(clog10(-Inf - i0)) = +Inf", __real__ result);
  check_eps ("imag(clog10(-Inf - i0)) = -pi*M_LOG10E", __imag__ result,
	     -M_PIl * M_LOG10El, CHOOSE (0, 3e-16, 2e-7));
  result = FUNC(clog10) (BUILD_COMPLEX (minus_infty, -1));
  check_isinfp ("real(clog10(-Inf - i1)) = +Inf", __real__ result);
  check_eps ("imag(clog10(-Inf - i1)) = -pi*M_LOG10E", __imag__ result,
	     -M_PIl * M_LOG10El, CHOOSE (0, 3e-16, 2e-7));

  result = FUNC(clog10) (BUILD_COMPLEX (plus_infty, 0));
  check_isinfp ("real(clog10(+Inf + i0)) = +Inf", __real__ result);
  check ("imag(clog10(+Inf + i0)) = 0", __imag__ result, 0);
  result = FUNC(clog10) (BUILD_COMPLEX (plus_infty, 1));
  check_isinfp ("real(clog10(+Inf + i1)) = +Inf", __real__ result);
  check ("imag(clog10(+Inf + i1)) = 0", __imag__ result, 0);
  result = FUNC(clog10) (BUILD_COMPLEX (plus_infty, minus_zero));
  check_isinfp ("real(clog10(+Inf - i0)) = +Inf", __real__ result);
  check ("imag(clog10(+Inf - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(clog10) (BUILD_COMPLEX (plus_infty, -1));
  check_isinfp ("real(clog10(+Inf - i1)) = +Inf", __real__ result);
  check ("imag(clog10(+Inf - i1)) = -0", __imag__ result, minus_zero);

  result = FUNC(clog10) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isinfp ("real(clog10(+Inf + i NaN)) = +Inf", __real__ result);
  check_isnan ("imag(clog10(+Inf + i NaN)) = NaN", __imag__ result);
  result = FUNC(clog10) (BUILD_COMPLEX (minus_infty, nan_value));
  check_isinfp ("real(clog10(-Inf + i NaN)) = +Inf", __real__ result);
  check_isnan ("imag(clog10(-Inf + i NaN)) = NaN", __imag__ result);

  result = FUNC(clog10) (BUILD_COMPLEX (nan_value, plus_infty));
  check_isinfp ("real(clog10(NaN + i Inf)) = +Inf", __real__ result);
  check_isnan ("imag(clog10(NaN + i Inf)) = NaN", __imag__ result);
  result = FUNC(clog10) (BUILD_COMPLEX (nan_value, minus_infty));
  check_isinfp ("real(clog10(NaN - i Inf)) = +Inf", __real__ result);
  check_isnan ("imag(clog10(NaN - i Inf)) = NaN", __imag__ result);

  result = FUNC(clog10) (BUILD_COMPLEX (0, nan_value));
  check_isnan_maybe_exc ("real(clog10(0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog10(0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog10) (BUILD_COMPLEX (3, nan_value));
  check_isnan_maybe_exc ("real(clog10(3 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog10(3 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog10) (BUILD_COMPLEX (minus_zero, nan_value));
  check_isnan_maybe_exc ("real(clog10(-0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog10(-0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog10) (BUILD_COMPLEX (-3, nan_value));
  check_isnan_maybe_exc ("real(clog10(-3 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog10(-3 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(clog10) (BUILD_COMPLEX (nan_value, 0));
  check_isnan_maybe_exc ("real(clog10(NaN + i0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog10(NaN + i0)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog10) (BUILD_COMPLEX (nan_value, 5));
  check_isnan_maybe_exc ("real(clog10(NaN + i5)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog10(NaN + i5)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog10) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan_maybe_exc ("real(clog10(NaN - i0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog10(NaN - i0)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(clog10) (BUILD_COMPLEX (nan_value, -5));
  check_isnan_maybe_exc ("real(clog10(NaN - i5)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(clog10(NaN - i5)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(clog10) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(clog10(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(clog10(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(clog10) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(clog10(0.7 + i 1.2)) == 0.14277...", __real__ result,
	     0.1427786545038868803L, CHOOSE (2e-17L, 6e-17, 2e-8));
  check_eps ("imag(clog10(0.7 + i 1.2)) == 0.45284...", __imag__ result,
	     0.4528483579352493248L, CHOOSE (6e-18, 6e-17, 6e-8));

  result = FUNC(clog10) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(clog10(-2 - i 3)) == 0.55697...", __real__ result,
	     0.5569716761534183846L, CHOOSE (6e-20L, 0, 0));
  check_eps ("imag(clog10(-2 - i 3)) == -0.93755...", __imag__ result,
	     -0.9375544629863747085L, CHOOSE (7e-19L, 2e-16, 3e-7));
}


static void
csqrt_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(csqrt) (BUILD_COMPLEX (0, 0));
  check ("real(csqrt(0 + i0)) = 0", __real__ result, 0);
  check ("imag(csqrt(0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(csqrt) (BUILD_COMPLEX (0, minus_zero));
  check ("real(csqrt(0 - i0)) = 0", __real__ result, 0);
  check ("imag(csqrt(0 - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(csqrt) (BUILD_COMPLEX (minus_zero, 0));
  check ("real(csqrt(-0 + i0)) = 0", __real__ result, 0);
  check ("imag(csqrt(-0 + i0)) = 0", __imag__ result, 0);
  result = FUNC(csqrt) (BUILD_COMPLEX (minus_zero, minus_zero));
  check ("real(csqrt(-0 - i0)) = 0", __real__ result, 0);
  check ("imag(csqrt(-0 - i0)) = -0", __imag__ result, minus_zero);

  result = FUNC(csqrt) (BUILD_COMPLEX (minus_infty, 0));
  check ("real(csqrt(-Inf + i0)) = 0", __real__ result, 0);
  check_isinfp ("imag(csqrt(-Inf + i0)) = +Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (minus_infty, 6));
  check ("real(csqrt(-Inf + i6)) = 0", __real__ result, 0);
  check_isinfp ("imag(csqrt(-Inf + i6)) = +Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (minus_infty, minus_zero));
  check ("real(csqrt(-Inf - i0)) = 0", __real__ result, 0);
  check_isinfn ("imag(csqrt(-Inf - i0)) = -Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (minus_infty, -6));
  check ("real(csqrt(-Inf - i6)) = 0", __real__ result, 0);
  check_isinfn ("imag(csqrt(-Inf - i6)) = -Inf", __imag__ result);

  result = FUNC(csqrt) (BUILD_COMPLEX (plus_infty, 0));
  check_isinfp ("real(csqrt(+Inf + i0)) = +Inf", __real__ result);
  check ("imag(csqrt(+Inf + i0)) = 0", __imag__ result, 0);
  result = FUNC(csqrt) (BUILD_COMPLEX (plus_infty, 6));
  check_isinfp ("real(csqrt(+Inf + i6)) = +Inf", __real__ result);
  check ("imag(csqrt(+Inf + i6)) = 0", __imag__ result, 0);
  result = FUNC(csqrt) (BUILD_COMPLEX (plus_infty, minus_zero));
  check_isinfp ("real(csqrt(+Inf - i0)) = +Inf", __real__ result);
  check ("imag(csqrt(+Inf - i0)) = -0", __imag__ result, minus_zero);
  result = FUNC(csqrt) (BUILD_COMPLEX (plus_infty, -6));
  check_isinfp ("real(csqrt(+Inf - i6)) = +Inf", __real__ result);
  check ("imag(csqrt(+Inf - i6)) = -0", __imag__ result, minus_zero);

  result = FUNC(csqrt) (BUILD_COMPLEX (0, plus_infty));
  check_isinfp ("real(csqrt(0 + i Inf)) = +Inf", __real__ result);
  check_isinfp ("imag(csqrt(0 + i Inf)) = +Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (4, plus_infty));
  check_isinfp ("real(csqrt(4 + i Inf)) = +Inf", __real__ result);
  check_isinfp ("imag(csqrt(4 + i Inf)) = +Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (plus_infty, plus_infty));
  check_isinfp ("real(csqrt(+Inf + i Inf)) = +Inf", __real__ result);
  check_isinfp ("imag(csqrt(+Inf + i Inf)) = +Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (minus_zero, plus_infty));
  check_isinfp ("real(csqrt(-0 + i Inf)) = +Inf", __real__ result);
  check_isinfp ("imag(csqrt(-0 + i Inf)) = +Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (-4, plus_infty));
  check_isinfp ("real(csqrt(-4 + i Inf)) = +Inf", __real__ result);
  check_isinfp ("imag(csqrt(-4 + i Inf)) = +Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (minus_infty, plus_infty));
  check_isinfp ("real(csqrt(-Inf + i Inf)) = +Inf", __real__ result);
  check_isinfp ("imag(csqrt(-Inf + i Inf)) = +Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (0, minus_infty));
  check_isinfp ("real(csqrt(0 - i Inf)) = +Inf", __real__ result);
  check_isinfn ("imag(csqrt(0 - i Inf)) = -Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (4, minus_infty));
  check_isinfp ("real(csqrt(4 - i Inf)) = +Inf", __real__ result);
  check_isinfn ("imag(csqrt(4 - i Inf)) = -Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (plus_infty, minus_infty));
  check_isinfp ("real(csqrt(+Inf - i Inf)) = +Inf", __real__ result);
  check_isinfn ("imag(csqrt(+Inf - i Inf)) = -Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (minus_zero, minus_infty));
  check_isinfp ("real(csqrt(-0 - i Inf)) = +Inf", __real__ result);
  check_isinfn ("imag(csqrt(-0 - i Inf)) = -Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (-4, minus_infty));
  check_isinfp ("real(csqrt(-4 - i Inf)) = +Inf", __real__ result);
  check_isinfn ("imag(csqrt(-4 - i Inf)) = -Inf", __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (minus_infty, minus_infty));
  check_isinfp ("real(csqrt(-Inf - i Inf)) = +Inf", __real__ result);
  check_isinfn ("imag(csqrt(-Inf - i Inf)) = -Inf", __imag__ result);

  result = FUNC(csqrt) (BUILD_COMPLEX (minus_infty, nan_value));
  check_isnan ("real(csqrt(-Inf + i NaN)) = NaN", __real__ result);
  check_isinfp ("imag(csqrt(-Inf + i NaN)) = +-Inf",
		FUNC(fabs) (__imag__ result));

  result = FUNC(csqrt) (BUILD_COMPLEX (plus_infty, nan_value));
  check_isinfp ("real(csqrt(+Inf + i NaN)) = +Inf", __real__ result);
  check_isnan ("imag(csqrt(+Inf + i NaN)) = NaN", __imag__ result);

  result = FUNC(csqrt) (BUILD_COMPLEX (0, nan_value));
  check_isnan_maybe_exc ("real(csqrt(0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csqrt(0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (1, nan_value));
  check_isnan_maybe_exc ("real(csqrt(1 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csqrt(1 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (minus_zero, nan_value));
  check_isnan_maybe_exc ("real(csqrt(-0 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csqrt(-0 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (-1, nan_value));
  check_isnan_maybe_exc ("real(csqrt(-1 + i NaN)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csqrt(-1 + i NaN)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(csqrt) (BUILD_COMPLEX (nan_value, 0));
  check_isnan_maybe_exc ("real(csqrt(NaN + i0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csqrt(NaN + i0)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (nan_value, 8));
  check_isnan_maybe_exc ("real(csqrt(NaN + i8)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csqrt(NaN + i8)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (nan_value, minus_zero));
  check_isnan_maybe_exc ("real(csqrt(NaN - i0)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csqrt(NaN - i0)) = NaN plus maybe invalid exception",
	       __imag__ result);
  result = FUNC(csqrt) (BUILD_COMPLEX (nan_value, -8));
  check_isnan_maybe_exc ("real(csqrt(NaN - i8)) = NaN plus maybe invalid exception",
			 __real__ result, INVALID_EXCEPTION);
  check_isnan ("imag(csqrt(NaN - i8)) = NaN plus maybe invalid exception",
	       __imag__ result);

  result = FUNC(csqrt) (BUILD_COMPLEX (nan_value, nan_value));
  check_isnan ("real(csqrt(NaN + i NaN)) = NaN", __real__ result);
  check_isnan ("imag(csqrt(NaN + i NaN)) = NaN", __imag__ result);

  result = FUNC(csqrt) (BUILD_COMPLEX (16.0, -30.0));
  check ("real(csqrt(16 - 30i)) = 5", __real__ result, 5.0);
  check ("imag(csqrt(16 - 30i)) = -3", __imag__ result, -3.0);

  result = FUNC(csqrt) (BUILD_COMPLEX (-1, 0));
  check ("real(csqrt(1 + i0) = 0", __real__ result, 0);
  check ("imag(csqrt(1 + i0) = 1", __imag__ result, 1);

  result = FUNC(csqrt) (BUILD_COMPLEX (0, 2));
  check ("real(csqrt(0 + i 2) = 1", __real__ result, 1);
  check ("imag(csqrt(0 + i 2) = 1", __imag__ result, 1);

  result = FUNC(csqrt) (BUILD_COMPLEX (119, 120));
  check ("real(csqrt(119 + i 120) = 12", __real__ result, 12);
  check ("imag(csqrt(119 + i 120) = 5", __imag__ result, 5);

  result = FUNC(csqrt) (BUILD_COMPLEX (0.7, 1.2));
  check_eps ("real(csqrt(0.7 + i 1.2)) == 1.02206...", __real__ result,
	     1.0220676100300264507L, CHOOSE (3e-17L, 3e-16, 2e-7));
  check_eps ("imag(csqrt(0.7 + i 1.2)) == 0.58704...", __imag__ result,
	     0.5870453129635652115L, CHOOSE (7e-18L, 0, 6e-8));

  result = FUNC(csqrt) (BUILD_COMPLEX (-2, -3));
  check_eps ("real(csqrt(-2 - i 3)) == 0.89597...", __real__ result,
	     0.8959774761298381247L, CHOOSE (6e-16L, 4e-16, 6e-8));
  check_eps ("imag(csqrt(-2 - i 3)) == -1.67414...", __imag__ result,
	     -1.6741492280355400404L, CHOOSE (0, 5e-16, 0));

  result = FUNC(csqrt) (BUILD_COMPLEX (-2, 3));
  check_eps ("real(csqrt(-2 + i 3)) == 0.89597...", __real__ result,
	     0.8959774761298381247L, CHOOSE (6e-20L, 4e-16, 6e-8));
  check_eps ("imag(csqrt(-2 + i 3)) == 1.67414...", __imag__ result,
	     1.6741492280355400404L, CHOOSE (0, 5e-16, 0));
}


static void
cpow_test (void)
{
  __complex__ MATHTYPE result;

  result = FUNC(cpow) (BUILD_COMPLEX (1, 0), BUILD_COMPLEX (0, 0));
  check ("real(cpow (1 + i0), (0 + i0)) == 0", __real__ result, 1);
  check ("imag(cpow (1 + i0), (0 + i0)) == 0", __imag__ result, 0);

  result = FUNC(cpow) (BUILD_COMPLEX (2, 0), BUILD_COMPLEX (10, 0));
  check_eps ("real(cpow (2 + i0), (10 + i0)) == 1024", __real__ result, 1024,
	     CHOOSE (6e-16L, 1.2e-13, 0));
  check ("imag(cpow (2 + i0), (10 + i0)) == 0", __imag__ result, 0);

  result = FUNC(cpow) (BUILD_COMPLEX (M_El, 0), BUILD_COMPLEX (0, 2 * M_PIl));
  check_eps ("real(cpow (e + i0), (0 + i 2*PI)) == 1", __real__ result, 1,
	     CHOOSE (0, 0, 6e-8));
  check_eps ("imag(cpow (e + i0), (0 + i 2*PI)) == 0", __imag__ result, 0,
	     CHOOSE (3e-18L, 3e-16, 4e-7));

  result = FUNC(cpow) (BUILD_COMPLEX (2, 3), BUILD_COMPLEX (4, 0));
  check_eps ("real(cpow (2 + i3), (4 + i0)) == -119", __real__ result, -119,
	     CHOOSE (9e-16L, 2e-14, 4e-5));
  check_eps ("imag(cpow (2 + i3), (4 + i0)) == -120", __imag__ result, -120,
	     CHOOSE (1e-15L, 0, 5e-5));
}


static void
cabs_test (void)
{
  /* cabs (x + iy) is specified as hypot (x,y) */
  MATHTYPE a;
  a = random_greater (0);
  check_isinfp_ext ("cabs (+inf + i x) == +inf",
		    FUNC(cabs) (BUILD_COMPLEX (plus_infty, a)), a);
  check_isinfp_ext ("cabs (-inf + i x) == +inf",
		    FUNC(cabs) (BUILD_COMPLEX (minus_infty, a)), a);

  check_isinfp ("cabs (+inf+ iNaN) == +inf",
		FUNC(cabs) (BUILD_COMPLEX (minus_infty, nan_value)));
  check_isinfp ("cabs (-inf+ iNaN) == +inf",
		FUNC(cabs) (BUILD_COMPLEX (minus_infty, nan_value)));

  check_isnan ("cabs (NaN+ iNaN) == NaN",
	       FUNC(cabs) (BUILD_COMPLEX (nan_value, nan_value)));

  a = FUNC(cabs) (BUILD_COMPLEX (12.4L, 0.7L));
  check ("cabs (x,y) == cabs (y,x)",
	 FUNC(cabs) (BUILD_COMPLEX (0.7L, 12.4L)), a);
  check ("cabs (x,y) == cabs (-x,y)",
	 FUNC(cabs) (BUILD_COMPLEX (-12.4L, 0.7L)), a);
  check ("cabs (x,y) == cabs (-y,x)",
	 FUNC(cabs) (BUILD_COMPLEX (-0.7L, 12.4L)), a);
  check ("cabs (x,y) == cabs (-x,-y)",
	 FUNC(cabs) (BUILD_COMPLEX (-12.4L, -0.7L)), a);
  check ("cabs (x,y) == cabs (-y,-x)",
	 FUNC(cabs) (BUILD_COMPLEX (-0.7L, -12.4L)), a);
  check ("cabs (x,0) == fabs (x)", FUNC(cabs) (BUILD_COMPLEX (-0.7L, 0)), 0.7L);
  check ("cabs (x,0) == fabs (x)", FUNC(cabs) (BUILD_COMPLEX (0.7L, 0)), 0.7L);
  check ("cabs (x,0) == fabs (x)", FUNC(cabs) (BUILD_COMPLEX (-1.0L, 0)), 1.0L);
  check ("cabs (x,0) == fabs (x)", FUNC(cabs) (BUILD_COMPLEX (1.0L, 0)), 1.0L);
  check ("cabs (x,0) == fabs (x)", FUNC(cabs) (BUILD_COMPLEX (-5.7e7L, 0)),
	 5.7e7L);
  check ("cabs (x,0) == fabs (x)", FUNC(cabs) (BUILD_COMPLEX (5.7e7L, 0)),
	 5.7e7L);

  check_eps ("cabs (0.7 + i 1.2) == 1.38924...", FUNC(cabs) (BUILD_COMPLEX (0.7, 1.2)),
	     1.3892443989449804508L, CHOOSE (7e-17L, 3e-16, 0));
}


static void
carg_test (void)
{
  /* carg (x + iy) is specified as atan2 (y, x) */
  MATHTYPE x;

  x = random_greater (0);
  check ("carg (x + i 0) == 0 for x > 0",
	 FUNC(carg) (BUILD_COMPLEX (x, 0)), 0);
  x = random_greater (0);
  check ("carg (x - i 0) == -0 for x > 0",
	 FUNC(carg) (BUILD_COMPLEX (x, minus_zero)), minus_zero);

  check ("carg (+0 + i 0) == +0", FUNC(carg) (BUILD_COMPLEX (0, 0)), 0);
  check ("carg (+0 - i 0) == -0", FUNC(carg) (BUILD_COMPLEX (0, minus_zero)),
	 minus_zero);

  x = -random_greater (0);
  check ("carg (x + i 0) == +pi for x < 0", FUNC(carg) (BUILD_COMPLEX (x, 0)),
	 M_PIl);

  x = -random_greater (0);
  check ("carg (x - i 0) == -pi for x < 0",
	 FUNC(carg) (BUILD_COMPLEX (x, minus_zero)), -M_PIl);

  check ("carg (-0 + i 0) == +pi", FUNC(carg) (BUILD_COMPLEX (minus_zero, 0)),
	 M_PIl);
  check ("carg (-0 - i 0) == -pi",
	 FUNC(carg) (BUILD_COMPLEX (minus_zero, minus_zero)), -M_PIl);

  x = random_greater (0);
  check ("carg (+0 + i y) == pi/2 for y > 0", FUNC(carg) (BUILD_COMPLEX (0, x)),
	 M_PI_2l);

  x = random_greater (0);
  check ("carg (-0 + i y) == pi/2 for y > 0",
	 FUNC(carg) (BUILD_COMPLEX (minus_zero, x)), M_PI_2l);

  x = random_less (0);
  check ("carg (+0 + i y) == -pi/2 for y < 0", FUNC(carg) (BUILD_COMPLEX (0, x)),
	 -M_PI_2l);

  x = random_less (0);
  check ("carg (-0 + i y) == -pi/2 for y < 0",
	 FUNC(carg) (BUILD_COMPLEX (minus_zero, x)), -M_PI_2l);

  x = random_greater (0);
  check ("carg (inf + i y) == +0 for finite y > 0",
	 FUNC(carg) (BUILD_COMPLEX (plus_infty, x)), 0);

  x = -random_greater (0);
  check ("carg (inf + i y) == -0 for finite y < 0",
	 FUNC(carg) (BUILD_COMPLEX (plus_infty, x)), minus_zero);

  x = random_value (-1e4, 1e4);
  check ("carg(x + i inf) == pi/2 for finite x",
	 FUNC(carg) (BUILD_COMPLEX (x, plus_infty)), M_PI_2l);

  x = random_value (-1e4, 1e4);
  check ("carg(x - i inf) == -pi/2 for finite x",
	 FUNC(carg) (BUILD_COMPLEX (x, minus_infty)), -M_PI_2l);

  x = random_greater (0);
  check ("carg (-inf + i y) == +pi for finite y > 0",
	 FUNC(carg) (BUILD_COMPLEX (minus_infty, x)), M_PIl);

  x = -random_greater (0);
  check ("carg (-inf + i y) == -pi for finite y < 0",
	 FUNC(carg) (BUILD_COMPLEX (minus_infty, x)), -M_PIl);

  check ("carg (+inf + i inf) == +pi/4",
	 FUNC(carg) (BUILD_COMPLEX (plus_infty, plus_infty)), M_PI_4l);

  check ("carg (+inf -i inf) == -pi/4",
	 FUNC(carg) (BUILD_COMPLEX (plus_infty, minus_infty)), -M_PI_4l);

  check ("carg (-inf +i inf) == +3*pi/4",
	 FUNC(carg) (BUILD_COMPLEX (minus_infty, plus_infty)), 3 * M_PI_4l);

  check ("carg (-inf -i inf) == -3*pi/4",
	 FUNC(carg) (BUILD_COMPLEX (minus_infty, minus_infty)), -3 * M_PI_4l);
}


static void
nearbyint_test (void)
{
  check ("nearbyint(+0) = 0", FUNC(nearbyint) (0.0), 0.0);
  check ("nearbyint(-0) = -0", FUNC(nearbyint) (minus_zero), minus_zero);
  check_isinfp ("nearbyint(+Inf) = +Inf", FUNC(nearbyint) (plus_infty));
  check_isinfn ("nearbyint(-Inf) = -Inf", FUNC(nearbyint) (minus_infty));
}


static void
rint_test (void)
{
  check ("rint(0) = 0", FUNC(rint) (0.0), 0.0);
  check ("rint(-0) = -0", FUNC(rint) (minus_zero), minus_zero);
  check_isinfp ("rint(+Inf) = +Inf", FUNC(rint) (plus_infty));
  check_isinfn ("rint(-Inf) = -Inf", FUNC(rint) (minus_infty));
}


static void
lrint_test (void)
{
  /* XXX this test is incomplete.  We need to have a way to specifiy
     the rounding method and test the critical cases.  So far, only
     unproblematic numbers are tested.  */

  check_long ("lrint(0) = 0", FUNC(lrint) (0.0), 0);
  check_long ("lrint(-0) = 0", FUNC(lrint) (minus_zero), 0);
  check_long ("lrint(0.2) = 0", FUNC(lrint) (0.2), 0);
  check_long ("lrint(-0.2) = 0", FUNC(lrint) (-0.2), 0);

  check_long ("lrint(1.4) = 1", FUNC(lrint) (1.4), 1);
  check_long ("lrint(-1.4) = -1", FUNC(lrint) (-1.4), -1);

  check_long ("lrint(8388600.3) = 8388600", FUNC(lrint) (8388600.3), 8388600);
  check_long ("lrint(-8388600.3) = -8388600", FUNC(lrint) (-8388600.3),
	      -8388600);
}


static void
llrint_test (void)
{
  /* XXX this test is incomplete.  We need to have a way to specifiy
     the rounding method and test the critical cases.  So far, only
     unproblematic numbers are tested.  */

  check_longlong ("llrint(0) = 0", FUNC(llrint) (0.0), 0);
  check_longlong ("llrint(-0) = 0", FUNC(llrint) (minus_zero), 0);
  check_longlong ("llrint(0.2) = 0", FUNC(llrint) (0.2), 0);
  check_longlong ("llrint(-0.2) = 0", FUNC(llrint) (-0.2), 0);

  check_longlong ("llrint(1.4) = 1", FUNC(llrint) (1.4), 1);
  check_longlong ("llrint(-1.4) = -1", FUNC(llrint) (-1.4), -1);

  check_longlong ("llrint(8388600.3) = 8388600", FUNC(llrint) (8388600.3),
		  8388600);
  check_longlong ("llrint(-8388600.3) = -8388600", FUNC(llrint) (-8388600.3),
		  -8388600);

  /* Test boundary conditions.  */
  /* 0x1FFFFF */
  check_longlong ("llrint(2097151.0) = 2097151", FUNC(llrint) (2097151.0),
		  2097151LL);
  /* 0x800000 */
  check_longlong ("llrint(8388608.0) = 8388608", FUNC(llrint) (8388608.0),
		  8388608LL);
  /* 0x1000000 */
  check_longlong ("llrint(16777216.0) = 16777216",
		  FUNC(llrint) (16777216.0), 16777216LL);
  /* 0x20000000000 */
  check_longlong ("llrint(2199023255552.0) = 2199023255552",
		  FUNC(llrint) (2199023255552.0), 2199023255552LL);
  /* 0x40000000000 */
  check_longlong ("llrint(4398046511104.0) = 4398046511104",
		  FUNC(llrint) (4398046511104.0), 4398046511104LL);
  /* 0x10000000000000 */
  check_longlong ("llrint(4503599627370496.0) = 4503599627370496",
		  FUNC(llrint) (4503599627370496.0), 4503599627370496LL);
  /* 0x10000080000000 */
  check_longlong ("llrint(4503601774854144.0) = 4503601774854144",
		  FUNC(llrint) (4503601774854144.0), 4503601774854144LL);
  /* 0x20000000000000 */
  check_longlong ("llrint(9007199254740992.0) = 9007199254740992",
		  FUNC(llrint) (9007199254740992.0), 9007199254740992LL);
  /* 0x80000000000000 */
  check_longlong ("llrint(36028797018963968.0) = 36028797018963968",
		  FUNC(llrint) (36028797018963968.0), 36028797018963968LL);
  /* 0x100000000000000 */
  check_longlong ("llrint(72057594037927936.0) = 72057594037927936",
		  FUNC(llrint) (72057594037927936.0), 72057594037927936LL);
}


static void
round_test (void)
{
  check ("round(0) = 0", FUNC(round) (0), 0);
  check ("round(-0) = -0", FUNC(round) (minus_zero), minus_zero);
  check ("round(0.2) = 0", FUNC(round) (0.2), 0.0);
  check ("round(-0.2) = -0", FUNC(round) (-0.2), minus_zero);
  check ("round(0.5) = 1", FUNC(round) (0.5), 1.0);
  check ("round(-0.5) = -1", FUNC(round) (-0.5), -1.0);
  check ("round(0.8) = 1", FUNC(round) (0.8), 1.0);
  check ("round(-0.8) = -1", FUNC(round) (-0.8), -1.0);
  check ("round(1.5) = 2", FUNC(round) (1.5), 2.0);
  check ("round(-1.5) = -2", FUNC(round) (-1.5), -2.0);
  check ("round(2097152.5) = 2097153", FUNC(round) (2097152.5), 2097153);
  check ("round(-2097152.5) = -2097153", FUNC(round) (-2097152.5), -2097153);
}


static void
lround_test (void)
{
  check_long ("lround(0) = 0", FUNC(lround) (0), 0);
  check_long ("lround(-0) = 0", FUNC(lround) (minus_zero), 0);
  check_long ("lround(0.2) = 0", FUNC(lround) (0.2), 0.0);
  check_long ("lround(-0.2) = 0", FUNC(lround) (-0.2), 0);
  check_long ("lround(0.5) = 1", FUNC(lround) (0.5), 1);
  check_long ("lround(-0.5) = -1", FUNC(lround) (-0.5), -1);
  check_long ("lround(0.8) = 1", FUNC(lround) (0.8), 1);
  check_long ("lround(-0.8) = -1", FUNC(lround) (-0.8), -1);
  check_long ("lround(1.5) = 2", FUNC(lround) (1.5), 2);
  check_long ("lround(-1.5) = -2", FUNC(lround) (-1.5), -2);
  check_long ("lround(22514.5) = 22515", FUNC(lround) (22514.5), 22515);
  check_long ("lround(-22514.5) = -22515", FUNC(lround) (-22514.5), -22515);
#ifndef TEST_FLOAT
  check_long ("lround(2097152.5) = 2097153", FUNC(lround) (2097152.5),
	      2097153);
  check_long ("lround(-2097152.5) = -2097153", FUNC(lround) (-2097152.5),
	      -2097153);
#endif
}


static void
llround_test (void)
{
  check_longlong ("llround(0) = 0", FUNC(llround) (0), 0);
  check_longlong ("llround(-0) = 0", FUNC(llround) (minus_zero), 0);
  check_longlong ("llround(0.2) = 0", FUNC(llround) (0.2), 0.0);
  check_longlong ("llround(-0.2) = 0", FUNC(llround) (-0.2), 0);
  check_longlong ("llround(0.5) = 1", FUNC(llround) (0.5), 1);
  check_longlong ("llround(-0.5) = -1", FUNC(llround) (-0.5), -1);
  check_longlong ("llround(0.8) = 1", FUNC(llround) (0.8), 1);
  check_longlong ("llround(-0.8) = -1", FUNC(llround) (-0.8), -1);
  check_longlong ("llround(1.5) = 2", FUNC(llround) (1.5), 2);
  check_longlong ("llround(-1.5) = -2", FUNC(llround) (-1.5), -2);
  check_longlong ("llround(22514.5) = 22515", FUNC(llround) (22514.5), 22515);
  check_longlong ("llround(-22514.5) = -22515", FUNC(llround) (-22514.5),
		  -22515);
#ifndef TEST_FLOAT
  check_longlong ("llround(2097152.5) = 2097153",
		  FUNC(llround) (2097152.5), 2097153);
  check_longlong ("llround(-2097152.5) = -2097153",
		  FUNC(llround) (-2097152.5), -2097153);
  check_longlong ("llround(34359738368.5) = 34359738369",
		  FUNC(llround) (34359738368.5), 34359738369ll);
  check_longlong ("llround(-34359738368.5) = -34359738369",
		  FUNC(llround) (-34359738368.5), -34359738369ll);
#endif

  /* Test boundary conditions.  */
  /* 0x1FFFFF */
  check_longlong ("llround(2097151.0) = 2097151", FUNC(llround) (2097151.0),
		  2097151LL);
  /* 0x800000 */
  check_longlong ("llround(8388608.0) = 8388608", FUNC(llround) (8388608.0),
		  8388608LL);
  /* 0x1000000 */
  check_longlong ("llround(16777216.0) = 16777216",
		  FUNC(llround) (16777216.0), 16777216LL);
  /* 0x20000000000 */
  check_longlong ("llround(2199023255552.0) = 2199023255552",
		  FUNC(llround) (2199023255552.0), 2199023255552LL);
  /* 0x40000000000 */
  check_longlong ("llround(4398046511104.0) = 4398046511104",
		  FUNC(llround) (4398046511104.0), 4398046511104LL);
  /* 0x10000000000000 */
  check_longlong ("llround(4503599627370496.0) = 4503599627370496",
		  FUNC(llround) (4503599627370496.0), 4503599627370496LL);
  /* 0x10000080000000 */
  check_longlong ("llrint(4503601774854144.0) = 4503601774854144",
		  FUNC(llrint) (4503601774854144.0), 4503601774854144LL);
  /* 0x20000000000000 */
  check_longlong ("llround(9007199254740992.0) = 9007199254740992",
		  FUNC(llround) (9007199254740992.0), 9007199254740992LL);
  /* 0x80000000000000 */
  check_longlong ("llround(36028797018963968.0) = 36028797018963968",
		  FUNC(llround) (36028797018963968.0), 36028797018963968LL);
  /* 0x100000000000000 */
  check_longlong ("llround(72057594037927936.0) = 72057594037927936",
		  FUNC(llround) (72057594037927936.0), 72057594037927936LL);
}


static void
fma_test (void)
{
  check ("fma(1.0, 2.0, 3.0) = 5.0", FUNC(fma) (1.0, 2.0, 3.0), 5.0);
  check_isnan ("fma(NaN, 2.0, 3.0) = NaN", FUNC(fma) (nan_value, 2.0, 3.0));
  check_isnan ("fma(1.0, NaN, 3.0) = NaN", FUNC(fma) (1.0, nan_value, 3.0));
  check_isnan_maybe_exc ("fma(1.0, 2.0, NaN) = NaN",
			 FUNC(fma) (1.0, 2.0, nan_value), INVALID_EXCEPTION);
  check_isnan_maybe_exc ("fma(+Inf, 0.0, NaN) = NaN",
			 FUNC(fma) (plus_infty, 0.0, nan_value),
			 INVALID_EXCEPTION);
  check_isnan_maybe_exc ("fma(-Inf, 0.0, NaN) = NaN",
			 FUNC(fma) (minus_infty, 0.0, nan_value),
			 INVALID_EXCEPTION);
  check_isnan_maybe_exc ("fma(0.0, +Inf, NaN) = NaN",
			 FUNC(fma) (0.0, plus_infty, nan_value),
			 INVALID_EXCEPTION);
  check_isnan_maybe_exc ("fma(0.0, -Inf, NaN) = NaN",
			 FUNC(fma) (0.0, minus_infty, nan_value),
			 INVALID_EXCEPTION);
  check_isnan_exc ("fma(+Inf, 0.0, 1.0) = NaN",
		   FUNC(fma) (plus_infty, 0.0, 1.0), INVALID_EXCEPTION);
  check_isnan_exc ("fma(-Inf, 0.0, 1.0) = NaN",
		   FUNC(fma) (minus_infty, 0.0, 1.0), INVALID_EXCEPTION);
  check_isnan_exc ("fma(0.0, +Inf, 1.0) = NaN",
		   FUNC(fma) (0.0, plus_infty, 1.0), INVALID_EXCEPTION);
  check_isnan_exc ("fma(0.0, -Inf, 1.0) = NaN",
		   FUNC(fma) (0.0, minus_infty, 1.0), INVALID_EXCEPTION);

  check_isnan_exc ("fma(+Inf, +Inf, -Inf) = NaN",
		   FUNC(fma) (plus_infty, plus_infty, minus_infty),
		   INVALID_EXCEPTION);
  check_isnan_exc ("fma(-Inf, +Inf, +Inf) = NaN",
		   FUNC(fma) (minus_infty, plus_infty, plus_infty),
		   INVALID_EXCEPTION);
  check_isnan_exc ("fma(+Inf, -Inf, +Inf) = NaN",
		   FUNC(fma) (plus_infty, minus_infty, plus_infty),
		   INVALID_EXCEPTION);
  check_isnan_exc ("fma(-Inf, -Inf, -Inf) = NaN",
		   FUNC(fma) (minus_infty, minus_infty, minus_infty),
		   INVALID_EXCEPTION);
}

static void
j0_test (void)
{
  errno = 0;
  FUNC(j0) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* j0 is the Bessel function of the first kind of order 0 */
  check_isnan ("j0 (NaN) = NaN", FUNC(j0) (nan_value));
  check ("j0 (+Inf) = 0", FUNC(j0) (plus_infty), 0);
  check ("j0 (-1.0) = 0.76519...", FUNC(j0) (-1.0), 0.76519768655796655145);
  check ("j0 (0) = 1", FUNC(j0) (0.0), 1.0);
  check ("j0 (0.1) = 0.99750...", FUNC(j0) (0.1), 0.99750156206604003228);
  check ("j0 (0.7) = 0.88120...", FUNC(j0) (0.7), 0.88120088860740528084);
  check ("j0 (1.0) = 0.76519...", FUNC(j0) (1.0), 0.76519768655796655145);
  check_eps ("j0 (1.5) = 0.51182...", FUNC(j0) (1.5), 0.51182767173591812875,
	     CHOOSE (0, 0, 6e-8));
  check_eps ("j0 (2.0) = 0.22389...", FUNC(j0) (2.0), 0.22389077914123566805,
	     CHOOSE(0, 3e-17, 1.5e-8));
  check_eps ("j0 (8.0) = 0.17165...", FUNC(j0) (8.0), 0.17165080713755390609,
	     CHOOSE(0, 0, 1.5e-8));
  check_eps ("j0 (10.0) = -0.24593...", FUNC(j0) (10.0), -0.24593576445134833520,
	     CHOOSE(0, 6e-17, 5e-8));
}

static void
j1_test (void)
{
  errno = 0;
  FUNC(j1) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* j1 is the Bessel function of the first kind of order 1 */
  check_isnan ("j1 (NaN) = NaN", FUNC(j1) (nan_value));
  check ("j1 (+Inf) = 0", FUNC(j1) (plus_infty), 0);

  check_eps ("j1 (-1.0) = -0.44005...", FUNC(j1) (-1.0),
	     -0.44005058574493351596, CHOOSE (0, 0, 3e-8));
  check ("j1 (0) = 0", FUNC(j1) (0.0), 0.0);
  check ("j1 (0.1) = 0.049937...", FUNC(j1) (0.1), 0.049937526036241997556);
  check ("j1 (0.7) = 0.32899...", FUNC(j1) (0.7), 0.32899574154005894785);
  check_eps ("j1 (1.0) = 0.44005...", FUNC(j1) (1.0), 0.44005058574493351596,
	     CHOOSE (0, 0, 3e-8));
  check_eps ("j1 (1.5) = 0.55793...", FUNC(j1) (1.5), 0.55793650791009964199,
	     CHOOSE (0, 0, 6e-8));
  check_eps ("j1 (2.0) = 0.57672...", FUNC(j1) (2.0), 0.57672480775687338720,
	     CHOOSE(0, 2e-16, 0));
  check_eps ("j1 (8.0) = 0.23463...", FUNC(j1) (8.0), 0.23463634685391462438,
	     CHOOSE(0, 0, 1.5e-8));
  check_eps ("j1 (10.0) = 0.04347...", FUNC(j1) (10.0), 0.043472746168861436670,
	     CHOOSE(0, 2e-17, 7.5e-9));
}

static void
jn_test (void)
{
  errno = 0;
  FUNC(jn) (1, 1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* jn is the Bessel function of the first kind of order n.  */

  /* jn (0, x) == j0 (x)  */
  check_isnan ("jn (0, NaN) = NaN", FUNC(jn) (0, nan_value));
  check ("jn (0, +Inf) = 0", FUNC(jn) (0, plus_infty), 0);
  check ("jn (0, -1.0) = 0.76519...", FUNC(jn) (0, -1.0), 0.76519768655796655145);
  check ("jn (0, 0.0) = 1", FUNC(jn) (0, 0.0), 1.0);
  check ("jn (0, 0.1) = 0.99750...", FUNC(jn) (0, 0.1), 0.99750156206604003228);
  check ("jn (0, 0.7) = 0.88120...", FUNC(jn) (0, 0.7), 0.88120088860740528084);
  check ("jn (0, 1.0) = 0.76519...", FUNC(jn) (0, 1.0), 0.76519768655796655145);
  check_eps ("jn (0, 1.5) = 0.51182...", FUNC(jn) (0, 1.5),
	     0.51182767173591812875, CHOOSE (0, 0, 6e-8));
  check_eps ("jn (0, 2.0) = 0.22389...", FUNC(jn) (0, 2.0), 0.22389077914123566805,
	     CHOOSE(0, 3e-17, 1.5e-8));
  check_eps ("jn (0, 8.0) = 0.17165...", FUNC(jn) (0, 8.0), 0.17165080713755390609,
	     CHOOSE(0, 0, 1.5e-8));
  check_eps ("jn (0, 10.0) = -0.24593...", FUNC(jn) (0, 10.0), -0.24593576445134833520,
	     CHOOSE(0, 6e-17, 4.5e-8));

  /* jn (1, x) == j1 (x)  */
  check_isnan ("jn (1, NaN) = NaN", FUNC(jn) (1, nan_value));
  check ("jn (1, +Inf) = 0", FUNC(jn) (1, plus_infty), 0);

  check_eps ("jn (1, -1.0) = -0.44005...", FUNC(jn) (1, -1.0),
	     -0.44005058574493351596, CHOOSE (0, 0, 3e-8));
  check ("jn (1, 0.0) = 0", FUNC(jn) (1, 0.0), 0.0);
  check ("jn (1, 0.1) = 0.049937...", FUNC(jn) (1, 0.1), 0.049937526036241997556);
  check ("jn (1, 0.7) = 0.32899...", FUNC(jn) (1, 0.7), 0.32899574154005894785);
  check_eps ("jn (1, 1.0) = 0.44005...", FUNC(jn) (1, 1.0),
	     0.44005058574493351596, CHOOSE (0, 0, 3e-8));
  check_eps ("jn (1, 1.5) = 0.55793...", FUNC(jn) (1, 1.5),
	     0.55793650791009964199, CHOOSE (0, 0, 6e-8));
  check_eps ("jn (1, 2.0) = 0.57672...", FUNC(jn) (1, 2.0), 0.57672480775687338720,
	     CHOOSE(0, 2e-16, 0));
  check_eps ("jn (1, 8.0) = 0.23463...", FUNC(jn) (1, 8.0), 0.23463634685391462438,
	     CHOOSE(0, 0, 1.5e-8));
  check_eps ("jn (1, 10.0) = 0.04347...", FUNC(jn) (1, 10.0), 0.043472746168861436670,
	     CHOOSE(0, 2e-17, 7.5e-9));

  /* jn (3, x)  */
  check_isnan ("jn (3, NaN) = NaN", FUNC(jn) (3, nan_value));
  check ("jn (3, +Inf) = 0", FUNC(jn) (3, plus_infty), 0);

  check_eps ("jn (3, -1.0) = -0.01956...", FUNC(jn) (3, -1.0),
	     -0.019563353982668405919, CHOOSE (0, 0, 1.9e-9));
  check ("jn (3, 0.0) = 0", FUNC(jn) (3, 0.0), 0.0);
  check_eps ("jn (3, 0.1) = 2.082...*10^-6", FUNC(jn) (3, 0.1), 0.000020820315754756261429,
	     CHOOSE(0, 4e-21, 0));
  check_eps ("jn (3, 0.7) = 0.00692...", FUNC(jn) (3, 0.7), 0.0069296548267508408077,
	     CHOOSE(0, 2e-18, 0));
  check_eps ("jn (3, 1.0) = 0.01956...", FUNC(jn) (3, 1.0),
	     0.019563353982668405919, CHOOSE (0, 0, 1.9e-9));
  check_eps ("jn (3, 2.0) = 0.12894...", FUNC(jn) (3, 2.0), 0.12894324947440205110,
	     CHOOSE(0, 3e-17, 1.5e-8));
  check_eps ("jn (3, 10.0) = 0.05837...", FUNC(jn) (3, 10.0), 0.058379379305186812343,
	     CHOOSE(0, 3e-17, 1.9e-8));

  /*  jn (10, x)  */
  check_isnan ("jn (10, NaN) = NaN", FUNC(jn) (10, nan_value));
  check ("jn (10, +Inf) = 0", FUNC(jn) (10, plus_infty), 0);

  check_eps ("jn (10, -1.0) = 2.6306...*10^-10", FUNC(jn) (10, -1.0), 0.26306151236874532070e-9,
	     CHOOSE(0, 0, 5.6e-17));
  check ("jn (10, 0) = 0", FUNC(jn) (10, 0.0), 0.0);
  check_eps ("jn (10, 0.1) = 2.6905...*10^-20", FUNC(jn) (10, 0.1), 0.26905328954342155795e-19,
	     CHOOSE(0, 2e-35, 9.7e-27));
  check_eps ("jn (10, 0.7) = 7.517...*10^-12", FUNC(jn) (10, 0.7), 0.75175911502153953928e-11,
	     CHOOSE(0, 7e-27, 1.8e-18));
  check_eps ("jn (10, 1.0) = 2.630...*10^-11", FUNC(jn) (10, 1.0), 0.26306151236874532070e-9,
	     CHOOSE(0, 0, 5.6e-17));
  check_eps ("jn (10, 2.0) = 2.515...*10^-7", FUNC(jn) (10, 2.0), 0.25153862827167367096e-6,
	     CHOOSE(0, 2e-22, 5.7e-14));
  check_eps ("jn (10, 10.0) = 0.20748...", FUNC(jn) (10, 10.0), 0.20748610663335885770,
	     CHOOSE(0, 2e-16, 1.4e-7));
}

static void
y0_test (void)
{
  errno = 0;
  FUNC(y0) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* y0 is the Bessel function of the second kind of order 0 */
  check_isinfn ("y0 (-1.0) = -inf", FUNC(y0) (-1.0));
  check_isinfn ("y0 (0) = -inf", FUNC(y0) (0.0));
  check_isnan ("y0 (NaN) = NaN", FUNC(y0) (nan_value));
  check ("y0 (+Inf) = 0", FUNC(y0) (plus_infty), 0);

  check_eps ("y0 (0.1) = -1.53423...", FUNC(y0) (0.1), -1.5342386513503668441,
	     CHOOSE(0, 3e-16, 2.4e-7));
  check_eps ("y0 (0.7) = -0.19066...", FUNC(y0) (0.7), -0.19066492933739506743,
	     CHOOSE(0, 6e-17, 1.5e-8));
  check_eps ("y0 (1.0) = 0.08825...", FUNC(y0) (1.0), 0.088256964215676957983,
	     CHOOSE(0, 2e-17, 7.5e-9));
  check_eps ("y0 (1.5) = 0.38244...", FUNC(y0) (1.5), 0.38244892379775884396,
	     CHOOSE(0, 6e-17, 3.0e-8));
  check_eps ("y0 (2.0) = 0.51037...", FUNC(y0) (2.0), 0.51037567264974511960,
	     CHOOSE(0, 2e-16, 6e-8));
  check_eps ("y0 (8.0) = 0.22352...", FUNC(y0) (8.0), 0.22352148938756622053,
	     CHOOSE(0, 3e-17, 1.5e-8));
  check_eps ("y0 (10.0) = 0.05567...", FUNC(y0) (10.0), 0.055671167283599391424,
	     CHOOSE(0, 2e-17, 3.8e-9));
}


static void
y1_test (void)
{
  errno = 0;
  FUNC(y1) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* y1 is the Bessel function of the second kind of order 1 */
  check_isinfn ("y1 (-1.0) = -inf", FUNC(y1) (-1.0));
  check_isinfn ("y1 (0) = -inf", FUNC(y1) (0.0));
  check ("y1 (+Inf) = 0", FUNC(y1) (plus_infty), 0);
  check_isnan ("y1 (NaN) = NaN", FUNC(y1) (nan_value));

  check_eps ("y1 (0.1) = -6.45895...", FUNC(y1) (0.1), -6.4589510947020269877,
	     CHOOSE(0, 9e-16, 9.6e-7));
  check_eps ("y1 (0.7) = -1.10324...", FUNC(y1) (0.7), -1.1032498719076333697,
	     CHOOSE(0, 3e-16, 1.2e-7));
  check_eps ("y1 (1.0) = -0.78121...", FUNC(y1) (1.0), -0.78121282130028871655,
	     CHOOSE(0, 2e-16, 0));
  check_eps ("y1 (1.5) = -0.41230...", FUNC(y1) (1.5), -0.41230862697391129595,
	     CHOOSE (0, 0, 6e-8));
  check_eps ("y1 (2.0) = -0.10703...", FUNC(y1) (2.0), -0.10703243154093754689,
	     CHOOSE(0, 2e-17, 1.5e-8));
  check_eps ("y1 (8.0) = -0.15806...", FUNC(y1) (8.0), -0.15806046173124749426,
	     CHOOSE(0, 0, 3.0e-8));
  check_eps ("y1 (10.0) = 0.24901...", FUNC(y1) (10.0), 0.24901542420695388392,
	     CHOOSE(0, 9e-17, 3.0e-8));
}

static void
yn_test (void)
{
  errno = 0;
  FUNC(yn) (1, 1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* yn is the Bessel function of the second kind of order n */

  /* yn (0, x) == y0 (x)  */
  check_isinfn ("yn (0, -1.0) = -inf", FUNC(yn) (0, -1.0));
  check_isinfn ("yn (0, 0) = -inf", FUNC(yn) (0, 0.0));
  check_isnan ("yn (0, NaN) = NaN", FUNC(yn) (0, nan_value));
  check ("yn (0, +Inf) = 0", FUNC(yn) (0, plus_infty), 0);

  check_eps ("yn (0, 0.1) = -1.53423...", FUNC(yn) (0, 0.1), -1.5342386513503668441,
	     CHOOSE(0, 3e-16, 9.6e-7));
  check_eps ("yn (0, 0.7) = -0.19066...", FUNC(yn) (0, 0.7), -0.19066492933739506743,
	     CHOOSE(0, 6e-17, 1.2e-7));
  check_eps ("yn (0, 1.0) = 0.08825...", FUNC(yn) (0, 1.0), 0.088256964215676957983,
	     CHOOSE(0, 2e-17, 3e-8));
  check_eps ("yn (0, 1.5) = 0.38244...", FUNC(yn) (0, 1.5), 0.38244892379775884396,
	     CHOOSE(0, 6e-17, 3e-8));
  check_eps ("yn (0, 2.0) = 0.51037...", FUNC(yn) (0, 2.0), 0.51037567264974511960,
	     CHOOSE(0, 2e-16, 6e-8));
  check_eps ("yn (0, 8.0) = 0.22352...", FUNC(yn) (0, 8.0), 0.22352148938756622053,
	     CHOOSE(0, 3e-17, 1.5e-8));
  check_eps ("yn (0, 10.0) = 0.05567...", FUNC(yn) (0, 10.0), 0.055671167283599391424,
	     CHOOSE(0, 2e-17, 3.8e-8));

  /* yn (1, x) == y1 (x)  */
  check_isinfn ("yn (1, -1.0) = -inf", FUNC(yn) (1, -1.0));
  check_isinfn ("yn (1, 0) = -inf", FUNC(yn) (1, 0.0));
  check ("yn (1, +Inf) = 0", FUNC(yn) (1, plus_infty), 0);
  check_isnan ("yn (1, NaN) = NaN", FUNC(yn) (1, nan_value));

  check_eps ("yn (1, 0.1) = -6.45895...", FUNC(yn) (1, 0.1), -6.4589510947020269877,
	     CHOOSE(0, 9e-16, 9.6e-7));
  check_eps ("yn (1, 0.7) = -1.10324...", FUNC(yn) (1, 0.7), -1.1032498719076333697,
	     CHOOSE(0, 3e-16, 1.2e-7));
  check_eps ("yn (1, 1.0) = -0.78121...", FUNC(yn) (1, 1.0), -0.78121282130028871655,
	     CHOOSE(0, 2e-16, 0));
  check_eps ("yn (1, 1.5) = -0.41230...", FUNC(yn) (1, 1.5), -0.41230862697391129595,
	     CHOOSE (0, 0, 3e-8));
  check_eps ("yn (1, 2.0) = -0.10703...", FUNC(yn) (1, 2.0), -0.10703243154093754689,
	     CHOOSE(0, 2e-17, 2e-8));
  check_eps ("yn (1, 8.0) = -0.15806...", FUNC(yn) (1, 8.0), -0.15806046173124749426,
	     CHOOSE(0, 0, 3e-8));
  check_eps ("yn (1, 10.0) = 0.24901...", FUNC(yn) (1, 10.0), 0.24901542420695388392,
	     CHOOSE(0, 9e-17, 3e-8));

  /* yn (3, x)  */
  check ("yn (3, +Inf) = 0", FUNC(yn) (3, plus_infty), 0);
  check_isnan ("yn (3, NaN) = NaN", FUNC(yn) (3, nan_value));

  check_eps ("yn (3, 0.1) = -5099.3...", FUNC(yn) (3, 0.1), -5099.3323786129048894,
	     CHOOSE(0, 1e-12, 9.8e-4));
  check_eps ("yn (3, 0.7) = -15.819...", FUNC(yn) (3, 0.7), -15.819479052819633505,
	     CHOOSE(0, 4e-15, 9.6e-7));
  check ("yn (3, 1.0) = -5.8215...", FUNC(yn) (3, 1.0), -5.8215176059647288478);
  check_eps ("yn (3, 2.0) = -1.1277...", FUNC(yn) (3, 2.0), -1.1277837768404277861,
	     CHOOSE(0, 3e-16, 1.2e-7));
  check_eps ("yn (3, 10.0) = -0.25136...", FUNC(yn) (3, 10.0), -0.25136265718383732978,
	     CHOOSE(0, 6e-17, 3e-8));

  /* yn (10, x)  */
  check ("yn (10, +Inf) = 0", FUNC(yn) (10, plus_infty), 0);
  check_isnan ("yn (10, NaN) = NaN", FUNC(yn) (10, nan_value));

  check_eps ("yn (10, 0.1) = -1.183...*10^18", FUNC(yn) (10, 0.1), -0.11831335132045197885e19,
	     CHOOSE(0, 6e2, 2.8e11));
  check_eps ("yn (10, 0.7) = -4.244...*10^9", FUNC(yn) (10, 0.7), -0.42447194260703866924e10,
	     CHOOSE(0, 3e-6, 8e2));
  check_eps ("yn (10, 1.0) = -1.216...*10^8", FUNC(yn) (10, 1.0), -0.12161801427868918929e9,
	     CHOOSE(0, 0, 9));
  check_eps ("yn (10, 2.0) = -129184.5...", FUNC(yn) (10, 2.0), -129184.54220803928264,
	     CHOOSE(0, 3e-11, 8e-3));
  check_eps ("yn (10, 10.0) = -0.35981...", FUNC(yn) (10, 10.0), -0.35981415218340272205,
	     CHOOSE(0, 6e-17, 3e-8));

}


/* Tests for the comparison macros */
typedef enum { is_less, is_equal, is_greater, is_unordered } comp_result;


static void
comparison2_test (MATHTYPE x, MATHTYPE y, comp_result comp)
{
  char buf[255];
  int result;
  int expected;

  expected = (comp == is_greater);
  sprintf (buf, "isgreater (%" PRINTF_EXPR ", %" PRINTF_EXPR ") == %d", x, y,
	   expected);
  result = (isgreater (x, y) == expected);
  check_bool (buf, result);

  expected = (comp == is_greater || comp == is_equal);
  sprintf (buf, "isgreaterequal (%" PRINTF_EXPR ", %" PRINTF_EXPR ") == %d", x, y,
	   expected);
  result = (isgreaterequal (x, y) == expected);
  check_bool (buf, result);

  expected = (comp == is_less);
  sprintf (buf, "isless (%" PRINTF_EXPR ", %" PRINTF_EXPR ") == %d", x, y,
	   expected);
  result = (isless (x, y) == expected);
  check_bool (buf, result);

  expected = (comp == is_less || comp == is_equal);
  sprintf (buf, "islessequal (%" PRINTF_EXPR ", %" PRINTF_EXPR ") == %d", x, y,
	   expected);
  result = (islessequal (x, y) == expected);
  check_bool (buf, result);

  expected = (comp == is_greater || comp == is_less);
  sprintf (buf, "islessgreater (%" PRINTF_EXPR ", %" PRINTF_EXPR ") == %d", x, y,
	   expected);
  result = (islessgreater (x, y) == expected);
  check_bool (buf, result);

  expected = (comp == is_unordered);
  sprintf (buf, "isunordered (%" PRINTF_EXPR ", %" PRINTF_EXPR ") == %d", x, y,
	   expected);
  result = (isunordered (x, y) == expected);
  check_bool (buf, result);
}


static void
comparison1_test (MATHTYPE x, MATHTYPE y, comp_result comp)
{
  comp_result comp_swap;
  switch (comp)
    {
    case is_less:
      comp_swap = is_greater;
      break;
    case is_greater:
      comp_swap = is_less;
      break;
    default:
      comp_swap = comp;
      break;
    }
  comparison2_test (x, y, comp);
  comparison2_test (y, x, comp_swap);
}


static void
comparisons_test (void)
{
  comparison1_test (1, 2, is_less);
  comparison1_test (-30, 30, is_less);
  comparison1_test (42, 42, is_equal);
  comparison1_test (1, plus_infty, is_less);
  comparison1_test (35, minus_infty, is_greater);
  comparison1_test (1, nan_value, is_unordered);
  comparison1_test (nan_value, nan_value, is_unordered);
  comparison1_test (plus_infty, nan_value, is_unordered);
  comparison1_test (minus_infty, nan_value, is_unordered);
  comparison1_test (plus_infty, minus_infty, is_greater);
}


static void
inverse_func_pair_test (const char *test_name,
			mathfunc f1, mathfunc inverse,
			MATHTYPE x, MATHTYPE epsilon)
{
  MATHTYPE a, b, difference;
  int result;

  a = f1 (x);
  (void) &a;
  b = inverse (a);
  (void) &b;

  output_new_test (test_name);
  result = check_equal (b, x, epsilon, &difference);
  output_result (test_name, result,
		 b, x, difference, PRINT, PRINT);
}


static void
inverse_functions (void)
{
  inverse_func_pair_test ("asin(sin(x)) == x",
			  FUNC(sin), FUNC(asin), 1.0,
			  CHOOSE (2e-18L, 0, 3e-7L));
  inverse_func_pair_test ("sin(asin(x)) == x",
			  FUNC(asin), FUNC(sin), 1.0, 0.0);

  inverse_func_pair_test ("acos(cos(x)) == x",
			  FUNC(cos), FUNC(acos), 1.0,
			  CHOOSE (4e-18L, 1e-15L, 0));
  inverse_func_pair_test ("cos(acos(x)) == x",
			  FUNC(acos), FUNC(cos), 1.0, 0.0);
  inverse_func_pair_test ("atan(tan(x)) == x",
			  FUNC(tan), FUNC(atan), 1.0,
			  CHOOSE (2e-18L, 0, 0));
  inverse_func_pair_test ("tan(atan(x)) == x",
			  FUNC(atan), FUNC(tan), 1.0,
			  CHOOSE (2e-18L, 1e-15L, 2e-7));

  inverse_func_pair_test ("asinh(sinh(x)) == x",
			  FUNC(sinh), FUNC(asinh), 1.0,
			  CHOOSE (1e-18L, 0, 1e-7));
  inverse_func_pair_test ("sinh(asinh(x)) == x",
			  FUNC(asinh), FUNC(sinh), 1.0,
			  CHOOSE (2e-18L, 2e-16L, 2e-7));

  inverse_func_pair_test ("acosh(cosh(x)) == x",
			  FUNC(cosh), FUNC(acosh), 1.0,
			  CHOOSE (1e-18L, 1e-15L, 6e-8));
  inverse_func_pair_test ("cosh(acosh(x)) == x",
			  FUNC(acosh), FUNC(cosh), 1.0, 0.0);

  inverse_func_pair_test ("atanh(tanh(x)) == x",
			  FUNC(tanh), FUNC(atanh), 1.0,
			  CHOOSE (1e-18L, 1e-15L, 0));
  inverse_func_pair_test ("tanh(atanh(x)) == x",
			  FUNC(atanh), FUNC(tanh), 1.0, 0.0);
}


/* Test sin and cos with the identity: sin(x)^2 + cos(x)^2 = 1.  */
static void
identities1_test (MATHTYPE x, MATHTYPE epsilon)
{
  MATHTYPE res1, res2, res3, diff;
  int result;

  res1 = FUNC(sin) (x);
  (void) &res1;
  res2 = FUNC(cos) (x);
  (void) &res2;
  res3 = res1 * res1 + res2 * res2;
  (void) &res3;

  output_new_test ("sin^2 + cos^2 == 1");
  result = check_equal (res3, 1.0, epsilon, &diff);
  output_result_ext ("sin^2 + cos^2 == 1", result,
		     res3, 1.0, diff, x, PRINT, PRINT);
}


/* Test sin, cos, tan with the following relation: tan = sin/cos.  */
static void
identities2_test (MATHTYPE x, MATHTYPE epsilon)
{
#ifndef TEST_INLINE
  MATHTYPE res1, res2, res3, res4, diff;
  int result;

  res1 = FUNC(sin) (x);
  (void) &res1;
  res2 = FUNC(cos) (x);
  (void) &res2;
  res3 = FUNC(tan) (x);
  (void) &res3;
  res4 = res1 / res2;
  (void) &res4;

  output_new_test ("sin/cos == tan");
  result = check_equal (res4, res3, epsilon, &diff);
  output_result_ext ("sin/cos == tan", result,
		     res4, res3, diff, x, PRINT, PRINT);
#endif
}


/* Test cosh and sinh with the identity cosh^2 - sinh^2 = 1.  */
static void
identities3_test (MATHTYPE x, MATHTYPE epsilon)
{
  MATHTYPE res1, res2, res3, diff;
  int result;

  res1 = FUNC(sinh) (x);
  (void) &res1;
  res2 = FUNC(cosh) (x);
  (void) &res2;
  res3 = res2 * res2 - res1 * res1;
  (void) &res3;

  output_new_test ("cosh^2 - sinh^2 == 1");
  result = check_equal (res3, 1.0, epsilon, &diff);
  output_result_ext ("cosh^2 - sinh^2 == 1", result,
		     res3, 1.0, diff, x, PRINT, PRINT);
}


static void
identities (void)
{
  identities1_test (0.2L, CHOOSE (1e-18L, 0, 2e-7));
  identities1_test (0.9L, CHOOSE (1e-18L, 2e-16, 2e-7));
  identities1_test (0, 0);
  identities1_test (-1, CHOOSE (1e-18L, 0, 1e-7));

  identities2_test (0.2L, CHOOSE (1e-19L, 1e-16, 0));
  identities2_test (0.9L, CHOOSE (3e-19L, 1e-15, 2e-7));
  identities2_test (0, 0);
  identities2_test (-1, CHOOSE (1e-18L, 1e-15, 2e-7));

  identities3_test (0.2L, CHOOSE (1e-18L, 0, 1e-7));
  identities3_test (0.9L, CHOOSE (1e-18L, 1e-15, 1e-6));
  identities3_test (0, CHOOSE (0, 0, 1e-6));
  identities3_test (-1, CHOOSE (1e-18L, 7e-16, 1e-6));
}


/*
   Let's test that basic arithmetic is working
   tests: Infinity and NaN
 */
static void
basic_tests (void)
{
  /* variables are declared volatile to forbid some compiler
     optimizations */
  volatile MATHTYPE Inf_var, NaN_var, zero_var, one_var;
  MATHTYPE x1, x2;

  zero_var = 0.0;
  one_var = 1.0;
  NaN_var = nan_value;
  Inf_var = one_var / zero_var;

  (void) &zero_var;
  (void) &one_var;
  (void) &NaN_var;
  (void) &Inf_var;

  /* Clear all exceptions.  The previous computations raised exceptions.  */
  feclearexcept (FE_ALL_EXCEPT);

  check_isinfp ("isinf (inf) == +1", Inf_var);
  check_isinfn ("isinf (-inf) == -1", -Inf_var);
  check_bool ("!isinf (1)", !(FUNC(isinf) (one_var)));
  check_bool ("!isinf (NaN)", !(FUNC(isinf) (NaN_var)));

  check_isnan ("isnan (NaN)", NaN_var);
  check_isnan ("isnan (-NaN)", -NaN_var);
  check_bool ("!isnan (1)", !(FUNC(isnan) (one_var)));
  check_bool ("!isnan (inf)", !(FUNC(isnan) (Inf_var)));

  check_bool ("inf == inf", Inf_var == Inf_var);
  check_bool ("-inf == -inf", -Inf_var == -Inf_var);
  check_bool ("inf != -inf", Inf_var != -Inf_var);
  check_bool ("NaN != NaN", NaN_var != NaN_var);

  /*
     the same tests but this time with NAN from <bits/nan.h>
     NAN is a double const
   */
  check_bool ("isnan (NAN)", isnan (NAN));
  check_bool ("isnan (-NAN)", isnan (-NAN));
  check_bool ("!isinf (NAN)", !(isinf (NAN)));
  check_bool ("!isinf (-NAN)", !(isinf (-NAN)));
  check_bool ("NAN != NAN", NAN != NAN);

  /*
     And again with the value returned by the `nan' function.
   */
  check_bool ("isnan (NAN)", FUNC(isnan) (FUNC(nan) ("")));
  check_bool ("isnan (-NAN)", FUNC(isnan) (-FUNC(nan) ("")));
  check_bool ("!isinf (NAN)", !(FUNC(isinf) (FUNC(nan) (""))));
  check_bool ("!isinf (-NAN)", !(FUNC(isinf) (-FUNC(nan) (""))));
  check_bool ("NAN != NAN", FUNC(nan) ("") != FUNC(nan) (""));

  /* test if EPSILON is ok */
  x1 = MATHCONST (1.0);
  x2 = x1 + CHOOSE (LDBL_EPSILON, DBL_EPSILON, FLT_EPSILON);
  check_bool ("1 != 1+EPSILON", x1 != x2);

  x1 = MATHCONST (1.0);
  x2 = x1 - CHOOSE (LDBL_EPSILON, DBL_EPSILON, FLT_EPSILON);
  check_bool ("1 != 1-EPSILON", x1 != x2);

  /* test if HUGE_VALx is ok */
  x1 = CHOOSE (HUGE_VALL, HUGE_VAL, HUGE_VALF);
  check_bool ("isinf (HUGE_VALx) == +1", isinf (x1) == +1);
  x1 = -CHOOSE (HUGE_VALL, HUGE_VAL, HUGE_VALF);
  check_bool ("isinf (-HUGE_VALx) == -1", isinf (x1) == -1);
}


static void
initialize (void)
{
  fpstack_test ("start *init*");
  plus_zero = 0.0;
  nan_value = plus_zero / plus_zero;	/* Suppress GCC warning */

  minus_zero = FUNC(copysign) (0.0, -1.0);
  plus_infty = CHOOSE (HUGE_VALL, HUGE_VAL, HUGE_VALF);
  minus_infty = CHOOSE (-HUGE_VALL, -HUGE_VAL, -HUGE_VALF);

  (void) &plus_zero;
  (void) &nan_value;
  (void) &minus_zero;
  (void) &plus_infty;
  (void) &minus_infty;

  /* Clear all exceptions.  From now on we must not get random exceptions.  */
  feclearexcept (FE_ALL_EXCEPT);

  /* Test to make sure we start correctly.  */
  fpstack_test ("end *init*");
}


static struct option long_options[] =
{
  {"verbose", optional_argument, NULL, 'v'},
  {"silent", no_argument, NULL, 's'},
  {0, 0, 0, 0}
};


static void
parse_options (int argc, char *argv[])
{
  int c;
  int option_index;

  verbose = 1;

  while (1)
    {
      c = getopt_long (argc, argv, "v::s",
		       long_options, &option_index);

      /* Detect the end of the options.  */
      if (c == -1)
	break;

      switch (c)
	{
	case 'v':
	  if (optarg)
	    verbose = (unsigned int) strtoul (optarg, NULL, 0);
	  else
	    verbose = 4;
	  break;
	case 's':
	  verbose = 0;
	default:
	  break;
	}
    }
}


int
main (int argc, char *argv[])
{

  parse_options (argc, argv);

  initialize ();
  printf (TEST_MSG);

  basic_tests ();

  /* keep the tests a wee bit ordered (according to ISO 9X) */
  /* classification functions */
  fpclassify_test ();
  isfinite_test ();
  isnormal_test ();
  signbit_test ();

  comparisons_test ();

  /* trigonometric functions */
  acos_test ();
  asin_test ();
  atan_test ();
  atan2_test ();
  cos_test ();
  sin_test ();
  sincos_test ();
  tan_test ();

  /* hyperbolic functions */
  acosh_test ();
  asinh_test ();
  atanh_test ();
  cosh_test ();
  sinh_test ();
  tanh_test ();

  /* exponential and logarithmic functions */
  exp_test ();
  exp10_test ();
  exp2_test ();
  expm1_test ();
  frexp_test ();
  ldexp_test ();
  log_test ();
  log10_test ();
  log1p_test ();
  log2_test ();
  logb_test ();
  modf_test ();
  ilogb_test ();
  scalb_test ();
  scalbn_test ();

  /* power and absolute value functions */
  cbrt_test ();
  fabs_test ();
  hypot_test ();
  pow_test ();
  sqrt_test ();

  /* error and gamma functions */
  erf_test ();
  erfc_test ();
  gamma_test ();
  tgamma_test ();
  lgamma_test ();

  /* nearest integer functions */
  ceil_test ();
  floor_test ();
  nearbyint_test ();
  rint_test ();
  lrint_test ();
  llrint_test ();
  round_test ();
  lround_test ();
  llround_test ();
  trunc_test ();

  /* remainder functions */
  fmod_test ();
  remainder_test ();
  remquo_test ();

  /* manipulation functions */
  copysign_test ();
  nextafter_test ();

  /* maximum, minimum and positive difference functions */
  fdim_test ();
  fmin_test ();
  fmax_test ();

  /* complex functions */
  cabs_test ();
  carg_test ();
  cexp_test ();
  csin_test ();
  csinh_test ();
  ccos_test ();
  ccosh_test ();
  clog_test ();
  clog10_test ();
  cacos_test ();
  cacosh_test ();
  casin_test ();
  casinh_test ();
  catan_test ();
  catanh_test ();
  ctan_test ();
  ctanh_test ();
  csqrt_test ();
  cpow_test ();

  /* multiply and add */
  fma_test ();

  /* Bessel functions  */
  j0_test ();
  j1_test ();
  jn_test ();
  y0_test ();
  y1_test ();
  yn_test ();

  /* special tests */
  identities ();
  inverse_functions ();

  printf ("\nTest suite completed:\n");
  printf ("  %d test cases plus %d tests for exception flags executed.\n",
	  noTests, noExcTests);
  if (noErrors)
    {
      printf ("  %d errors occured.\n", noErrors);
      exit (1);
    }
  printf ("  All tests passed successfully.\n");
  exit (0);
}
