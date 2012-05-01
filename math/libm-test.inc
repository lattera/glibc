/* Copyright (C) 1997-2006, 2007, 2009, 2010, 2011, 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 1997.

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

/* Part of testsuite for libm.

   This file is processed by a perl script.  The resulting file has to
   be included by a master file that defines:

   Macros:
   FUNC(function): converts general function name (like cos) to
   name with correct suffix (e.g. cosl or cosf)
   MATHCONST(x):   like FUNC but for constants (e.g convert 0.0 to 0.0L)
   FLOAT:	   floating point type to test
   - TEST_MSG:	   informal message to be displayed
   CHOOSE(Clongdouble,Cdouble,Cfloat,Cinlinelongdouble,Cinlinedouble,Cinlinefloat):
   chooses one of the parameters as delta for testing
   equality
   PRINTF_EXPR	   Floating point conversion specification to print a variable
   of type FLOAT with printf.  PRINTF_EXPR just contains
   the specifier, not the percent and width arguments,
   e.g. "f".
   PRINTF_XEXPR	   Like PRINTF_EXPR, but print in hexadecimal format.
   PRINTF_NEXPR Like PRINTF_EXPR, but print nice.  */

/* This testsuite has currently tests for:
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
   scalb, scalbn, scalbln, signbit, sin, sincos, sinh, sqrt, tan, tanh, tgamma, trunc,
   y0, y1, yn, significand

   and for the following complex math functions:
   cabs, cacos, cacosh, carg, casin, casinh, catan, catanh,
   ccos, ccosh, cexp, clog, cpow, cproj, csin, csinh, csqrt, ctan, ctanh.

   At the moment the following functions aren't tested:
   drem, nan

   Parameter handling is primitive in the moment:
   --verbose=[0..3] for different levels of output:
   0: only error count
   1: basic report on failed tests (default)
   2: full report on all tests
   -v for full output (equals --verbose=3)
   -u for generation of an ULPs file
 */

/* "Philosophy":

   This suite tests some aspects of the correct implementation of
   mathematical functions in libm.  Some simple, specific parameters
   are tested for correctness but there's no exhaustive
   testing.  Handling of specific inputs (e.g. infinity, not-a-number)
   is also tested.  Correct handling of exceptions is checked
   against.  These implemented tests should check all cases that are
   specified in ISO C99.

   Exception testing: At the moment only divide-by-zero, invalid and
   overflow exceptions are tested.  Underflow and inexact exceptions
   aren't checked at the moment.

   NaN values: There exist signalling and quiet NaNs.  This implementation
   only uses quiet NaN as parameter but does not differenciate
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


   To Do: All parameter should be numbers that can be represented as
   exact floating point values.  Currently some values cannot be
   represented exactly and therefore the result is not the expected
   result.  For this we will use 36 digits so that numbers can be
   represented exactly.  */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include "libm-test-ulps.h"
#include <complex.h>
#include <math.h>
#include <float.h>
#include <fenv.h>
#include <limits.h>

#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <argp.h>

/* Possible exceptions */
#define NO_EXCEPTION			0x0
#define INVALID_EXCEPTION		0x1
#define DIVIDE_BY_ZERO_EXCEPTION	0x2
#define OVERFLOW_EXCEPTION		0x4
/* The next flags signals that those exceptions are allowed but not required.   */
#define INVALID_EXCEPTION_OK		0x8
#define DIVIDE_BY_ZERO_EXCEPTION_OK	0x10
#define OVERFLOW_EXCEPTION_OK		0x20
#define EXCEPTIONS_OK INVALID_EXCEPTION_OK+DIVIDE_BY_ZERO_EXCEPTION_OK
/* Some special test flags, passed togther with exceptions.  */
#define IGNORE_ZERO_INF_SIGN		0x40

/* Various constants (we must supply them precalculated for accuracy).  */
#define M_PI_6l			.52359877559829887307710723054658383L
#define M_E2l			7.389056098930650227230427460575008L
#define M_E3l			20.085536923187667740928529654581719L
#define M_2_SQRT_PIl		3.5449077018110320545963349666822903L	/* 2 sqrt (M_PIl)  */
#define M_SQRT_PIl		1.7724538509055160272981674833411451L	/* sqrt (M_PIl)  */
#define M_LOG_SQRT_PIl		0.57236494292470008707171367567652933L	/* log(sqrt(M_PIl))  */
#define M_LOG_2_SQRT_PIl	1.265512123484645396488945797134706L	/* log(2*sqrt(M_PIl))  */
#define M_PI_34l		(M_PIl - M_PI_4l)		/* 3*pi/4 */
#define M_PI_34_LOG10El		(M_PIl - M_PI_4l) * M_LOG10El
#define M_PI2_LOG10El		M_PI_2l * M_LOG10El
#define M_PI4_LOG10El		M_PI_4l * M_LOG10El
#define M_PI_LOG10El		M_PIl * M_LOG10El
#define M_SQRT_2_2		0.70710678118654752440084436210484903L /* sqrt (2) / 2 */

static FILE *ulps_file;	/* File to document difference.  */
static int output_ulps;	/* Should ulps printed?  */

static int noErrors;	/* number of errors */
static int noTests;	/* number of tests (without testing exceptions) */
static int noExcTests;	/* number of tests for exception flags */
static int noXFails;	/* number of expected failures.  */
static int noXPasses;	/* number of unexpected passes.  */

static int verbose;
static int output_max_error;	/* Should the maximal errors printed?  */
static int output_points;	/* Should the single function results printed?  */
static int ignore_max_ulp;	/* Should we ignore max_ulp?  */

static FLOAT minus_zero, plus_zero;
static FLOAT plus_infty, minus_infty, nan_value, max_value, min_value;

static FLOAT max_error, real_max_error, imag_max_error;


#define BUILD_COMPLEX(real, imag) \
  ({ __complex__ FLOAT __retval;					      \
     __real__ __retval = (real);					      \
     __imag__ __retval = (imag);					      \
     __retval; })

#define BUILD_COMPLEX_INT(real, imag) \
  ({ __complex__ int __retval;						      \
     __real__ __retval = (real);					      \
     __imag__ __retval = (imag);					      \
     __retval; })


#define MANT_DIG CHOOSE ((LDBL_MANT_DIG-1), (DBL_MANT_DIG-1), (FLT_MANT_DIG-1),  \
			 (LDBL_MANT_DIG-1), (DBL_MANT_DIG-1), (FLT_MANT_DIG-1))

static void
init_max_error (void)
{
  max_error = 0;
  real_max_error = 0;
  imag_max_error = 0;
  feclearexcept (FE_ALL_EXCEPT);
}

static void
set_max_error (FLOAT current, FLOAT *curr_max_error)
{
  if (current > *curr_max_error)
    *curr_max_error = current;
}


/* Should the message print to screen?  This depends on the verbose flag,
   and the test status.  */
static int
print_screen (int ok, int xfail)
{
  if (output_points
      && (verbose > 1
	  || (verbose == 1 && ok == xfail)))
    return 1;
  return 0;
}


/* Should the message print to screen?  This depends on the verbose flag,
   and the test status.  */
static int
print_screen_max_error (int ok, int xfail)
{
  if (output_max_error
      && (verbose > 1
	  || ((verbose == 1) && (ok == xfail))))
    return 1;
  return 0;
}

/* Update statistic counters.  */
static void
update_stats (int ok, int xfail)
{
  ++noTests;
  if (ok && xfail)
    ++noXPasses;
  else if (!ok && xfail)
    ++noXFails;
  else if (!ok && !xfail)
    ++noErrors;
}

static void
print_ulps (const char *test_name, FLOAT ulp)
{
  if (output_ulps)
    {
      fprintf (ulps_file, "Test \"%s\":\n", test_name);
      fprintf (ulps_file, "%s: %.0" PRINTF_NEXPR "\n",
	       CHOOSE("ldouble", "double", "float",
		      "ildouble", "idouble", "ifloat"),
	       FUNC(ceil) (ulp));
    }
}

static void
print_function_ulps (const char *function_name, FLOAT ulp)
{
  if (output_ulps)
    {
      fprintf (ulps_file, "Function: \"%s\":\n", function_name);
      fprintf (ulps_file, "%s: %.0" PRINTF_NEXPR "\n",
	       CHOOSE("ldouble", "double", "float",
		      "ildouble", "idouble", "ifloat"),
	       FUNC(ceil) (ulp));
    }
}


static void
print_complex_function_ulps (const char *function_name, FLOAT real_ulp,
			     FLOAT imag_ulp)
{
  if (output_ulps)
    {
      if (real_ulp != 0.0)
	{
	  fprintf (ulps_file, "Function: Real part of \"%s\":\n", function_name);
	  fprintf (ulps_file, "%s: %.0" PRINTF_NEXPR "\n",
		   CHOOSE("ldouble", "double", "float",
			  "ildouble", "idouble", "ifloat"),
		   FUNC(ceil) (real_ulp));
	}
      if (imag_ulp != 0.0)
	{
	  fprintf (ulps_file, "Function: Imaginary part of \"%s\":\n", function_name);
	  fprintf (ulps_file, "%s: %.0" PRINTF_NEXPR "\n",
		   CHOOSE("ldouble", "double", "float",
			  "ildouble", "idouble", "ifloat"),
		   FUNC(ceil) (imag_ulp));
	}


    }
}



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


static void
print_max_error (const char *func_name, FLOAT allowed, int xfail)
{
  int ok = 0;

  if (max_error == 0.0 || (max_error <= allowed && !ignore_max_ulp))
    {
      ok = 1;
    }

  if (!ok)
    print_function_ulps (func_name, max_error);


  if (print_screen_max_error (ok, xfail))
    {
      printf ("Maximal error of `%s'\n", func_name);
      printf (" is      : %.0" PRINTF_NEXPR " ulp\n", FUNC(ceil) (max_error));
      printf (" accepted: %.0" PRINTF_NEXPR " ulp\n", FUNC(ceil) (allowed));
    }

  update_stats (ok, xfail);
}


static void
print_complex_max_error (const char *func_name, __complex__ FLOAT allowed,
			 __complex__ int xfail)
{
  int ok = 0;

  if ((real_max_error == 0 && imag_max_error == 0)
      || (real_max_error <= __real__ allowed
	  && imag_max_error <= __imag__ allowed
	  && !ignore_max_ulp))
    {
      ok = 1;
    }

  if (!ok)
    print_complex_function_ulps (func_name, real_max_error, imag_max_error);


  if (print_screen_max_error (ok, xfail))
    {
      printf ("Maximal error of real part of: %s\n", func_name);
      printf (" is      : %.0" PRINTF_NEXPR " ulp\n",
	      FUNC(ceil) (real_max_error));
      printf (" accepted: %.0" PRINTF_NEXPR " ulp\n",
	      FUNC(ceil) (__real__ allowed));
      printf ("Maximal error of imaginary part of: %s\n", func_name);
      printf (" is      : %.0" PRINTF_NEXPR " ulp\n",
	      FUNC(ceil) (imag_max_error));
      printf (" accepted: %.0" PRINTF_NEXPR " ulp\n",
	      FUNC(ceil) (__imag__ allowed));
    }

  update_stats (ok, xfail);
}


/* Test whether a given exception was raised.  */
static void
test_single_exception (const char *test_name,
		       int exception,
		       int exc_flag,
		       int fe_flag,
		       const char *flag_name)
{
#ifndef TEST_INLINE
  int ok = 1;
  if (exception & exc_flag)
    {
      if (fetestexcept (fe_flag))
	{
	  if (print_screen (1, 0))
	    printf ("Pass: %s: Exception \"%s\" set\n", test_name, flag_name);
	}
      else
	{
	  ok = 0;
	  if (print_screen (0, 0))
	    printf ("Failure: %s: Exception \"%s\" not set\n",
		    test_name, flag_name);
	}
    }
  else
    {
      if (fetestexcept (fe_flag))
	{
	  ok = 0;
	  if (print_screen (0, 0))
	    printf ("Failure: %s: Exception \"%s\" set\n",
		    test_name, flag_name);
	}
      else
	{
	  if (print_screen (1, 0))
	    printf ("%s: Exception \"%s\" not set\n", test_name,
		    flag_name);
	}
    }
  if (!ok)
    ++noErrors;

#endif
}


/* Test whether exceptions given by EXCEPTION are raised.  Ignore thereby
   allowed but not required exceptions.
*/
static void
test_exceptions (const char *test_name, int exception)
{
  ++noExcTests;
#ifdef FE_DIVBYZERO
  if ((exception & DIVIDE_BY_ZERO_EXCEPTION_OK) == 0)
    test_single_exception (test_name, exception,
			   DIVIDE_BY_ZERO_EXCEPTION, FE_DIVBYZERO,
			   "Divide by zero");
#endif
#ifdef FE_INVALID
  if ((exception & INVALID_EXCEPTION_OK) == 0)
    test_single_exception (test_name, exception, INVALID_EXCEPTION, FE_INVALID,
			 "Invalid operation");
#endif
#ifdef FE_OVERFLOW
  if ((exception & OVERFLOW_EXCEPTION_OK) == 0)
    test_single_exception (test_name, exception, OVERFLOW_EXCEPTION,
			   FE_OVERFLOW, "Overflow");
#endif
  feclearexcept (FE_ALL_EXCEPT);
}


static void
check_float_internal (const char *test_name, FLOAT computed, FLOAT expected,
		      FLOAT max_ulp, int xfail, int exceptions,
		      FLOAT *curr_max_error)
{
  int ok = 0;
  int print_diff = 0;
  FLOAT diff = 0;
  FLOAT ulp = 0;

  test_exceptions (test_name, exceptions);
  if (isnan (computed) && isnan (expected))
    ok = 1;
  else if (isinf (computed) && isinf (expected))
    {
      /* Test for sign of infinities.  */
      if ((exceptions & IGNORE_ZERO_INF_SIGN) == 0
	  && signbit (computed) != signbit (expected))
	{
	  ok = 0;
	  printf ("infinity has wrong sign.\n");
	}
      else
	ok = 1;
    }
  /* Don't calc ulp for NaNs or infinities.  */
  else if (isinf (computed) || isnan (computed) || isinf (expected) || isnan (expected))
    ok = 0;
  else
    {
      diff = FUNC(fabs) (computed - expected);
      switch (fpclassify (expected))
	{
	case FP_ZERO:
	  /* ilogb (0) isn't allowed. */
	  ulp = diff / FUNC(ldexp) (1.0, - MANT_DIG);
	  break;
	case FP_NORMAL:
	  ulp = diff / FUNC(ldexp) (1.0, FUNC(ilogb) (expected) - MANT_DIG);
	  break;
	case FP_SUBNORMAL:
	  ulp = (FUNC(ldexp) (diff, MANT_DIG)
		 / FUNC(ldexp) (1.0, FUNC(ilogb) (expected)));
	  break;
	default:
	  /* It should never happen. */
	  abort ();
	  break;
	}
      set_max_error (ulp, curr_max_error);
      print_diff = 1;
      if ((exceptions & IGNORE_ZERO_INF_SIGN) == 0
	  && computed == 0.0 && expected == 0.0
	  && signbit(computed) != signbit (expected))
	ok = 0;
      else if (ulp <= 0.5 || (ulp <= max_ulp && !ignore_max_ulp))
	ok = 1;
      else
	{
	  ok = 0;
	  print_ulps (test_name, ulp);
	}

    }
  if (print_screen (ok, xfail))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
	      computed, computed);
      printf (" should be:  % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n",
	      expected, expected);
      if (print_diff)
	{
	  printf (" difference: % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR
		  "\n", diff, diff);
	  printf (" ulp       : % .4" PRINTF_NEXPR "\n", ulp);
	  printf (" max.ulp   : % .4" PRINTF_NEXPR "\n", max_ulp);
	}
    }
  update_stats (ok, xfail);

  fpstack_test (test_name);
}


static void
check_float (const char *test_name, FLOAT computed, FLOAT expected,
	     FLOAT max_ulp, int xfail, int exceptions)
{
  check_float_internal (test_name, computed, expected, max_ulp, xfail,
			exceptions, &max_error);
}


static void
check_complex (const char *test_name, __complex__ FLOAT computed,
	       __complex__ FLOAT expected,
	       __complex__ FLOAT max_ulp, __complex__ int xfail,
	       int exception)
{
  FLOAT part_comp, part_exp, part_max_ulp;
  int part_xfail;
  char str[200];

  sprintf (str, "Real part of: %s", test_name);
  part_comp = __real__ computed;
  part_exp = __real__ expected;
  part_max_ulp = __real__ max_ulp;
  part_xfail = __real__ xfail;

  check_float_internal (str, part_comp, part_exp, part_max_ulp, part_xfail,
			exception, &real_max_error);

  sprintf (str, "Imaginary part of: %s", test_name);
  part_comp = __imag__ computed;
  part_exp = __imag__ expected;
  part_max_ulp = __imag__ max_ulp;
  part_xfail = __imag__ xfail;

  /* Don't check again for exceptions, just pass through the
     zero/inf sign test.  */
  check_float_internal (str, part_comp, part_exp, part_max_ulp, part_xfail,
			exception & IGNORE_ZERO_INF_SIGN,
			&imag_max_error);
}


/* Check that computed and expected values are equal (int values).  */
static void
check_int (const char *test_name, int computed, int expected, int max_ulp,
	   int xfail, int exceptions)
{
  int diff = computed - expected;
  int ok = 0;

  test_exceptions (test_name, exceptions);
  noTests++;
  if (abs (diff) <= max_ulp)
    ok = 1;

  if (!ok)
    print_ulps (test_name, diff);

  if (print_screen (ok, xfail))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %d\n", computed);
      printf (" should be:  %d\n", expected);
    }

  update_stats (ok, xfail);
  fpstack_test (test_name);
}


/* Check that computed and expected values are equal (long int values).  */
static void
check_long (const char *test_name, long int computed, long int expected,
	    long int max_ulp, int xfail, int exceptions)
{
  long int diff = computed - expected;
  int ok = 0;

  test_exceptions (test_name, exceptions);
  noTests++;
  if (labs (diff) <= max_ulp)
    ok = 1;

  if (!ok)
    print_ulps (test_name, diff);

  if (print_screen (ok, xfail))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %ld\n", computed);
      printf (" should be:  %ld\n", expected);
    }

  update_stats (ok, xfail);
  fpstack_test (test_name);
}


/* Check that computed value is true/false.  */
static void
check_bool (const char *test_name, int computed, int expected,
	    long int max_ulp, int xfail, int exceptions)
{
  int ok = 0;

  test_exceptions (test_name, exceptions);
  noTests++;
  if ((computed == 0) == (expected == 0))
    ok = 1;

  if (print_screen (ok, xfail))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %d\n", computed);
      printf (" should be:  %d\n", expected);
    }

  update_stats (ok, xfail);
  fpstack_test (test_name);
}


/* check that computed and expected values are equal (long int values) */
static void
check_longlong (const char *test_name, long long int computed,
		long long int expected,
		long long int max_ulp, int xfail,
		int exceptions)
{
  long long int diff = computed - expected;
  int ok = 0;

  test_exceptions (test_name, exceptions);
  noTests++;
  if (llabs (diff) <= max_ulp)
    ok = 1;

  if (!ok)
    print_ulps (test_name, diff);

  if (print_screen (ok, xfail))
    {
      if (!ok)
	printf ("Failure:");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %lld\n", computed);
      printf (" should be:  %lld\n", expected);
    }

  update_stats (ok, xfail);
  fpstack_test (test_name);
}



/* This is to prevent messages from the SVID libm emulation.  */
int
matherr (struct exception *x __attribute__ ((unused)))
{
  return 1;
}


/****************************************************************************
  Tests for single functions of libm.
  Please keep them alphabetically sorted!
****************************************************************************/

static void
acos_test (void)
{
  errno = 0;
  FUNC(acos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acos);

  TEST_f_f (acos, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (acos, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (acos, nan_value, nan_value);

  /* |x| > 1: */
  TEST_f_f (acos, 1.125L, nan_value, INVALID_EXCEPTION);
  TEST_f_f (acos, -1.125L, nan_value, INVALID_EXCEPTION);
  TEST_f_f (acos, max_value, nan_value, INVALID_EXCEPTION);
  TEST_f_f (acos, -max_value, nan_value, INVALID_EXCEPTION);

  TEST_f_f (acos, 0, M_PI_2l);
  TEST_f_f (acos, minus_zero, M_PI_2l);
  TEST_f_f (acos, 1, 0);
  TEST_f_f (acos, -1, M_PIl);
  TEST_f_f (acos, 0.5, M_PI_6l*2.0);
  TEST_f_f (acos, -0.5, M_PI_6l*4.0);
  TEST_f_f (acos, 0.75L, 0.722734247813415611178377352641333362L);
  TEST_f_f (acos, 2e-17L, 1.57079632679489659923132169163975144L);
  TEST_f_f (acos, 0.0625L, 1.50825556499840522843072005474337068L);
  TEST_f_f (acos, 0x0.ffffffp0L, 3.4526698471620358760324948263873649728491e-4L);
  TEST_f_f (acos, -0x0.ffffffp0L, 3.1412473866050770348750401337968641476999L);
#ifndef TEST_FLOAT
  TEST_f_f (acos, 0x0.ffffffff8p0L, 1.5258789062648029736620564947844627548516e-5L);
  TEST_f_f (acos, -0x0.ffffffff8p0L, 3.1415773948007305904329067627145550395696L);
  TEST_f_f (acos, 0x0.ffffffffffffp0L, 8.4293697021788088529885473244391795127130e-8L);
  TEST_f_f (acos, -0x0.ffffffffffffp0L, 3.1415925692960962166745548533940296398054L);
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
  TEST_f_f (acos, 0x0.ffffffffffffffffp0L, 3.2927225399135962333718255320079907245059e-10L);
  TEST_f_f (acos, -0x0.ffffffffffffffffp0L, 3.1415926532605209844712837599423203309964L);
#endif
  END (acos);
}


static void
acos_test_tonearest (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(acos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acos_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_f_f (acos, 0, M_PI_2l);
      TEST_f_f (acos, minus_zero, M_PI_2l);
      TEST_f_f (acos, 1, 0);
      TEST_f_f (acos, -1, M_PIl);
      TEST_f_f (acos, 0.5, M_PI_6l*2.0);
      TEST_f_f (acos, -0.5, M_PI_6l*4.0);
    }

  fesetround (save_round_mode);

  END (acos_tonearest);
}


static void
acos_test_towardzero (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(acos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acos_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_f_f (acos, 0, M_PI_2l);
      TEST_f_f (acos, minus_zero, M_PI_2l);
      TEST_f_f (acos, 1, 0);
      TEST_f_f (acos, -1, M_PIl);
      TEST_f_f (acos, 0.5, M_PI_6l*2.0);
      TEST_f_f (acos, -0.5, M_PI_6l*4.0);
    }

  fesetround (save_round_mode);

  END (acos_towardzero);
}


static void
acos_test_downward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(acos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acos_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_f_f (acos, 0, M_PI_2l);
      TEST_f_f (acos, minus_zero, M_PI_2l);
      TEST_f_f (acos, 1, 0);
      TEST_f_f (acos, -1, M_PIl);
      TEST_f_f (acos, 0.5, M_PI_6l*2.0);
      TEST_f_f (acos, -0.5, M_PI_6l*4.0);
    }

  fesetround (save_round_mode);

  END (acos_downward);
}


static void
acos_test_upward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(acos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acos_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_f_f (acos, 0, M_PI_2l);
      TEST_f_f (acos, minus_zero, M_PI_2l);
      TEST_f_f (acos, 1, 0);
      TEST_f_f (acos, -1, M_PIl);
      TEST_f_f (acos, 0.5, M_PI_6l*2.0);
      TEST_f_f (acos, -0.5, M_PI_6l*4.0);
    }

  fesetround (save_round_mode);

  END (acos_upward);
}

static void
acosh_test (void)
{
  errno = 0;
  FUNC(acosh) (7);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acosh);

  TEST_f_f (acosh, plus_infty, plus_infty);
  TEST_f_f (acosh, minus_infty, nan_value, INVALID_EXCEPTION);

  /* x < 1:  */
  TEST_f_f (acosh, -1.125L, nan_value, INVALID_EXCEPTION);
  TEST_f_f (acosh, -max_value, nan_value, INVALID_EXCEPTION);

  TEST_f_f (acosh, 1, 0);
  TEST_f_f (acosh, 7, 2.63391579384963341725009269461593689L);

  END (acosh);
}

static void
asin_test (void)
{
  errno = 0;
  FUNC(asin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asin);

  TEST_f_f (asin, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (asin, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (asin, nan_value, nan_value);

  /* asin x == NaN plus invalid exception for |x| > 1.  */
  TEST_f_f (asin, 1.125L, nan_value, INVALID_EXCEPTION);
  TEST_f_f (asin, -1.125L, nan_value, INVALID_EXCEPTION);
  TEST_f_f (asin, max_value, nan_value, INVALID_EXCEPTION);
  TEST_f_f (asin, -max_value, nan_value, INVALID_EXCEPTION);

  TEST_f_f (asin, 0, 0);
  TEST_f_f (asin, minus_zero, minus_zero);
  TEST_f_f (asin, 0.5, M_PI_6l);
  TEST_f_f (asin, -0.5, -M_PI_6l);
  TEST_f_f (asin, 1.0, M_PI_2l);
  TEST_f_f (asin, -1.0, -M_PI_2l);
  TEST_f_f (asin, 0.75L, 0.848062078981481008052944338998418080L);
  TEST_f_f (asin, 0x0.ffffffp0L, 1.5704510598101804156437184421571127056013L);
  TEST_f_f (asin, -0x0.ffffffp0L, -1.5704510598101804156437184421571127056013L);
#ifndef TEST_FLOAT
  TEST_f_f (asin, 0x0.ffffffff8p0L, 1.5707810680058339712015850710748035974710L);
  TEST_f_f (asin, -0x0.ffffffff8p0L, -1.5707810680058339712015850710748035974710L);
  TEST_f_f (asin, 0x0.ffffffffffffp0L, 1.5707962425011995974432331617542781977068L);
  TEST_f_f (asin, -0x0.ffffffffffffp0L, -1.5707962425011995974432331617542781977068L);
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
  TEST_f_f (asin, 0x0.ffffffffffffffffp0L, 1.5707963264656243652399620683025688888978L);
  TEST_f_f (asin, -0x0.ffffffffffffffffp0L, -1.5707963264656243652399620683025688888978L);
#endif

  END (asin);
}


static void
asin_test_tonearest (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(asin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asin_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_f_f (asin, 0, 0);
      TEST_f_f (asin, minus_zero, minus_zero);
      TEST_f_f (asin, 0.5, M_PI_6l);
      TEST_f_f (asin, -0.5, -M_PI_6l);
      TEST_f_f (asin, 1.0, M_PI_2l);
      TEST_f_f (asin, -1.0, -M_PI_2l);
    }

  fesetround (save_round_mode);

  END (asin_tonearest);
}


static void
asin_test_towardzero (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(asin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asin_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_f_f (asin, 0, 0);
      TEST_f_f (asin, minus_zero, minus_zero);
      TEST_f_f (asin, 0.5, M_PI_6l);
      TEST_f_f (asin, -0.5, -M_PI_6l);
      TEST_f_f (asin, 1.0, M_PI_2l);
      TEST_f_f (asin, -1.0, -M_PI_2l);
    }

  fesetround (save_round_mode);

  END (asin_towardzero);
}


static void
asin_test_downward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(asin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asin_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_f_f (asin, 0, 0);
      TEST_f_f (asin, minus_zero, minus_zero);
      TEST_f_f (asin, 0.5, M_PI_6l);
      TEST_f_f (asin, -0.5, -M_PI_6l);
      TEST_f_f (asin, 1.0, M_PI_2l);
      TEST_f_f (asin, -1.0, -M_PI_2l);
    }

  fesetround (save_round_mode);

  END (asin_downward);
}


static void
asin_test_upward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(asin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asin_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_f_f (asin, 0, 0);
      TEST_f_f (asin, minus_zero, minus_zero);
      TEST_f_f (asin, 0.5, M_PI_6l);
      TEST_f_f (asin, -0.5, -M_PI_6l);
      TEST_f_f (asin, 1.0, M_PI_2l);
      TEST_f_f (asin, -1.0, -M_PI_2l);
    }

  fesetround (save_round_mode);

  END (asin_upward);
}

static void
asinh_test (void)
{
  errno = 0;
  FUNC(asinh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asinh);

  TEST_f_f (asinh, 0, 0);
  TEST_f_f (asinh, minus_zero, minus_zero);
#ifndef TEST_INLINE
  TEST_f_f (asinh, plus_infty, plus_infty);
  TEST_f_f (asinh, minus_infty, minus_infty);
#endif
  TEST_f_f (asinh, nan_value, nan_value);
  TEST_f_f (asinh, 0.75L, 0.693147180559945309417232121458176568L);

  END (asinh);
}

static void
atan_test (void)
{
  errno = 0;
  FUNC(atan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (atan);

  TEST_f_f (atan, 0, 0);
  TEST_f_f (atan, minus_zero, minus_zero);

  TEST_f_f (atan, plus_infty, M_PI_2l);
  TEST_f_f (atan, minus_infty, -M_PI_2l);
  TEST_f_f (atan, nan_value, nan_value);

  TEST_f_f (atan, 1, M_PI_4l);
  TEST_f_f (atan, -1, -M_PI_4l);

  TEST_f_f (atan, 0.75L, 0.643501108793284386802809228717322638L);

  END (atan);
}



static void
atanh_test (void)
{
  errno = 0;
  FUNC(atanh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (atanh);


  TEST_f_f (atanh, 0, 0);
  TEST_f_f (atanh, minus_zero, minus_zero);

  TEST_f_f (atanh, 1, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (atanh, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (atanh, nan_value, nan_value);

  /* atanh (x) == NaN plus invalid exception if |x| > 1.  */
  TEST_f_f (atanh, 1.125L, nan_value, INVALID_EXCEPTION);
  TEST_f_f (atanh, -1.125L, nan_value, INVALID_EXCEPTION);
  TEST_f_f (atanh, max_value, nan_value, INVALID_EXCEPTION);
  TEST_f_f (atanh, -max_value, nan_value, INVALID_EXCEPTION);

  TEST_f_f (atanh, 0.75L, 0.972955074527656652552676371721589865L);

  END (atanh);
}

static void
atan2_test (void)
{
  errno = 0;
  FUNC(atan2) (-0, 1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (atan2);

  /* atan2 (0,x) == 0 for x > 0.  */
  TEST_ff_f (atan2, 0, 1, 0);

  /* atan2 (-0,x) == -0 for x > 0.  */
  TEST_ff_f (atan2, minus_zero, 1, minus_zero);

  TEST_ff_f (atan2, 0, 0, 0);
  TEST_ff_f (atan2, minus_zero, 0, minus_zero);

  /* atan2 (+0,x) == +pi for x < 0.  */
  TEST_ff_f (atan2, 0, -1, M_PIl);

  /* atan2 (-0,x) == -pi for x < 0.  */
  TEST_ff_f (atan2, minus_zero, -1, -M_PIl);

  TEST_ff_f (atan2, 0, minus_zero, M_PIl);
  TEST_ff_f (atan2, minus_zero, minus_zero, -M_PIl);

  /* atan2 (y,+0) == pi/2 for y > 0.  */
  TEST_ff_f (atan2, 1, 0, M_PI_2l);

  /* atan2 (y,-0) == pi/2 for y > 0.  */
  TEST_ff_f (atan2, 1, minus_zero, M_PI_2l);

  /* atan2 (y,+0) == -pi/2 for y < 0.  */
  TEST_ff_f (atan2, -1, 0, -M_PI_2l);

  /* atan2 (y,-0) == -pi/2 for y < 0.  */
  TEST_ff_f (atan2, -1, minus_zero, -M_PI_2l);

  /* atan2 (y,inf) == +0 for finite y > 0.  */
  TEST_ff_f (atan2, 1, plus_infty, 0);

  /* atan2 (y,inf) == -0 for finite y < 0.  */
  TEST_ff_f (atan2, -1, plus_infty, minus_zero);

  /* atan2(+inf, x) == pi/2 for finite x.  */
  TEST_ff_f (atan2, plus_infty, -1, M_PI_2l);

  /* atan2(-inf, x) == -pi/2 for finite x.  */
  TEST_ff_f (atan2, minus_infty, 1, -M_PI_2l);

  /* atan2 (y,-inf) == +pi for finite y > 0.  */
  TEST_ff_f (atan2, 1, minus_infty, M_PIl);

  /* atan2 (y,-inf) == -pi for finite y < 0.  */
  TEST_ff_f (atan2, -1, minus_infty, -M_PIl);

  TEST_ff_f (atan2, plus_infty, plus_infty, M_PI_4l);
  TEST_ff_f (atan2, minus_infty, plus_infty, -M_PI_4l);
  TEST_ff_f (atan2, plus_infty, minus_infty, M_PI_34l);
  TEST_ff_f (atan2, minus_infty, minus_infty, -M_PI_34l);
  TEST_ff_f (atan2, nan_value, nan_value, nan_value);

  TEST_ff_f (atan2, max_value, max_value, M_PI_4l);

  TEST_ff_f (atan2, 0.75L, 1, 0.643501108793284386802809228717322638L);
  TEST_ff_f (atan2, -0.75L, 1.0L, -0.643501108793284386802809228717322638L);
  TEST_ff_f (atan2, 0.75L, -1.0L, 2.49809154479650885165983415456218025L);
  TEST_ff_f (atan2, -0.75L, -1.0L, -2.49809154479650885165983415456218025L);
  TEST_ff_f (atan2, 0.390625L, .00029L, 1.57005392693128974780151246612928941L);
  TEST_ff_f (atan2, 1.390625L, 0.9296875L, 0.981498387184244311516296577615519772L);

  TEST_ff_f (atan2, -0.00756827042671106339L, -.001792735857538728036L, -1.80338464113663849327153994379639112L);

  END (atan2);
}

static void
cabs_test (void)
{
  errno = 0;
  FUNC(cabs) (BUILD_COMPLEX (0.7L, 12.4L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cabs);

  /* cabs (x + iy) is specified as hypot (x,y) */

  /* cabs (+inf + i x) == +inf.  */
  TEST_c_f (cabs, plus_infty, 1.0, plus_infty);
  /* cabs (-inf + i x) == +inf.  */
  TEST_c_f (cabs, minus_infty, 1.0, plus_infty);

  TEST_c_f (cabs, minus_infty, nan_value, plus_infty);
  TEST_c_f (cabs, minus_infty, nan_value, plus_infty);

  TEST_c_f (cabs, nan_value, nan_value, nan_value);

  /* cabs (x,y) == cabs (y,x).  */
  TEST_c_f (cabs, 0.75L, 12.390625L, 12.4133028598606664302388810868156657L);
  /* cabs (x,y) == cabs (-x,y).  */
  TEST_c_f (cabs, -12.390625L, 0.75L, 12.4133028598606664302388810868156657L);
  /* cabs (x,y) == cabs (-y,x).  */
  TEST_c_f (cabs, -0.75L, 12.390625L, 12.4133028598606664302388810868156657L);
  /* cabs (x,y) == cabs (-x,-y).  */
  TEST_c_f (cabs, -12.390625L, -0.75L, 12.4133028598606664302388810868156657L);
  /* cabs (x,y) == cabs (-y,-x).  */
  TEST_c_f (cabs, -0.75L, -12.390625L, 12.4133028598606664302388810868156657L);
  /* cabs (x,0) == fabs (x).  */
  TEST_c_f (cabs, -0.75L, 0, 0.75L);
  TEST_c_f (cabs, 0.75L, 0, 0.75L);
  TEST_c_f (cabs, -1.0L, 0, 1.0L);
  TEST_c_f (cabs, 1.0L, 0, 1.0L);
  TEST_c_f (cabs, -5.7e7L, 0, 5.7e7L);
  TEST_c_f (cabs, 5.7e7L, 0, 5.7e7L);

  TEST_c_f (cabs, 0.75L, 1.25L, 1.45773797371132511771853821938639577L);

  END (cabs);
}


static void
cacos_test (void)
{
  errno = 0;
  FUNC(cacos) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cacos);


  TEST_c_c (cacos, 0, 0, M_PI_2l, minus_zero);
  TEST_c_c (cacos, minus_zero, 0, M_PI_2l, minus_zero);
  TEST_c_c (cacos, minus_zero, minus_zero, M_PI_2l, 0.0);
  TEST_c_c (cacos, 0, minus_zero, M_PI_2l, 0.0);

  TEST_c_c (cacos, minus_infty, plus_infty, M_PI_34l, minus_infty);
  TEST_c_c (cacos, minus_infty, minus_infty, M_PI_34l, plus_infty);

  TEST_c_c (cacos, plus_infty, plus_infty, M_PI_4l, minus_infty);
  TEST_c_c (cacos, plus_infty, minus_infty, M_PI_4l, plus_infty);

  TEST_c_c (cacos, -10.0, plus_infty, M_PI_2l, minus_infty);
  TEST_c_c (cacos, -10.0, minus_infty, M_PI_2l, plus_infty);
  TEST_c_c (cacos, 0, plus_infty, M_PI_2l, minus_infty);
  TEST_c_c (cacos, 0, minus_infty, M_PI_2l, plus_infty);
  TEST_c_c (cacos, 0.1L, plus_infty, M_PI_2l, minus_infty);
  TEST_c_c (cacos, 0.1L, minus_infty, M_PI_2l, plus_infty);

  TEST_c_c (cacos, minus_infty, 0, M_PIl, minus_infty);
  TEST_c_c (cacos, minus_infty, minus_zero, M_PIl, plus_infty);
  TEST_c_c (cacos, minus_infty, 100, M_PIl, minus_infty);
  TEST_c_c (cacos, minus_infty, -100, M_PIl, plus_infty);

  TEST_c_c (cacos, plus_infty, 0, 0.0, minus_infty);
  TEST_c_c (cacos, plus_infty, minus_zero, 0.0, plus_infty);
  TEST_c_c (cacos, plus_infty, 0.5, 0.0, minus_infty);
  TEST_c_c (cacos, plus_infty, -0.5, 0.0, plus_infty);

  TEST_c_c (cacos, plus_infty, nan_value, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (cacos, minus_infty, nan_value, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (cacos, 0, nan_value, M_PI_2l, nan_value);
  TEST_c_c (cacos, minus_zero, nan_value, M_PI_2l, nan_value);

  TEST_c_c (cacos, nan_value, plus_infty, nan_value, minus_infty);
  TEST_c_c (cacos, nan_value, minus_infty, nan_value, plus_infty);

  TEST_c_c (cacos, 10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cacos, -10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (cacos, nan_value, 0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cacos, nan_value, -0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (cacos, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (cacos, plus_zero, -1.5L, M_PI_2l, 1.194763217287109304111930828519090523536L);
  TEST_c_c (cacos, minus_zero, -1.5L, M_PI_2l, 1.194763217287109304111930828519090523536L);
  TEST_c_c (cacos, plus_zero, -1.0L, M_PI_2l, 0.8813735870195430252326093249797923090282L);
  TEST_c_c (cacos, minus_zero, -1.0L, M_PI_2l, 0.8813735870195430252326093249797923090282L);
  TEST_c_c (cacos, plus_zero, -0.5L, M_PI_2l, 0.4812118250596034474977589134243684231352L);
  TEST_c_c (cacos, minus_zero, -0.5L, M_PI_2l, 0.4812118250596034474977589134243684231352L);
  TEST_c_c (cacos, plus_zero, 0.5L, M_PI_2l, -0.4812118250596034474977589134243684231352L);
  TEST_c_c (cacos, minus_zero, 0.5L, M_PI_2l, -0.4812118250596034474977589134243684231352L);
  TEST_c_c (cacos, plus_zero, 1.0L, M_PI_2l, -0.8813735870195430252326093249797923090282L);
  TEST_c_c (cacos, minus_zero, 1.0L, M_PI_2l, -0.8813735870195430252326093249797923090282L);
  TEST_c_c (cacos, plus_zero, 1.5L, M_PI_2l, -1.194763217287109304111930828519090523536L);
  TEST_c_c (cacos, minus_zero, 1.5L, M_PI_2l, -1.194763217287109304111930828519090523536L);

  TEST_c_c (cacos, -1.5L, plus_zero, M_PIl, -0.9624236501192068949955178268487368462704L);
  TEST_c_c (cacos, -1.5L, minus_zero, M_PIl, 0.9624236501192068949955178268487368462704L);
  TEST_c_c (cacos, -1.0L, plus_zero, M_PIl, minus_zero);
  TEST_c_c (cacos, -1.0L, minus_zero, M_PIl, plus_zero);
  TEST_c_c (cacos, -0.5L, plus_zero, 2.094395102393195492308428922186335256131L, minus_zero);
  TEST_c_c (cacos, -0.5L, minus_zero, 2.094395102393195492308428922186335256131L, plus_zero);
  TEST_c_c (cacos, 0.5L, plus_zero, 1.047197551196597746154214461093167628066L, minus_zero);
  TEST_c_c (cacos, 0.5L, minus_zero, 1.047197551196597746154214461093167628066L, plus_zero);
  TEST_c_c (cacos, 1.0L, plus_zero, plus_zero, minus_zero);
  TEST_c_c (cacos, 1.0L, minus_zero, plus_zero, plus_zero);
  TEST_c_c (cacos, 1.5L, plus_zero, plus_zero, -0.9624236501192068949955178268487368462704L);
  TEST_c_c (cacos, 1.5L, minus_zero, plus_zero, 0.9624236501192068949955178268487368462704L);

  TEST_c_c (cacos, 0.75L, 1.25L, 1.11752014915610270578240049553777969L, -1.13239363160530819522266333696834467L);
  TEST_c_c (cacos, -2, -3, 2.1414491111159960199416055713254211L, 1.9833870299165354323470769028940395L);

  END (cacos, complex);
}

static void
cacosh_test (void)
{
  errno = 0;
  FUNC(cacosh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cacosh);


  TEST_c_c (cacosh, 0, 0, 0.0, M_PI_2l);
  TEST_c_c (cacosh, minus_zero, 0, 0.0, M_PI_2l);
  TEST_c_c (cacosh, 0, minus_zero, 0.0, -M_PI_2l);
  TEST_c_c (cacosh, minus_zero, minus_zero, 0.0, -M_PI_2l);
  TEST_c_c (cacosh, minus_infty, plus_infty, plus_infty, M_PI_34l);
  TEST_c_c (cacosh, minus_infty, minus_infty, plus_infty, -M_PI_34l);

  TEST_c_c (cacosh, plus_infty, plus_infty, plus_infty, M_PI_4l);
  TEST_c_c (cacosh, plus_infty, minus_infty, plus_infty, -M_PI_4l);

  TEST_c_c (cacosh, -10.0, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (cacosh, -10.0, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (cacosh, 0, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (cacosh, 0, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (cacosh, 0.1L, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (cacosh, 0.1L, minus_infty, plus_infty, -M_PI_2l);

  TEST_c_c (cacosh, minus_infty, 0, plus_infty, M_PIl);
  TEST_c_c (cacosh, minus_infty, minus_zero, plus_infty, -M_PIl);
  TEST_c_c (cacosh, minus_infty, 100, plus_infty, M_PIl);
  TEST_c_c (cacosh, minus_infty, -100, plus_infty, -M_PIl);

  TEST_c_c (cacosh, plus_infty, 0, plus_infty, 0.0);
  TEST_c_c (cacosh, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (cacosh, plus_infty, 0.5, plus_infty, 0.0);
  TEST_c_c (cacosh, plus_infty, -0.5, plus_infty, minus_zero);

  TEST_c_c (cacosh, plus_infty, nan_value, plus_infty, nan_value);
  TEST_c_c (cacosh, minus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (cacosh, 0, nan_value, nan_value, nan_value);
  TEST_c_c (cacosh, minus_zero, nan_value, nan_value, nan_value);

  TEST_c_c (cacosh, nan_value, plus_infty, plus_infty, nan_value);
  TEST_c_c (cacosh, nan_value, minus_infty, plus_infty, nan_value);

  TEST_c_c (cacosh, 10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cacosh, -10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (cacosh, nan_value, 0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cacosh, nan_value, -0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (cacosh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (cacosh, plus_zero, -1.5L, 1.194763217287109304111930828519090523536L, -M_PI_2l);
  TEST_c_c (cacosh, minus_zero, -1.5L, 1.194763217287109304111930828519090523536L, -M_PI_2l);
  TEST_c_c (cacosh, plus_zero, -1.0L, 0.8813735870195430252326093249797923090282L, -M_PI_2l);
  TEST_c_c (cacosh, minus_zero, -1.0L, 0.8813735870195430252326093249797923090282L, -M_PI_2l);
  TEST_c_c (cacosh, plus_zero, -0.5L, 0.4812118250596034474977589134243684231352L, -M_PI_2l);
  TEST_c_c (cacosh, minus_zero, -0.5L, 0.4812118250596034474977589134243684231352L, -M_PI_2l);
  TEST_c_c (cacosh, plus_zero, 0.5L, 0.4812118250596034474977589134243684231352L, M_PI_2l);
  TEST_c_c (cacosh, minus_zero, 0.5L, 0.4812118250596034474977589134243684231352L, M_PI_2l);
  TEST_c_c (cacosh, plus_zero, 1.0L, 0.8813735870195430252326093249797923090282L, M_PI_2l);
  TEST_c_c (cacosh, minus_zero, 1.0L, 0.8813735870195430252326093249797923090282L, M_PI_2l);
  TEST_c_c (cacosh, plus_zero, 1.5L, 1.194763217287109304111930828519090523536L, M_PI_2l);
  TEST_c_c (cacosh, minus_zero, 1.5L, 1.194763217287109304111930828519090523536L, M_PI_2l);

  TEST_c_c (cacosh, -1.5L, plus_zero, 0.9624236501192068949955178268487368462704L, M_PIl);
  TEST_c_c (cacosh, -1.5L, minus_zero, 0.9624236501192068949955178268487368462704L, -M_PIl);
  TEST_c_c (cacosh, -1.0L, plus_zero, plus_zero, M_PIl);
  TEST_c_c (cacosh, -1.0L, minus_zero, plus_zero, -M_PIl);
  TEST_c_c (cacosh, -0.5L, plus_zero, plus_zero, 2.094395102393195492308428922186335256131L);
  TEST_c_c (cacosh, -0.5L, minus_zero, plus_zero, -2.094395102393195492308428922186335256131L);
  TEST_c_c (cacosh, 0.5L, plus_zero, plus_zero, 1.047197551196597746154214461093167628066L);
  TEST_c_c (cacosh, 0.5L, minus_zero, plus_zero, -1.047197551196597746154214461093167628066L);
  TEST_c_c (cacosh, 1.0L, plus_zero, plus_zero, plus_zero);
  TEST_c_c (cacosh, 1.0L, minus_zero, plus_zero, minus_zero);
  TEST_c_c (cacosh, 1.5L, plus_zero, 0.9624236501192068949955178268487368462704L, plus_zero);
  TEST_c_c (cacosh, 1.5L, minus_zero, 0.9624236501192068949955178268487368462704L, minus_zero);

  TEST_c_c (cacosh, 0.75L, 1.25L, 1.13239363160530819522266333696834467L, 1.11752014915610270578240049553777969L);
  TEST_c_c (cacosh, -2, -3, 1.9833870299165354323470769028940395L, -2.1414491111159960199416055713254211L);

  END (cacosh, complex);
}


static void
carg_test (void)
{
  START (carg);

  /* carg (x + iy) is specified as atan2 (y, x) */

  /* carg (x + i 0) == 0 for x > 0.  */
  TEST_c_f (carg, 2.0, 0, 0);
  /* carg (x - i 0) == -0 for x > 0.  */
  TEST_c_f (carg, 2.0, minus_zero, minus_zero);

  TEST_c_f (carg, 0, 0, 0);
  TEST_c_f (carg, 0, minus_zero, minus_zero);

  /* carg (x + i 0) == +pi for x < 0.  */
  TEST_c_f (carg, -2.0, 0, M_PIl);

  /* carg (x - i 0) == -pi for x < 0.  */
  TEST_c_f (carg, -2.0, minus_zero, -M_PIl);

  TEST_c_f (carg, minus_zero, 0, M_PIl);
  TEST_c_f (carg, minus_zero, minus_zero, -M_PIl);

  /* carg (+0 + i y) == pi/2 for y > 0.  */
  TEST_c_f (carg, 0, 2.0, M_PI_2l);

  /* carg (-0 + i y) == pi/2 for y > 0.  */
  TEST_c_f (carg, minus_zero, 2.0, M_PI_2l);

  /* carg (+0 + i y) == -pi/2 for y < 0.  */
  TEST_c_f (carg, 0, -2.0, -M_PI_2l);

  /* carg (-0 + i y) == -pi/2 for y < 0.  */
  TEST_c_f (carg, minus_zero, -2.0, -M_PI_2l);

  /* carg (inf + i y) == +0 for finite y > 0.  */
  TEST_c_f (carg, plus_infty, 2.0, 0);

  /* carg (inf + i y) == -0 for finite y < 0.  */
  TEST_c_f (carg, plus_infty, -2.0, minus_zero);

  /* carg(x + i inf) == pi/2 for finite x.  */
  TEST_c_f (carg, 10.0, plus_infty, M_PI_2l);

  /* carg(x - i inf) == -pi/2 for finite x.  */
  TEST_c_f (carg, 10.0, minus_infty, -M_PI_2l);

  /* carg (-inf + i y) == +pi for finite y > 0.  */
  TEST_c_f (carg, minus_infty, 10.0, M_PIl);

  /* carg (-inf + i y) == -pi for finite y < 0.  */
  TEST_c_f (carg, minus_infty, -10.0, -M_PIl);

  TEST_c_f (carg, plus_infty, plus_infty, M_PI_4l);

  TEST_c_f (carg, plus_infty, minus_infty, -M_PI_4l);

  TEST_c_f (carg, minus_infty, plus_infty, 3 * M_PI_4l);

  TEST_c_f (carg, minus_infty, minus_infty, -3 * M_PI_4l);

  TEST_c_f (carg, nan_value, nan_value, nan_value);

  END (carg);
}

static void
casin_test (void)
{
  errno = 0;
  FUNC(casin) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (casin);

  TEST_c_c (casin, 0, 0, 0.0, 0.0);
  TEST_c_c (casin, minus_zero, 0, minus_zero, 0.0);
  TEST_c_c (casin, 0, minus_zero, 0.0, minus_zero);
  TEST_c_c (casin, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (casin, plus_infty, plus_infty, M_PI_4l, plus_infty);
  TEST_c_c (casin, plus_infty, minus_infty, M_PI_4l, minus_infty);
  TEST_c_c (casin, minus_infty, plus_infty, -M_PI_4l, plus_infty);
  TEST_c_c (casin, minus_infty, minus_infty, -M_PI_4l, minus_infty);

  TEST_c_c (casin, -10.0, plus_infty, minus_zero, plus_infty);
  TEST_c_c (casin, -10.0, minus_infty, minus_zero, minus_infty);
  TEST_c_c (casin, 0, plus_infty, 0.0, plus_infty);
  TEST_c_c (casin, 0, minus_infty, 0.0, minus_infty);
  TEST_c_c (casin, minus_zero, plus_infty, minus_zero, plus_infty);
  TEST_c_c (casin, minus_zero, minus_infty, minus_zero, minus_infty);
  TEST_c_c (casin, 0.1L, plus_infty, 0.0, plus_infty);
  TEST_c_c (casin, 0.1L, minus_infty, 0.0, minus_infty);

  TEST_c_c (casin, minus_infty, 0, -M_PI_2l, plus_infty);
  TEST_c_c (casin, minus_infty, minus_zero, -M_PI_2l, minus_infty);
  TEST_c_c (casin, minus_infty, 100, -M_PI_2l, plus_infty);
  TEST_c_c (casin, minus_infty, -100, -M_PI_2l, minus_infty);

  TEST_c_c (casin, plus_infty, 0, M_PI_2l, plus_infty);
  TEST_c_c (casin, plus_infty, minus_zero, M_PI_2l, minus_infty);
  TEST_c_c (casin, plus_infty, 0.5, M_PI_2l, plus_infty);
  TEST_c_c (casin, plus_infty, -0.5, M_PI_2l, minus_infty);

  TEST_c_c (casin, nan_value, plus_infty, nan_value, plus_infty);
  TEST_c_c (casin, nan_value, minus_infty, nan_value, minus_infty);

  TEST_c_c (casin, 0.0, nan_value, 0.0, nan_value);
  TEST_c_c (casin, minus_zero, nan_value, minus_zero, nan_value);

  TEST_c_c (casin, plus_infty, nan_value, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (casin, minus_infty, nan_value, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (casin, nan_value, 10.5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (casin, nan_value, -10.5, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (casin, 0.75, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (casin, -0.75, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (casin, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (casin, plus_zero, -1.5L, plus_zero, -1.194763217287109304111930828519090523536L);
  TEST_c_c (casin, minus_zero, -1.5L, minus_zero, -1.194763217287109304111930828519090523536L);
  TEST_c_c (casin, plus_zero, -1.0L, plus_zero, -0.8813735870195430252326093249797923090282L);
  TEST_c_c (casin, minus_zero, -1.0L, minus_zero, -0.8813735870195430252326093249797923090282L);
  TEST_c_c (casin, plus_zero, -0.5L, plus_zero, -0.4812118250596034474977589134243684231352L);
  TEST_c_c (casin, minus_zero, -0.5L, minus_zero, -0.4812118250596034474977589134243684231352L);
  TEST_c_c (casin, plus_zero, 0.5L, plus_zero, 0.4812118250596034474977589134243684231352L);
  TEST_c_c (casin, minus_zero, 0.5L, minus_zero, 0.4812118250596034474977589134243684231352L);
  TEST_c_c (casin, plus_zero, 1.0L, plus_zero, 0.8813735870195430252326093249797923090282L);
  TEST_c_c (casin, minus_zero, 1.0L, minus_zero, 0.8813735870195430252326093249797923090282L);
  TEST_c_c (casin, plus_zero, 1.5L, plus_zero, 1.194763217287109304111930828519090523536L);
  TEST_c_c (casin, minus_zero, 1.5L, minus_zero, 1.194763217287109304111930828519090523536L);

  TEST_c_c (casin, -1.5L, plus_zero, -M_PI_2l, 0.9624236501192068949955178268487368462704L);
  TEST_c_c (casin, -1.5L, minus_zero, -M_PI_2l, -0.9624236501192068949955178268487368462704L);
  TEST_c_c (casin, -1.0L, plus_zero, -M_PI_2l, plus_zero);
  TEST_c_c (casin, -1.0L, minus_zero, -M_PI_2l, minus_zero);
  TEST_c_c (casin, -0.5L, plus_zero, -0.5235987755982988730771072305465838140329L, plus_zero);
  TEST_c_c (casin, -0.5L, minus_zero, -0.5235987755982988730771072305465838140329L, minus_zero);
  TEST_c_c (casin, 0.5L, plus_zero, 0.5235987755982988730771072305465838140329L, plus_zero);
  TEST_c_c (casin, 0.5L, minus_zero, 0.5235987755982988730771072305465838140329L, minus_zero);
  TEST_c_c (casin, 1.0L, plus_zero, M_PI_2l, plus_zero);
  TEST_c_c (casin, 1.0L, minus_zero, M_PI_2l, minus_zero);
  TEST_c_c (casin, 1.5L, plus_zero, M_PI_2l, 0.9624236501192068949955178268487368462704L);
  TEST_c_c (casin, 1.5L, minus_zero, M_PI_2l, -0.9624236501192068949955178268487368462704L);

  TEST_c_c (casin, 0.75L, 1.25L, 0.453276177638793913448921196101971749L, 1.13239363160530819522266333696834467L);
  TEST_c_c (casin, -2, -3, -0.57065278432109940071028387968566963L, -1.9833870299165354323470769028940395L);

  END (casin, complex);
}


static void
casinh_test (void)
{
  errno = 0;
  FUNC(casinh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (casinh);

  TEST_c_c (casinh, 0, 0, 0.0, 0.0);
  TEST_c_c (casinh, minus_zero, 0, minus_zero, 0);
  TEST_c_c (casinh, 0, minus_zero, 0.0, minus_zero);
  TEST_c_c (casinh, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (casinh, plus_infty, plus_infty, plus_infty, M_PI_4l);
  TEST_c_c (casinh, plus_infty, minus_infty, plus_infty, -M_PI_4l);
  TEST_c_c (casinh, minus_infty, plus_infty, minus_infty, M_PI_4l);
  TEST_c_c (casinh, minus_infty, minus_infty, minus_infty, -M_PI_4l);

  TEST_c_c (casinh, -10.0, plus_infty, minus_infty, M_PI_2l);
  TEST_c_c (casinh, -10.0, minus_infty, minus_infty, -M_PI_2l);
  TEST_c_c (casinh, 0, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (casinh, 0, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (casinh, minus_zero, plus_infty, minus_infty, M_PI_2l);
  TEST_c_c (casinh, minus_zero, minus_infty, minus_infty, -M_PI_2l);
  TEST_c_c (casinh, 0.1L, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (casinh, 0.1L, minus_infty, plus_infty, -M_PI_2l);

  TEST_c_c (casinh, minus_infty, 0, minus_infty, 0.0);
  TEST_c_c (casinh, minus_infty, minus_zero, minus_infty, minus_zero);
  TEST_c_c (casinh, minus_infty, 100, minus_infty, 0.0);
  TEST_c_c (casinh, minus_infty, -100, minus_infty, minus_zero);

  TEST_c_c (casinh, plus_infty, 0, plus_infty, 0.0);
  TEST_c_c (casinh, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (casinh, plus_infty, 0.5, plus_infty, 0.0);
  TEST_c_c (casinh, plus_infty, -0.5, plus_infty, minus_zero);

  TEST_c_c (casinh, plus_infty, nan_value, plus_infty, nan_value);
  TEST_c_c (casinh, minus_infty, nan_value, minus_infty, nan_value);

  TEST_c_c (casinh, nan_value, 0, nan_value, 0.0);
  TEST_c_c (casinh, nan_value, minus_zero, nan_value, minus_zero);

  TEST_c_c (casinh, nan_value, plus_infty, plus_infty, nan_value, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (casinh, nan_value, minus_infty, plus_infty, nan_value, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (casinh, 10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (casinh, -10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (casinh, nan_value, 0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (casinh, -0.75, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (casinh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (casinh, plus_zero, -1.5L, 0.9624236501192068949955178268487368462704L, -M_PI_2l);
  TEST_c_c (casinh, minus_zero, -1.5L, -0.9624236501192068949955178268487368462704L, -M_PI_2l);
  TEST_c_c (casinh, plus_zero, -1.0L, plus_zero, -M_PI_2l);
  TEST_c_c (casinh, minus_zero, -1.0L, minus_zero, -M_PI_2l);
  TEST_c_c (casinh, plus_zero, -0.5L, plus_zero, -0.5235987755982988730771072305465838140329L);
  TEST_c_c (casinh, minus_zero, -0.5L, minus_zero, -0.5235987755982988730771072305465838140329L);
  TEST_c_c (casinh, plus_zero, 0.5L, plus_zero, 0.5235987755982988730771072305465838140329L);
  TEST_c_c (casinh, minus_zero, 0.5L, minus_zero, 0.5235987755982988730771072305465838140329L);
  TEST_c_c (casinh, plus_zero, 1.0L, plus_zero, M_PI_2l);
  TEST_c_c (casinh, minus_zero, 1.0L, minus_zero, M_PI_2l);
  TEST_c_c (casinh, plus_zero, 1.5L, 0.9624236501192068949955178268487368462704L, M_PI_2l);
  TEST_c_c (casinh, minus_zero, 1.5L, -0.9624236501192068949955178268487368462704L, M_PI_2l);

  TEST_c_c (casinh, -1.5L, plus_zero, -1.194763217287109304111930828519090523536L, plus_zero);
  TEST_c_c (casinh, -1.5L, minus_zero, -1.194763217287109304111930828519090523536L, minus_zero);
  TEST_c_c (casinh, -1.0L, plus_zero, -0.8813735870195430252326093249797923090282L, plus_zero);
  TEST_c_c (casinh, -1.0L, minus_zero, -0.8813735870195430252326093249797923090282L, minus_zero);
  TEST_c_c (casinh, -0.5L, plus_zero, -0.4812118250596034474977589134243684231352L, plus_zero);
  TEST_c_c (casinh, -0.5L, minus_zero, -0.4812118250596034474977589134243684231352L, minus_zero);
  TEST_c_c (casinh, 0.5L, plus_zero, 0.4812118250596034474977589134243684231352L, plus_zero);
  TEST_c_c (casinh, 0.5L, minus_zero, 0.4812118250596034474977589134243684231352L, minus_zero);
  TEST_c_c (casinh, 1.0L, plus_zero, 0.8813735870195430252326093249797923090282L, plus_zero);
  TEST_c_c (casinh, 1.0L, minus_zero, 0.8813735870195430252326093249797923090282L, minus_zero);
  TEST_c_c (casinh, 1.5L, plus_zero, 1.194763217287109304111930828519090523536L, plus_zero);
  TEST_c_c (casinh, 1.5L, minus_zero, 1.194763217287109304111930828519090523536L, minus_zero);

  TEST_c_c (casinh, 0.75L, 1.25L, 1.03171853444778027336364058631006594L, 0.911738290968487636358489564316731207L);
  TEST_c_c (casinh, -2, -3, -1.9686379257930962917886650952454982L, -0.96465850440760279204541105949953237L);

  END (casinh, complex);
}


static void
catan_test (void)
{
  errno = 0;
  FUNC(catan) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (catan);

  TEST_c_c (catan, 0, 0, 0, 0);
  TEST_c_c (catan, minus_zero, 0, minus_zero, 0);
  TEST_c_c (catan, 0, minus_zero, 0, minus_zero);
  TEST_c_c (catan, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (catan, plus_infty, plus_infty, M_PI_2l, 0);
  TEST_c_c (catan, plus_infty, minus_infty, M_PI_2l, minus_zero);
  TEST_c_c (catan, minus_infty, plus_infty, -M_PI_2l, 0);
  TEST_c_c (catan, minus_infty, minus_infty, -M_PI_2l, minus_zero);


  TEST_c_c (catan, plus_infty, -10.0, M_PI_2l, minus_zero);
  TEST_c_c (catan, minus_infty, -10.0, -M_PI_2l, minus_zero);
  TEST_c_c (catan, plus_infty, minus_zero, M_PI_2l, minus_zero);
  TEST_c_c (catan, minus_infty, minus_zero, -M_PI_2l, minus_zero);
  TEST_c_c (catan, plus_infty, 0.0, M_PI_2l, 0);
  TEST_c_c (catan, minus_infty, 0.0, -M_PI_2l, 0);
  TEST_c_c (catan, plus_infty, 0.1L, M_PI_2l, 0);
  TEST_c_c (catan, minus_infty, 0.1L, -M_PI_2l, 0);

  TEST_c_c (catan, 0.0, minus_infty, M_PI_2l, minus_zero);
  TEST_c_c (catan, minus_zero, minus_infty, -M_PI_2l, minus_zero);
  TEST_c_c (catan, 100.0, minus_infty, M_PI_2l, minus_zero);
  TEST_c_c (catan, -100.0, minus_infty, -M_PI_2l, minus_zero);

  TEST_c_c (catan, 0.0, plus_infty, M_PI_2l, 0);
  TEST_c_c (catan, minus_zero, plus_infty, -M_PI_2l, 0);
  TEST_c_c (catan, 0.5, plus_infty, M_PI_2l, 0);
  TEST_c_c (catan, -0.5, plus_infty, -M_PI_2l, 0);

  TEST_c_c (catan, nan_value, 0.0, nan_value, 0);
  TEST_c_c (catan, nan_value, minus_zero, nan_value, minus_zero);

  TEST_c_c (catan, nan_value, plus_infty, nan_value, 0);
  TEST_c_c (catan, nan_value, minus_infty, nan_value, minus_zero);

  TEST_c_c (catan, 0.0, nan_value, nan_value, nan_value);
  TEST_c_c (catan, minus_zero, nan_value, nan_value, nan_value);

  TEST_c_c (catan, plus_infty, nan_value, M_PI_2l, 0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (catan, minus_infty, nan_value, -M_PI_2l, 0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (catan, nan_value, 10.5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (catan, nan_value, -10.5, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (catan, 0.75, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (catan, -0.75, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (catan, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (catan, 0.75L, 1.25L, 1.10714871779409050301706546017853704L, 0.549306144334054845697622618461262852L);
  TEST_c_c (catan, -2, -3, -1.4099210495965755225306193844604208L, -0.22907268296853876629588180294200276L);

  END (catan, complex);
}

static void
catanh_test (void)
{
  errno = 0;
  FUNC(catanh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (catanh);

  TEST_c_c (catanh, 0, 0, 0.0, 0.0);
  TEST_c_c (catanh, minus_zero, 0, minus_zero, 0.0);
  TEST_c_c (catanh, 0, minus_zero, 0.0, minus_zero);
  TEST_c_c (catanh, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (catanh, plus_infty, plus_infty, 0.0, M_PI_2l);
  TEST_c_c (catanh, plus_infty, minus_infty, 0.0, -M_PI_2l);
  TEST_c_c (catanh, minus_infty, plus_infty, minus_zero, M_PI_2l);
  TEST_c_c (catanh, minus_infty, minus_infty, minus_zero, -M_PI_2l);

  TEST_c_c (catanh, -10.0, plus_infty, minus_zero, M_PI_2l);
  TEST_c_c (catanh, -10.0, minus_infty, minus_zero, -M_PI_2l);
  TEST_c_c (catanh, minus_zero, plus_infty, minus_zero, M_PI_2l);
  TEST_c_c (catanh, minus_zero, minus_infty, minus_zero, -M_PI_2l);
  TEST_c_c (catanh, 0, plus_infty, 0.0, M_PI_2l);
  TEST_c_c (catanh, 0, minus_infty, 0.0, -M_PI_2l);
  TEST_c_c (catanh, 0.1L, plus_infty, 0.0, M_PI_2l);
  TEST_c_c (catanh, 0.1L, minus_infty, 0.0, -M_PI_2l);

  TEST_c_c (catanh, minus_infty, 0, minus_zero, M_PI_2l);
  TEST_c_c (catanh, minus_infty, minus_zero, minus_zero, -M_PI_2l);
  TEST_c_c (catanh, minus_infty, 100, minus_zero, M_PI_2l);
  TEST_c_c (catanh, minus_infty, -100, minus_zero, -M_PI_2l);

  TEST_c_c (catanh, plus_infty, 0, 0.0, M_PI_2l);
  TEST_c_c (catanh, plus_infty, minus_zero, 0.0, -M_PI_2l);
  TEST_c_c (catanh, plus_infty, 0.5, 0.0, M_PI_2l);
  TEST_c_c (catanh, plus_infty, -0.5, 0.0, -M_PI_2l);

  TEST_c_c (catanh, 0, nan_value, 0.0, nan_value);
  TEST_c_c (catanh, minus_zero, nan_value, minus_zero, nan_value);

  TEST_c_c (catanh, plus_infty, nan_value, 0.0, nan_value);
  TEST_c_c (catanh, minus_infty, nan_value, minus_zero, nan_value);

  TEST_c_c (catanh, nan_value, 0, nan_value, nan_value);
  TEST_c_c (catanh, nan_value, minus_zero, nan_value, nan_value);

  TEST_c_c (catanh, nan_value, plus_infty, 0.0, M_PI_2l, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (catanh, nan_value, minus_infty, 0.0, -M_PI_2l, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (catanh, 10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (catanh, -10.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (catanh, nan_value, 0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (catanh, nan_value, -0.75, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (catanh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (catanh, 0.75L, 1.25L, 0.261492138795671927078652057366532140L, 0.996825126463918666098902241310446708L);
  TEST_c_c (catanh, -2, -3, -0.14694666622552975204743278515471595L, -1.3389725222944935611241935759091443L);

  END (catanh, complex);
}

static void
cbrt_test (void)
{
  errno = 0;
  FUNC(cbrt) (8);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cbrt);

  TEST_f_f (cbrt, 0.0, 0.0);
  TEST_f_f (cbrt, minus_zero, minus_zero);

  TEST_f_f (cbrt, plus_infty, plus_infty);
  TEST_f_f (cbrt, minus_infty, minus_infty);
  TEST_f_f (cbrt, nan_value, nan_value);

  TEST_f_f (cbrt, -0.001L, -0.1L);
  TEST_f_f (cbrt, 8, 2);
  TEST_f_f (cbrt, -27.0, -3.0);
  TEST_f_f (cbrt, 0.9921875L, 0.997389022060725270579075195353955217L);
  TEST_f_f (cbrt, 0.75L, 0.908560296416069829445605878163630251L);

  END (cbrt);
}


static void
ccos_test (void)
{
  errno = 0;
  FUNC(ccos) (BUILD_COMPLEX (0, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ccos);

  TEST_c_c (ccos, 0.0, 0.0, 1.0, minus_zero);
  TEST_c_c (ccos, minus_zero, 0.0, 1.0, 0.0);
  TEST_c_c (ccos, 0.0, minus_zero, 1.0, 0.0);
  TEST_c_c (ccos, minus_zero, minus_zero, 1.0, minus_zero);

  TEST_c_c (ccos, plus_infty, 0.0, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccos, plus_infty, minus_zero, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccos, minus_infty, 0.0, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccos, minus_infty, minus_zero, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccos, 0.0, plus_infty, plus_infty, minus_zero);
  TEST_c_c (ccos, 0.0, minus_infty, plus_infty, 0.0);
  TEST_c_c (ccos, minus_zero, plus_infty, plus_infty, 0.0);
  TEST_c_c (ccos, minus_zero, minus_infty, plus_infty, minus_zero);

  TEST_c_c (ccos, plus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, minus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, plus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, minus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ccos, 4.625, plus_infty, minus_infty, plus_infty);
  TEST_c_c (ccos, 4.625, minus_infty, minus_infty, minus_infty);
  TEST_c_c (ccos, -4.625, plus_infty, minus_infty, minus_infty);
  TEST_c_c (ccos, -4.625, minus_infty, minus_infty, plus_infty);

  TEST_c_c (ccos, plus_infty, 6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, plus_infty, -6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, minus_infty, 6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccos, minus_infty, -6.75, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ccos, nan_value, 0.0, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccos, nan_value, minus_zero, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccos, nan_value, plus_infty, plus_infty, nan_value);
  TEST_c_c (ccos, nan_value, minus_infty, plus_infty, nan_value);

  TEST_c_c (ccos, nan_value, 9.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccos, nan_value, -9.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccos, 0.0, nan_value, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccos, minus_zero, nan_value, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccos, 10.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccos, -10.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccos, plus_infty, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccos, minus_infty, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccos, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (ccos, 0.75L, 1.25L, 1.38173873063425888530729933139078645L, -1.09193013555397466170919531722024128L);
  TEST_c_c (ccos, -2, -3, -4.18962569096880723013255501961597373L, -9.10922789375533659797919726277886212L);

  END (ccos, complex);
}


static void
ccosh_test (void)
{
  errno = 0;
  FUNC(ccosh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ccosh);

  TEST_c_c (ccosh, 0.0, 0.0, 1.0, 0.0);
  TEST_c_c (ccosh, minus_zero, 0.0, 1.0, minus_zero);
  TEST_c_c (ccosh, 0.0, minus_zero, 1.0, minus_zero);
  TEST_c_c (ccosh, minus_zero, minus_zero, 1.0, 0.0);

  TEST_c_c (ccosh, 0.0, plus_infty, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccosh, minus_zero, plus_infty, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccosh, 0.0, minus_infty, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccosh, minus_zero, minus_infty, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccosh, plus_infty, 0.0, plus_infty, 0.0);
  TEST_c_c (ccosh, minus_infty, 0.0, plus_infty, minus_zero);
  TEST_c_c (ccosh, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (ccosh, minus_infty, minus_zero, plus_infty, 0.0);

  TEST_c_c (ccosh, plus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, minus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, plus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, minus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ccosh, plus_infty, 4.625, minus_infty, minus_infty);
  TEST_c_c (ccosh, minus_infty, 4.625, minus_infty, plus_infty);
  TEST_c_c (ccosh, plus_infty, -4.625, minus_infty, plus_infty);
  TEST_c_c (ccosh, minus_infty, -4.625, minus_infty, minus_infty);

  TEST_c_c (ccosh, 6.75, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, -6.75, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, 6.75, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ccosh, -6.75, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ccosh, 0.0, nan_value, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccosh, minus_zero, nan_value, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccosh, plus_infty, nan_value, plus_infty, nan_value);
  TEST_c_c (ccosh, minus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (ccosh, 9.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccosh, -9.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccosh, nan_value, 0.0, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ccosh, nan_value, minus_zero, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ccosh, nan_value, 10.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccosh, nan_value, -10.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccosh, nan_value, plus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ccosh, nan_value, minus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ccosh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (ccosh, 0.75L, 1.25L, 0.408242591877968807788852146397499084L, 0.780365930845853240391326216300863152L);

  TEST_c_c (ccosh, -2, -3, -3.72454550491532256547397070325597253L, 0.511822569987384608834463849801875634L);

  END (ccosh, complex);
}


static void
ceil_test (void)
{
  START (ceil);

  TEST_f_f (ceil, 0.0, 0.0);
  TEST_f_f (ceil, minus_zero, minus_zero);
  TEST_f_f (ceil, plus_infty, plus_infty);
  TEST_f_f (ceil, minus_infty, minus_infty);
  TEST_f_f (ceil, nan_value, nan_value);

  TEST_f_f (ceil, M_PIl, 4.0);
  TEST_f_f (ceil, -M_PIl, -3.0);
  TEST_f_f (ceil, 0.1, 1.0);
  TEST_f_f (ceil, 0.25, 1.0);
  TEST_f_f (ceil, 0.625, 1.0);
  TEST_f_f (ceil, -0.1, minus_zero);
  TEST_f_f (ceil, -0.25, minus_zero);
  TEST_f_f (ceil, -0.625, minus_zero);

#ifdef TEST_LDOUBLE
  /* The result can only be represented in long double.  */
  TEST_f_f (ceil, 4503599627370495.5L, 4503599627370496.0L);
  TEST_f_f (ceil, 4503599627370496.25L, 4503599627370497.0L);
  TEST_f_f (ceil, 4503599627370496.5L, 4503599627370497.0L);
  TEST_f_f (ceil, 4503599627370496.75L, 4503599627370497.0L);
  TEST_f_f (ceil, 4503599627370497.5L, 4503599627370498.0L);

  TEST_f_f (ceil, -4503599627370495.5L, -4503599627370495.0L);
  TEST_f_f (ceil, -4503599627370496.25L, -4503599627370496.0L);
  TEST_f_f (ceil, -4503599627370496.5L, -4503599627370496.0L);
  TEST_f_f (ceil, -4503599627370496.75L, -4503599627370496.0L);
  TEST_f_f (ceil, -4503599627370497.5L, -4503599627370497.0L);

# if LDBL_MANT_DIG > 100
  TEST_f_f (ceil, 4503599627370494.5000000000001L, 4503599627370495.0L);
  TEST_f_f (ceil, 4503599627370495.5000000000001L, 4503599627370496.0L);
  TEST_f_f (ceil, 4503599627370496.5000000000001L, 4503599627370497.0L);
  TEST_f_f (ceil, -4503599627370494.5000000000001L, -4503599627370494.0L);
  TEST_f_f (ceil, -4503599627370495.5000000000001L, -4503599627370495.0L);
  TEST_f_f (ceil, -4503599627370496.5000000000001L, -4503599627370496.0L);
# endif

  TEST_f_f (ceil, 9007199254740991.5L, 9007199254740992.0L);
  TEST_f_f (ceil, 9007199254740992.25L, 9007199254740993.0L);
  TEST_f_f (ceil, 9007199254740992.5L, 9007199254740993.0L);
  TEST_f_f (ceil, 9007199254740992.75L, 9007199254740993.0L);
  TEST_f_f (ceil, 9007199254740993.5L, 9007199254740994.0L);

  TEST_f_f (ceil, -9007199254740991.5L, -9007199254740991.0L);
  TEST_f_f (ceil, -9007199254740992.25L, -9007199254740992.0L);
  TEST_f_f (ceil, -9007199254740992.5L, -9007199254740992.0L);
  TEST_f_f (ceil, -9007199254740992.75L, -9007199254740992.0L);
  TEST_f_f (ceil, -9007199254740993.5L, -9007199254740993.0L);

# if LDBL_MANT_DIG > 100
  TEST_f_f (ceil, 9007199254740991.0000000000001L, 9007199254740992.0L);
  TEST_f_f (ceil, 9007199254740992.0000000000001L, 9007199254740993.0L);
  TEST_f_f (ceil, 9007199254740993.0000000000001L, 9007199254740994.0L);
  TEST_f_f (ceil, 9007199254740991.5000000000001L, 9007199254740992.0L);
  TEST_f_f (ceil, 9007199254740992.5000000000001L, 9007199254740993.0L);
  TEST_f_f (ceil, 9007199254740993.5000000000001L, 9007199254740994.0L);

  TEST_f_f (ceil, -9007199254740991.0000000000001L, -9007199254740991.0L);
  TEST_f_f (ceil, -9007199254740992.0000000000001L, -9007199254740992.0L);
  TEST_f_f (ceil, -9007199254740993.0000000000001L, -9007199254740993.0L);
  TEST_f_f (ceil, -9007199254740991.5000000000001L, -9007199254740991.0L);
  TEST_f_f (ceil, -9007199254740992.5000000000001L, -9007199254740992.0L);
  TEST_f_f (ceil, -9007199254740993.5000000000001L, -9007199254740993.0L);
# endif

  TEST_f_f (ceil, 72057594037927935.5L, 72057594037927936.0L);
  TEST_f_f (ceil, 72057594037927936.25L, 72057594037927937.0L);
  TEST_f_f (ceil, 72057594037927936.5L, 72057594037927937.0L);
  TEST_f_f (ceil, 72057594037927936.75L, 72057594037927937.0L);
  TEST_f_f (ceil, 72057594037927937.5L, 72057594037927938.0L);

  TEST_f_f (ceil, -72057594037927935.5L, -72057594037927935.0L);
  TEST_f_f (ceil, -72057594037927936.25L, -72057594037927936.0L);
  TEST_f_f (ceil, -72057594037927936.5L, -72057594037927936.0L);
  TEST_f_f (ceil, -72057594037927936.75L, -72057594037927936.0L);
  TEST_f_f (ceil, -72057594037927937.5L, -72057594037927937.0L);

  TEST_f_f (ceil, 10141204801825835211973625643007.5L, 10141204801825835211973625643008.0L);
  TEST_f_f (ceil, 10141204801825835211973625643008.25L, 10141204801825835211973625643009.0L);
  TEST_f_f (ceil, 10141204801825835211973625643008.5L, 10141204801825835211973625643009.0L);
  TEST_f_f (ceil, 10141204801825835211973625643008.75L, 10141204801825835211973625643009.0L);
  TEST_f_f (ceil, 10141204801825835211973625643009.5L, 10141204801825835211973625643010.0L);
#endif

  END (ceil);
}


static void
cexp_test (void)
{
  errno = 0;
  FUNC(cexp) (BUILD_COMPLEX (0, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cexp);

  TEST_c_c (cexp, plus_zero, plus_zero, 1, 0.0);
  TEST_c_c (cexp, minus_zero, plus_zero, 1, 0.0);
  TEST_c_c (cexp, plus_zero, minus_zero, 1, minus_zero);
  TEST_c_c (cexp, minus_zero, minus_zero, 1, minus_zero);

  TEST_c_c (cexp, plus_infty, plus_zero, plus_infty, 0.0);
  TEST_c_c (cexp, plus_infty, minus_zero, plus_infty, minus_zero);

  TEST_c_c (cexp, minus_infty, plus_zero, 0.0, 0.0);
  TEST_c_c (cexp, minus_infty, minus_zero, 0.0, minus_zero);

  TEST_c_c (cexp, 0.0, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (cexp, minus_zero, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (cexp, 0.0, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (cexp, minus_zero, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (cexp, 100.0, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (cexp, -100.0, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (cexp, 100.0, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (cexp, -100.0, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (cexp, minus_infty, 2.0, minus_zero, 0.0);
  TEST_c_c (cexp, minus_infty, 4.0, minus_zero, minus_zero);
  TEST_c_c (cexp, plus_infty, 2.0, minus_infty, plus_infty);
  TEST_c_c (cexp, plus_infty, 4.0, minus_infty, minus_infty);

  TEST_c_c (cexp, plus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (cexp, plus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (cexp, minus_infty, plus_infty, 0.0, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (cexp, minus_infty, minus_infty, 0.0, minus_zero, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (cexp, minus_infty, nan_value, 0, 0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (cexp, plus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (cexp, nan_value, 0.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cexp, nan_value, 1.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (cexp, nan_value, plus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cexp, 0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cexp, 1, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (cexp, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (cexp, 0.75L, 1.25L, 0.667537446429131586942201977015932112L, 2.00900045494094876258347228145863909L);
  TEST_c_c (cexp, -2.0, -3.0, -0.13398091492954261346140525546115575L, -0.019098516261135196432576240858800925L);

  TEST_c_c (cexp, 0, 0x1p65, 0.99888622066058013610642172179340364209972L, -0.047183876212354673805106149805700013943218L);
  TEST_c_c (cexp, 0, -0x1p65, 0.99888622066058013610642172179340364209972L, 0.047183876212354673805106149805700013943218L);
  TEST_c_c (cexp, 50, 0x1p127, 4.053997150228616856622417636046265337193e21L, 3.232070315463388524466674772633810238819e21L);

#ifndef TEST_FLOAT
  TEST_c_c (cexp, 0, 1e22, 0.5232147853951389454975944733847094921409L, -0.8522008497671888017727058937530293682618L);
  TEST_c_c (cexp, 0, 0x1p1023, -0.826369834614147994500785680811743734805L, 0.5631277798508840134529434079444683477104L);
  TEST_c_c (cexp, 500, 0x1p1023, -1.159886268932754433233243794561351783426e217L, 7.904017694554466595359379965081774849708e216L);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_c_c (cexp, 0, 0x1p16383L, 0.9210843909921906206874509522505756251609L, 0.3893629985894208126948115852610595405563L);
  TEST_c_c (cexp, -10000, 0x1p16383L, 1.045876464564882298442774542991176546722e-4343L, 4.421154026488516836023811173959413420548e-4344L);
#endif

  TEST_c_c (cexp, 88.75, 0.75, 2.558360358486542817001900410314204322891e38L, 2.383359453227311447654736314679677655100e38L);
  TEST_c_c (cexp, -95, 0.75, 4.039714446238306526889476684000081624047e-42L, 3.763383677300535390271646960780570275931e-42L);

#ifndef TEST_FLOAT
  TEST_c_c (cexp, 709.8125, 0.75, 1.355121963080879535248452862759108365762e308L, 1.262426823598609432507811340856186873507e308L);
  TEST_c_c (cexp, -720, 0.75, 1.486960657116368433685753325516638551722e-313L, 1.385247284245720590980701226843815229385e-313L);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_c_c (cexp, 11356.5625, 0.75, 9.052188470850960144814815984311663764287e4931L, 8.432986734191301036267148978260970230200e4931L);
  TEST_c_c (cexp, -11370, 0.75, 8.631121063182211587489310508568170739592e-4939L, 8.040721827809267291427062346918413482824e-4939L);
#endif

#ifdef TEST_FLOAT
  TEST_c_c (cexp, 180, 0x1p-149, plus_infty, 2.087071793345235105931967606907855310664e33L, OVERFLOW_EXCEPTION);
#endif

#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MAX_EXP == 1024)
  TEST_c_c (cexp, 1440, 0x1p-1074, plus_infty, 1.196295853897226111293303155636183216483e302L, OVERFLOW_EXCEPTION);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_c_c (cexp, 22730, 0x1p-16434L, plus_infty, 2.435706297811211974162115164702304105374e4924L, OVERFLOW_EXCEPTION);
#endif

  TEST_c_c (cexp, 1e6, 0, plus_infty, 0, OVERFLOW_EXCEPTION);
  TEST_c_c (cexp, 1e6, min_value, plus_infty, plus_infty, OVERFLOW_EXCEPTION);
  TEST_c_c (cexp, 1e6, -min_value, plus_infty, minus_infty, OVERFLOW_EXCEPTION);

  END (cexp, complex);
}


static void
cimag_test (void)
{
  START (cimag);
  TEST_c_f (cimag, 1.0, 0.0, 0.0);
  TEST_c_f (cimag, 1.0, minus_zero, minus_zero);
  TEST_c_f (cimag, 1.0, nan_value, nan_value);
  TEST_c_f (cimag, nan_value, nan_value, nan_value);
  TEST_c_f (cimag, 1.0, plus_infty, plus_infty);
  TEST_c_f (cimag, 1.0, minus_infty, minus_infty);
  TEST_c_f (cimag, 2.0, 3.0, 3.0);

  END (cimag);
}

static void
clog_test (void)
{
  errno = 0;
  FUNC(clog) (BUILD_COMPLEX (-2, -3));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (clog);

  TEST_c_c (clog, minus_zero, 0, minus_infty, M_PIl, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_c_c (clog, minus_zero, minus_zero, minus_infty, -M_PIl, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_c_c (clog, 0, 0, minus_infty, 0.0, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_c_c (clog, 0, minus_zero, minus_infty, minus_zero, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_c_c (clog, minus_infty, plus_infty, plus_infty, M_PI_34l);
  TEST_c_c (clog, minus_infty, minus_infty, plus_infty, -M_PI_34l);

  TEST_c_c (clog, plus_infty, plus_infty, plus_infty, M_PI_4l);
  TEST_c_c (clog, plus_infty, minus_infty, plus_infty, -M_PI_4l);

  TEST_c_c (clog, 0, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (clog, 3, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (clog, minus_zero, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (clog, -3, plus_infty, plus_infty, M_PI_2l);
  TEST_c_c (clog, 0, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (clog, 3, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (clog, minus_zero, minus_infty, plus_infty, -M_PI_2l);
  TEST_c_c (clog, -3, minus_infty, plus_infty, -M_PI_2l);

  TEST_c_c (clog, minus_infty, 0, plus_infty, M_PIl);
  TEST_c_c (clog, minus_infty, 1, plus_infty, M_PIl);
  TEST_c_c (clog, minus_infty, minus_zero, plus_infty, -M_PIl);
  TEST_c_c (clog, minus_infty, -1, plus_infty, -M_PIl);

  TEST_c_c (clog, plus_infty, 0, plus_infty, 0.0);
  TEST_c_c (clog, plus_infty, 1, plus_infty, 0.0);
  TEST_c_c (clog, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (clog, plus_infty, -1, plus_infty, minus_zero);

  TEST_c_c (clog, plus_infty, nan_value, plus_infty, nan_value);
  TEST_c_c (clog, minus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (clog, nan_value, plus_infty, plus_infty, nan_value);
  TEST_c_c (clog, nan_value, minus_infty, plus_infty, nan_value);

  TEST_c_c (clog, 0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, 3, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, minus_zero, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, -3, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (clog, nan_value, 0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, nan_value, 5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, nan_value, minus_zero, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog, nan_value, -5, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (clog, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (clog, 0.75L, 1.25L, 0.376885901188190075998919126749298416L, 1.03037682652431246378774332703115153L);
  TEST_c_c (clog, -2, -3, 1.2824746787307683680267437207826593L, -2.1587989303424641704769327722648368L);

  TEST_c_c (clog, 0x1.fffffep+127L, 0x1.fffffep+127L, 89.06941264234832570836679262104313101776L, M_PI_4l);
  TEST_c_c (clog, 0x1.fffffep+127L, 1.0L, 88.72283905206835305365817656031404273372L, 2.938736052218037251011746307725933020145e-39L);
  TEST_c_c (clog, 0x1p-149L, 0x1p-149L, -102.9323563131518784484589700365392203592L, M_PI_4l);
  TEST_c_c (clog, 0x1p-147L, 0x1p-147L, -101.5460619520319878296245057936228672231L, M_PI_4l);

#ifndef TEST_FLOAT
  TEST_c_c (clog, 0x1.fffffffffffffp+1023L, 0x1.fffffffffffffp+1023L, 710.1292864836639693869320059713862337880L, M_PI_4l);
  TEST_c_c (clog, 0x1.fffffffffffffp+1023L, 0x1p+1023L, 709.8942846690411016323109979483151967689L, 0.4636476090008061606231772164674799632783L);
  TEST_c_c (clog, 0x1p-1074L, 0x1p-1074L, -744.0934983311012896593986823853525458290L, M_PI_4l);
  TEST_c_c (clog, 0x1p-1073L, 0x1p-1073L, -743.4003511505413443499814502638943692610L, M_PI_4l);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_c_c (clog, 0x1.fp+16383L, 0x1.fp+16383L, 11356.83823118610934184548269774874545400L, M_PI_4l);
  TEST_c_c (clog, 0x1.fp+16383L, 0x1p+16383L, 11356.60974243783798653123798337822335902L, 0.4764674194737066993385333770295162295856L);
  TEST_c_c (clog, 0x1p-16440L, 0x1p-16441L, -11395.22807662984378194141292922726786191L, 0.4636476090008061162142562314612144020285L);
#endif

  END (clog, complex);
}


static void
clog10_test (void)
{
  errno = 0;
  FUNC(clog10) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (clog10);

  TEST_c_c (clog10, minus_zero, 0, minus_infty, M_PIl, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_c_c (clog10, minus_zero, minus_zero, minus_infty, -M_PIl, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_c_c (clog10, 0, 0, minus_infty, 0.0, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_c_c (clog10, 0, minus_zero, minus_infty, minus_zero, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_c_c (clog10, minus_infty, plus_infty, plus_infty, M_PI_34_LOG10El);

  TEST_c_c (clog10, plus_infty, plus_infty, plus_infty, M_PI4_LOG10El);
  TEST_c_c (clog10, plus_infty, minus_infty, plus_infty, -M_PI4_LOG10El);

  TEST_c_c (clog10, 0, plus_infty, plus_infty, M_PI2_LOG10El);
  TEST_c_c (clog10, 3, plus_infty, plus_infty, M_PI2_LOG10El);
  TEST_c_c (clog10, minus_zero, plus_infty, plus_infty, M_PI2_LOG10El);
  TEST_c_c (clog10, -3, plus_infty, plus_infty, M_PI2_LOG10El);
  TEST_c_c (clog10, 0, minus_infty, plus_infty, -M_PI2_LOG10El);
  TEST_c_c (clog10, 3, minus_infty, plus_infty, -M_PI2_LOG10El);
  TEST_c_c (clog10, minus_zero, minus_infty, plus_infty, -M_PI2_LOG10El);
  TEST_c_c (clog10, -3, minus_infty, plus_infty, -M_PI2_LOG10El);

  TEST_c_c (clog10, minus_infty, 0, plus_infty, M_PI_LOG10El);
  TEST_c_c (clog10, minus_infty, 1, plus_infty, M_PI_LOG10El);
  TEST_c_c (clog10, minus_infty, minus_zero, plus_infty, -M_PI_LOG10El);
  TEST_c_c (clog10, minus_infty, -1, plus_infty, -M_PI_LOG10El);

  TEST_c_c (clog10, plus_infty, 0, plus_infty, 0.0);
  TEST_c_c (clog10, plus_infty, 1, plus_infty, 0.0);
  TEST_c_c (clog10, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (clog10, plus_infty, -1, plus_infty, minus_zero);

  TEST_c_c (clog10, plus_infty, nan_value, plus_infty, nan_value);
  TEST_c_c (clog10, minus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (clog10, nan_value, plus_infty, plus_infty, nan_value);
  TEST_c_c (clog10, nan_value, minus_infty, plus_infty, nan_value);

  TEST_c_c (clog10, 0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, 3, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, minus_zero, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, -3, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (clog10, nan_value, 0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, nan_value, 5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, nan_value, minus_zero, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (clog10, nan_value, -5, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (clog10, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (clog10, 0.75L, 1.25L, 0.163679467193165171449476605077428975L, 0.447486970040493067069984724340855636L);
  TEST_c_c (clog10, -2, -3, 0.556971676153418384603252578971164214L, -0.937554462986374708541507952140189646L);

  TEST_c_c (clog10, 0x1.fffffep+127L, 0x1.fffffep+127L, 38.68235441693561449174780668781319348761L, M_PI4_LOG10El);
  TEST_c_c (clog10, 0x1.fffffep+127L, 1.0L, 38.53183941910362389414093724045094697423L, 1.276276851248440096917018665609900318458e-39L);
  TEST_c_c (clog10, 0x1p-149L, 0x1p-149L, -44.70295435610120748924022586658721447508L, M_PI4_LOG10El);
  TEST_c_c (clog10, 0x1p-147L, 0x1p-147L, -44.10089436477324509881274807713822842154L, M_PI4_LOG10El);

#ifndef TEST_FLOAT
  TEST_c_c (clog10, 0x1.fffffffffffffp+1023L, 0x1.fffffffffffffp+1023L, 308.4052305577487344482591243175787477115L, M_PI4_LOG10El);
  TEST_c_c (clog10, 0x1.fffffffffffffp+1023L, 0x1p+1023L, 308.3031705664207720674749211936626341569L, 0.2013595981366865903254995612594728746470L);
  TEST_c_c (clog10, 0x1p-1074L, 0x1p-1074L, -323.1557003452838130619487034867432642357L, M_PI4_LOG10El);
  TEST_c_c (clog10, 0x1p-1073L, 0x1p-1073L, -322.8546703496198318667349645920187712089L, M_PI4_LOG10El);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_c_c (clog10, 0x1.fp+16383L, 0x1.fp+16383L, 4932.212175672014259683102930239951947672L, M_PI4_LOG10El);
  TEST_c_c (clog10, 0x1.fp+16383L, 0x1p+16383L, 4932.112944269463028900262609694408579449L, 0.2069271710841128115912940666587802677383L);
  TEST_c_c (clog10, 0x1p-16440L, 0x1p-16441L, -4948.884673709346821106688037612752099609L, 0.2013595981366865710389502301937289472543L);
#endif

  END (clog10, complex);
}


static void
conj_test (void)
{
  START (conj);
  TEST_c_c (conj, 0.0, 0.0, 0.0, minus_zero);
  TEST_c_c (conj, 0.0, minus_zero, 0.0, 0.0);
  TEST_c_c (conj, nan_value, nan_value, nan_value, nan_value);
  TEST_c_c (conj, plus_infty, minus_infty, plus_infty, plus_infty);
  TEST_c_c (conj, plus_infty, plus_infty, plus_infty, minus_infty);
  TEST_c_c (conj, 1.0, 2.0, 1.0, -2.0);
  TEST_c_c (conj, 3.0, -4.0, 3.0, 4.0);

  END (conj, complex);
}


static void
copysign_test (void)
{
  START (copysign);

  TEST_ff_f (copysign, 0, 4, 0);
  TEST_ff_f (copysign, 0, -4, minus_zero);
  TEST_ff_f (copysign, minus_zero, 4, 0);
  TEST_ff_f (copysign, minus_zero, -4, minus_zero);

  TEST_ff_f (copysign, plus_infty, 0, plus_infty);
  TEST_ff_f (copysign, plus_infty, minus_zero, minus_infty);
  TEST_ff_f (copysign, minus_infty, 0, plus_infty);
  TEST_ff_f (copysign, minus_infty, minus_zero, minus_infty);

  TEST_ff_f (copysign, 0, plus_infty, 0);
  TEST_ff_f (copysign, 0, minus_zero, minus_zero);
  TEST_ff_f (copysign, minus_zero, plus_infty, 0);
  TEST_ff_f (copysign, minus_zero, minus_zero, minus_zero);

  /* XXX More correctly we would have to check the sign of the NaN.  */
  TEST_ff_f (copysign, nan_value, 0, nan_value);
  TEST_ff_f (copysign, nan_value, minus_zero, nan_value);
  TEST_ff_f (copysign, -nan_value, 0, nan_value);
  TEST_ff_f (copysign, -nan_value, minus_zero, nan_value);

  END (copysign);
}


static void
cos_test (void)
{
  errno = 0;
  FUNC(cos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cos);

  TEST_f_f (cos, 0, 1);
  TEST_f_f (cos, minus_zero, 1);
  errno = 0;
  TEST_f_f (cos, plus_infty, nan_value, INVALID_EXCEPTION);
  check_int ("errno for cos(+inf) == EDOM", errno, EDOM, 0, 0, 0);
  errno = 0;
  TEST_f_f (cos, minus_infty, nan_value, INVALID_EXCEPTION);
  check_int ("errno for cos(-inf) == EDOM", errno, EDOM, 0, 0, 0);
  errno = 0;
  TEST_f_f (cos, nan_value, nan_value);
  check_int ("errno for cos(NaN) unchanged", errno, 0, 0, 0, 0);

  TEST_f_f (cos, M_PI_6l * 2.0, 0.5);
  TEST_f_f (cos, M_PI_6l * 4.0, -0.5);
  TEST_f_f (cos, M_PI_2l, 0);

  TEST_f_f (cos, 0.75L, 0.731688868873820886311838753000084544L);

  TEST_f_f (cos, 0x1p65, 0.99888622066058013610642172179340364209972L);
  TEST_f_f (cos, -0x1p65, 0.99888622066058013610642172179340364209972L);

#ifdef TEST_DOUBLE
  TEST_f_f (cos, 0.80190127184058835, 0.69534156199418473);
#endif

#ifndef TEST_FLOAT
  TEST_f_f (cos, 1e22, 0.5232147853951389454975944733847094921409L);
  TEST_f_f (cos, 0x1p1023, -0.826369834614147994500785680811743734805L);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_f_f (cos, 0x1p16383L, 0.9210843909921906206874509522505756251609L);
#endif

  END (cos);
}


static void
cos_test_tonearest (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(cos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cos_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_f_f (cos, 1, 0.5403023058681397174009366074429766037323L);
      TEST_f_f (cos, 2, -0.4161468365471423869975682295007621897660L);
      TEST_f_f (cos, 3, -0.9899924966004454572715727947312613023937L);
      TEST_f_f (cos, 4, -0.6536436208636119146391681830977503814241L);
      TEST_f_f (cos, 5, 0.2836621854632262644666391715135573083344L);
      TEST_f_f (cos, 6, 0.9601702866503660205456522979229244054519L);
      TEST_f_f (cos, 7, 0.7539022543433046381411975217191820122183L);
      TEST_f_f (cos, 8, -0.1455000338086135258688413818311946826093L);
      TEST_f_f (cos, 9, -0.9111302618846769883682947111811653112463L);
      TEST_f_f (cos, 10, -0.8390715290764524522588639478240648345199L);
    }

  fesetround (save_round_mode);

  END (cos_tonearest);
}


static void
cos_test_towardzero (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(cos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cos_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_f_f (cos, 1, 0.5403023058681397174009366074429766037323L);
      TEST_f_f (cos, 2, -0.4161468365471423869975682295007621897660L);
      TEST_f_f (cos, 3, -0.9899924966004454572715727947312613023937L);
      TEST_f_f (cos, 4, -0.6536436208636119146391681830977503814241L);
      TEST_f_f (cos, 5, 0.2836621854632262644666391715135573083344L);
      TEST_f_f (cos, 6, 0.9601702866503660205456522979229244054519L);
      TEST_f_f (cos, 7, 0.7539022543433046381411975217191820122183L);
      TEST_f_f (cos, 8, -0.1455000338086135258688413818311946826093L);
      TEST_f_f (cos, 9, -0.9111302618846769883682947111811653112463L);
      TEST_f_f (cos, 10, -0.8390715290764524522588639478240648345199L);
    }

  fesetround (save_round_mode);

  END (cos_towardzero);
}


static void
cos_test_downward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(cos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cos_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_f_f (cos, 1, 0.5403023058681397174009366074429766037323L);
      TEST_f_f (cos, 2, -0.4161468365471423869975682295007621897660L);
      TEST_f_f (cos, 3, -0.9899924966004454572715727947312613023937L);
      TEST_f_f (cos, 4, -0.6536436208636119146391681830977503814241L);
      TEST_f_f (cos, 5, 0.2836621854632262644666391715135573083344L);
      TEST_f_f (cos, 6, 0.9601702866503660205456522979229244054519L);
      TEST_f_f (cos, 7, 0.7539022543433046381411975217191820122183L);
      TEST_f_f (cos, 8, -0.1455000338086135258688413818311946826093L);
      TEST_f_f (cos, 9, -0.9111302618846769883682947111811653112463L);
      TEST_f_f (cos, 10, -0.8390715290764524522588639478240648345199L);
    }

  fesetround (save_round_mode);

  END (cos_downward);
}


static void
cos_test_upward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(cos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cos_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_f_f (cos, 1, 0.5403023058681397174009366074429766037323L);
      TEST_f_f (cos, 2, -0.4161468365471423869975682295007621897660L);
      TEST_f_f (cos, 3, -0.9899924966004454572715727947312613023937L);
      TEST_f_f (cos, 4, -0.6536436208636119146391681830977503814241L);
      TEST_f_f (cos, 5, 0.2836621854632262644666391715135573083344L);
      TEST_f_f (cos, 6, 0.9601702866503660205456522979229244054519L);
      TEST_f_f (cos, 7, 0.7539022543433046381411975217191820122183L);
      TEST_f_f (cos, 8, -0.1455000338086135258688413818311946826093L);
      TEST_f_f (cos, 9, -0.9111302618846769883682947111811653112463L);
      TEST_f_f (cos, 10, -0.8390715290764524522588639478240648345199L);
    }

  fesetround (save_round_mode);

  END (cos_upward);
}


static void
cosh_test (void)
{
  errno = 0;
  FUNC(cosh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cosh);
  TEST_f_f (cosh, 0, 1);
  TEST_f_f (cosh, minus_zero, 1);

#ifndef TEST_INLINE
  TEST_f_f (cosh, plus_infty, plus_infty);
  TEST_f_f (cosh, minus_infty, plus_infty);
#endif
  TEST_f_f (cosh, nan_value, nan_value);

  TEST_f_f (cosh, 0.75L, 1.29468328467684468784170818539018176L);

  END (cosh);
}


static void
cosh_test_tonearest (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(cosh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cosh_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_f_f (cosh, 22, 1792456423.065795780980053377632656584997L);
      TEST_f_f (cosh, 23, 4872401723.124451300068625740569997090344L);
      TEST_f_f (cosh, 24, 13244561064.92173614708845674912733665919L);
    }

  fesetround (save_round_mode);

  END (cosh_tonearest);
}


static void
cosh_test_towardzero (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(cosh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cosh_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_f_f (cosh, 22, 1792456423.065795780980053377632656584997L);
      TEST_f_f (cosh, 23, 4872401723.124451300068625740569997090344L);
      TEST_f_f (cosh, 24, 13244561064.92173614708845674912733665919L);
    }

  fesetround (save_round_mode);

  END (cosh_towardzero);
}


static void
cosh_test_downward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(cosh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cosh_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_f_f (cosh, 22, 1792456423.065795780980053377632656584997L);
      TEST_f_f (cosh, 23, 4872401723.124451300068625740569997090344L);
      TEST_f_f (cosh, 24, 13244561064.92173614708845674912733665919L);
    }

  fesetround (save_round_mode);

  END (cosh_downward);
}


static void
cosh_test_upward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(cosh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cosh_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_f_f (cosh, 22, 1792456423.065795780980053377632656584997L);
      TEST_f_f (cosh, 23, 4872401723.124451300068625740569997090344L);
      TEST_f_f (cosh, 24, 13244561064.92173614708845674912733665919L);
    }

  fesetround (save_round_mode);

  END (cosh_upward);
}


static void
cpow_test (void)
{
  errno = 0;
  FUNC(cpow) (BUILD_COMPLEX (1, 0), BUILD_COMPLEX (0, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cpow);

  TEST_cc_c (cpow, 1, 0, 0, 0, 1.0, 0.0);
  TEST_cc_c (cpow, 2, 0, 10, 0, 1024.0, 0.0);

  TEST_cc_c (cpow, M_El, 0, 0, 2 * M_PIl, 1.0, 0.0);
  TEST_cc_c (cpow, 2, 3, 4, 0, -119.0, -120.0);

  TEST_cc_c (cpow, nan_value, nan_value, nan_value, nan_value, nan_value, nan_value);

  TEST_cc_c (cpow, 0.75L, 1.25L, 0.75L, 1.25L, 0.117506293914473555420279832210420483L, 0.346552747708338676483025352060418001L);
  TEST_cc_c (cpow, 0.75L, 1.25L, 1.0L, 1.0L, 0.0846958290317209430433805274189191353L, 0.513285749182902449043287190519090481L);
  TEST_cc_c (cpow, 0.75L, 1.25L, 1.0L, 0.0L, 0.75L, 1.25L);
  TEST_cc_c (cpow, 0.75L, 1.25L, 0.0L, 1.0L, 0.331825439177608832276067945276730566L, 0.131338600281188544930936345230903032L);

  END (cpow, complex);
}


static void
cproj_test (void)
{
  START (cproj);
  TEST_c_c (cproj, 0.0, 0.0, 0.0, 0.0);
  TEST_c_c (cproj, minus_zero, minus_zero, minus_zero, minus_zero);
  TEST_c_c (cproj, 0.0, minus_zero, 0.0, minus_zero);
  TEST_c_c (cproj, minus_zero, 0.0, minus_zero, 0.0);

  TEST_c_c (cproj, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (cproj, plus_infty, plus_infty, plus_infty, 0.0);
  TEST_c_c (cproj, plus_infty, minus_infty, plus_infty, minus_zero);
  TEST_c_c (cproj, minus_infty, plus_infty, plus_infty, 0.0);
  TEST_c_c (cproj, minus_infty, minus_infty, plus_infty, minus_zero);

  TEST_c_c (cproj, 1.0, 0.0, 1.0, 0.0);
  TEST_c_c (cproj, 2.0, 3.0, 2.0, 3.0);

  END (cproj, complex);
}


static void
creal_test (void)
{
  START (creal);
  TEST_c_f (creal, 0.0, 1.0, 0.0);
  TEST_c_f (creal, minus_zero, 1.0, minus_zero);
  TEST_c_f (creal, nan_value, 1.0, nan_value);
  TEST_c_f (creal, nan_value, nan_value, nan_value);
  TEST_c_f (creal, plus_infty, 1.0, plus_infty);
  TEST_c_f (creal, minus_infty, 1.0, minus_infty);
  TEST_c_f (creal, 2.0, 3.0, 2.0);

  END (creal);
}

static void
csin_test (void)
{
  errno = 0;
  FUNC(csin) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (csin);

  TEST_c_c (csin, 0.0, 0.0, 0.0, 0.0);
  TEST_c_c (csin, minus_zero, 0.0, minus_zero, 0.0);
  TEST_c_c (csin, 0.0, minus_zero, 0, minus_zero);
  TEST_c_c (csin, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (csin, 0.0, plus_infty, 0.0, plus_infty);
  TEST_c_c (csin, minus_zero, plus_infty, minus_zero, plus_infty);
  TEST_c_c (csin, 0.0, minus_infty, 0.0, minus_infty);
  TEST_c_c (csin, minus_zero, minus_infty, minus_zero, minus_infty);

  TEST_c_c (csin, plus_infty, 0.0, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, minus_infty, 0.0, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, plus_infty, minus_zero, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, minus_infty, minus_zero, nan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csin, plus_infty, plus_infty, nan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, minus_infty, plus_infty, nan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, plus_infty, minus_infty, nan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, minus_infty, minus_infty, nan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csin, plus_infty, 6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csin, plus_infty, -6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csin, minus_infty, 6.75, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csin, minus_infty, -6.75,  nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (csin, 4.625, plus_infty, minus_infty, minus_infty);
  TEST_c_c (csin, 4.625, minus_infty, minus_infty, plus_infty);
  TEST_c_c (csin, -4.625, plus_infty, plus_infty, minus_infty);
  TEST_c_c (csin, -4.625, minus_infty, plus_infty, plus_infty);

  TEST_c_c (csin, nan_value, 0.0, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, nan_value, minus_zero, nan_value, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csin, nan_value, plus_infty, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csin, nan_value, minus_infty, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csin, nan_value, 9.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csin, nan_value, -9.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csin, 0.0, nan_value, 0.0, nan_value);
  TEST_c_c (csin, minus_zero, nan_value, minus_zero, nan_value);

  TEST_c_c (csin, 10.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csin, nan_value, -10.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csin, plus_infty, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csin, minus_infty, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csin, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (csin, 0.75L, 1.25L, 1.28722291002649188575873510790565441L, 1.17210635989270256101081285116138863L);
  TEST_c_c (csin, -2, -3, -9.15449914691142957346729954460983256L, 4.16890695996656435075481305885375484L);

  END (csin, complex);
}


static void
csinh_test (void)
{
  errno = 0;
  FUNC(csinh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (csinh);

  TEST_c_c (csinh, 0.0, 0.0, 0.0, 0.0);
  TEST_c_c (csinh, minus_zero, 0.0, minus_zero, 0.0);
  TEST_c_c (csinh, 0.0, minus_zero, 0.0, minus_zero);
  TEST_c_c (csinh, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (csinh, 0.0, plus_infty, 0.0, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_zero, plus_infty, 0.0, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, 0.0, minus_infty, 0.0, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_zero, minus_infty, 0.0, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csinh, plus_infty, 0.0, plus_infty, 0.0);
  TEST_c_c (csinh, minus_infty, 0.0, minus_infty, 0.0);
  TEST_c_c (csinh, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (csinh, minus_infty, minus_zero, minus_infty, minus_zero);

  TEST_c_c (csinh, plus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, plus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csinh, plus_infty, 4.625, minus_infty, minus_infty);
  TEST_c_c (csinh, minus_infty, 4.625, plus_infty, minus_infty);
  TEST_c_c (csinh, plus_infty, -4.625, minus_infty, plus_infty);
  TEST_c_c (csinh, minus_infty, -4.625, plus_infty, plus_infty);

  TEST_c_c (csinh, 6.75, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csinh, -6.75, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csinh, 6.75, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (csinh, -6.75, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (csinh, 0.0, nan_value, 0.0, nan_value, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_zero, nan_value, 0.0, nan_value, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csinh, plus_infty, nan_value, plus_infty, nan_value, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (csinh, minus_infty, nan_value, plus_infty, nan_value, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csinh, 9.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csinh, -9.0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csinh, nan_value, 0.0, nan_value, 0.0);
  TEST_c_c (csinh, nan_value, minus_zero, nan_value, minus_zero);

  TEST_c_c (csinh, nan_value, 10.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csinh, nan_value, -10.0, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csinh, nan_value, plus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csinh, nan_value, minus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csinh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (csinh, 0.75L, 1.25L, 0.259294854551162779153349830618433028L, 1.22863452409509552219214606515777594L);
  TEST_c_c (csinh, -2, -3, 3.59056458998577995201256544779481679L, -0.530921086248519805267040090660676560L);

  END (csinh, complex);
}


static void
csqrt_test (void)
{
  errno = 0;
  FUNC(csqrt) (BUILD_COMPLEX (-1, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (csqrt);

  TEST_c_c (csqrt, 0, 0, 0.0, 0.0);
  TEST_c_c (csqrt, 0, minus_zero, 0, minus_zero);
  TEST_c_c (csqrt, minus_zero, 0, 0.0, 0.0);
  TEST_c_c (csqrt, minus_zero, minus_zero, 0.0, minus_zero);

  TEST_c_c (csqrt, minus_infty, 0, 0.0, plus_infty);
  TEST_c_c (csqrt, minus_infty, 6, 0.0, plus_infty);
  TEST_c_c (csqrt, minus_infty, minus_zero, 0.0, minus_infty);
  TEST_c_c (csqrt, minus_infty, -6, 0.0, minus_infty);

  TEST_c_c (csqrt, plus_infty, 0, plus_infty, 0.0);
  TEST_c_c (csqrt, plus_infty, 6, plus_infty, 0.0);
  TEST_c_c (csqrt, plus_infty, minus_zero, plus_infty, minus_zero);
  TEST_c_c (csqrt, plus_infty, -6, plus_infty, minus_zero);

  TEST_c_c (csqrt, 0, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, 4, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, plus_infty, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, minus_zero, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, -4, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, minus_infty, plus_infty, plus_infty, plus_infty);
  TEST_c_c (csqrt, 0, minus_infty, plus_infty, minus_infty);
  TEST_c_c (csqrt, 4, minus_infty, plus_infty, minus_infty);
  TEST_c_c (csqrt, plus_infty, minus_infty, plus_infty, minus_infty);
  TEST_c_c (csqrt, minus_zero, minus_infty, plus_infty, minus_infty);
  TEST_c_c (csqrt, -4, minus_infty, plus_infty, minus_infty);
  TEST_c_c (csqrt, minus_infty, minus_infty, plus_infty, minus_infty);

  TEST_c_c (csqrt, minus_infty, nan_value, nan_value, plus_infty, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (csqrt, plus_infty, nan_value, plus_infty, nan_value);

  TEST_c_c (csqrt, 0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, 1, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, minus_zero, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, -1, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csqrt, nan_value, 0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, nan_value, 8, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, nan_value, minus_zero, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (csqrt, nan_value, -8, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (csqrt, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (csqrt, 16.0, -30.0, 5.0, -3.0);
  TEST_c_c (csqrt, -1, 0, 0.0, 1.0);
  TEST_c_c (csqrt, 0, 2, 1.0, 1.0);
  TEST_c_c (csqrt, 119, 120, 12.0, 5.0);
  TEST_c_c (csqrt, 0.75L, 1.25L, 1.05065169626078392338656675760808326L, 0.594868882070379067881984030639932657L);
  TEST_c_c (csqrt, -2, -3, 0.89597747612983812471573375529004348L, -1.6741492280355400404480393008490519L);
  TEST_c_c (csqrt, -2, 3, 0.89597747612983812471573375529004348L, 1.6741492280355400404480393008490519L);
  /* Principal square root should be returned (i.e., non-negative real
     part).  */
  TEST_c_c (csqrt, 0, -1, M_SQRT_2_2, -M_SQRT_2_2);

  TEST_c_c (csqrt, 0x1.fffffep+127L, 0x1.fffffep+127L, 2.026714405498316804978751017492482558075e+19L, 8.394925938143272988211878516208015586281e+18L);
  TEST_c_c (csqrt, 0x1.fffffep+127L, 1.0L, 1.844674352395372953599975585936590505260e+19L, 2.710505511993121390769065968615872097053e-20L);
  TEST_c_c (csqrt, 0x1p-149L, 0x1p-149L, 4.112805464342778798097003462770175200803e-23L, 1.703579802732953750368659735601389709551e-23L);
  TEST_c_c (csqrt, 0x1p-147L, 0x1p-147L, 8.225610928685557596194006925540350401606e-23L, 3.407159605465907500737319471202779419102e-23L);

#ifndef TEST_FLOAT
  TEST_c_c (csqrt, 0x1.fffffffffffffp+1023L, 0x1.fffffffffffffp+1023L, 1.473094556905565378990473658199034571917e+154L, 6.101757441282702188537080005372547713595e+153L);
  TEST_c_c (csqrt, 0x1.fffffffffffffp+1023L, 0x1p+1023L, 1.379778091031440685006200821918878702861e+154L, 3.257214233483129514781233066898042490248e+153L);
  TEST_c_c (csqrt, 0x1p-1074L, 0x1p-1074L, 2.442109726130830256743814843868934877597e-162L, 1.011554969366634726113090867589031782487e-162L);
  TEST_c_c (csqrt, 0x1p-1073L, 0x1p-1073L, 3.453664695497464982856905711457966660085e-162L, 1.430554756764195530630723976279903095110e-162L);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_c_c (csqrt, 0x1.fp+16383L, 0x1.fp+16383L, 1.179514222452201722651836720466795901016e+2466L, 4.885707879516577666702435054303191575148e+2465L);
  TEST_c_c (csqrt, 0x1.fp+16383L, 0x1p+16383L, 1.106698967236475180613254276996359485630e+2466L, 2.687568007603946993388538156299100955642e+2465L);
  TEST_c_c (csqrt, 0x1p-16440L, 0x1p-16441L, 3.514690655930285351254618340783294558136e-2475L,  8.297059146828716918029689466551384219370e-2476L);
#endif

  END (csqrt, complex);
}

static void
ctan_test (void)
{
  errno = 0;
  FUNC(ctan) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctan);

  TEST_c_c (ctan, 0, 0, 0.0, 0.0);
  TEST_c_c (ctan, 0, minus_zero, 0.0, minus_zero);
  TEST_c_c (ctan, minus_zero, 0, minus_zero, 0.0);
  TEST_c_c (ctan, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (ctan, 0, plus_infty, 0.0, 1.0);
  TEST_c_c (ctan, 1, plus_infty, 0.0, 1.0);
  TEST_c_c (ctan, minus_zero, plus_infty, minus_zero, 1.0);
  TEST_c_c (ctan, -1, plus_infty, minus_zero, 1.0);

  TEST_c_c (ctan, 0, minus_infty, 0.0, -1.0);
  TEST_c_c (ctan, 1, minus_infty, 0.0, -1.0);
  TEST_c_c (ctan, minus_zero, minus_infty, minus_zero, -1.0);
  TEST_c_c (ctan, -1, minus_infty, minus_zero, -1.0);

  TEST_c_c (ctan, plus_infty, 0, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, plus_infty, 2, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, minus_infty, 0, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, minus_infty, 2, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, plus_infty, minus_zero, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, plus_infty, -2, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, minus_infty, minus_zero, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctan, minus_infty, -2, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ctan, nan_value, plus_infty, 0.0, 1.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ctan, nan_value, minus_infty, 0.0, -1.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ctan, 0, nan_value, 0.0, nan_value);
  TEST_c_c (ctan, minus_zero, nan_value, minus_zero, nan_value);

  TEST_c_c (ctan, 0.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctan, -4.5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ctan, nan_value, 0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctan, nan_value, 5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctan, nan_value, minus_zero, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctan, nan_value, -0.25, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ctan, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (ctan, 0.75L, 1.25L, 0.160807785916206426725166058173438663L, 0.975363285031235646193581759755216379L);
  TEST_c_c (ctan, -2, -3, 0.376402564150424829275122113032269084e-2L, -1.00323862735360980144635859782192726L);

  TEST_c_c (ctan, 1, 45, 1.490158918874345552942703234806348520895e-39L, 1.000000000000000000000000000000000000001L);
  TEST_c_c (ctan, 1, 47, 2.729321264492904590777293425576722354636e-41L, 1.0);

#ifndef TEST_FLOAT
  TEST_c_c (ctan, 1, 355, 8.140551093483276762350406321792653551513e-309L, 1.0);
  TEST_c_c (ctan, 1, 365, 1.677892637497921890115075995898773550884e-317L, 1.0);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_c_c (ctan, 1, 5680, 4.725214596136812019616700920476949798307e-4934L, 1.0);
  TEST_c_c (ctan, 1, 5690, 9.739393181626937151720816611272607059057e-4943L, 1.0);
#endif

  TEST_c_c (ctan, 0x3.243f6cp-1, 0, -2.287733242885645987394874673945769518150e7L, 0.0);

  TEST_c_c (ctan, 0x1p127, 1, 0.2446359391192790896381501310437708987204L, 0.9101334047676183761532873794426475906201L);

#ifndef TEST_FLOAT
  TEST_c_c (ctan, 0x1p1023, 1, -0.2254627924997545057926782581695274244229L, 0.8786063118883068695462540226219865087189L);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_c_c (ctan, 0x1p16383L, 1, 0.1608598776370396607204448234354670036772L, 0.8133818522051542536316746743877629761488L);
#endif

  TEST_c_c (ctan, 50000, 50000, plus_zero, 1.0);
  TEST_c_c (ctan, 50000, -50000, plus_zero, -1.0);
  TEST_c_c (ctan, -50000, 50000, minus_zero, 1.0);
  TEST_c_c (ctan, -50000, -50000, minus_zero, -1.0);

  END (ctan, complex);
}


static void
ctanh_test (void)
{
  errno = 0;
  FUNC(ctanh) (BUILD_COMPLEX (0, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctanh);

  TEST_c_c (ctanh, 0, 0, 0.0, 0.0);
  TEST_c_c (ctanh, 0, minus_zero, 0.0, minus_zero);
  TEST_c_c (ctanh, minus_zero, 0, minus_zero, 0.0);
  TEST_c_c (ctanh, minus_zero, minus_zero, minus_zero, minus_zero);

  TEST_c_c (ctanh, plus_infty, 0, 1.0, 0.0);
  TEST_c_c (ctanh, plus_infty, 1, 1.0, 0.0);
  TEST_c_c (ctanh, plus_infty, minus_zero, 1.0, minus_zero);
  TEST_c_c (ctanh, plus_infty, -1, 1.0, minus_zero);
  TEST_c_c (ctanh, minus_infty, 0, -1.0, 0.0);
  TEST_c_c (ctanh, minus_infty, 1, -1.0, 0.0);
  TEST_c_c (ctanh, minus_infty, minus_zero, -1.0, minus_zero);
  TEST_c_c (ctanh, minus_infty, -1, -1.0, minus_zero);

  TEST_c_c (ctanh, 0, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, 2, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, 0, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, 2, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, minus_zero, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, -2, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, minus_zero, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_c_c (ctanh, -2, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);

  TEST_c_c (ctanh, plus_infty, nan_value, 1.0, 0.0, IGNORE_ZERO_INF_SIGN);
  TEST_c_c (ctanh, minus_infty, nan_value, -1.0, 0.0, IGNORE_ZERO_INF_SIGN);

  TEST_c_c (ctanh, nan_value, 0, nan_value, 0.0);
  TEST_c_c (ctanh, nan_value, minus_zero, nan_value, minus_zero);

  TEST_c_c (ctanh, nan_value, 0.5, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctanh, nan_value, -4.5, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ctanh, 0, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctanh, 5, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctanh, minus_zero, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_c_c (ctanh, -0.25, nan_value, nan_value, nan_value, INVALID_EXCEPTION_OK);

  TEST_c_c (ctanh, nan_value, nan_value, nan_value, nan_value);

  TEST_c_c (ctanh, 0, M_PI_4l, 0.0, 1.0);

  TEST_c_c (ctanh, 0.75L, 1.25L, 1.37260757053378320258048606571226857L, 0.385795952609750664177596760720790220L);
  TEST_c_c (ctanh, -2, -3, -0.965385879022133124278480269394560686L, 0.988437503832249372031403430350121098e-2L);

  TEST_c_c (ctanh, 45, 1, 1.000000000000000000000000000000000000001L, 1.490158918874345552942703234806348520895e-39L);
  TEST_c_c (ctanh, 47, 1, 1.0, 2.729321264492904590777293425576722354636e-41L);

#ifndef TEST_FLOAT
  TEST_c_c (ctanh, 355, 1, 1.0, 8.140551093483276762350406321792653551513e-309L);
  TEST_c_c (ctanh, 365, 1, 1.0, 1.677892637497921890115075995898773550884e-317L);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_c_c (ctanh, 5680, 1, 1.0, 4.725214596136812019616700920476949798307e-4934L);
  TEST_c_c (ctanh, 5690, 1, 1.0, 9.739393181626937151720816611272607059057e-4943L);
#endif

  TEST_c_c (ctanh, 0, 0x3.243f6cp-1, 0.0, -2.287733242885645987394874673945769518150e7L);

  TEST_c_c (ctanh, 1, 0x1p127, 0.9101334047676183761532873794426475906201L, 0.2446359391192790896381501310437708987204L);

#ifndef TEST_FLOAT
  TEST_c_c (ctanh, 1, 0x1p1023, 0.8786063118883068695462540226219865087189L, -0.2254627924997545057926782581695274244229L);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_c_c (ctanh, 1, 0x1p16383L, 0.8133818522051542536316746743877629761488L, 0.1608598776370396607204448234354670036772L);
#endif

  TEST_c_c (ctanh, 50000, 50000, 1.0, plus_zero);
  TEST_c_c (ctanh, 50000, -50000, 1.0, minus_zero);
  TEST_c_c (ctanh, -50000, 50000, -1.0, plus_zero);
  TEST_c_c (ctanh, -50000, -50000, -1.0, minus_zero);

  END (ctanh, complex);
}


static void
erf_test (void)
{
  errno = 0;
  FUNC(erf) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (erf);

  TEST_f_f (erf, 0, 0);
  TEST_f_f (erf, minus_zero, minus_zero);
  TEST_f_f (erf, plus_infty, 1);
  TEST_f_f (erf, minus_infty, -1);
  TEST_f_f (erf, nan_value, nan_value);

  TEST_f_f (erf, 0.125L, 0.140316204801333817393029446521623398L);
  TEST_f_f (erf, 0.75L, 0.711155633653515131598937834591410777L);
  TEST_f_f (erf, 1.25L, 0.922900128256458230136523481197281140L);
  TEST_f_f (erf, 2.0L, 0.995322265018952734162069256367252929L);
  TEST_f_f (erf, 4.125L, 0.999999994576599200434933994687765914L);
  TEST_f_f (erf, 27.0L, 1.0L);

  END (erf);
}


static void
erfc_test (void)
{
  errno = 0;
  FUNC(erfc) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (erfc);

  TEST_f_f (erfc, plus_infty, 0.0);
  TEST_f_f (erfc, minus_infty, 2.0);
  TEST_f_f (erfc, 0.0, 1.0);
  TEST_f_f (erfc, minus_zero, 1.0);
  TEST_f_f (erfc, nan_value, nan_value);

  TEST_f_f (erfc, 0.125L, 0.859683795198666182606970553478376602L);
  TEST_f_f (erfc, 0.75L, 0.288844366346484868401062165408589223L);
  TEST_f_f (erfc, 1.25L, 0.0770998717435417698634765188027188596L);
  TEST_f_f (erfc, 2.0L, 0.00467773498104726583793074363274707139L);
  TEST_f_f (erfc, 0x1.f7303cp+1L, 2.705500297238986897105236321218861842255e-8L);
  TEST_f_f (erfc, 4.125L, 0.542340079956506600531223408575531062e-8L);
  TEST_f_f (erfc, 0x1.ffa002p+2L, 1.233585992097580296336099501489175967033e-29L);
  TEST_f_f (erfc, 0x1.ffffc8p+2L, 1.122671365033056305522366683719541099329e-29L);
#ifdef TEST_LDOUBLE
  /* The result can only be represented in long double.  */
# if LDBL_MIN_10_EXP < -319
  TEST_f_f (erfc, 27.0L, 0.523704892378925568501606768284954709e-318L);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_f_f (erfc, 0x1.ffff56789abcdef0123456789a8p+2L, 1.123161416304655390092138725253789378459e-29L);
# endif
#endif

  END (erfc);
}


static void
exp_test (void)
{
  errno = 0;
  FUNC(exp) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp);

  TEST_f_f (exp, 0, 1);
  TEST_f_f (exp, minus_zero, 1);

#ifndef TEST_INLINE
  TEST_f_f (exp, plus_infty, plus_infty);
  TEST_f_f (exp, minus_infty, 0);
#endif
  TEST_f_f (exp, nan_value, nan_value);
  TEST_f_f (exp, 1, M_El);

  TEST_f_f (exp, 2, M_E2l);
  TEST_f_f (exp, 3, M_E3l);
  TEST_f_f (exp, 0.75L, 2.11700001661267466854536981983709561L);
  TEST_f_f (exp, 50.0L, 5184705528587072464087.45332293348538L);
  TEST_f_f (exp, 88.72269439697265625L, 3.40233126623160774937554134772290447915e38L);
#if defined TEST_LDOUBLE && __LDBL_MAX_EXP__ > 1024
  /* The result can only be represented in sane long double.  */
  TEST_f_f (exp, 1000.0L, 0.197007111401704699388887935224332313e435L);
#endif

  /* Bug 13922: OVERFLOW exception may be missing.  */
  TEST_f_f (exp, max_value, plus_infty, OVERFLOW_EXCEPTION_OK);
  TEST_f_f (exp, -max_value, 0);

  END (exp);
}


static void
exp_test_tonearest (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(exp) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_f_f (exp, 1, M_El);
      TEST_f_f (exp, 2, M_E2l);
      TEST_f_f (exp, 3, M_E3l);
    }

  fesetround (save_round_mode);

  END (exp_tonearest);
}


static void
exp_test_towardzero (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(exp) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_f_f (exp, 1, M_El);
      TEST_f_f (exp, 2, M_E2l);
      TEST_f_f (exp, 3, M_E3l);
    }

  fesetround (save_round_mode);

  END (exp_towardzero);
}


static void
exp_test_downward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(exp) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_f_f (exp, 1, M_El);
      TEST_f_f (exp, 2, M_E2l);
      TEST_f_f (exp, 3, M_E3l);
    }

  fesetround (save_round_mode);

  END (exp_downward);
}


static void
exp_test_upward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(exp) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_f_f (exp, 1, M_El);
      TEST_f_f (exp, 2, M_E2l);
      TEST_f_f (exp, 3, M_E3l);
    }

  fesetround (save_round_mode);

  END (exp_upward);
}


static void
exp10_test (void)
{
  errno = 0;
  FUNC(exp10) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp10);

  TEST_f_f (exp10, 0, 1);
  TEST_f_f (exp10, minus_zero, 1);

  TEST_f_f (exp10, plus_infty, plus_infty);
  TEST_f_f (exp10, minus_infty, 0);
  TEST_f_f (exp10, nan_value, nan_value);
  TEST_f_f (exp10, 3, 1000);
  TEST_f_f (exp10, -1, 0.1L);
#ifdef TEST_FLOAT /* Bug 13884: inaccurate results except for float.  */
  TEST_f_f (exp10, 36, 1.0e36L);
#endif
  TEST_f_f (exp10, 1e6, plus_infty, OVERFLOW_EXCEPTION);
  TEST_f_f (exp10, -1e6, 0);
#ifndef TEST_LDOUBLE /* Bug 13914: spurious exceptions.  */
  TEST_f_f (exp10, max_value, plus_infty, OVERFLOW_EXCEPTION);
  TEST_f_f (exp10, -max_value, 0);
#endif
  TEST_f_f (exp10, 0.75L, 5.62341325190349080394951039776481231L);

  END (exp10);
}


static void
exp2_test (void)
{
  errno = 0;
  FUNC(exp2) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp2);

  TEST_f_f (exp2, 0, 1);
  TEST_f_f (exp2, minus_zero, 1);
  TEST_f_f (exp2, plus_infty, plus_infty);
  TEST_f_f (exp2, minus_infty, 0);
  TEST_f_f (exp2, nan_value, nan_value);

  TEST_f_f (exp2, 10, 1024);
  TEST_f_f (exp2, -1, 0.5);
  TEST_f_f (exp2, 1e6, plus_infty, OVERFLOW_EXCEPTION);
  TEST_f_f (exp2, -1e6, 0);
  TEST_f_f (exp2, max_value, plus_infty, OVERFLOW_EXCEPTION);
  TEST_f_f (exp2, -max_value, 0);
  TEST_f_f (exp2, 0.75L, 1.68179283050742908606225095246642979L);

  TEST_f_f (exp2, 100.5, 1.792728671193156477399422023278661496394e+30L);
  TEST_f_f (exp2, 127, 0x1p127);
  TEST_f_f (exp2, -149, 0x1p-149);

#ifndef TEST_FLOAT
  TEST_f_f (exp2, 1000.25, 1.274245659452564874772384918171765416737e+301L);
  TEST_f_f (exp2, 1023, 0x1p1023);
  TEST_f_f (exp2, -1074, 0x1p-1074);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_f_f (exp2, 16383, 0x1p16383L);
  TEST_f_f (exp2, -16400, 0x1p-16400L);
#endif

  END (exp2);
}


static void
expm1_test (void)
{
  errno = 0;
  FUNC(expm1) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (expm1);

  TEST_f_f (expm1, 0, 0);
  TEST_f_f (expm1, minus_zero, minus_zero);

#ifndef TEST_INLINE
  TEST_f_f (expm1, plus_infty, plus_infty);
  TEST_f_f (expm1, minus_infty, -1);
#endif
  TEST_f_f (expm1, nan_value, nan_value);

  TEST_f_f (expm1, 1, M_El - 1.0);
  TEST_f_f (expm1, 0.75L, 1.11700001661267466854536981983709561L);

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_f_f (expm1, 11356.25L, 9.05128237311923300051376115753226014206e+4931L);
#endif

  errno = 0;
  /* Bug 13787: OVERFLOW exception may be missing.  */
  TEST_f_f (expm1, 100000.0, plus_infty, OVERFLOW_EXCEPTION_OK);
  check_int ("errno for expm1(large) == ERANGE", errno, ERANGE, 0, 0, 0);
  /* Bug 13787: OVERFLOW exception may be missing.  */
  TEST_f_f (expm1, max_value, plus_infty, OVERFLOW_EXCEPTION_OK);
#ifndef TEST_LDOUBLE /* Bug 13923.  */
  TEST_f_f (expm1, -max_value, -1);
#endif

  END (expm1);
}


static void
fabs_test (void)
{
  START (fabs);

  TEST_f_f (fabs, 0, 0);
  TEST_f_f (fabs, minus_zero, 0);

  TEST_f_f (fabs, plus_infty, plus_infty);
  TEST_f_f (fabs, minus_infty, plus_infty);
  TEST_f_f (fabs, nan_value, nan_value);

  TEST_f_f (fabs, 38.0, 38.0);
  TEST_f_f (fabs, -M_El, M_El);

  END (fabs);
}


static void
fdim_test (void)
{
  START (fdim);

  TEST_ff_f (fdim, 0, 0, 0);
  TEST_ff_f (fdim, 9, 0, 9);
  TEST_ff_f (fdim, 0, 9, 0);
  TEST_ff_f (fdim, -9, 0, 0);
  TEST_ff_f (fdim, 0, -9, 9);

  TEST_ff_f (fdim, plus_infty, 9, plus_infty);
  TEST_ff_f (fdim, plus_infty, -9, plus_infty);
  TEST_ff_f (fdim, minus_infty, 9, 0);
  TEST_ff_f (fdim, minus_infty, -9, 0);
  TEST_ff_f (fdim, 9, minus_infty, plus_infty);
  TEST_ff_f (fdim, -9, minus_infty, plus_infty);
  TEST_ff_f (fdim, 9, plus_infty, 0);
  TEST_ff_f (fdim, -9, plus_infty, 0);

  TEST_ff_f (fdim, 0, nan_value, nan_value);
  TEST_ff_f (fdim, 9, nan_value, nan_value);
  TEST_ff_f (fdim, -9, nan_value, nan_value);
  TEST_ff_f (fdim, nan_value, 9, nan_value);
  TEST_ff_f (fdim, nan_value, -9, nan_value);
  TEST_ff_f (fdim, plus_infty, nan_value, nan_value);
  TEST_ff_f (fdim, minus_infty, nan_value, nan_value);
  TEST_ff_f (fdim, nan_value, plus_infty, nan_value);
  TEST_ff_f (fdim, nan_value, minus_infty, nan_value);
  TEST_ff_f (fdim, nan_value, nan_value, nan_value);

  TEST_ff_f (fdim, plus_infty, plus_infty, 0);

  END (fdim);
}


static void
floor_test (void)
{
  START (floor);

  TEST_f_f (floor, 0.0, 0.0);
  TEST_f_f (floor, minus_zero, minus_zero);
  TEST_f_f (floor, plus_infty, plus_infty);
  TEST_f_f (floor, minus_infty, minus_infty);
  TEST_f_f (floor, nan_value, nan_value);

  TEST_f_f (floor, M_PIl, 3.0);
  TEST_f_f (floor, -M_PIl, -4.0);

  TEST_f_f (floor, 0.1, 0.0);
  TEST_f_f (floor, 0.25, 0.0);
  TEST_f_f (floor, 0.625, 0.0);
  TEST_f_f (floor, -0.1, -1.0);
  TEST_f_f (floor, -0.25, -1.0);
  TEST_f_f (floor, -0.625, -1.0);

#ifdef TEST_LDOUBLE
  /* The result can only be represented in long double.  */
  TEST_f_f (floor, 4503599627370495.5L, 4503599627370495.0L);
  TEST_f_f (floor, 4503599627370496.25L, 4503599627370496.0L);
  TEST_f_f (floor, 4503599627370496.5L, 4503599627370496.0L);
  TEST_f_f (floor, 4503599627370496.75L, 4503599627370496.0L);
  TEST_f_f (floor, 4503599627370497.5L, 4503599627370497.0L);
# if LDBL_MANT_DIG > 100
  TEST_f_f (floor, 4503599627370494.5000000000001L, 4503599627370494.0L);
  TEST_f_f (floor, 4503599627370495.5000000000001L, 4503599627370495.0L);
  TEST_f_f (floor, 4503599627370496.5000000000001L, 4503599627370496.0L);
# endif

  TEST_f_f (floor, -4503599627370495.5L, -4503599627370496.0L);
  TEST_f_f (floor, -4503599627370496.25L, -4503599627370497.0L);
  TEST_f_f (floor, -4503599627370496.5L, -4503599627370497.0L);
  TEST_f_f (floor, -4503599627370496.75L, -4503599627370497.0L);
  TEST_f_f (floor, -4503599627370497.5L, -4503599627370498.0L);
# if LDBL_MANT_DIG > 100
  TEST_f_f (floor, -4503599627370494.5000000000001L, -4503599627370495.0L);
  TEST_f_f (floor, -4503599627370495.5000000000001L, -4503599627370496.0L);
  TEST_f_f (floor, -4503599627370496.5000000000001L, -4503599627370497.0L);
# endif

  TEST_f_f (floor, 9007199254740991.5L, 9007199254740991.0L);
  TEST_f_f (floor, 9007199254740992.25L, 9007199254740992.0L);
  TEST_f_f (floor, 9007199254740992.5L, 9007199254740992.0L);
  TEST_f_f (floor, 9007199254740992.75L, 9007199254740992.0L);
  TEST_f_f (floor, 9007199254740993.5L, 9007199254740993.0L);

# if LDBL_MANT_DIG > 100
  TEST_f_f (floor, 9007199254740991.0000000000001L, 9007199254740991.0L);
  TEST_f_f (floor, 9007199254740992.0000000000001L, 9007199254740992.0L);
  TEST_f_f (floor, 9007199254740993.0000000000001L, 9007199254740993.0L);
  TEST_f_f (floor, 9007199254740991.5000000000001L, 9007199254740991.0L);
  TEST_f_f (floor, 9007199254740992.5000000000001L, 9007199254740992.0L);
  TEST_f_f (floor, 9007199254740993.5000000000001L, 9007199254740993.0L);
# endif

  TEST_f_f (floor, -9007199254740991.5L, -9007199254740992.0L);
  TEST_f_f (floor, -9007199254740992.25L, -9007199254740993.0L);
  TEST_f_f (floor, -9007199254740992.5L, -9007199254740993.0L);
  TEST_f_f (floor, -9007199254740992.75L, -9007199254740993.0L);
  TEST_f_f (floor, -9007199254740993.5L, -9007199254740994.0L);

# if LDBL_MANT_DIG > 100
  TEST_f_f (floor, -9007199254740991.0000000000001L, -9007199254740992.0L);
  TEST_f_f (floor, -9007199254740992.0000000000001L, -9007199254740993.0L);
  TEST_f_f (floor, -9007199254740993.0000000000001L, -9007199254740994.0L);
  TEST_f_f (floor, -9007199254740991.5000000000001L, -9007199254740992.0L);
  TEST_f_f (floor, -9007199254740992.5000000000001L, -9007199254740993.0L);
  TEST_f_f (floor, -9007199254740993.5000000000001L, -9007199254740994.0L);
# endif

  TEST_f_f (floor, 72057594037927935.5L, 72057594037927935.0L);
  TEST_f_f (floor, 72057594037927936.25L, 72057594037927936.0L);
  TEST_f_f (floor, 72057594037927936.5L, 72057594037927936.0L);
  TEST_f_f (floor, 72057594037927936.75L, 72057594037927936.0L);
  TEST_f_f (floor, 72057594037927937.5L, 72057594037927937.0L);

  TEST_f_f (floor, -72057594037927935.5L, -72057594037927936.0L);
  TEST_f_f (floor, -72057594037927936.25L, -72057594037927937.0L);
  TEST_f_f (floor, -72057594037927936.5L, -72057594037927937.0L);
  TEST_f_f (floor, -72057594037927936.75L, -72057594037927937.0L);
  TEST_f_f (floor, -72057594037927937.5L, -72057594037927938.0L);

  TEST_f_f (floor, 10141204801825835211973625643007.5L, 10141204801825835211973625643007.0L);
  TEST_f_f (floor, 10141204801825835211973625643008.25L, 10141204801825835211973625643008.0L);
  TEST_f_f (floor, 10141204801825835211973625643008.5L, 10141204801825835211973625643008.0L);
  TEST_f_f (floor, 10141204801825835211973625643008.75L, 10141204801825835211973625643008.0L);
  TEST_f_f (floor, 10141204801825835211973625643009.5L, 10141204801825835211973625643009.0L);

  TEST_f_f (floor, 0xf.ffffffffffffff8p+47L, 0xf.fffffffffffep+47L);
  TEST_f_f (floor, -0x8.000000000000004p+48L, -0x8.000000000001p+48L);
#endif

  END (floor);
}


static void
fma_test (void)
{
  START (fma);

  TEST_fff_f (fma, 1.0, 2.0, 3.0, 5.0);
  TEST_fff_f (fma, nan_value, 2.0, 3.0, nan_value);
  TEST_fff_f (fma, 1.0, nan_value, 3.0, nan_value);
  TEST_fff_f (fma, 1.0, 2.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_fff_f (fma, plus_infty, 0.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_fff_f (fma, minus_infty, 0.0, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_fff_f (fma, 0.0, plus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_fff_f (fma, 0.0, minus_infty, nan_value, nan_value, INVALID_EXCEPTION_OK);
  TEST_fff_f (fma, plus_infty, 0.0, 1.0, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, minus_infty, 0.0, 1.0, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, 0.0, plus_infty, 1.0, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, 0.0, minus_infty, 1.0, nan_value, INVALID_EXCEPTION);

  TEST_fff_f (fma, plus_infty, plus_infty, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, minus_infty, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, plus_infty, minus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, minus_infty, minus_infty, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, plus_infty, 3.5L, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, minus_infty, -7.5L, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, -13.5L, plus_infty, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_fff_f (fma, minus_infty, 7.5L, plus_infty, nan_value, INVALID_EXCEPTION);

  TEST_fff_f (fma, 1.25L, 0.75L, 0.0625L, 1.0L);

  FLOAT fltmax = CHOOSE (LDBL_MAX, DBL_MAX, FLT_MAX,
			 LDBL_MAX, DBL_MAX, FLT_MAX);
  TEST_fff_f (fma, -fltmax, -fltmax, minus_infty, minus_infty);
  TEST_fff_f (fma, fltmax / 2, fltmax / 2, minus_infty, minus_infty);
  TEST_fff_f (fma, -fltmax, fltmax, plus_infty, plus_infty);
  TEST_fff_f (fma, fltmax / 2, -fltmax / 4, plus_infty, plus_infty);
  TEST_fff_f (fma, plus_infty, 4, plus_infty, plus_infty);
  TEST_fff_f (fma, 2, minus_infty, minus_infty, minus_infty);
  TEST_fff_f (fma, minus_infty, minus_infty, plus_infty, plus_infty);
  TEST_fff_f (fma, plus_infty, minus_infty, minus_infty, minus_infty);

#if defined (TEST_FLOAT) && FLT_MANT_DIG == 24
  TEST_fff_f (fma, 0x1.7ff8p+13, 0x1.000002p+0, 0x1.ffffp-24, 0x1.7ff802p+13);
  TEST_fff_f (fma, 0x1.fffp+0, 0x1.00001p+0, -0x1.fffp+0, 0x1.fffp-20);
  TEST_fff_f (fma, 0x1.9abcdep+127, 0x0.9abcdep-126, -0x1.f08948p+0, 0x1.bb421p-25);
  TEST_fff_f (fma, 0x1.9abcdep+100, 0x0.9abcdep-126, -0x1.f08948p-27, 0x1.bb421p-52);
  TEST_fff_f (fma, 0x1.fffffep+127, 0x1.001p+0, -0x1.fffffep+127, 0x1.fffffep+115);
  TEST_fff_f (fma, -0x1.fffffep+127, 0x1.fffffep+0, 0x1.fffffep+127, -0x1.fffffap+127);
  TEST_fff_f (fma, 0x1.fffffep+127, 2.0, -0x1.fffffep+127, 0x1.fffffep+127);
#endif
#if defined (TEST_DOUBLE) && DBL_MANT_DIG == 53
  TEST_fff_f (fma, 0x1.7fp+13, 0x1.0000000000001p+0, 0x1.ffep-48, 0x1.7f00000000001p+13);
  TEST_fff_f (fma, 0x1.fffp+0, 0x1.0000000000001p+0, -0x1.fffp+0, 0x1.fffp-52);
  TEST_fff_f (fma, 0x1.0000002p+0, 0x1.ffffffcp-1, 0x1p-300, 1.0);
  TEST_fff_f (fma, 0x1.0000002p+0, 0x1.ffffffcp-1, -0x1p-300, 0x1.fffffffffffffp-1);
  TEST_fff_f (fma, 0x1.deadbeef2feedp+1023, 0x0.deadbeef2feedp-1022, -0x1.a05f8c01a4bfbp+1, 0x1.0989687bc9da4p-53);
  TEST_fff_f (fma, 0x1.deadbeef2feedp+900, 0x0.deadbeef2feedp-1022, -0x1.a05f8c01a4bfbp-122, 0x1.0989687bc9da4p-176);
  TEST_fff_f (fma, 0x1.fffffffffffffp+1023, 0x1.001p+0, -0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1011);
  TEST_fff_f (fma, -0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+0, 0x1.fffffffffffffp+1023, -0x1.ffffffffffffdp+1023);
  TEST_fff_f (fma, 0x1.fffffffffffffp+1023, 2.0, -0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023);
  TEST_fff_f (fma, 0x1.6a09e667f3bccp-538, 0x1.6a09e667f3bccp-538, 0.0, 0.0);
  TEST_fff_f (fma, 0x1.deadbeef2feedp-495, 0x1.deadbeef2feedp-495, -0x1.bf86a5786a574p-989, 0x0.0000042625a1fp-1022);
  TEST_fff_f (fma, 0x1.deadbeef2feedp-503, 0x1.deadbeef2feedp-503, -0x1.bf86a5786a574p-1005, 0x0.0000000004262p-1022);
  TEST_fff_f (fma, 0x1p-537, 0x1p-538, 0x1p-1074, 0x0.0000000000002p-1022);
  TEST_fff_f (fma, 0x1.7fffff8p-968, 0x1p-106, 0x0.000001p-1022, 0x0.0000010000001p-1022);
  TEST_fff_f (fma, 0x1.4000004p-967, 0x1p-106, 0x0.000001p-1022, 0x0.0000010000003p-1022);
  TEST_fff_f (fma, 0x1.4p-967, -0x1p-106, -0x0.000001p-1022, -0x0.0000010000002p-1022);
  TEST_fff_f (fma, -0x1.19cab66d73e17p-959, 0x1.c7108a8c5ff51p-107, -0x0.80b0ad65d9b64p-1022, -0x0.80b0ad65d9d59p-1022);
  TEST_fff_f (fma, -0x1.d2eaed6e8e9d3p-979, -0x1.4e066c62ac9ddp-63, -0x0.9245e6b003454p-1022, -0x0.9245c09c5fb5dp-1022);
  TEST_fff_f (fma, 0x1.153d650bb9f06p-907, 0x1.2d01230d48407p-125, -0x0.b278d5acfc3cp-1022, -0x0.b22757123bbe9p-1022);
  TEST_fff_f (fma, -0x1.fffffffffffffp-711, 0x1.fffffffffffffp-275, 0x1.fffffe00007ffp-983, 0x1.7ffffe00007ffp-983);
#endif
#if defined (TEST_LDOUBLE) && LDBL_MANT_DIG == 64
  TEST_fff_f (fma, -0x8.03fcp+3696L, 0xf.fffffffffffffffp-6140L, 0x8.3ffffffffffffffp-2450L, -0x8.01ecp-2440L);
  TEST_fff_f (fma, 0x9.fcp+2033L, -0x8.000e1f000ff800fp-3613L, -0xf.fffffffffffc0ffp-1579L, -0xd.fc119fb093ed092p-1577L);
  TEST_fff_f (fma, 0xc.7fc000003ffffffp-1194L, 0x8.1e0003fffffffffp+15327L, -0x8.fffep+14072L, 0xc.ae9f164020effffp+14136L);
  TEST_fff_f (fma, -0x8.0001fc000000003p+1798L, 0xcp-2230L, 0x8.f7e000000000007p-468L, -0xc.0002f9ffee10404p-429L);
  TEST_fff_f (fma, 0xc.0000000000007ffp+10130L, -0x8.000000000000001p+4430L, 0xc.07000000001ffffp+14513L, -0xb.fffffffffffd7e4p+14563L);
  TEST_fff_f (fma, 0xb.ffffp-4777L, 0x8.000000fffffffffp-11612L, -0x0.3800fff8p-16385L, 0x5.c7fe80c7ffeffffp-16385L);
#endif
#if defined (TEST_LDOUBLE) && LDBL_MANT_DIG == 113
  TEST_fff_f (fma, 0x1.bb2de33e02ccbbfa6e245a7c1f71p-2584L, -0x1.6b500daf0580d987f1bc0cadfcddp-13777L, 0x1.613cd91d9fed34b33820e5ab9d8dp-16378L, -0x1.3a79fb50eb9ce887cffa0f09bd9fp-16360L);
  TEST_fff_f (fma, -0x1.f949b880cacb0f0c61540105321dp-5954L, -0x1.3876cec84b4140f3bd6198731b7ep-10525L, -0x0.a5dc1c6cfbc498c54fb0b504bf19p-16382L, -0x0.a5dc1c6cfbc498c54fb0b5038abbp-16382L);
  TEST_fff_f (fma, -0x1.0000fffffffffp-16221L, 0x1.0000001fffff8007fep-239L, 0x0.ff87ffffffffffffe000003fffffp-16382L, 0x0.ff87ffffffffffffdffc003bff7fp-16382L);
  TEST_fff_f (fma, -0x1.ac79c9376ef447f3827c9e9de008p-2228L, -0x1.5ba830022b6139e21fbe7270cad8p-6314L, 0x1.e8282b6a26bb6a9daf5c8e73e9f9p-8616L, 0x1.22f14a0253878a730cd1aee373adp-8541L);
  TEST_fff_f (fma, -0x1.c69749ec574caaa2ab8e97ddb9f3p+2652L, 0x1.f34235ff9d095449c29b4831b62dp+3311L, 0x1.fbe4302df23354dbd0c4d3cfe606p+5879L, -0x1.bb473bfdfb7a6e18886ce6e57eafp+5964L);
  TEST_fff_f (fma, -0x1.ca8835fc6ecfb5398625fc891be5p-1686L, 0x1.621e1972bbe2180e5be9dd7d8df5p-7671L, -0x1.7d2d21b73b52cf20dec2a83902a4p-9395L, -0x1.3d2322191c9c88bc68a62ab8042cp-9356L);
  TEST_fff_f (fma, -0x1.55cff679ec49c2541fab41fc843ep-11819L, 0x1.e60e9f464f9e8df0509647c7c971p+12325L, 0x1.eaa2a7649d765c2f564f7a5beca7p+454L, -0x1.447e29fa7e406a285f4e350fcf86p+507L);
  TEST_fff_f (fma, 0x1.f0e7b1454908576f2537d863cf9bp+11432L, 0x1.cdce52f09d4ca76e68706f34b5d5p-1417L, -0x1.2e986187c70f146235ea2066e486p+9979L, 0x1.c030dad3cc5643f3dd0f5619f661p+10016L);
  TEST_fff_f (fma, 0x1.f102f7da4a57a3a4aab620e29452p-3098L, -0x1.cc06a4ff40248f9e2dcc4b6afd84p-11727L, 0x1.d512a11126b5ac8ed8973b8580c8p-14849L, -0x1.be8f1cf737ab4d1c31c54f5ec23bp-14824L);
  TEST_fff_f (fma, -0x1.fc47ac7434b993cd8dcb2b431f25p-3816L, 0x1.fbc9750da8468852d84558e1db6dp-5773L, -0x1.00a98abf783f75c40fe5b7a37d86p-9607L, -0x1.f81917b166f45e763cfcc057e2adp-9588L);
  TEST_fff_f (fma, 0x1.00000000000007ffffffffffffffp-9045L, -0x1.ffffffffffff80000001ffffffffp+4773L, -0x1.f8p-4316L, -0x1.00000000000f88000000fffffdffp-4271L);
  TEST_fff_f (fma, 0x1.4e922764c90701d4a2f21d01893dp-8683L, -0x1.955a12e2d7c9447c27fa022fc865p+212L, -0x1.e9634462eaef96528b90b6944578p-8521L, -0x1.08e1783184a371943d3598e10865p-8470L);
  TEST_fff_f (fma, 0x1.801181509c03bdbef10d6165588cp-15131L, 0x1.ad86f8e57d3d40bfa8007780af63p-368L, -0x1.6e9df0dab1c9f1d7a6043c390741p-15507L, 0x1.417c9b2b15e2ad57dc9e0e920844p-15498L);
#endif

  END (fma);
}


static void
fmax_test (void)
{
  START (fmax);

  TEST_ff_f (fmax, 0, 0, 0);
  TEST_ff_f (fmax, minus_zero, minus_zero, minus_zero);
  TEST_ff_f (fmax, 9, 0, 9);
  TEST_ff_f (fmax, 0, 9, 9);
  TEST_ff_f (fmax, -9, 0, 0);
  TEST_ff_f (fmax, 0, -9, 0);

  TEST_ff_f (fmax, plus_infty, 9, plus_infty);
  TEST_ff_f (fmax, 0, plus_infty, plus_infty);
  TEST_ff_f (fmax, -9, plus_infty, plus_infty);
  TEST_ff_f (fmax, plus_infty, -9, plus_infty);

  TEST_ff_f (fmax, minus_infty, 9, 9);
  TEST_ff_f (fmax, minus_infty, -9, -9);
  TEST_ff_f (fmax, 9, minus_infty, 9);
  TEST_ff_f (fmax, -9, minus_infty, -9);

  TEST_ff_f (fmax, 0, nan_value, 0);
  TEST_ff_f (fmax, 9, nan_value, 9);
  TEST_ff_f (fmax, -9, nan_value, -9);
  TEST_ff_f (fmax, nan_value, 0, 0);
  TEST_ff_f (fmax, nan_value, 9, 9);
  TEST_ff_f (fmax, nan_value, -9, -9);
  TEST_ff_f (fmax, plus_infty, nan_value, plus_infty);
  TEST_ff_f (fmax, minus_infty, nan_value, minus_infty);
  TEST_ff_f (fmax, nan_value, plus_infty, plus_infty);
  TEST_ff_f (fmax, nan_value, minus_infty, minus_infty);
  TEST_ff_f (fmax, nan_value, nan_value, nan_value);

  END (fmax);
}


static void
fmin_test (void)
{
  START (fmin);

  TEST_ff_f (fmin, 0, 0, 0);
  TEST_ff_f (fmin, minus_zero, minus_zero, minus_zero);
  TEST_ff_f (fmin, 9, 0, 0);
  TEST_ff_f (fmin, 0, 9, 0);
  TEST_ff_f (fmin, -9, 0, -9);
  TEST_ff_f (fmin, 0, -9, -9);

  TEST_ff_f (fmin, plus_infty, 9, 9);
  TEST_ff_f (fmin, 9, plus_infty, 9);
  TEST_ff_f (fmin, plus_infty, -9, -9);
  TEST_ff_f (fmin, -9, plus_infty, -9);
  TEST_ff_f (fmin, minus_infty, 9, minus_infty);
  TEST_ff_f (fmin, minus_infty, -9, minus_infty);
  TEST_ff_f (fmin, 9, minus_infty, minus_infty);
  TEST_ff_f (fmin, -9, minus_infty, minus_infty);

  TEST_ff_f (fmin, 0, nan_value, 0);
  TEST_ff_f (fmin, 9, nan_value, 9);
  TEST_ff_f (fmin, -9, nan_value, -9);
  TEST_ff_f (fmin, nan_value, 0, 0);
  TEST_ff_f (fmin, nan_value, 9, 9);
  TEST_ff_f (fmin, nan_value, -9, -9);
  TEST_ff_f (fmin, plus_infty, nan_value, plus_infty);
  TEST_ff_f (fmin, minus_infty, nan_value, minus_infty);
  TEST_ff_f (fmin, nan_value, plus_infty, plus_infty);
  TEST_ff_f (fmin, nan_value, minus_infty, minus_infty);
  TEST_ff_f (fmin, nan_value, nan_value, nan_value);

  END (fmin);
}


static void
fmod_test (void)
{
  errno = 0;
  FUNC(fmod) (6.5, 2.3L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (fmod);

  /* fmod (+0, y) == +0 for y != 0.  */
  TEST_ff_f (fmod, 0, 3, 0);

  /* fmod (-0, y) == -0 for y != 0.  */
  TEST_ff_f (fmod, minus_zero, 3, minus_zero);

  /* fmod (+inf, y) == NaN plus invalid exception.  */
  errno = 0;
  TEST_ff_f (fmod, plus_infty, 3, nan_value, INVALID_EXCEPTION);
  check_int ("errno for fmod(Inf,3) unchanged", errno, EDOM, 0, 0, 0);
  /* fmod (-inf, y) == NaN plus invalid exception.  */
  errno = 0;
  TEST_ff_f (fmod, minus_infty, 3, nan_value, INVALID_EXCEPTION);
  check_int ("errno for fmod(-Inf,3) unchanged", errno, EDOM, 0, 0, 0);
  /* fmod (x, +0) == NaN plus invalid exception.  */
  errno = 0;
  TEST_ff_f (fmod, 3, 0, nan_value, INVALID_EXCEPTION);
  check_int ("errno for fmod(3,0) unchanged", errno, EDOM, 0, 0, 0);
  /* fmod (x, -0) == NaN plus invalid exception.  */
  TEST_ff_f (fmod, 3, minus_zero, nan_value, INVALID_EXCEPTION);

  /* fmod (x, +inf) == x for x not infinite.  */
  TEST_ff_f (fmod, 3.0, plus_infty, 3.0);
  /* fmod (x, -inf) == x for x not infinite.  */
  TEST_ff_f (fmod, 3.0, minus_infty, 3.0);

  TEST_ff_f (fmod, nan_value, nan_value, nan_value);

  TEST_ff_f (fmod, 6.5, 2.25L, 2.0L);
  TEST_ff_f (fmod, -6.5, 2.25L, -2.0L);
  TEST_ff_f (fmod, 6.5, -2.25L, 2.0L);
  TEST_ff_f (fmod, -6.5, -2.25L, -2.0L);

  END (fmod);
}


static void
fpclassify_test (void)
{
  START (fpclassify);

  TEST_f_i (fpclassify, nan_value, FP_NAN);
  TEST_f_i (fpclassify, plus_infty, FP_INFINITE);
  TEST_f_i (fpclassify, minus_infty, FP_INFINITE);
  TEST_f_i (fpclassify, plus_zero, FP_ZERO);
  TEST_f_i (fpclassify, minus_zero, FP_ZERO);
  TEST_f_i (fpclassify, 1000, FP_NORMAL);

  END (fpclassify);
}


static void
frexp_test (void)
{
  int x;

  START (frexp);

  TEST_fI_f1 (frexp, plus_infty, plus_infty, IGNORE);
  TEST_fI_f1 (frexp, minus_infty, minus_infty, IGNORE);
  TEST_fI_f1 (frexp, nan_value, nan_value, IGNORE);

  TEST_fI_f1 (frexp, 0.0, 0.0, 0.0);
  TEST_fI_f1 (frexp, minus_zero, minus_zero, 0.0);

  TEST_fI_f1 (frexp, 12.8L, 0.8L, 4);
  TEST_fI_f1 (frexp, -27.34L, -0.854375L, 5);

  END (frexp);
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

  START (gamma);

  TEST_f_f (gamma, plus_infty, plus_infty);
  TEST_f_f (gamma, 0, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (gamma, -3, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (gamma, minus_infty, plus_infty);
  TEST_f_f (gamma, nan_value, nan_value);

  TEST_f_f1 (gamma, 1, 0, 1);
  TEST_f_f1 (gamma, 3, M_LN2l, 1);

  TEST_f_f1 (gamma, 0.5, M_LOG_SQRT_PIl, 1);
  TEST_f_f1 (gamma, -0.5, M_LOG_2_SQRT_PIl, -1);

  END (gamma);
}

static void
hypot_test (void)
{
  errno = 0;
  FUNC(hypot) (0.7L, 12.4L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (hypot);

  TEST_ff_f (hypot, plus_infty, 1, plus_infty, IGNORE_ZERO_INF_SIGN);
  TEST_ff_f (hypot, minus_infty, 1, plus_infty, IGNORE_ZERO_INF_SIGN);

#ifndef TEST_INLINE
  TEST_ff_f (hypot, plus_infty, nan_value, plus_infty);
  TEST_ff_f (hypot, minus_infty, nan_value, plus_infty);
  TEST_ff_f (hypot, nan_value, plus_infty, plus_infty);
  TEST_ff_f (hypot, nan_value, minus_infty, plus_infty);
#endif

  TEST_ff_f (hypot, nan_value, nan_value, nan_value);

  /* hypot (x,y) == hypot (+-x, +-y)  */
  TEST_ff_f (hypot, 0.7L, 12.4L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, -0.7L, 12.4L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, 0.7L, -12.4L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, -0.7L, -12.4L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, 12.4L, 0.7L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, -12.4L, 0.7L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, 12.4L, -0.7L, 12.419742348374220601176836866763271L);
  TEST_ff_f (hypot, -12.4L, -0.7L, 12.419742348374220601176836866763271L);

  /*  hypot (x,0) == fabs (x)  */
  TEST_ff_f (hypot, 0.75L, 0, 0.75L);
  TEST_ff_f (hypot, -0.75L, 0, 0.75L);
  TEST_ff_f (hypot, -5.7e7, 0, 5.7e7L);

  TEST_ff_f (hypot, 0.75L, 1.25L, 1.45773797371132511771853821938639577L);

#if !(defined TEST_FLOAT && defined TEST_INLINE)
  TEST_ff_f (hypot, 0x3p125L, 0x4p125L, 0x5p125L);
  TEST_ff_f (hypot, 0x1.234566p-126L, 0x1.234566p-126L, 1.891441686191081936598531534017449451173e-38L);
#endif

#ifndef TEST_FLOAT
  TEST_ff_f (hypot, 0x3p1021L, 0x4p1021L, 0x5p1021L);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384 && !defined TEST_INLINE
  TEST_ff_f (hypot, 0x3p16381L, 0x4p16381L, 0x5p16381L);
#endif

  END (hypot);
}


static void
ilogb_test (void)
{
  START (ilogb);

  TEST_f_i (ilogb, 1, 0);
  TEST_f_i (ilogb, M_El, 1);
  TEST_f_i (ilogb, 1024, 10);
  TEST_f_i (ilogb, -2000, 10);

  /* ilogb (0.0) == FP_ILOGB0 plus invalid exception  */
  errno = 0;
  TEST_f_i (ilogb, 0.0, FP_ILOGB0, INVALID_EXCEPTION);
  check_int ("errno for ilogb(0.0) unchanged", errno, EDOM, 0, 0, 0);
  /* ilogb (NaN) == FP_ILOGBNAN plus invalid exception  */
  errno = 0;
  TEST_f_i (ilogb, nan_value, FP_ILOGBNAN, INVALID_EXCEPTION);
  check_int ("errno for ilogb(NaN) unchanged", errno, EDOM, 0, 0, 0);
  /* ilogb (inf) == INT_MAX plus invalid exception  */
  errno = 0;
  TEST_f_i (ilogb, plus_infty, INT_MAX, INVALID_EXCEPTION);
  check_int ("errno for ilogb(Inf) unchanged", errno, EDOM, 0, 0, 0);
  /* ilogb (-inf) == INT_MAX plus invalid exception  */
  errno = 0;
  TEST_f_i (ilogb, minus_infty, INT_MAX, INVALID_EXCEPTION);
  check_int ("errno for ilogb(-Inf) unchanged", errno, EDOM, 0, 0, 0);

  END (ilogb);
}

static void
isfinite_test (void)
{
  START (isfinite);

  TEST_f_b (isfinite, 0, 1);
  TEST_f_b (isfinite, minus_zero, 1);
  TEST_f_b (isfinite, 10, 1);
  TEST_f_b (isfinite, plus_infty, 0);
  TEST_f_b (isfinite, minus_infty, 0);
  TEST_f_b (isfinite, nan_value, 0);

  END (isfinite);
}

static void
isnormal_test (void)
{
  START (isnormal);

  TEST_f_b (isnormal, 0, 0);
  TEST_f_b (isnormal, minus_zero, 0);
  TEST_f_b (isnormal, 10, 1);
  TEST_f_b (isnormal, plus_infty, 0);
  TEST_f_b (isnormal, minus_infty, 0);
  TEST_f_b (isnormal, nan_value, 0);

  END (isnormal);
}

static void
j0_test (void)
{
  FLOAT s, c;
  errno = 0;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
  FUNC(j0) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (j0);

  /* j0 is the Bessel function of the first kind of order 0 */
  TEST_f_f (j0, nan_value, nan_value);
  TEST_f_f (j0, plus_infty, 0);
  TEST_f_f (j0, -1.0, 0.765197686557966551449717526102663221L);
  TEST_f_f (j0, 0.0, 1.0);
  TEST_f_f (j0, 0.125L, 0.996097563041985204620768999453174712L);
  TEST_f_f (j0, 0.75L, 0.864242275166648623555731103820923211L);
  TEST_f_f (j0, 1.0, 0.765197686557966551449717526102663221L);
  TEST_f_f (j0, 1.5, 0.511827671735918128749051744283411720L);
  TEST_f_f (j0, 2.0, 0.223890779141235668051827454649948626L);
  TEST_f_f (j0, 8.0, 0.171650807137553906090869407851972001L);
  TEST_f_f (j0, 10.0, -0.245935764451348335197760862485328754L);
  TEST_f_f (j0, 4.0, -3.9714980986384737228659076845169804197562E-1L);
  TEST_f_f (j0, -4.0, -3.9714980986384737228659076845169804197562E-1L);

  TEST_f_f (j0, 0x1.d7ce3ap+107L, 2.775523647291230802651040996274861694514e-17L);

#ifndef TEST_FLOAT
  TEST_f_f (j0, -0x1.001000001p+593L, -3.927269966354206207832593635798954916263e-90L);
#endif

  END (j0);
}


static void
j1_test (void)
{
  FLOAT s, c;
  errno = 0;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
  FUNC(j1) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* j1 is the Bessel function of the first kind of order 1 */

  START (j1);

  TEST_f_f (j1, nan_value, nan_value);
  TEST_f_f (j1, plus_infty, 0);

  TEST_f_f (j1, -1.0, -0.440050585744933515959682203718914913L);
  TEST_f_f (j1, 0.0, 0.0);
  TEST_f_f (j1, 0.125L, 0.0623780091344946810942311355879361177L);
  TEST_f_f (j1, 0.75L, 0.349243602174862192523281016426251335L);
  TEST_f_f (j1, 1.0, 0.440050585744933515959682203718914913L);
  TEST_f_f (j1, 1.5, 0.557936507910099641990121213156089400L);
  TEST_f_f (j1, 2.0, 0.576724807756873387202448242269137087L);
  TEST_f_f (j1, 8.0, 0.234636346853914624381276651590454612L);
  TEST_f_f (j1, 10.0, 0.0434727461688614366697487680258592883L);

  TEST_f_f (j1, 0x1.3ffp+74L, 1.818984347516051243459364437186082741567e-12L);

#ifndef TEST_FLOAT
  TEST_f_f (j1, 0x1.ff00000000002p+840L, 1.846591691699331493194965158699937660696e-127L);
#endif

  END (j1);
}

static void
jn_test (void)
{
  FLOAT s, c;
  errno = 0;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
  FUNC(jn) (1, 1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* jn is the Bessel function of the first kind of order n.  */
  START (jn);

  /* jn (0, x) == j0 (x)  */
  TEST_ff_f (jn, 0, nan_value, nan_value);
  TEST_ff_f (jn, 0, plus_infty, 0);
  TEST_ff_f (jn, 0, -1.0, 0.765197686557966551449717526102663221L);
  TEST_ff_f (jn, 0, 0.0, 1.0);
  TEST_ff_f (jn, 0, 0.125L, 0.996097563041985204620768999453174712L);
  TEST_ff_f (jn, 0, 0.75L, 0.864242275166648623555731103820923211L);
  TEST_ff_f (jn, 0, 1.0, 0.765197686557966551449717526102663221L);
  TEST_ff_f (jn, 0, 1.5, 0.511827671735918128749051744283411720L);
  TEST_ff_f (jn, 0, 2.0, 0.223890779141235668051827454649948626L);
  TEST_ff_f (jn, 0, 8.0, 0.171650807137553906090869407851972001L);
  TEST_ff_f (jn, 0, 10.0, -0.245935764451348335197760862485328754L);
  TEST_ff_f (jn, 0, 4.0, -3.9714980986384737228659076845169804197562E-1L);
  TEST_ff_f (jn, 0, -4.0, -3.9714980986384737228659076845169804197562E-1L);

  /* jn (1, x) == j1 (x)  */
  TEST_ff_f (jn, 1, nan_value, nan_value);
  TEST_ff_f (jn, 1, plus_infty, 0);
  TEST_ff_f (jn, 1, -1.0, -0.440050585744933515959682203718914913L);
  TEST_ff_f (jn, 1, 0.0, 0.0);
  TEST_ff_f (jn, 1, 0.125L, 0.0623780091344946810942311355879361177L);
  TEST_ff_f (jn, 1, 0.75L, 0.349243602174862192523281016426251335L);
  TEST_ff_f (jn, 1, 1.0, 0.440050585744933515959682203718914913L);
  TEST_ff_f (jn, 1, 1.5, 0.557936507910099641990121213156089400L);
  TEST_ff_f (jn, 1, 2.0, 0.576724807756873387202448242269137087L);
  TEST_ff_f (jn, 1, 8.0, 0.234636346853914624381276651590454612L);
  TEST_ff_f (jn, 1, 10.0, 0.0434727461688614366697487680258592883L);

  /* jn (3, x)  */
  TEST_ff_f (jn, 3, nan_value, nan_value);
  TEST_ff_f (jn, 3, plus_infty, 0);

  TEST_ff_f (jn, 3, -1.0, -0.0195633539826684059189053216217515083L);
  TEST_ff_f (jn, 3, 0.0, 0.0);
  TEST_ff_f (jn, 3, 0.125L, 0.406503832554912875023029337653442868e-4L);
  TEST_ff_f (jn, 3, 0.75L, 0.848438342327410884392755236884386804e-2L);
  TEST_ff_f (jn, 3, 1.0, 0.0195633539826684059189053216217515083L);
  TEST_ff_f (jn, 3, 2.0, 0.128943249474402051098793332969239835L);
  TEST_ff_f (jn, 3, 10.0, 0.0583793793051868123429354784103409563L);

  /*  jn (10, x)  */
  TEST_ff_f (jn, 10, nan_value, nan_value);
  TEST_ff_f (jn, 10, plus_infty, 0);

  TEST_ff_f (jn, 10, -1.0, 0.263061512368745320699785368779050294e-9L);
  TEST_ff_f (jn, 10, 0.0, 0.0);
  TEST_ff_f (jn, 10, 0.125L, 0.250543369809369890173993791865771547e-18L);
  TEST_ff_f (jn, 10, 0.75L, 0.149621713117596814698712483621682835e-10L);
  TEST_ff_f (jn, 10, 1.0, 0.263061512368745320699785368779050294e-9L);
  TEST_ff_f (jn, 10, 2.0, 0.251538628271673670963516093751820639e-6L);
  TEST_ff_f (jn, 10, 10.0, 0.207486106633358857697278723518753428L);

  /* BZ #11589 .*/
  TEST_ff_f (jn, 2, 2.4048255576957729L, 0.43175480701968038399746111312430703L);
  TEST_ff_f (jn, 3, 2.4048255576957729L, 0.19899990535769083404042146764530813L);
  TEST_ff_f (jn, 4, 2.4048255576957729L, 0.647466661641779720084932282551219891E-1L);
  TEST_ff_f (jn, 5, 2.4048255576957729L, 0.163892432048058525099230549946147698E-1L);
  TEST_ff_f (jn, 6, 2.4048255576957729L, 0.34048184720278336646673682895929161E-2L);
  TEST_ff_f (jn, 7, 2.4048255576957729L, 0.60068836573295394221291569249883076E-3L);
  TEST_ff_f (jn, 8, 2.4048255576957729L, 0.92165786705344923232879022467054148E-4L);
  TEST_ff_f (jn, 9, 2.4048255576957729L, 0.12517270977961513005428966643852564E-4L);

  TEST_ff_f (jn, 2, 0x1.ffff62p+99L, -4.43860668048170034334926693188979974489e-16L);

  END (jn);
}


static void
ldexp_test (void)
{
  TEST_ff_f (ldexp, 0, 0, 0);
  TEST_ff_f (ldexp, minus_zero, 0, minus_zero);

  TEST_ff_f (ldexp, plus_infty, 1, plus_infty);
  TEST_ff_f (ldexp, minus_infty, 1, minus_infty);
  TEST_ff_f (ldexp, nan_value, 1, nan_value);

  TEST_ff_f (ldexp, 0.8L, 4, 12.8L);
  TEST_ff_f (ldexp, -0.854375L, 5, -27.34L);

  /* ldexp (x, 0) == x.  */
  TEST_ff_f (ldexp, 1.0L, 0L, 1.0L);
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

  START (lgamma);

  TEST_f_f (lgamma, plus_infty, plus_infty);
  TEST_f_f (lgamma, 0, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("signgam for lgamma(0) == 1", signgam, 1, 0, 0, 0);
  TEST_f_f (lgamma, minus_zero, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("signgam for lgamma(-0) == -1", signgam, -1, 0, 0, 0);
  TEST_f_f (lgamma, nan_value, nan_value);

  /* lgamma (x) == +inf plus divide by zero exception for integer x <= 0.  */
  errno = 0;
  TEST_f_f (lgamma, -3, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for lgamma(-integer) == ERANGE", errno, ERANGE, 0, 0, 0);
  TEST_f_f (lgamma, minus_infty, plus_infty);
  TEST_f_f (lgamma, -max_value, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (lgamma, max_value, plus_infty, OVERFLOW_EXCEPTION);

  TEST_f_f1 (lgamma, 1, 0, 1);

  TEST_f_f1 (lgamma, 3, M_LN2l, 1);

  TEST_f_f1 (lgamma, 0.5, M_LOG_SQRT_PIl, 1);
  TEST_f_f1 (lgamma, -0.5, M_LOG_2_SQRT_PIl, -1);
  TEST_f_f1 (lgamma, 0.7L, 0.260867246531666514385732417016759578L, 1);
  TEST_f_f1 (lgamma, 1.2L, -0.853740900033158497197028392998854470e-1L, 1);

  END (lgamma);
}


static void
lrint_test (void)
{
  /* XXX this test is incomplete.  We need to have a way to specifiy
     the rounding method and test the critical cases.  So far, only
     unproblematic numbers are tested.  */

  START (lrint);

  TEST_f_l (lrint, 0.0, 0);
  TEST_f_l (lrint, minus_zero, 0);
  TEST_f_l (lrint, 0.2L, 0);
  TEST_f_l (lrint, -0.2L, 0);

  TEST_f_l (lrint, 1.4L, 1);
  TEST_f_l (lrint, -1.4L, -1);

  TEST_f_l (lrint, 8388600.3L, 8388600);
  TEST_f_l (lrint, -8388600.3L, -8388600);

  TEST_f_l (lrint, 1071930.0008, 1071930);
#ifndef TEST_FLOAT
  TEST_f_l (lrint, 1073741824.01, 1073741824);
# if LONG_MAX > 281474976710656
  TEST_f_l (lrint, 281474976710656.025, 281474976710656);
# endif
#endif

  END (lrint);
}


static void
lrint_test_tonearest (void)
{
  int save_round_mode;
  START (lrint_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_f_l (lrint, 0.0, 0);
      TEST_f_l (lrint, minus_zero, 0);
      TEST_f_l (lrint, 0.2L, 0);
      TEST_f_l (lrint, -0.2L, 0);
      TEST_f_l (lrint, 0.5L, 0);
      TEST_f_l (lrint, -0.5L, 0);
      TEST_f_l (lrint, 0.8L, 1);
      TEST_f_l (lrint, -0.8L, -1);

      TEST_f_l (lrint, 1.4L, 1);
      TEST_f_l (lrint, -1.4L, -1);

      TEST_f_l (lrint, 8388600.3L, 8388600);
      TEST_f_l (lrint, -8388600.3L, -8388600);

      TEST_f_l (lrint, 1071930.0008, 1071930);
#ifndef TEST_FLOAT
      TEST_f_l (lrint, 1073741824.01, 1073741824);
# if LONG_MAX > 281474976710656
      TEST_f_l (lrint, 281474976710656.025, 281474976710656);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (lrint_tonearest);
}


static void
lrint_test_towardzero (void)
{
  int save_round_mode;
  START (lrint_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_f_l (lrint, 0.0, 0);
      TEST_f_l (lrint, minus_zero, 0);
      TEST_f_l (lrint, 0.2L, 0);
      TEST_f_l (lrint, -0.2L, 0);
      TEST_f_l (lrint, 0.5L, 0);
      TEST_f_l (lrint, -0.5L, 0);
      TEST_f_l (lrint, 0.8L, 0);
      TEST_f_l (lrint, -0.8L, 0);

      TEST_f_l (lrint, 1.4L, 1);
      TEST_f_l (lrint, -1.4L, -1);

      TEST_f_l (lrint, 8388600.3L, 8388600);
      TEST_f_l (lrint, -8388600.3L, -8388600);

      TEST_f_l (lrint, 1071930.0008, 1071930);
#ifndef TEST_FLOAT
      TEST_f_l (lrint, 1073741824.01, 1073741824);
# if LONG_MAX > 281474976710656
      TEST_f_l (lrint, 281474976710656.025, 281474976710656);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (lrint_towardzero);
}


static void
lrint_test_downward (void)
{
  int save_round_mode;
  START (lrint_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_f_l (lrint, 0.0, 0);
      TEST_f_l (lrint, minus_zero, 0);
      TEST_f_l (lrint, 0.2L, 0);
      TEST_f_l (lrint, -0.2L, -1);
      TEST_f_l (lrint, 0.5L, 0);
      TEST_f_l (lrint, -0.5L, -1);
      TEST_f_l (lrint, 0.8L, 0);
      TEST_f_l (lrint, -0.8L, -1);

      TEST_f_l (lrint, 1.4L, 1);
      TEST_f_l (lrint, -1.4L, -2);

      TEST_f_l (lrint, 8388600.3L, 8388600);
      TEST_f_l (lrint, -8388600.3L, -8388601);

      TEST_f_l (lrint, 1071930.0008, 1071930);
#ifndef TEST_FLOAT
      TEST_f_l (lrint, 1073741824.01, 1073741824);
# if LONG_MAX > 281474976710656
      TEST_f_l (lrint, 281474976710656.025, 281474976710656);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (lrint_downward);
}


static void
lrint_test_upward (void)
{
  int save_round_mode;
  START (lrint_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_f_l (lrint, 0.0, 0);
      TEST_f_l (lrint, minus_zero, 0);
      TEST_f_l (lrint, 0.2L, 1);
      TEST_f_l (lrint, -0.2L, 0);
      TEST_f_l (lrint, 0.5L, 1);
      TEST_f_l (lrint, -0.5L, 0);
      TEST_f_l (lrint, 0.8L, 1);
      TEST_f_l (lrint, -0.8L, 0);

      TEST_f_l (lrint, 1.4L, 2);
      TEST_f_l (lrint, -1.4L, -1);

      TEST_f_l (lrint, 8388600.3L, 8388601);
      TEST_f_l (lrint, -8388600.3L, -8388600);

#ifndef TEST_FLOAT
      TEST_f_l (lrint, 1071930.0008, 1071931);
      TEST_f_l (lrint, 1073741824.01, 1073741825);
# if LONG_MAX > 281474976710656 && defined (TEST_LDOUBLE)
      TEST_f_l (lrint, 281474976710656.025, 281474976710656);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (lrint_upward);
}


static void
llrint_test (void)
{
  /* XXX this test is incomplete.  We need to have a way to specifiy
     the rounding method and test the critical cases.  So far, only
     unproblematic numbers are tested.  */

  START (llrint);

  TEST_f_L (llrint, 0.0, 0);
  TEST_f_L (llrint, minus_zero, 0);
  TEST_f_L (llrint, 0.2L, 0);
  TEST_f_L (llrint, -0.2L, 0);

  TEST_f_L (llrint, 1.4L, 1);
  TEST_f_L (llrint, -1.4L, -1);

  TEST_f_L (llrint, 8388600.3L, 8388600);
  TEST_f_L (llrint, -8388600.3L, -8388600);

  TEST_f_l (llrint, 1071930.0008, 1071930);

  /* Test boundary conditions.  */
  /* 0x1FFFFF */
  TEST_f_L (llrint, 2097151.0,2097151LL);
  /* 0x800000 */
  TEST_f_L (llrint, 8388608.0, 8388608LL);
  /* 0x1000000 */
  TEST_f_L (llrint, 16777216.0, 16777216LL);
  /* 0x20000000000 */
  TEST_f_L (llrint, 2199023255552.0, 2199023255552LL);
  /* 0x40000000000 */
  TEST_f_L (llrint, 4398046511104.0, 4398046511104LL);
  /* 0x1000000000000 */
  TEST_f_L (llrint, 281474976710656.0, 281474976710656LL);
  /* 0x10000000000000 */
  TEST_f_L (llrint, 4503599627370496.0, 4503599627370496LL);
  /* 0x10000080000000 */
  TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL);
  /* 0x20000000000000 */
  TEST_f_L (llrint, 9007199254740992.0, 9007199254740992LL);
  /* 0x80000000000000 */
  TEST_f_L (llrint, 36028797018963968.0, 36028797018963968LL);
  /* 0x100000000000000 */
  TEST_f_L (llrint, 72057594037927936.0, 72057594037927936LL);
#ifdef TEST_LDOUBLE
  /* The input can only be represented in long double.  */
  TEST_f_L (llrint, 4503599627370495.5L, 4503599627370496LL);
  TEST_f_L (llrint, 4503599627370496.25L, 4503599627370496LL);
  TEST_f_L (llrint, 4503599627370496.5L, 4503599627370496LL);
  TEST_f_L (llrint, 4503599627370496.75L, 4503599627370497LL);
  TEST_f_L (llrint, 4503599627370497.5L, 4503599627370498LL);

  TEST_f_L (llrint, -4503599627370495.5L, -4503599627370496LL);
  TEST_f_L (llrint, -4503599627370496.25L, -4503599627370496LL);
  TEST_f_L (llrint, -4503599627370496.5L, -4503599627370496LL);
  TEST_f_L (llrint, -4503599627370496.75L, -4503599627370497LL);
  TEST_f_L (llrint, -4503599627370497.5L, -4503599627370498LL);

# if LDBL_MANT_DIG > 100
  TEST_f_L (llrint, 4503599627370495.4999999999999L, 4503599627370495LL);
  TEST_f_L (llrint, 4503599627370496.4999999999999L, 4503599627370496LL);
  TEST_f_L (llrint, 4503599627370497.4999999999999L, 4503599627370497LL);
  TEST_f_L (llrint, 4503599627370494.5000000000001L, 4503599627370495LL);
  TEST_f_L (llrint, 4503599627370495.5000000000001L, 4503599627370496LL);
  TEST_f_L (llrint, 4503599627370496.5000000000001L, 4503599627370497LL);

  TEST_f_L (llrint, -4503599627370495.4999999999999L, -4503599627370495LL);
  TEST_f_L (llrint, -4503599627370496.4999999999999L, -4503599627370496LL);
  TEST_f_L (llrint, -4503599627370497.4999999999999L, -4503599627370497LL);
  TEST_f_L (llrint, -4503599627370494.5000000000001L, -4503599627370495LL);
  TEST_f_L (llrint, -4503599627370495.5000000000001L, -4503599627370496LL);
  TEST_f_L (llrint, -4503599627370496.5000000000001L, -4503599627370497LL);
#endif

  TEST_f_L (llrint, 9007199254740991.5L, 9007199254740992LL);
  TEST_f_L (llrint, 9007199254740992.25L, 9007199254740992LL);
  TEST_f_L (llrint, 9007199254740992.5L, 9007199254740992LL);
  TEST_f_L (llrint, 9007199254740992.75L, 9007199254740993LL);
  TEST_f_L (llrint, 9007199254740993.5L, 9007199254740994LL);

  TEST_f_L (llrint, -9007199254740991.5L, -9007199254740992LL);
  TEST_f_L (llrint, -9007199254740992.25L, -9007199254740992LL);
  TEST_f_L (llrint, -9007199254740992.5L, -9007199254740992LL);
  TEST_f_L (llrint, -9007199254740992.75L, -9007199254740993LL);
  TEST_f_L (llrint, -9007199254740993.5L, -9007199254740994LL);

# if LDBL_MANT_DIG > 100
  TEST_f_L (llrint, 9007199254740991.4999999999999L, 9007199254740991LL);
  TEST_f_L (llrint, 9007199254740992.4999999999999L, 9007199254740992LL);
  TEST_f_L (llrint, 9007199254740993.4999999999999L, 9007199254740993LL);
  TEST_f_L (llrint, 9007199254740991.5000000000001L, 9007199254740992LL);
  TEST_f_L (llrint, 9007199254740992.5000000000001L, 9007199254740993LL);
  TEST_f_L (llrint, 9007199254740993.5000000000001L, 9007199254740994LL);

  TEST_f_L (llrint, -9007199254740991.4999999999999L, -9007199254740991LL);
  TEST_f_L (llrint, -9007199254740992.4999999999999L, -9007199254740992LL);
  TEST_f_L (llrint, -9007199254740993.4999999999999L, -9007199254740993LL);
  TEST_f_L (llrint, -9007199254740991.5000000000001L, -9007199254740992LL);
  TEST_f_L (llrint, -9007199254740992.5000000000001L, -9007199254740993LL);
  TEST_f_L (llrint, -9007199254740993.5000000000001L, -9007199254740994LL);
#endif

  TEST_f_L (llrint, 72057594037927935.5L, 72057594037927936LL);
  TEST_f_L (llrint, 72057594037927936.25L, 72057594037927936LL);
  TEST_f_L (llrint, 72057594037927936.5L, 72057594037927936LL);
  TEST_f_L (llrint, 72057594037927936.75L, 72057594037927937LL);
  TEST_f_L (llrint, 72057594037927937.5L, 72057594037927938LL);

  TEST_f_L (llrint, -72057594037927935.5L, -72057594037927936LL);
  TEST_f_L (llrint, -72057594037927936.25L, -72057594037927936LL);
  TEST_f_L (llrint, -72057594037927936.5L, -72057594037927936LL);
  TEST_f_L (llrint, -72057594037927936.75L, -72057594037927937LL);
  TEST_f_L (llrint, -72057594037927937.5L, -72057594037927938LL);

# if LDBL_MANT_DIG > 100
  TEST_f_L (llrint, 9223372036854775805.5L, 9223372036854775806LL);
  TEST_f_L (llrint, -9223372036854775805.5L, -9223372036854775806LL);
  TEST_f_L (llrint, 9223372036854775806.0L, 9223372036854775806LL);
  TEST_f_L (llrint, -9223372036854775806.0L, -9223372036854775806LL);
  TEST_f_L (llrint, 9223372036854775806.25L, 9223372036854775806LL);
  TEST_f_L (llrint, -9223372036854775806.25L, -9223372036854775806LL);
  TEST_f_L (llrint, 9223372036854775806.5L, 9223372036854775806L);
  TEST_f_L (llrint, -9223372036854775806.5L, -9223372036854775806LL);
  TEST_f_L (llrint, 9223372036854775806.75L, 9223372036854775807LL);
  TEST_f_L (llrint, -9223372036854775806.75L, -9223372036854775807LL);
  TEST_f_L (llrint, 9223372036854775807.0L, 9223372036854775807LL);
  TEST_f_L (llrint, -9223372036854775807.0L, -9223372036854775807LL);
# endif
#endif

  END (llrint);
}

static void
llrint_test_tonearest (void)
{
  int save_round_mode;
  START (llrint_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_f_L (llrint, 0.0, 0);
      TEST_f_L (llrint, minus_zero, 0);
      TEST_f_L (llrint, 0.2L, 0);
      TEST_f_L (llrint, -0.2L, 0);

      TEST_f_L (llrint, 1.4L, 1);
      TEST_f_L (llrint, -1.4L, -1);

      TEST_f_L (llrint, 8388600.3L, 8388600);
      TEST_f_L (llrint, -8388600.3L, -8388600);

      TEST_f_l (llrint, 1071930.0008, 1071930);

      /* Test boundary conditions.  */
      /* 0x1FFFFF */
      TEST_f_L (llrint, 2097151.0,2097151LL);
      /* 0x800000 */
      TEST_f_L (llrint, 8388608.0, 8388608LL);
      /* 0x1000000 */
      TEST_f_L (llrint, 16777216.0, 16777216LL);
      /* 0x20000000000 */
      TEST_f_L (llrint, 2199023255552.0, 2199023255552LL);
      /* 0x40000000000 */
      TEST_f_L (llrint, 4398046511104.0, 4398046511104LL);
      /* 0x1000000000000 */
      TEST_f_L (llrint, 281474976710656.0, 281474976710656LL);
      /* 0x10000000000000 */
      TEST_f_L (llrint, 4503599627370496.0, 4503599627370496LL);
      /* 0x10000080000000 */
      TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL);
      /* 0x20000000000000 */
      TEST_f_L (llrint, 9007199254740992.0, 9007199254740992LL);
      /* 0x80000000000000 */
      TEST_f_L (llrint, 36028797018963968.0, 36028797018963968LL);
      /* 0x100000000000000 */
      TEST_f_L (llrint, 72057594037927936.0, 72057594037927936LL);
#ifdef TEST_LDOUBLE
      /* The input can only be represented in long double.  */
      TEST_f_L (llrint, 4503599627370495.5L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370496.25L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370496.5L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370496.75L, 4503599627370497LL);
      TEST_f_L (llrint, 4503599627370497.5L, 4503599627370498LL);

      TEST_f_L (llrint, -4503599627370495.5L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370496.25L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370496.5L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370496.75L, -4503599627370497LL);
      TEST_f_L (llrint, -4503599627370497.5L, -4503599627370498LL);

# if LDBL_MANT_DIG > 100
      TEST_f_L (llrint, 4503599627370495.4999999999999L, 4503599627370495LL);
      TEST_f_L (llrint, 4503599627370496.4999999999999L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370497.4999999999999L, 4503599627370497LL);
      TEST_f_L (llrint, 4503599627370494.5000000000001L, 4503599627370495LL);
      TEST_f_L (llrint, 4503599627370495.5000000000001L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370496.5000000000001L, 4503599627370497LL);

      TEST_f_L (llrint, -4503599627370495.4999999999999L, -4503599627370495LL);
      TEST_f_L (llrint, -4503599627370496.4999999999999L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370497.4999999999999L, -4503599627370497LL);
      TEST_f_L (llrint, -4503599627370494.5000000000001L, -4503599627370495LL);
      TEST_f_L (llrint, -4503599627370495.5000000000001L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370496.5000000000001L, -4503599627370497LL);
#endif

      TEST_f_L (llrint, 9007199254740991.5L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740992.25L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740992.5L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740992.75L, 9007199254740993LL);
      TEST_f_L (llrint, 9007199254740993.5L, 9007199254740994LL);

      TEST_f_L (llrint, -9007199254740991.5L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740992.25L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740992.5L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740992.75L, -9007199254740993LL);
      TEST_f_L (llrint, -9007199254740993.5L, -9007199254740994LL);

# if LDBL_MANT_DIG > 100
      TEST_f_L (llrint, 9007199254740991.4999999999999L, 9007199254740991LL);
      TEST_f_L (llrint, 9007199254740992.4999999999999L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740993.4999999999999L, 9007199254740993LL);
      TEST_f_L (llrint, 9007199254740991.5000000000001L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740992.5000000000001L, 9007199254740993LL);
      TEST_f_L (llrint, 9007199254740993.5000000000001L, 9007199254740994LL);

      TEST_f_L (llrint, -9007199254740991.4999999999999L, -9007199254740991LL);
      TEST_f_L (llrint, -9007199254740992.4999999999999L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740993.4999999999999L, -9007199254740993LL);
      TEST_f_L (llrint, -9007199254740991.5000000000001L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740992.5000000000001L, -9007199254740993LL);
      TEST_f_L (llrint, -9007199254740993.5000000000001L, -9007199254740994LL);
#endif

      TEST_f_L (llrint, 72057594037927935.5L, 72057594037927936LL);
      TEST_f_L (llrint, 72057594037927936.25L, 72057594037927936LL);
      TEST_f_L (llrint, 72057594037927936.5L, 72057594037927936LL);
      TEST_f_L (llrint, 72057594037927936.75L, 72057594037927937LL);
      TEST_f_L (llrint, 72057594037927937.5L, 72057594037927938LL);

      TEST_f_L (llrint, -72057594037927935.5L, -72057594037927936LL);
      TEST_f_L (llrint, -72057594037927936.25L, -72057594037927936LL);
      TEST_f_L (llrint, -72057594037927936.5L, -72057594037927936LL);
      TEST_f_L (llrint, -72057594037927936.75L, -72057594037927937LL);
      TEST_f_L (llrint, -72057594037927937.5L, -72057594037927938LL);

# if LDBL_MANT_DIG > 100
      TEST_f_L (llrint, 9223372036854775805.5L, 9223372036854775806LL);
      TEST_f_L (llrint, -9223372036854775805.5L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.0L, 9223372036854775806LL);
      TEST_f_L (llrint, -9223372036854775806.0L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.25L, 9223372036854775806LL);
      TEST_f_L (llrint, -9223372036854775806.25L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.5L, 9223372036854775806L);
      TEST_f_L (llrint, -9223372036854775806.5L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.75L, 9223372036854775807LL);
      TEST_f_L (llrint, -9223372036854775806.75L, -9223372036854775807LL);
      TEST_f_L (llrint, 9223372036854775807.0L, 9223372036854775807LL);
      TEST_f_L (llrint, -9223372036854775807.0L, -9223372036854775807LL);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (llrint_tonearest);
}

static void
llrint_test_towardzero (void)
{
  int save_round_mode;
  START (llrint_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_f_L (llrint, 0.0, 0);
      TEST_f_L (llrint, minus_zero, 0);
      TEST_f_L (llrint, 0.2L, 0);
      TEST_f_L (llrint, -0.2L, 0);

      TEST_f_L (llrint, 1.4L, 1);
      TEST_f_L (llrint, -1.4L, -1);

      TEST_f_L (llrint, 8388600.3L, 8388600);
      TEST_f_L (llrint, -8388600.3L, -8388600);

      TEST_f_l (llrint, 1071930.0008, 1071930);

      /* Test boundary conditions.  */
      /* 0x1FFFFF */
      TEST_f_L (llrint, 2097151.0,2097151LL);
      /* 0x800000 */
      TEST_f_L (llrint, 8388608.0, 8388608LL);
      /* 0x1000000 */
      TEST_f_L (llrint, 16777216.0, 16777216LL);
      /* 0x20000000000 */
      TEST_f_L (llrint, 2199023255552.0, 2199023255552LL);
      /* 0x40000000000 */
      TEST_f_L (llrint, 4398046511104.0, 4398046511104LL);
      /* 0x1000000000000 */
      TEST_f_L (llrint, 281474976710656.0, 281474976710656LL);
      /* 0x10000000000000 */
      TEST_f_L (llrint, 4503599627370496.0, 4503599627370496LL);
      /* 0x10000080000000 */
      TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL);
      /* 0x20000000000000 */
      TEST_f_L (llrint, 9007199254740992.0, 9007199254740992LL);
      /* 0x80000000000000 */
      TEST_f_L (llrint, 36028797018963968.0, 36028797018963968LL);
      /* 0x100000000000000 */
      TEST_f_L (llrint, 72057594037927936.0, 72057594037927936LL);
#ifdef TEST_LDOUBLE
      /* The input can only be represented in long double.  */
      TEST_f_L (llrint, 4503599627370495.5L, 4503599627370495LL);
      TEST_f_L (llrint, 4503599627370496.25L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370496.5L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370496.75L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370497.5L, 4503599627370497LL);

      TEST_f_L (llrint, -4503599627370495.5L, -4503599627370495LL);
      TEST_f_L (llrint, -4503599627370496.25L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370496.5L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370496.75L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370497.5L, -4503599627370497LL);

# if LDBL_MANT_DIG > 100
      TEST_f_L (llrint, 4503599627370495.4999999999999L, 4503599627370495LL);
      TEST_f_L (llrint, 4503599627370496.4999999999999L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370497.4999999999999L, 4503599627370497LL);
      TEST_f_L (llrint, 4503599627370494.5000000000001L, 4503599627370494LL);
      TEST_f_L (llrint, 4503599627370495.5000000000001L, 4503599627370495LL);
      TEST_f_L (llrint, 4503599627370496.5000000000001L, 4503599627370496LL);

      TEST_f_L (llrint, -4503599627370495.4999999999999L, -4503599627370495LL);
      TEST_f_L (llrint, -4503599627370496.4999999999999L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370497.4999999999999L, -4503599627370497LL);
      TEST_f_L (llrint, -4503599627370494.5000000000001L, -4503599627370494LL);
      TEST_f_L (llrint, -4503599627370495.5000000000001L, -4503599627370495LL);
      TEST_f_L (llrint, -4503599627370496.5000000000001L, -4503599627370496LL);
#endif

      TEST_f_L (llrint, 9007199254740991.5L, 9007199254740991LL);
      TEST_f_L (llrint, 9007199254740992.25L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740992.5L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740992.75L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740993.5L, 9007199254740993LL);

      TEST_f_L (llrint, -9007199254740991.5L, -9007199254740991LL);
      TEST_f_L (llrint, -9007199254740992.25L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740992.5L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740992.75L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740993.5L, -9007199254740993LL);

# if LDBL_MANT_DIG > 100
      TEST_f_L (llrint, 9007199254740991.4999999999999L, 9007199254740991LL);
      TEST_f_L (llrint, 9007199254740992.4999999999999L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740993.4999999999999L, 9007199254740993LL);
      TEST_f_L (llrint, 9007199254740991.5000000000001L, 9007199254740991LL);
      TEST_f_L (llrint, 9007199254740992.5000000000001L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740993.5000000000001L, 9007199254740993LL);

      TEST_f_L (llrint, -9007199254740991.4999999999999L, -9007199254740991LL);
      TEST_f_L (llrint, -9007199254740992.4999999999999L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740993.4999999999999L, -9007199254740993LL);
      TEST_f_L (llrint, -9007199254740991.5000000000001L, -9007199254740991LL);
      TEST_f_L (llrint, -9007199254740992.5000000000001L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740993.5000000000001L, -9007199254740993LL);
#endif

      TEST_f_L (llrint, 72057594037927935.5L, 72057594037927935LL);
      TEST_f_L (llrint, 72057594037927936.25L, 72057594037927936LL);
      TEST_f_L (llrint, 72057594037927936.5L, 72057594037927936LL);
      TEST_f_L (llrint, 72057594037927936.75L, 72057594037927936LL);
      TEST_f_L (llrint, 72057594037927937.5L, 72057594037927937LL);

      TEST_f_L (llrint, -72057594037927935.5L, -72057594037927935LL);
      TEST_f_L (llrint, -72057594037927936.25L, -72057594037927936LL);
      TEST_f_L (llrint, -72057594037927936.5L, -72057594037927936LL);
      TEST_f_L (llrint, -72057594037927936.75L, -72057594037927936LL);
      TEST_f_L (llrint, -72057594037927937.5L, -72057594037927937LL);

# if LDBL_MANT_DIG > 100
      TEST_f_L (llrint, 9223372036854775805.5L, 9223372036854775805LL);
      TEST_f_L (llrint, -9223372036854775805.5L, -9223372036854775805LL);
      TEST_f_L (llrint, 9223372036854775806.0L, 9223372036854775806LL);
      TEST_f_L (llrint, -9223372036854775806.0L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.25L, 9223372036854775806LL);
      TEST_f_L (llrint, -9223372036854775806.25L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.5L, 9223372036854775806L);
      TEST_f_L (llrint, -9223372036854775806.5L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.75L, 9223372036854775806LL);
      TEST_f_L (llrint, -9223372036854775806.75L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775807.0L, 9223372036854775807LL);
      TEST_f_L (llrint, -9223372036854775807.0L, -9223372036854775807LL);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (llrint_towardzero);
}

static void
llrint_test_downward (void)
{
  int save_round_mode;
  START (llrint_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_f_L (llrint, 0.0, 0);
      TEST_f_L (llrint, minus_zero, 0);
      TEST_f_L (llrint, 0.2L, 0);
      TEST_f_L (llrint, -0.2L, -1);

      TEST_f_L (llrint, 1.4L, 1);
      TEST_f_L (llrint, -1.4L, -2);

      TEST_f_L (llrint, 8388600.3L, 8388600);
      TEST_f_L (llrint, -8388600.3L, -8388601);

      TEST_f_l (llrint, 1071930.0008, 1071930);

      /* Test boundary conditions.  */
      /* 0x1FFFFF */
      TEST_f_L (llrint, 2097151.0,2097151LL);
      /* 0x800000 */
      TEST_f_L (llrint, 8388608.0, 8388608LL);
      /* 0x1000000 */
      TEST_f_L (llrint, 16777216.0, 16777216LL);
      /* 0x20000000000 */
      TEST_f_L (llrint, 2199023255552.0, 2199023255552LL);
      /* 0x40000000000 */
      TEST_f_L (llrint, 4398046511104.0, 4398046511104LL);
      /* 0x1000000000000 */
      TEST_f_L (llrint, 281474976710656.0, 281474976710656LL);
      /* 0x10000000000000 */
      TEST_f_L (llrint, 4503599627370496.0, 4503599627370496LL);
      /* 0x10000080000000 */
      TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL);
      /* 0x20000000000000 */
      TEST_f_L (llrint, 9007199254740992.0, 9007199254740992LL);
      /* 0x80000000000000 */
      TEST_f_L (llrint, 36028797018963968.0, 36028797018963968LL);
      /* 0x100000000000000 */
      TEST_f_L (llrint, 72057594037927936.0, 72057594037927936LL);
#ifdef TEST_LDOUBLE
      /* The input can only be represented in long double.  */
      TEST_f_L (llrint, 4503599627370495.5L, 4503599627370495LL);
      TEST_f_L (llrint, 4503599627370496.25L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370496.5L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370496.75L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370497.5L, 4503599627370497LL);

      TEST_f_L (llrint, 4503599627370495.4999999999999L, 4503599627370495LL);
      TEST_f_L (llrint, 4503599627370496.4999999999999L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370497.4999999999999L, 4503599627370497LL);
      TEST_f_L (llrint, 4503599627370494.5000000000001L, 4503599627370494LL);
      TEST_f_L (llrint, 4503599627370495.5000000000001L, 4503599627370495LL);
      TEST_f_L (llrint, 4503599627370496.5000000000001L, 4503599627370496LL);

      TEST_f_L (llrint, -4503599627370495.5L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370496.25L, -4503599627370497LL);
      TEST_f_L (llrint, -4503599627370496.5L, -4503599627370497LL);
      TEST_f_L (llrint, -4503599627370496.75L, -4503599627370497LL);
      TEST_f_L (llrint, -4503599627370497.5L, -4503599627370498LL);

      TEST_f_L (llrint, -4503599627370495.4999999999999L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370496.4999999999999L, -4503599627370497LL);
      TEST_f_L (llrint, -4503599627370497.4999999999999L, -4503599627370498LL);
      TEST_f_L (llrint, -4503599627370494.5000000000001L, -4503599627370495LL);
      TEST_f_L (llrint, -4503599627370495.5000000000001L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370496.5000000000001L, -4503599627370497LL);

      TEST_f_L (llrint, 9007199254740991.5L, 9007199254740991LL);
      TEST_f_L (llrint, 9007199254740992.25L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740992.5L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740992.75L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740993.5L, 9007199254740993LL);

      TEST_f_L (llrint, 9007199254740991.4999999999999L, 9007199254740991LL);
      TEST_f_L (llrint, 9007199254740992.4999999999999L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740993.4999999999999L, 9007199254740993LL);
      TEST_f_L (llrint, 9007199254740991.5000000000001L, 9007199254740991LL);
      TEST_f_L (llrint, 9007199254740992.5000000000001L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740993.5000000000001L, 9007199254740993LL);

      TEST_f_L (llrint, -9007199254740991.5L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740992.25L, -9007199254740993LL);
      TEST_f_L (llrint, -9007199254740992.5L, -9007199254740993LL);
      TEST_f_L (llrint, -9007199254740992.75L, -9007199254740993LL);
      TEST_f_L (llrint, -9007199254740993.5L, -9007199254740994LL);

      TEST_f_L (llrint, -9007199254740991.4999999999999L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740992.4999999999999L, -9007199254740993LL);
      TEST_f_L (llrint, -9007199254740993.4999999999999L, -9007199254740994LL);
      TEST_f_L (llrint, -9007199254740991.5000000000001L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740992.5000000000001L, -9007199254740993LL);
      TEST_f_L (llrint, -9007199254740993.5000000000001L, -9007199254740994LL);

      TEST_f_L (llrint, 72057594037927935.5L, 72057594037927935LL);
      TEST_f_L (llrint, 72057594037927936.25L, 72057594037927936LL);
      TEST_f_L (llrint, 72057594037927936.5L, 72057594037927936LL);
      TEST_f_L (llrint, 72057594037927936.75L, 72057594037927936LL);
      TEST_f_L (llrint, 72057594037927937.5L, 72057594037927937LL);

      TEST_f_L (llrint, -72057594037927935.5L, -72057594037927936LL);
      TEST_f_L (llrint, -72057594037927936.25L, -72057594037927937LL);
      TEST_f_L (llrint, -72057594037927936.5L, -72057594037927937LL);
      TEST_f_L (llrint, -72057594037927936.75L, -72057594037927937LL);
      TEST_f_L (llrint, -72057594037927937.5L, -72057594037927938LL);

# if LDBL_MANT_DIG > 100
      TEST_f_L (llrint, 9223372036854775805.5L, 9223372036854775805LL);
      TEST_f_L (llrint, -9223372036854775805.5L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.0L, 9223372036854775806LL);
      TEST_f_L (llrint, -9223372036854775806.0L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.25L, 9223372036854775806LL);
      TEST_f_L (llrint, -9223372036854775806.25L, -9223372036854775807LL);
      TEST_f_L (llrint, 9223372036854775806.5L, 9223372036854775806L);
      TEST_f_L (llrint, -9223372036854775806.5L, -9223372036854775807LL);
      TEST_f_L (llrint, 9223372036854775806.75L, 9223372036854775806LL);
      TEST_f_L (llrint, -9223372036854775806.75L, -9223372036854775807LL);
      TEST_f_L (llrint, 9223372036854775807.0L, 9223372036854775807LL);
      TEST_f_L (llrint, -9223372036854775807.0L, -9223372036854775807LL);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (llrint_downward);
}

static void
llrint_test_upward (void)
{
  int save_round_mode;
  START (llrint_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_f_L (llrint, 0.0, 0);
      TEST_f_L (llrint, minus_zero, 0);
      TEST_f_L (llrint, 0.2L, 1);
      TEST_f_L (llrint, -0.2L, 0);

      TEST_f_L (llrint, 1.4L, 2);
      TEST_f_L (llrint, -1.4L, -1);

      TEST_f_L (llrint, 8388600.3L, 8388601);
      TEST_f_L (llrint, -8388600.3L, -8388600);
#ifndef TEST_FLOAT
      TEST_f_l (llrint, 1071930.0008, 1071931);
#endif
      /* Test boundary conditions.  */
      /* 0x1FFFFF */
      TEST_f_L (llrint, 2097151.0,2097151LL);
      /* 0x800000 */
      TEST_f_L (llrint, 8388608.0, 8388608LL);
      /* 0x1000000 */
      TEST_f_L (llrint, 16777216.0, 16777216LL);
      /* 0x20000000000 */
      TEST_f_L (llrint, 2199023255552.0, 2199023255552LL);
      /* 0x40000000000 */
      TEST_f_L (llrint, 4398046511104.0, 4398046511104LL);
      /* 0x1000000000000 */
      TEST_f_L (llrint, 281474976710656.0, 281474976710656LL);
      /* 0x10000000000000 */
      TEST_f_L (llrint, 4503599627370496.0, 4503599627370496LL);
      /* 0x10000080000000 */
      TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL);
      /* 0x20000000000000 */
      TEST_f_L (llrint, 9007199254740992.0, 9007199254740992LL);
      /* 0x80000000000000 */
      TEST_f_L (llrint, 36028797018963968.0, 36028797018963968LL);
      /* 0x100000000000000 */
      TEST_f_L (llrint, 72057594037927936.0, 72057594037927936LL);
#ifdef TEST_LDOUBLE
      /* The input can only be represented in long double.  */
      TEST_f_L (llrint, 4503599627370495.5L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370496.25L, 4503599627370497LL);
      TEST_f_L (llrint, 4503599627370496.5L, 4503599627370497LL);
      TEST_f_L (llrint, 4503599627370496.75L, 4503599627370497LL);
      TEST_f_L (llrint, 4503599627370497.5L, 4503599627370498LL);

      TEST_f_L (llrint, 4503599627370495.4999999999999L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370496.4999999999999L, 4503599627370497LL);
      TEST_f_L (llrint, 4503599627370497.4999999999999L, 4503599627370498LL);
      TEST_f_L (llrint, 4503599627370494.5000000000001L, 4503599627370495LL);
      TEST_f_L (llrint, 4503599627370495.5000000000001L, 4503599627370496LL);
      TEST_f_L (llrint, 4503599627370496.5000000000001L, 4503599627370497LL);

      TEST_f_L (llrint, -4503599627370495.5L, -4503599627370495LL);
      TEST_f_L (llrint, -4503599627370496.25L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370496.5L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370496.75L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370497.5L, -4503599627370497LL);

      TEST_f_L (llrint, -4503599627370495.4999999999999L, -4503599627370495LL);
      TEST_f_L (llrint, -4503599627370496.4999999999999L, -4503599627370496LL);
      TEST_f_L (llrint, -4503599627370497.4999999999999L, -4503599627370497LL);
      TEST_f_L (llrint, -4503599627370494.5000000000001L, -4503599627370494LL);
      TEST_f_L (llrint, -4503599627370495.5000000000001L, -4503599627370495LL);
      TEST_f_L (llrint, -4503599627370496.5000000000001L, -4503599627370496LL);

      TEST_f_L (llrint, 9007199254740991.5L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740992.25L, 9007199254740993LL);
      TEST_f_L (llrint, 9007199254740992.5L, 9007199254740993LL);
      TEST_f_L (llrint, 9007199254740992.75L, 9007199254740993LL);
      TEST_f_L (llrint, 9007199254740993.5L, 9007199254740994LL);

      TEST_f_L (llrint, 9007199254740991.4999999999999L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740992.4999999999999L, 9007199254740993LL);
      TEST_f_L (llrint, 9007199254740993.4999999999999L, 9007199254740994LL);
      TEST_f_L (llrint, 9007199254740991.5000000000001L, 9007199254740992LL);
      TEST_f_L (llrint, 9007199254740992.5000000000001L, 9007199254740993LL);
      TEST_f_L (llrint, 9007199254740993.5000000000001L, 9007199254740994LL);

      TEST_f_L (llrint, -9007199254740991.5L, -9007199254740991LL);
      TEST_f_L (llrint, -9007199254740992.25L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740992.5L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740992.75L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740993.5L, -9007199254740993LL);

      TEST_f_L (llrint, -9007199254740991.4999999999999L, -9007199254740991LL);
      TEST_f_L (llrint, -9007199254740992.4999999999999L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740993.4999999999999L, -9007199254740993LL);
      TEST_f_L (llrint, -9007199254740991.5000000000001L, -9007199254740991LL);
      TEST_f_L (llrint, -9007199254740992.5000000000001L, -9007199254740992LL);
      TEST_f_L (llrint, -9007199254740993.5000000000001L, -9007199254740993LL);

      TEST_f_L (llrint, 72057594037927935.5L, 72057594037927936LL);
      TEST_f_L (llrint, 72057594037927936.25L, 72057594037927937LL);
      TEST_f_L (llrint, 72057594037927936.5L, 72057594037927937LL);
      TEST_f_L (llrint, 72057594037927936.75L, 72057594037927937LL);
      TEST_f_L (llrint, 72057594037927937.5L, 72057594037927938LL);

      TEST_f_L (llrint, -72057594037927935.5L, -72057594037927935LL);
      TEST_f_L (llrint, -72057594037927936.25L, -72057594037927936LL);
      TEST_f_L (llrint, -72057594037927936.5L, -72057594037927936LL);
      TEST_f_L (llrint, -72057594037927936.75L, -72057594037927936LL);
      TEST_f_L (llrint, -72057594037927937.5L, -72057594037927937LL);

# if LDBL_MANT_DIG > 100
      TEST_f_L (llrint, 9223372036854775805.5L, 9223372036854775806LL);
      TEST_f_L (llrint, -9223372036854775805.5L, -9223372036854775805LL);
      TEST_f_L (llrint, 9223372036854775806.0L, 9223372036854775806LL);
      TEST_f_L (llrint, -9223372036854775806.0L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.25L, 9223372036854775807LL);
      TEST_f_L (llrint, -9223372036854775806.25L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.5L, 9223372036854775807L);
      TEST_f_L (llrint, -9223372036854775806.5L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775806.75L, 9223372036854775807LL);
      TEST_f_L (llrint, -9223372036854775806.75L, -9223372036854775806LL);
      TEST_f_L (llrint, 9223372036854775807.0L, 9223372036854775807LL);
      TEST_f_L (llrint, -9223372036854775807.0L, -9223372036854775807LL);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (llrint_upward);
}


static void
log_test (void)
{
  errno = 0;
  FUNC(log) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;
  START (log);

  TEST_f_f (log, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (log, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_f_f (log, 1, 0);

  TEST_f_f (log, -1, nan_value, INVALID_EXCEPTION);
  TEST_f_f (log, -max_value, nan_value, INVALID_EXCEPTION);
  TEST_f_f (log, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (log, plus_infty, plus_infty);
  TEST_f_f (log, nan_value, nan_value);

  TEST_f_f (log, M_El, 1);
  TEST_f_f (log, 1.0 / M_El, -1);
  TEST_f_f (log, 2, M_LN2l);
  TEST_f_f (log, 10, M_LN10l);
  TEST_f_f (log, 0.75L, -0.287682072451780927439219005993827432L);

  END (log);
}


static void
log10_test (void)
{
  errno = 0;
  FUNC(log10) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (log10);

  TEST_f_f (log10, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (log10, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_f_f (log10, 1, 0);

  /* log10 (x) == NaN plus invalid exception if x < 0.  */
  TEST_f_f (log10, -1, nan_value, INVALID_EXCEPTION);
  TEST_f_f (log10, -max_value, nan_value, INVALID_EXCEPTION);
  TEST_f_f (log10, minus_infty, nan_value, INVALID_EXCEPTION);

  TEST_f_f (log10, plus_infty, plus_infty);
  TEST_f_f (log10, nan_value, nan_value);

  TEST_f_f (log10, 0.1L, -1);
  TEST_f_f (log10, 10.0, 1);
  TEST_f_f (log10, 100.0, 2);
  TEST_f_f (log10, 10000.0, 4);
  TEST_f_f (log10, M_El, M_LOG10El);
  TEST_f_f (log10, 0.75L, -0.124938736608299953132449886193870744L);

  END (log10);
}


static void
log1p_test (void)
{
  errno = 0;
  FUNC(log1p) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (log1p);

  TEST_f_f (log1p, 0, 0);
  TEST_f_f (log1p, minus_zero, minus_zero);

  TEST_f_f (log1p, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (log1p, -2, nan_value, INVALID_EXCEPTION);
  TEST_f_f (log1p, -max_value, nan_value, INVALID_EXCEPTION);
  TEST_f_f (log1p, minus_infty, nan_value, INVALID_EXCEPTION);

  TEST_f_f (log1p, plus_infty, plus_infty);
  TEST_f_f (log1p, nan_value, nan_value);

  TEST_f_f (log1p, M_El - 1.0, 1);

  TEST_f_f (log1p, -0.25L, -0.287682072451780927439219005993827432L);
  TEST_f_f (log1p, -0.875, -2.07944154167983592825169636437452970L);

  END (log1p);
}


static void
log2_test (void)
{
  errno = 0;
  FUNC(log2) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (log2);

  TEST_f_f (log2, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (log2, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_f_f (log2, 1, 0);

  TEST_f_f (log2, -1, nan_value, INVALID_EXCEPTION);
  TEST_f_f (log2, -max_value, nan_value, INVALID_EXCEPTION);
  TEST_f_f (log2, minus_infty, nan_value, INVALID_EXCEPTION);

  TEST_f_f (log2, plus_infty, plus_infty);
  TEST_f_f (log2, nan_value, nan_value);

  TEST_f_f (log2, M_El, M_LOG2El);
  TEST_f_f (log2, 2.0, 1);
  TEST_f_f (log2, 16.0, 4);
  TEST_f_f (log2, 256.0, 8);
  TEST_f_f (log2, 0.75L, -.415037499278843818546261056052183492L);

  END (log2);
}


static void
logb_test (void)
{
  START (logb);

  TEST_f_f (logb, plus_infty, plus_infty);
  TEST_f_f (logb, minus_infty, plus_infty);

  TEST_f_f (logb, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);

  TEST_f_f (logb, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (logb, nan_value, nan_value);

  TEST_f_f (logb, 1, 0);
  TEST_f_f (logb, M_El, 1);
  TEST_f_f (logb, 1024, 10);
  TEST_f_f (logb, -2000, 10);

  END (logb);
}

static void
logb_test_downward (void)
{
  int save_round_mode;
  errno = 0;

  FUNC(logb) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (logb_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {

      /* IEEE 754-2008 says (section 5.3.3) that "logB(1) is +0.".  Libm
         should not return -0 from logb in any rounding mode.  PowerPC32 has
         failed with this test for power4 logb (and logbl on all PowerPC
         platforms) in the past due to instruction selection.  GCC PR 52775
         provides the availability of the fcfid insn in 32-bit mode which
         eliminates the use of fsub in this instance and prevents the negative
         signed 0.0.  */

      /* BZ #887  */
      TEST_f_f (logb, 1.000e+0, plus_zero);
    }

  fesetround (save_round_mode);

  END (logb_downward);
}

static void
lround_test (void)
{
  START (lround);

  TEST_f_l (lround, 0, 0);
  TEST_f_l (lround, minus_zero, 0);
  TEST_f_l (lround, 0.2L, 0.0);
  TEST_f_l (lround, -0.2L, 0);
  TEST_f_l (lround, 0.5, 1);
  TEST_f_l (lround, -0.5, -1);
  TEST_f_l (lround, 0.8L, 1);
  TEST_f_l (lround, -0.8L, -1);
  TEST_f_l (lround, 1.5, 2);
  TEST_f_l (lround, -1.5, -2);
  TEST_f_l (lround, 22514.5, 22515);
  TEST_f_l (lround, -22514.5, -22515);
  TEST_f_l (lround, 1071930.0008, 1071930);
#ifndef TEST_FLOAT
  TEST_f_l (lround, 1073741824.01, 1073741824);
# if LONG_MAX > 281474976710656
  TEST_f_l (lround, 281474976710656.025, 281474976710656);
  TEST_f_l (lround, 18014398509481974, 18014398509481974);
# endif
  TEST_f_l (lround, 2097152.5, 2097153);
  TEST_f_l (lround, -2097152.5, -2097153);
  /* nextafter(0.5,-1)  */
  TEST_f_l (lround, 0x1.fffffffffffffp-2, 0);
  /* nextafter(-0.5,1)  */
  TEST_f_l (lround, -0x1.fffffffffffffp-2, 0);
#else
  /* nextafter(0.5,-1)  */
  TEST_f_l (lround, 0x1.fffffp-2, 0);
  /* nextafter(-0.5,1)  */
  TEST_f_l (lround, -0x1.fffffp-2, 0);
  TEST_f_l (lround, 0x1.fffffep+23, 16777215);
  TEST_f_l (lround, -0x1.fffffep+23, -16777215);
#endif
  END (lround);
}


static void
llround_test (void)
{
  START (llround);

  TEST_f_L (llround, 0, 0);
  TEST_f_L (llround, minus_zero, 0);
  TEST_f_L (llround, 0.2L, 0.0);
  TEST_f_L (llround, -0.2L, 0);
  TEST_f_L (llround, 0.5, 1);
  TEST_f_L (llround, -0.5, -1);
  TEST_f_L (llround, 0.8L, 1);
  TEST_f_L (llround, -0.8L, -1);
  TEST_f_L (llround, 1.5, 2);
  TEST_f_L (llround, -1.5, -2);
  TEST_f_L (llround, 22514.5, 22515);
  TEST_f_L (llround, -22514.5, -22515);
  TEST_f_l (llround, 1071930.0008, 1071930);
#ifndef TEST_FLOAT
  TEST_f_L (llround, 2097152.5, 2097153);
  TEST_f_L (llround, -2097152.5, -2097153);
  TEST_f_L (llround, 34359738368.5, 34359738369ll);
  TEST_f_L (llround, -34359738368.5, -34359738369ll);
  TEST_f_L (llround, -3.65309740835E17, -365309740835000000LL);
#endif

  /* Test boundary conditions.  */
  /* 0x1FFFFF */
  TEST_f_L (llround, 2097151.0, 2097151LL);
  /* 0x800000 */
  TEST_f_L (llround, 8388608.0, 8388608LL);
  /* 0x1000000 */
  TEST_f_L (llround, 16777216.0, 16777216LL);
  /* 0x20000000000 */
  TEST_f_L (llround, 2199023255552.0, 2199023255552LL);
  /* 0x40000000000 */
  TEST_f_L (llround, 4398046511104.0, 4398046511104LL);
  /* 0x1000000000000 */
  TEST_f_L (llround, 281474976710656.0, 281474976710656LL);
  /* 0x10000000000000 */
  TEST_f_L (llround, 4503599627370496.0, 4503599627370496LL);
  /* 0x10000080000000 */
  TEST_f_L (llround, 4503601774854144.0, 4503601774854144LL);
  /* 0x20000000000000 */
  TEST_f_L (llround, 9007199254740992.0, 9007199254740992LL);
  /* 0x80000000000000 */
  TEST_f_L (llround, 36028797018963968.0, 36028797018963968LL);
  /* 0x100000000000000 */
  TEST_f_L (llround, 72057594037927936.0, 72057594037927936LL);

#ifndef TEST_FLOAT
  /* 0x100000000 */
  TEST_f_L (llround, 4294967295.5, 4294967296LL);
  /* 0x200000000 */
  TEST_f_L (llround, 8589934591.5, 8589934592LL);

  /* nextafter(0.5,-1)  */
  TEST_f_L (llround, 0x1.fffffffffffffp-2, 0);
  /* nextafter(-0.5,1)  */
  TEST_f_L (llround, -0x1.fffffffffffffp-2, 0);
  /* On PowerPC an exponent of '52' is the largest incrementally
   * representable sequence of whole-numbers in the 'double' range.  We test
   * lround to make sure that a guard bit set during the lround operation
   * hasn't forced an erroneous shift giving us an incorrect result.  The odd
   * numbers between +-(2^52+1 and 2^53-1) are affected since they have the
   * rightmost bit set.  */
  /* +-(2^52+1)  */
  TEST_f_L (llround, 0x1.0000000000001p+52,4503599627370497LL);
  TEST_f_L (llround, -0x1.0000000000001p+52,-4503599627370497LL);
  /* +-(2^53-1): Input is the last (positive and negative) incrementally
   * representable whole-number in the 'double' range that might round
   * erroneously.  */
  TEST_f_L (llround, 0x1.fffffffffffffp+52, 9007199254740991LL);
  TEST_f_L (llround, -0x1.fffffffffffffp+52, -9007199254740991LL);
#else
  /* nextafter(0.5,-1)  */
  TEST_f_L (llround, 0x1.fffffep-2, 0);
  /* nextafter(-0.5,1)  */
  TEST_f_L (llround, -0x1.fffffep-2, 0);
  /* As above, on PowerPC an exponent of '23' is the largest incrementally
   * representable sequence of whole-numbers in the 'float' range.
   * Likewise, numbers between +-(2^23+1 and 2^24-1) are affected.  */
  TEST_f_L (llround, 0x1.000002p+23,8388609);
  TEST_f_L (llround, -0x1.000002p+23,-8388609);
  TEST_f_L (llround, 0x1.fffffep+23, 16777215);
  TEST_f_L (llround, -0x1.fffffep+23, -16777215);
#endif


#ifdef TEST_LDOUBLE
  /* The input can only be represented in long double.  */
  TEST_f_L (llround, 4503599627370495.5L, 4503599627370496LL);
  TEST_f_L (llround, 4503599627370496.25L, 4503599627370496LL);
  TEST_f_L (llround, 4503599627370496.5L, 4503599627370497LL);
  TEST_f_L (llround, 4503599627370496.75L, 4503599627370497LL);
  TEST_f_L (llround, 4503599627370497.5L, 4503599627370498LL);

# if LDBL_MANT_DIG > 100
  TEST_f_L (llround, 4503599627370495.4999999999999L, 4503599627370495LL);
  TEST_f_L (llround, 4503599627370496.4999999999999L, 4503599627370496LL);
  TEST_f_L (llround, 4503599627370497.4999999999999L, 4503599627370497LL);
  TEST_f_L (llround, 4503599627370494.5000000000001L, 4503599627370495LL);
  TEST_f_L (llround, 4503599627370495.5000000000001L, 4503599627370496LL);
  TEST_f_L (llround, 4503599627370496.5000000000001L, 4503599627370497LL);

  TEST_f_L (llround, -4503599627370495.4999999999999L, -4503599627370495LL);
  TEST_f_L (llround, -4503599627370496.4999999999999L, -4503599627370496LL);
  TEST_f_L (llround, -4503599627370497.4999999999999L, -4503599627370497LL);
  TEST_f_L (llround, -4503599627370494.5000000000001L, -4503599627370495LL);
  TEST_f_L (llround, -4503599627370495.5000000000001L, -4503599627370496LL);
  TEST_f_L (llround, -4503599627370496.5000000000001L, -4503599627370497LL);
# endif

  TEST_f_L (llround, -4503599627370495.5L, -4503599627370496LL);
  TEST_f_L (llround, -4503599627370496.25L, -4503599627370496LL);
  TEST_f_L (llround, -4503599627370496.5L, -4503599627370497LL);
  TEST_f_L (llround, -4503599627370496.75L, -4503599627370497LL);
  TEST_f_L (llround, -4503599627370497.5L, -4503599627370498LL);

  TEST_f_L (llround, 9007199254740991.5L, 9007199254740992LL);
  TEST_f_L (llround, 9007199254740992.25L, 9007199254740992LL);
  TEST_f_L (llround, 9007199254740992.5L, 9007199254740993LL);
  TEST_f_L (llround, 9007199254740992.75L, 9007199254740993LL);
  TEST_f_L (llround, 9007199254740993.5L, 9007199254740994LL);

# if LDBL_MANT_DIG > 100
  TEST_f_L (llround, 9007199254740991.4999999999999L, 9007199254740991LL);
  TEST_f_L (llround, 9007199254740992.4999999999999L, 9007199254740992LL);
  TEST_f_L (llround, 9007199254740993.4999999999999L, 9007199254740993LL);
  TEST_f_L (llround, 9007199254740991.5000000000001L, 9007199254740992LL);
  TEST_f_L (llround, 9007199254740992.5000000000001L, 9007199254740993LL);
  TEST_f_L (llround, 9007199254740993.5000000000001L, 9007199254740994LL);

  TEST_f_L (llround, -9007199254740991.4999999999999L, -9007199254740991LL);
  TEST_f_L (llround, -9007199254740992.4999999999999L, -9007199254740992LL);
  TEST_f_L (llround, -9007199254740993.4999999999999L, -9007199254740993LL);
  TEST_f_L (llround, -9007199254740991.5000000000001L, -9007199254740992LL);
  TEST_f_L (llround, -9007199254740992.5000000000001L, -9007199254740993LL);
  TEST_f_L (llround, -9007199254740993.5000000000001L, -9007199254740994LL);
# endif

  TEST_f_L (llround, -9007199254740991.5L, -9007199254740992LL);
  TEST_f_L (llround, -9007199254740992.25L, -9007199254740992LL);
  TEST_f_L (llround, -9007199254740992.5L, -9007199254740993LL);
  TEST_f_L (llround, -9007199254740992.75L, -9007199254740993LL);
  TEST_f_L (llround, -9007199254740993.5L, -9007199254740994LL);

  TEST_f_L (llround, 72057594037927935.5L, 72057594037927936LL);
  TEST_f_L (llround, 72057594037927936.25L, 72057594037927936LL);
  TEST_f_L (llround, 72057594037927936.5L, 72057594037927937LL);
  TEST_f_L (llround, 72057594037927936.75L, 72057594037927937LL);
  TEST_f_L (llround, 72057594037927937.5L, 72057594037927938LL);

  TEST_f_L (llround, -72057594037927935.5L, -72057594037927936LL);
  TEST_f_L (llround, -72057594037927936.25L, -72057594037927936LL);
  TEST_f_L (llround, -72057594037927936.5L, -72057594037927937LL);
  TEST_f_L (llround, -72057594037927936.75L, -72057594037927937LL);
  TEST_f_L (llround, -72057594037927937.5L, -72057594037927938LL);

  TEST_f_L (llround, 9223372036854775806.25L, 9223372036854775806LL);
  TEST_f_L (llround, -9223372036854775806.25L, -9223372036854775806LL);
  TEST_f_L (llround, 9223372036854775806.5L, 9223372036854775807LL);
  TEST_f_L (llround, -9223372036854775806.5L, -9223372036854775807LL);
  TEST_f_L (llround, 9223372036854775807.0L, 9223372036854775807LL);
  TEST_f_L (llround, -9223372036854775807.0L, -9223372036854775807LL);
#endif

  END (llround);
}

static void
modf_test (void)
{
  FLOAT x;

  START (modf);

  TEST_fF_f1 (modf, plus_infty, 0, plus_infty);
  TEST_fF_f1 (modf, minus_infty, minus_zero, minus_infty);
  TEST_fF_f1 (modf, nan_value, nan_value, nan_value);
  TEST_fF_f1 (modf, 0, 0, 0);
  TEST_fF_f1 (modf, 1.5, 0.5, 1);
  TEST_fF_f1 (modf, 2.5, 0.5, 2);
  TEST_fF_f1 (modf, -2.5, -0.5, -2);
  TEST_fF_f1 (modf, 20, 0, 20);
  TEST_fF_f1 (modf, 21, 0, 21);
  TEST_fF_f1 (modf, 89.5, 0.5, 89);

  END (modf);
}


static void
nearbyint_test (void)
{
  START (nearbyint);

  TEST_f_f (nearbyint, 0.0, 0.0);
  TEST_f_f (nearbyint, minus_zero, minus_zero);
  TEST_f_f (nearbyint, plus_infty, plus_infty);
  TEST_f_f (nearbyint, minus_infty, minus_infty);
  TEST_f_f (nearbyint, nan_value, nan_value);

  /* Subnormal values */
  TEST_f_f (nearbyint, -8.98847e+307, -8.98847e+307);
  TEST_f_f (nearbyint, -4.45015e-308, minus_zero);

  /* Default rounding mode is round to nearest.  */
  TEST_f_f (nearbyint, 0.5, 0.0);
  TEST_f_f (nearbyint, 1.5, 2.0);
  TEST_f_f (nearbyint, -0.5, minus_zero);
  TEST_f_f (nearbyint, -1.5, -2.0);

  TEST_f_f (nearbyint, 262144.75, 262145.0);
  TEST_f_f (nearbyint, 262142.75, 262143.0);
  TEST_f_f (nearbyint, 524286.75, 524287.0);
  TEST_f_f (nearbyint, 524288.75, 524289.0);

  TEST_f_f (nearbyint, 1048576.75, 1048577.0);
  TEST_f_f (nearbyint, 2097152.75, 2097153.0);
  TEST_f_f (nearbyint, 2492472.75, 2492473.0);
  TEST_f_f (nearbyint, 2886220.75, 2886221.0);
  TEST_f_f (nearbyint, 3058792.75, 3058793.0);
  TEST_f_f (nearbyint, -1048576.75, -1048577.0);
  TEST_f_f (nearbyint, -2097152.75, -2097153.0);
  TEST_f_f (nearbyint, -2492472.75, -2492473.0);
  TEST_f_f (nearbyint, -2886220.75, -2886221.0);
  TEST_f_f (nearbyint, -3058792.75, -3058793.0);
#ifndef TEST_FLOAT
  TEST_f_f (nearbyint, 70368744177664.75, 70368744177665.0);
  TEST_f_f (nearbyint, 140737488355328.75, 140737488355329.0);
  TEST_f_f (nearbyint, 281474976710656.75, 281474976710657.0);
  TEST_f_f (nearbyint, 562949953421312.75, 562949953421313.0);
  TEST_f_f (nearbyint, 1125899906842624.75, 1125899906842625.0);
  TEST_f_f (nearbyint, -70368744177664.75, -70368744177665.0);
  TEST_f_f (nearbyint, -140737488355328.75, -140737488355329.0);
  TEST_f_f (nearbyint, -281474976710656.75, -281474976710657.0);
  TEST_f_f (nearbyint, -562949953421312.75, -562949953421313.0);
  TEST_f_f (nearbyint, -1125899906842624.75, -1125899906842625.0);
#endif

  END (nearbyint);
}

static void
nextafter_test (void)
{

  START (nextafter);

  TEST_ff_f (nextafter, 0, 0, 0);
  TEST_ff_f (nextafter, minus_zero, 0, 0);
  TEST_ff_f (nextafter, 0, minus_zero, minus_zero);
  TEST_ff_f (nextafter, minus_zero, minus_zero, minus_zero);

  TEST_ff_f (nextafter, 9, 9, 9);
  TEST_ff_f (nextafter, -9, -9, -9);
  TEST_ff_f (nextafter, plus_infty, plus_infty, plus_infty);
  TEST_ff_f (nextafter, minus_infty, minus_infty, minus_infty);

  TEST_ff_f (nextafter, nan_value, 1.1L, nan_value);
  TEST_ff_f (nextafter, 1.1L, nan_value, nan_value);
  TEST_ff_f (nextafter, nan_value, nan_value, nan_value);

  FLOAT fltmax = CHOOSE (LDBL_MAX, DBL_MAX, FLT_MAX,
			 LDBL_MAX, DBL_MAX, FLT_MAX);
  TEST_ff_f (nextafter, fltmax, plus_infty, plus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (nextafter, -fltmax, minus_infty, minus_infty, OVERFLOW_EXCEPTION);

#ifdef TEST_LDOUBLE
  // XXX Enable once gcc is fixed.
  //TEST_ff_f (nextafter, 0x0.00000040000000000000p-16385L, -0.1L, 0x0.0000003ffffffff00000p-16385L);
#endif

  /* XXX We need the hexadecimal FP number representation here for further
     tests.  */

  END (nextafter);
}


static void
nexttoward_test (void)
{
  START (nexttoward);
  TEST_ff_f (nexttoward, 0, 0, 0);
  TEST_ff_f (nexttoward, minus_zero, 0, 0);
  TEST_ff_f (nexttoward, 0, minus_zero, minus_zero);
  TEST_ff_f (nexttoward, minus_zero, minus_zero, minus_zero);

  TEST_ff_f (nexttoward, 9, 9, 9);
  TEST_ff_f (nexttoward, -9, -9, -9);
  TEST_ff_f (nexttoward, plus_infty, plus_infty, plus_infty);
  TEST_ff_f (nexttoward, minus_infty, minus_infty, minus_infty);

  TEST_ff_f (nexttoward, nan_value, 1.1L, nan_value);
  TEST_ff_f (nexttoward, 1.1L, nan_value, nan_value);
  TEST_ff_f (nexttoward, nan_value, nan_value, nan_value);

#ifdef TEST_FLOAT
  TEST_ff_f (nexttoward, 1.0, 1.1L, 0x1.000002p0);
  TEST_ff_f (nexttoward, 1.0, LDBL_MAX, 0x1.000002p0);
  TEST_ff_f (nexttoward, 1.0, 0x1.0000000000001p0, 0x1.000002p0);
  TEST_ff_f (nexttoward, 1.0, 0.9L, 0x0.ffffffp0);
  TEST_ff_f (nexttoward, 1.0, -LDBL_MAX, 0x0.ffffffp0);
  TEST_ff_f (nexttoward, 1.0, 0x0.fffffffffffff8p0, 0x0.ffffffp0);
  TEST_ff_f (nexttoward, -1.0, -1.1L, -0x1.000002p0);
  TEST_ff_f (nexttoward, -1.0, -LDBL_MAX, -0x1.000002p0);
  TEST_ff_f (nexttoward, -1.0, -0x1.0000000000001p0, -0x1.000002p0);
  TEST_ff_f (nexttoward, -1.0, -0.9L, -0x0.ffffffp0);
  TEST_ff_f (nexttoward, -1.0, LDBL_MAX, -0x0.ffffffp0);
  TEST_ff_f (nexttoward, -1.0, -0x0.fffffffffffff8p0, -0x0.ffffffp0);
  TEST_ff_f (nexttoward, -0x1.3p-145, -0xap-148L, -0x1.4p-145);
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (nexttoward, 1.0, 0x1.000000000000002p0L, 0x1.000002p0);
  TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffp0L, 0x0.ffffffp0);
  TEST_ff_f (nexttoward, -1.0, -0x1.000000000000002p0L, -0x1.000002p0);
  TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffp0L, -0x0.ffffffp0);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (nexttoward, 1.0, 0x1.000000000000000000000000008p0L, 0x1.000002p0);
  TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffcp0L, 0x0.ffffffp0);
  TEST_ff_f (nexttoward, -1.0, -0x1.000000000000000000000000008p0L, -0x1.000002p0);
  TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffcp0L, -0x0.ffffffp0);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (nexttoward, 1.0, 0x1.0000000000000000000000000001p0L, 0x1.000002p0);
  TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffp0);
  TEST_ff_f (nexttoward, -1.0, -0x1.0000000000000000000000000001p0L, -0x1.000002p0);
  TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffp0);
# endif
#endif
#ifdef TEST_DOUBLE
  TEST_ff_f (nexttoward, 1.0, 1.1L, 0x1.0000000000001p0);
  TEST_ff_f (nexttoward, 1.0, LDBL_MAX, 0x1.0000000000001p0);
  TEST_ff_f (nexttoward, 1.0, 0x1.0000000000001p0, 0x1.0000000000001p0);
  TEST_ff_f (nexttoward, 1.0, 0.9L, 0x0.fffffffffffff8p0);
  TEST_ff_f (nexttoward, 1.0, -LDBL_MAX, 0x0.fffffffffffff8p0);
  TEST_ff_f (nexttoward, 1.0, 0x0.fffffffffffff8p0, 0x0.fffffffffffff8p0);
  TEST_ff_f (nexttoward, -1.0, -1.1L, -0x1.0000000000001p0);
  TEST_ff_f (nexttoward, -1.0, -LDBL_MAX, -0x1.0000000000001p0);
  TEST_ff_f (nexttoward, -1.0, -0x1.0000000000001p0, -0x1.0000000000001p0);
  TEST_ff_f (nexttoward, -1.0, -0.9L, -0x0.fffffffffffff8p0);
  TEST_ff_f (nexttoward, -1.0, LDBL_MAX, -0x0.fffffffffffff8p0);
  TEST_ff_f (nexttoward, -1.0, -0x0.fffffffffffff8p0, -0x0.fffffffffffff8p0);
  TEST_ff_f (nexttoward, -1.0, -0x8.00346dc5d6388p-3L, -0x1.0000000000001p0);
  TEST_ff_f (nexttoward, 0x1p-1074, 0x1p-1073L, 0x1p-1073);
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (nexttoward, 1.0, 0x1.000000000000002p0L, 0x1.0000000000001p0);
  TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffp0L, 0x0.fffffffffffff8p0);
  TEST_ff_f (nexttoward, -1.0, -0x1.000000000000002p0L, -0x1.0000000000001p0);
  TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffp0L, -0x0.fffffffffffff8p0);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (nexttoward, 1.0, 0x1.000000000000000000000000008p0L, 0x1.0000000000001p0);
  TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffcp0L, 0x0.fffffffffffff8p0);
  TEST_ff_f (nexttoward, -1.0, -0x1.000000000000000000000000008p0L, -0x1.0000000000001p0);
  TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffcp0L, -0x0.fffffffffffff8p0);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (nexttoward, 1.0, 0x1.0000000000000000000000000001p0L, 0x1.0000000000001p0);
  TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.fffffffffffff8p0);
  TEST_ff_f (nexttoward, -1.0, -0x1.0000000000000000000000000001p0L, -0x1.0000000000001p0);
  TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.fffffffffffff8p0);
# endif
#endif

  END (nexttoward);
}


static void
pow_test (void)
{

  errno = 0;
  FUNC(pow) (0, 0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (pow);

  TEST_ff_f (pow, 0, 0, 1);
  TEST_ff_f (pow, 0, minus_zero, 1);
  TEST_ff_f (pow, minus_zero, 0, 1);
  TEST_ff_f (pow, minus_zero, minus_zero, 1);

  TEST_ff_f (pow, 10, 0, 1);
  TEST_ff_f (pow, 10, minus_zero, 1);
  TEST_ff_f (pow, -10, 0, 1);
  TEST_ff_f (pow, -10, minus_zero, 1);

  TEST_ff_f (pow, nan_value, 0, 1);
  TEST_ff_f (pow, nan_value, minus_zero, 1);


#ifndef TEST_INLINE
  TEST_ff_f (pow, 1.1L, plus_infty, plus_infty);
  TEST_ff_f (pow, plus_infty, plus_infty, plus_infty);
  TEST_ff_f (pow, -1.1L, plus_infty, plus_infty);
  TEST_ff_f (pow, minus_infty, plus_infty, plus_infty);

  TEST_ff_f (pow, 0.9L, plus_infty, 0);
  TEST_ff_f (pow, 1e-7L, plus_infty, 0);
  TEST_ff_f (pow, -0.9L, plus_infty, 0);
  TEST_ff_f (pow, -1e-7L, plus_infty, 0);

  TEST_ff_f (pow, 1.1L, minus_infty, 0);
  TEST_ff_f (pow, plus_infty, minus_infty, 0);
  TEST_ff_f (pow, -1.1L, minus_infty, 0);
  TEST_ff_f (pow, minus_infty, minus_infty, 0);

  TEST_ff_f (pow, 0.9L, minus_infty, plus_infty);
  TEST_ff_f (pow, 1e-7L, minus_infty, plus_infty);
  TEST_ff_f (pow, -0.9L, minus_infty, plus_infty);
  TEST_ff_f (pow, -1e-7L, minus_infty, plus_infty);

  TEST_ff_f (pow, plus_infty, 1e-7L, plus_infty);
  TEST_ff_f (pow, plus_infty, 1, plus_infty);
  TEST_ff_f (pow, plus_infty, 1e7L, plus_infty);

  TEST_ff_f (pow, plus_infty, -1e-7L, 0);
  TEST_ff_f (pow, plus_infty, -1, 0);
  TEST_ff_f (pow, plus_infty, -1e7L, 0);

  TEST_ff_f (pow, minus_infty, 1, minus_infty);
  TEST_ff_f (pow, minus_infty, 11, minus_infty);
  TEST_ff_f (pow, minus_infty, 1001, minus_infty);

  TEST_ff_f (pow, minus_infty, 2, plus_infty);
  TEST_ff_f (pow, minus_infty, 12, plus_infty);
  TEST_ff_f (pow, minus_infty, 1002, plus_infty);
  TEST_ff_f (pow, minus_infty, 0.1L, plus_infty);
  TEST_ff_f (pow, minus_infty, 1.1L, plus_infty);
  TEST_ff_f (pow, minus_infty, 11.1L, plus_infty);
  TEST_ff_f (pow, minus_infty, 1001.1L, plus_infty);

  TEST_ff_f (pow, minus_infty, -1, minus_zero);
  TEST_ff_f (pow, minus_infty, -11, minus_zero);
  TEST_ff_f (pow, minus_infty, -1001, minus_zero);

  TEST_ff_f (pow, minus_infty, -2, 0);
  TEST_ff_f (pow, minus_infty, -12, 0);
  TEST_ff_f (pow, minus_infty, -1002, 0);
  TEST_ff_f (pow, minus_infty, -0.1L, 0);
  TEST_ff_f (pow, minus_infty, -1.1L, 0);
  TEST_ff_f (pow, minus_infty, -11.1L, 0);
  TEST_ff_f (pow, minus_infty, -1001.1L, 0);
#endif

  TEST_ff_f (pow, nan_value, nan_value, nan_value);
  TEST_ff_f (pow, 0, nan_value, nan_value);
  TEST_ff_f (pow, 1, nan_value, 1);
  TEST_ff_f (pow, -1, nan_value, nan_value);
  TEST_ff_f (pow, nan_value, 1, nan_value);
  TEST_ff_f (pow, nan_value, -1, nan_value);

  /* pow (x, NaN) == NaN.  */
  TEST_ff_f (pow, 3.0, nan_value, nan_value);
  TEST_ff_f (pow, minus_zero, nan_value, nan_value);
  TEST_ff_f (pow, plus_infty, nan_value, nan_value);
  TEST_ff_f (pow, -3.0, nan_value, nan_value);
  TEST_ff_f (pow, minus_infty, nan_value, nan_value);

  TEST_ff_f (pow, nan_value, 3.0, nan_value);
  TEST_ff_f (pow, nan_value, -3.0, nan_value);
  TEST_ff_f (pow, nan_value, plus_infty, nan_value);
  TEST_ff_f (pow, nan_value, minus_infty, nan_value);
  TEST_ff_f (pow, nan_value, 2.5, nan_value);
  TEST_ff_f (pow, nan_value, -2.5, nan_value);

  TEST_ff_f (pow, 1, plus_infty, 1);
  TEST_ff_f (pow, -1, plus_infty, 1);
  TEST_ff_f (pow, 1, minus_infty, 1);
  TEST_ff_f (pow, -1, minus_infty, 1);
  TEST_ff_f (pow, 1, 1, 1);
  TEST_ff_f (pow, 1, -1, 1);
  TEST_ff_f (pow, 1, 1.25, 1);
  TEST_ff_f (pow, 1, -1.25, 1);
  TEST_ff_f (pow, 1, 0x1p62L, 1);
  TEST_ff_f (pow, 1, 0x1p63L, 1);
  TEST_ff_f (pow, 1, 0x1p64L, 1);
  TEST_ff_f (pow, 1, 0x1p72L, 1);

  /* pow (x, +-0) == 1.  */
  TEST_ff_f (pow, plus_infty, 0, 1);
  TEST_ff_f (pow, plus_infty, minus_zero, 1);
  TEST_ff_f (pow, minus_infty, 0, 1);
  TEST_ff_f (pow, minus_infty, minus_zero, 1);
  TEST_ff_f (pow, 32.75L, 0, 1);
  TEST_ff_f (pow, 32.75L, minus_zero, 1);
  TEST_ff_f (pow, -32.75L, 0, 1);
  TEST_ff_f (pow, -32.75L, minus_zero, 1);
  TEST_ff_f (pow, 0x1p72L, 0, 1);
  TEST_ff_f (pow, 0x1p72L, minus_zero, 1);
  TEST_ff_f (pow, 0x1p-72L, 0, 1);
  TEST_ff_f (pow, 0x1p-72L, minus_zero, 1);

  TEST_ff_f (pow, -0.1L, 1.1L, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -0.1L, -1.1L, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -10.1L, 1.1L, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -10.1L, -1.1L, nan_value, INVALID_EXCEPTION);

  errno = 0;
  TEST_ff_f (pow, 0, -1, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, 0, -11, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, 0, -0xffffff, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
#ifndef TEST_FLOAT
  errno = 0;
  TEST_ff_f (pow, 0, -0x1.fffffffffffffp+52L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  errno = 0;
  TEST_ff_f (pow, 0, -0x1.fffffffffffffffep+63L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
# endif
# if LDBL_MANT_DIG >= 106
  errno = 0;
  TEST_ff_f (pow, 0, -0x1.ffffffffffffffffffffffffff8p+105L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
# endif
# if LDBL_MANT_DIG >= 113
  errno = 0;
  TEST_ff_f (pow, 0, -0x1.ffffffffffffffffffffffffffffp+112L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
# endif
#endif
  TEST_ff_f (pow, minus_zero, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -11L, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0xffffff, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0x1fffffe, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-even) == ERANGE", errno, ERANGE, 0, 0, 0);
#ifndef TEST_FLOAT
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0x1.fffffffffffffp+52L, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0x1.fffffffffffffp+53L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-even) == ERANGE", errno, ERANGE, 0, 0, 0);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0x1.fffffffffffffffep+63L, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0x1.fffffffffffffffep+64L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-even) == ERANGE", errno, ERANGE, 0, 0, 0);
# endif
# if LDBL_MANT_DIG >= 106
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0x1.ffffffffffffffffffffffffff8p+105L, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0x1.ffffffffffffffffffffffffff8p+106L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-even) == ERANGE", errno, ERANGE, 0, 0, 0);
# endif
# if LDBL_MANT_DIG >= 113
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0x1.ffffffffffffffffffffffffffffp+112L, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-odd) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0x1.ffffffffffffffffffffffffffffp+113L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-even) == ERANGE", errno, ERANGE, 0, 0, 0);
# endif
#endif

  errno = 0;
  TEST_ff_f (pow, 0, -2, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-even) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, 0, -11.1L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-num) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, 0, -0x1p24, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-num) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, 0, -0x1p127, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-num) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, 0, -max_value, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(0,-num) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -2, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-even) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -11.1L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-num) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0x1p24, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-num) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -0x1p127, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-num) == ERANGE", errno, ERANGE, 0, 0, 0);
  errno = 0;
  TEST_ff_f (pow, minus_zero, -max_value, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  check_int ("errno for pow(-0,-num) == ERANGE", errno, ERANGE, 0, 0, 0);

  TEST_ff_f (pow, 0x1p72L, 0x1p72L, plus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, 10, -0x1p72L, 0);
  TEST_ff_f (pow, max_value, max_value, plus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, 10, -max_value, 0);

  TEST_ff_f (pow, 0, 1, 0);
  TEST_ff_f (pow, 0, 11, 0);

  TEST_ff_f (pow, minus_zero, 1, minus_zero);
  TEST_ff_f (pow, minus_zero, 11, minus_zero);

  TEST_ff_f (pow, 0, 2, 0);
  TEST_ff_f (pow, 0, 11.1L, 0);

  TEST_ff_f (pow, minus_zero, 2, 0);
  TEST_ff_f (pow, minus_zero, 11.1L, 0);
  TEST_ff_f (pow, 0, plus_infty, 0);
  TEST_ff_f (pow, minus_zero, plus_infty, 0);

#ifndef TEST_INLINE
  /* pow (x, +inf) == +inf for |x| > 1.  */
  TEST_ff_f (pow, 1.5, plus_infty, plus_infty);

  /* pow (x, +inf) == +0 for |x| < 1.  */
  TEST_ff_f (pow, 0.5, plus_infty, 0.0);

  /* pow (x, -inf) == +0 for |x| > 1.  */
  TEST_ff_f (pow, 1.5, minus_infty, 0.0);

  /* pow (x, -inf) == +inf for |x| < 1.  */
  TEST_ff_f (pow, 0.5, minus_infty, plus_infty);
#endif

  /* pow (+inf, y) == +inf for y > 0.  */
  TEST_ff_f (pow, plus_infty, 2, plus_infty);
  TEST_ff_f (pow, plus_infty, 0xffffff, plus_infty);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, plus_infty, 0x1.fffffffffffffp+52L, plus_infty);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, plus_infty, 0x1.fffffffffffffffep+63L, plus_infty);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, plus_infty, 0x1.ffffffffffffffffffffffffff8p+105L, plus_infty);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, plus_infty, 0x1.ffffffffffffffffffffffffffffp+112L, plus_infty);
# endif
#endif
  TEST_ff_f (pow, plus_infty, 0x1p24, plus_infty);
  TEST_ff_f (pow, plus_infty, 0x1p127, plus_infty);
  TEST_ff_f (pow, plus_infty, max_value, plus_infty);

  /* pow (+inf, y) == +0 for y < 0.  */
  TEST_ff_f (pow, plus_infty, -1, 0.0);
  TEST_ff_f (pow, plus_infty, -0xffffff, 0.0);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, plus_infty, -0x1.fffffffffffffp+52L, 0.0);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, plus_infty, -0x1.fffffffffffffffep+63L, 0.0);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, plus_infty, -0x1.ffffffffffffffffffffffffff8p+105L, 0.0);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, plus_infty, -0x1.ffffffffffffffffffffffffffffp+112L, 0.0);
# endif
#endif
  TEST_ff_f (pow, plus_infty, -0x1p24, 0.0);
  TEST_ff_f (pow, plus_infty, -0x1p127, 0.0);
  TEST_ff_f (pow, plus_infty, -max_value, 0.0);

  /* pow (-inf, y) == -inf for y an odd integer > 0.  */
  TEST_ff_f (pow, minus_infty, 27, minus_infty);
  TEST_ff_f (pow, minus_infty, 0xffffff, minus_infty);
  TEST_ff_f (pow, minus_infty, 0x1fffffe, plus_infty);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, minus_infty, 0x1.fffffffffffffp+52L, minus_infty);
  TEST_ff_f (pow, minus_infty, 0x1.fffffffffffffp+53L, plus_infty);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, minus_infty, 0x1.fffffffffffffffep+63L, minus_infty);
  TEST_ff_f (pow, minus_infty, 0x1.fffffffffffffffep+64L, plus_infty);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, minus_infty, 0x1.ffffffffffffffffffffffffff8p+105L, minus_infty);
  TEST_ff_f (pow, minus_infty, 0x1.ffffffffffffffffffffffffff8p+106L, plus_infty);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, minus_infty, 0x1.ffffffffffffffffffffffffffffp+112L, minus_infty);
  TEST_ff_f (pow, minus_infty, 0x1.ffffffffffffffffffffffffffffp+113L, plus_infty);
# endif
#endif

  /* pow (-inf, y) == +inf for y > 0 and not an odd integer.  */
  TEST_ff_f (pow, minus_infty, 28, plus_infty);
  TEST_ff_f (pow, minus_infty, 0x1p24, plus_infty);
  TEST_ff_f (pow, minus_infty, 0x1p127, plus_infty);
  TEST_ff_f (pow, minus_infty, max_value, plus_infty);

  /* pow (-inf, y) == -0 for y an odd integer < 0. */
  TEST_ff_f (pow, minus_infty, -3, minus_zero);
  TEST_ff_f (pow, minus_infty, -0xffffff, minus_zero);
  TEST_ff_f (pow, minus_infty, -0x1fffffe, plus_zero);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, minus_infty, -0x1.fffffffffffffp+52L, minus_zero);
  TEST_ff_f (pow, minus_infty, -0x1.fffffffffffffp+53L, plus_zero);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, minus_infty, -0x1.fffffffffffffffep+63L, minus_zero);
  TEST_ff_f (pow, minus_infty, -0x1.fffffffffffffffep+64L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, minus_infty, -0x1.ffffffffffffffffffffffffff8p+105L, minus_zero);
  TEST_ff_f (pow, minus_infty, -0x1.ffffffffffffffffffffffffff8p+106L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, minus_infty, -0x1.ffffffffffffffffffffffffffffp+112L, minus_zero);
  TEST_ff_f (pow, minus_infty, -0x1.ffffffffffffffffffffffffffffp+113L, plus_zero);
# endif
#endif
  /* pow (-inf, y) == +0 for y < 0 and not an odd integer.  */
  TEST_ff_f (pow, minus_infty, -2.0, 0.0);
  TEST_ff_f (pow, minus_infty, -0x1p24, 0.0);
  TEST_ff_f (pow, minus_infty, -0x1p127, 0.0);
  TEST_ff_f (pow, minus_infty, -max_value, 0.0);

  /* pow (+0, y) == +0 for y an odd integer > 0.  */
  TEST_ff_f (pow, 0.0, 27, 0.0);
  TEST_ff_f (pow, 0.0, 0xffffff, 0.0);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, 0.0, 0x1.fffffffffffffp+52L, 0.0);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, 0.0, 0x1.fffffffffffffffep+63L, 0.0);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, 0.0, 0x1.ffffffffffffffffffffffffff8p+105L, 0.0);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, 0.0, 0x1.ffffffffffffffffffffffffffffp+112L, 0.0);
# endif
#endif

  /* pow (-0, y) == -0 for y an odd integer > 0.  */
  TEST_ff_f (pow, minus_zero, 27, minus_zero);
  TEST_ff_f (pow, minus_zero, 0xffffff, minus_zero);
  TEST_ff_f (pow, minus_zero, 0x1fffffe, plus_zero);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, minus_zero, 0x1.fffffffffffffp+52L, minus_zero);
  TEST_ff_f (pow, minus_zero, 0x1.fffffffffffffp+53L, plus_zero);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, minus_zero, 0x1.fffffffffffffffep+63L, minus_zero);
  TEST_ff_f (pow, minus_zero, 0x1.fffffffffffffffep+64L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, minus_zero, 0x1.ffffffffffffffffffffffffff8p+105L, minus_zero);
  TEST_ff_f (pow, minus_zero, 0x1.ffffffffffffffffffffffffff8p+106L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, minus_zero, 0x1.ffffffffffffffffffffffffffffp+112L, minus_zero);
  TEST_ff_f (pow, minus_zero, 0x1.ffffffffffffffffffffffffffffp+113L, plus_zero);
# endif
#endif

  /* pow (+0, y) == +0 for y > 0 and not an odd integer.  */
  TEST_ff_f (pow, 0.0, 4, 0.0);
  TEST_ff_f (pow, 0.0, 0x1p24, 0.0);
  TEST_ff_f (pow, 0.0, 0x1p127, 0.0);
  TEST_ff_f (pow, 0.0, max_value, 0.0);

  /* pow (-0, y) == +0 for y > 0 and not an odd integer.  */
  TEST_ff_f (pow, minus_zero, 4, 0.0);
  TEST_ff_f (pow, minus_zero, 0x1p24, 0.0);
  TEST_ff_f (pow, minus_zero, 0x1p127, 0.0);
  TEST_ff_f (pow, minus_zero, max_value, 0.0);

  TEST_ff_f (pow, 16, 0.25L, 2);
  TEST_ff_f (pow, 0x1p64L, 0.125L, 256);
  TEST_ff_f (pow, 2, 4, 16);
  TEST_ff_f (pow, 256, 8, 0x1p64L);

  TEST_ff_f (pow, 0.75L, 1.25L, 0.697953644326574699205914060237425566L);

#if defined TEST_DOUBLE || defined TEST_LDOUBLE
  TEST_ff_f (pow, -7.49321e+133, -9.80818e+16, 0);
#endif

  TEST_ff_f (pow, -1.0, -0xffffff, -1.0);
  TEST_ff_f (pow, -1.0, -0x1fffffe, 1.0);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, -1.0, -0x1.fffffffffffffp+52L, -1.0);
  TEST_ff_f (pow, -1.0, -0x1.fffffffffffffp+53L, 1.0);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, -1.0, -0x1.fffffffffffffffep+63L, -1.0);
  TEST_ff_f (pow, -1.0, -0x1.fffffffffffffffep+64L, 1.0);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, -1.0, -0x1.ffffffffffffffffffffffffff8p+105L, -1.0);
  TEST_ff_f (pow, -1.0, -0x1.ffffffffffffffffffffffffff8p+106L, 1.0);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, -1.0, -0x1.ffffffffffffffffffffffffffffp+112L, -1.0);
  TEST_ff_f (pow, -1.0, -0x1.ffffffffffffffffffffffffffffp+113L, 1.0);
# endif
#endif
  TEST_ff_f (pow, -1.0, -max_value, 1.0);

  TEST_ff_f (pow, -1.0, 0xffffff, -1.0);
  TEST_ff_f (pow, -1.0, 0x1fffffe, 1.0);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, -1.0, 0x1.fffffffffffffp+52L, -1.0);
  TEST_ff_f (pow, -1.0, 0x1.fffffffffffffp+53L, 1.0);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, -1.0, 0x1.fffffffffffffffep+63L, -1.0);
  TEST_ff_f (pow, -1.0, 0x1.fffffffffffffffep+64L, 1.0);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, -1.0, 0x1.ffffffffffffffffffffffffff8p+105L, -1.0);
  TEST_ff_f (pow, -1.0, 0x1.ffffffffffffffffffffffffff8p+106L, 1.0);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, -1.0, 0x1.ffffffffffffffffffffffffffffp+112L, -1.0);
  TEST_ff_f (pow, -1.0, 0x1.ffffffffffffffffffffffffffffp+113L, 1.0);
# endif
#endif
  TEST_ff_f (pow, -1.0, max_value, 1.0);

  TEST_ff_f (pow, -2.0, 126, 0x1p126);
  TEST_ff_f (pow, -2.0, 127, -0x1p127);
  TEST_ff_f (pow, -2.0, -126, 0x1p-126);
  TEST_ff_f (pow, -2.0, -127, -0x1p-127);

  TEST_ff_f (pow, -2.0, -0xffffff, minus_zero);
  TEST_ff_f (pow, -2.0, -0x1fffffe, plus_zero);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, -2.0, -0x1.fffffffffffffp+52L, minus_zero);
  TEST_ff_f (pow, -2.0, -0x1.fffffffffffffp+53L, plus_zero);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, -2.0, -0x1.fffffffffffffffep+63L, minus_zero);
  TEST_ff_f (pow, -2.0, -0x1.fffffffffffffffep+64L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, -2.0, -0x1.ffffffffffffffffffffffffff8p+105L, minus_zero);
  TEST_ff_f (pow, -2.0, -0x1.ffffffffffffffffffffffffff8p+106L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, -2.0, -0x1.ffffffffffffffffffffffffffffp+112L, minus_zero);
  TEST_ff_f (pow, -2.0, -0x1.ffffffffffffffffffffffffffffp+113L, plus_zero);
# endif
#endif
  TEST_ff_f (pow, -2.0, -max_value, plus_zero);

  TEST_ff_f (pow, -2.0, 0xffffff, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -2.0, 0x1fffffe, plus_infty, OVERFLOW_EXCEPTION);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, -2.0, 0x1.fffffffffffffp+52L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -2.0, 0x1.fffffffffffffp+53L, plus_infty, OVERFLOW_EXCEPTION);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, -2.0, 0x1.fffffffffffffffep+63L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -2.0, 0x1.fffffffffffffffep+64L, plus_infty, OVERFLOW_EXCEPTION);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, -2.0, 0x1.ffffffffffffffffffffffffff8p+105L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -2.0, 0x1.ffffffffffffffffffffffffff8p+106L, plus_infty, OVERFLOW_EXCEPTION);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, -2.0, 0x1.ffffffffffffffffffffffffffffp+112L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -2.0, 0x1.ffffffffffffffffffffffffffffp+113L, plus_infty, OVERFLOW_EXCEPTION);
# endif
#endif
  TEST_ff_f (pow, -2.0, max_value, plus_infty, OVERFLOW_EXCEPTION);

  TEST_ff_f (pow, -max_value, 0.5, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -max_value, 1.5, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -max_value, 1000.5, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -max_value, -2, plus_zero);
  TEST_ff_f (pow, -max_value, -3, minus_zero);
  TEST_ff_f (pow, -max_value, 2, plus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -max_value, 3, minus_infty, OVERFLOW_EXCEPTION);

  TEST_ff_f (pow, -max_value, -0xffffff, minus_zero);
  TEST_ff_f (pow, -max_value, -0x1fffffe, plus_zero);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, -max_value, -0x1.fffffffffffffp+52L, minus_zero);
  TEST_ff_f (pow, -max_value, -0x1.fffffffffffffp+53L, plus_zero);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, -max_value, -0x1.fffffffffffffffep+63L, minus_zero);
  TEST_ff_f (pow, -max_value, -0x1.fffffffffffffffep+64L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, -max_value, -0x1.ffffffffffffffffffffffffff8p+105L, minus_zero);
  TEST_ff_f (pow, -max_value, -0x1.ffffffffffffffffffffffffff8p+106L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, -max_value, -0x1.ffffffffffffffffffffffffffffp+112L, minus_zero);
  TEST_ff_f (pow, -max_value, -0x1.ffffffffffffffffffffffffffffp+113L, plus_zero);
# endif
#endif
  TEST_ff_f (pow, -max_value, -max_value, plus_zero);

  TEST_ff_f (pow, -max_value, 0xffffff, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -max_value, 0x1fffffe, plus_infty, OVERFLOW_EXCEPTION);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, -max_value, 0x1.fffffffffffffp+52L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -max_value, 0x1.fffffffffffffp+53L, plus_infty, OVERFLOW_EXCEPTION);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, -max_value, 0x1.fffffffffffffffep+63L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -max_value, 0x1.fffffffffffffffep+64L, plus_infty, OVERFLOW_EXCEPTION);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, -max_value, 0x1.ffffffffffffffffffffffffff8p+105L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -max_value, 0x1.ffffffffffffffffffffffffff8p+106L, plus_infty, OVERFLOW_EXCEPTION);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, -max_value, 0x1.ffffffffffffffffffffffffffffp+112L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -max_value, 0x1.ffffffffffffffffffffffffffffp+113L, plus_infty, OVERFLOW_EXCEPTION);
# endif
#endif
  TEST_ff_f (pow, -max_value, max_value, plus_infty, OVERFLOW_EXCEPTION);

  TEST_ff_f (pow, -0.5, 126, 0x1p-126);
  TEST_ff_f (pow, -0.5, 127, -0x1p-127);
  TEST_ff_f (pow, -0.5, -126, 0x1p126);
  TEST_ff_f (pow, -0.5, -127, -0x1p127);

  TEST_ff_f (pow, -0.5, -0xffffff, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -0.5, -0x1fffffe, plus_infty, OVERFLOW_EXCEPTION);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, -0.5, -0x1.fffffffffffffp+52L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -0.5, -0x1.fffffffffffffp+53L, plus_infty, OVERFLOW_EXCEPTION);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, -0.5, -0x1.fffffffffffffffep+63L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -0.5, -0x1.fffffffffffffffep+64L, plus_infty, OVERFLOW_EXCEPTION);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, -0.5, -0x1.ffffffffffffffffffffffffff8p+105L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -0.5, -0x1.ffffffffffffffffffffffffff8p+106L, plus_infty, OVERFLOW_EXCEPTION);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, -0.5, -0x1.ffffffffffffffffffffffffffffp+112L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -0.5, -0x1.ffffffffffffffffffffffffffffp+113L, plus_infty, OVERFLOW_EXCEPTION);
# endif
#endif
  TEST_ff_f (pow, -0.5, -max_value, plus_infty, OVERFLOW_EXCEPTION);

  TEST_ff_f (pow, -0.5, 0xffffff, minus_zero);
  TEST_ff_f (pow, -0.5, 0x1fffffe, plus_zero);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, -0.5, 0x1.fffffffffffffp+52L, minus_zero);
  TEST_ff_f (pow, -0.5, 0x1.fffffffffffffp+53L, plus_zero);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, -0.5, 0x1.fffffffffffffffep+63L, minus_zero);
  TEST_ff_f (pow, -0.5, 0x1.fffffffffffffffep+64L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, -0.5, 0x1.ffffffffffffffffffffffffff8p+105L, minus_zero);
  TEST_ff_f (pow, -0.5, 0x1.ffffffffffffffffffffffffff8p+106L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, -0.5, 0x1.ffffffffffffffffffffffffffffp+112L, minus_zero);
  TEST_ff_f (pow, -0.5, 0x1.ffffffffffffffffffffffffffffp+113L, plus_zero);
# endif
#endif
  TEST_ff_f (pow, -0.5, max_value, plus_zero);

  TEST_ff_f (pow, -min_value, 0.5, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -min_value, 1.5, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -min_value, 1000.5, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (pow, -min_value, -2, plus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -min_value, -3, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -min_value, 1, -min_value);
  TEST_ff_f (pow, -min_value, 2, plus_zero);
  TEST_ff_f (pow, -min_value, 3, minus_zero);

  TEST_ff_f (pow, -min_value, -0xffffff, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -min_value, -0x1fffffe, plus_infty, OVERFLOW_EXCEPTION);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, -min_value, -0x1.fffffffffffffp+52L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -min_value, -0x1.fffffffffffffp+53L, plus_infty, OVERFLOW_EXCEPTION);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, -min_value, -0x1.fffffffffffffffep+63L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -min_value, -0x1.fffffffffffffffep+64L, plus_infty, OVERFLOW_EXCEPTION);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, -min_value, -0x1.ffffffffffffffffffffffffff8p+105L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -min_value, -0x1.ffffffffffffffffffffffffff8p+106L, plus_infty, OVERFLOW_EXCEPTION);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, -min_value, -0x1.ffffffffffffffffffffffffffffp+112L, minus_infty, OVERFLOW_EXCEPTION);
  TEST_ff_f (pow, -min_value, -0x1.ffffffffffffffffffffffffffffp+113L, plus_infty, OVERFLOW_EXCEPTION);
# endif
#endif
  TEST_ff_f (pow, -min_value, -max_value, plus_infty, OVERFLOW_EXCEPTION);

  TEST_ff_f (pow, -min_value, 0xffffff, minus_zero);
  TEST_ff_f (pow, -min_value, 0x1fffffe, plus_zero);
#ifndef TEST_FLOAT
  TEST_ff_f (pow, -min_value, 0x1.fffffffffffffp+52L, minus_zero);
  TEST_ff_f (pow, -min_value, 0x1.fffffffffffffp+53L, plus_zero);
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
  TEST_ff_f (pow, -min_value, 0x1.fffffffffffffffep+63L, minus_zero);
  TEST_ff_f (pow, -min_value, 0x1.fffffffffffffffep+64L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 106
  TEST_ff_f (pow, -min_value, 0x1.ffffffffffffffffffffffffff8p+105L, minus_zero);
  TEST_ff_f (pow, -min_value, 0x1.ffffffffffffffffffffffffff8p+106L, plus_zero);
# endif
# if LDBL_MANT_DIG >= 113
  TEST_ff_f (pow, -min_value, 0x1.ffffffffffffffffffffffffffffp+112L, minus_zero);
  TEST_ff_f (pow, -min_value, 0x1.ffffffffffffffffffffffffffffp+113L, plus_zero);
# endif
#endif
  TEST_ff_f (pow, -min_value, max_value, plus_zero);

#ifndef TEST_LDOUBLE /* Bug 13881.  */
  TEST_ff_f (pow, 0x0.ffffffp0, 10, 0.999999403953712118183885036774764444747L);
  TEST_ff_f (pow, 0x0.ffffffp0, 100, 0.999994039553108359406305079606228341585L);
  TEST_ff_f (pow, 0x0.ffffffp0, 1000, 0.9999403971297699052276650144650733772182L);
  TEST_ff_f (pow, 0x0.ffffffp0, 0x1p24, 0.3678794302077803437135155590023422899744L);
  TEST_ff_f (pow, 0x0.ffffffp0, 0x1p30, 1.603807831524924233828134753069728224044e-28L);
  TEST_ff_f (pow, 0x0.ffffffp0, 0x1.234566p30, 2.374884712135295099971443365381007297732e-32L);
  TEST_ff_f (pow, 0x0.ffffffp0, -10, 1.000000596046643153205170848674671339688L);
  TEST_ff_f (pow, 0x0.ffffffp0, -100, 1.000005960482418779499387594989252621451L);
  TEST_ff_f (pow, 0x0.ffffffp0, -1000, 1.000059606422943986382898964231519867906L);
  TEST_ff_f (pow, 0x0.ffffffp0, -0x1p24, 2.7182819094701610539628664526874952929416L);
  TEST_ff_f (pow, 0x0.ffffffp0, -0x1p30, 6.2351609734265057988914412331288163636075e+27L);
  TEST_ff_f (pow, 0x0.ffffffp0, -0x1.234566p30, 4.2107307141696353498921307077142537353515e+31L);
  TEST_ff_f (pow, 0x1.000002p0, 0x1p24, 7.3890552180866447284268641248075832310141L);
  TEST_ff_f (pow, 0x1.000002p0, 0x1.234566p29, 4.2107033006507495188536371520637025716256e+31L);
  TEST_ff_f (pow, 0x1.000002p0, -0x1.234566p29, 2.3749001736727769098946062325205705312166e-32L);
#endif

  /* Bug 13881: powl inaccurate so these tests disabled for long double.  */
#if !defined TEST_FLOAT && !defined TEST_LDOUBLE
  TEST_ff_f (pow, 0x0.fffffffffffff8p0L, 0x1.23456789abcdfp62L, 1.0118762747827252817436395051178295138220e-253L);
  TEST_ff_f (pow, 0x0.fffffffffffff8p0L, -0x1.23456789abcdfp62L, 9.8826311568054561811190162420900667121992e+252L);
  TEST_ff_f (pow, 0x1.0000000000001p0L, 0x1.23456789abcdfp61L, 9.8826311568044974397135026217687399395481e+252L);
  TEST_ff_f (pow, 0x1.0000000000001p0L, -0x1.23456789abcdfp61L, 1.0118762747828234466621210689458255908670e-253L);
#endif

  END (pow);
}


static void
pow_test_tonearest (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(pow) (0, 0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (pow_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_ff_f (pow, 1.0625L, 1.125L, 1.070582293028761362162622578677070098674L);
      TEST_ff_f (pow, 1.5L, 1.03125L, 1.519127098714743184071644334163037684948L);
    }

  fesetround (save_round_mode);

  END (pow_tonearest);
}


static void
pow_test_towardzero (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(pow) (0, 0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (pow_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_ff_f (pow, 1.0625L, 1.125L, 1.070582293028761362162622578677070098674L);
      TEST_ff_f (pow, 1.5L, 1.03125L, 1.519127098714743184071644334163037684948L);
    }

  fesetround (save_round_mode);

  END (pow_towardzero);
}


static void
pow_test_downward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(pow) (0, 0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (pow_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_ff_f (pow, 1.0625L, 1.125L, 1.070582293028761362162622578677070098674L);
      TEST_ff_f (pow, 1.5L, 1.03125L, 1.519127098714743184071644334163037684948L);
    }

  fesetround (save_round_mode);

  END (pow_downward);
}


static void
pow_test_upward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(pow) (0, 0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (pow_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_ff_f (pow, 1.0625L, 1.125L, 1.070582293028761362162622578677070098674L);
      TEST_ff_f (pow, 1.5L, 1.03125L, 1.519127098714743184071644334163037684948L);
    }

  fesetround (save_round_mode);

  END (pow_upward);
}


static void
remainder_test (void)
{
  errno = 0;
  FUNC(remainder) (1.625, 1.0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (remainder);

  errno = 0;
  TEST_ff_f (remainder, 1, 0, nan_value, INVALID_EXCEPTION);
  check_int ("errno for remainder(1, 0) = EDOM ", errno, EDOM, 0, 0, 0);
  errno = 0;
  TEST_ff_f (remainder, 1, minus_zero, nan_value, INVALID_EXCEPTION);
  check_int ("errno for remainder(1, -0) = EDOM ", errno, EDOM, 0, 0, 0);
  errno = 0;
  TEST_ff_f (remainder, plus_infty, 1, nan_value, INVALID_EXCEPTION);
  check_int ("errno for remainder(INF, 1) = EDOM ", errno, EDOM, 0, 0, 0);
  errno = 0;
  TEST_ff_f (remainder, minus_infty, 1, nan_value, INVALID_EXCEPTION);
  check_int ("errno for remainder(-INF, 1) = EDOM ", errno, EDOM, 0, 0, 0);
  errno = 0;
  TEST_ff_f (remainder, nan_value, nan_value, nan_value);
  check_int ("errno for remainder(NAN, NAN) unchanged", errno, 0, 0, 0, 0);
  errno = 0;
  TEST_ff_f (remainder, 0, nan_value, nan_value);
  check_int ("errno for remainder(0, NAN) unchanged", errno, 0, 0, 0, 0);
  errno = 0;
  TEST_ff_f (remainder, nan_value, 0, nan_value);
  check_int ("errno for remainder(NaN, 0) unchanged", errno, 0, 0, 0, 0);

  TEST_ff_f (remainder, 1.625, 1.0, -0.375);
  TEST_ff_f (remainder, -1.625, 1.0, 0.375);
  TEST_ff_f (remainder, 1.625, -1.0, -0.375);
  TEST_ff_f (remainder, -1.625, -1.0, 0.375);
  TEST_ff_f (remainder, 5.0, 2.0, 1.0);
  TEST_ff_f (remainder, 3.0, 2.0, -1.0);

  END (remainder);
}

static void
remquo_test (void)
{
  /* x is needed.  */
  int x;

  errno = 0;
  FUNC(remquo) (1.625, 1.0, &x);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (remquo);

  TEST_ffI_f1 (remquo, 1, 0, nan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, 1, minus_zero, nan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, plus_infty, 1, nan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, minus_infty, 1, nan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, nan_value, nan_value, nan_value, IGNORE);

  TEST_ffI_f1 (remquo, 1.625, 1.0, -0.375, 2);
  TEST_ffI_f1 (remquo, -1.625, 1.0, 0.375, -2);
  TEST_ffI_f1 (remquo, 1.625, -1.0, -0.375, -2);
  TEST_ffI_f1 (remquo, -1.625, -1.0, 0.375, 2);

  TEST_ffI_f1 (remquo, 5, 2, 1, 2);
  TEST_ffI_f1 (remquo, 3, 2, -1, 2);

  END (remquo);
}

static void
rint_test (void)
{
  START (rint);

  TEST_f_f (rint, 0.0, 0.0);
  TEST_f_f (rint, minus_zero, minus_zero);
  TEST_f_f (rint, plus_infty, plus_infty);
  TEST_f_f (rint, minus_infty, minus_infty);

  /* Default rounding mode is round to even.  */
  TEST_f_f (rint, 0.5, 0.0);
  TEST_f_f (rint, 1.5, 2.0);
  TEST_f_f (rint, 2.5, 2.0);
  TEST_f_f (rint, 3.5, 4.0);
  TEST_f_f (rint, 4.5, 4.0);
  TEST_f_f (rint, -0.5, -0.0);
  TEST_f_f (rint, -1.5, -2.0);
  TEST_f_f (rint, -2.5, -2.0);
  TEST_f_f (rint, -3.5, -4.0);
  TEST_f_f (rint, -4.5, -4.0);
  TEST_f_f (rint, 0.1, 0.0);
  TEST_f_f (rint, 0.25, 0.0);
  TEST_f_f (rint, 0.625, 1.0);
  TEST_f_f (rint, -0.1, -0.0);
  TEST_f_f (rint, -0.25, -0.0);
  TEST_f_f (rint, -0.625, -1.0);
  TEST_f_f (rint, 262144.75, 262145.0);
  TEST_f_f (rint, 262142.75, 262143.0);
  TEST_f_f (rint, 524286.75, 524287.0);
  TEST_f_f (rint, 524288.75, 524289.0);
  TEST_f_f (rint, 1048576.75, 1048577.0);
  TEST_f_f (rint, 2097152.75, 2097153.0);
  TEST_f_f (rint, -1048576.75, -1048577.0);
  TEST_f_f (rint, -2097152.75, -2097153.0);
#ifndef TEST_FLOAT
  TEST_f_f (rint, 70368744177664.75, 70368744177665.0);
  TEST_f_f (rint, 140737488355328.75, 140737488355329.0);
  TEST_f_f (rint, 281474976710656.75, 281474976710657.0);
  TEST_f_f (rint, 562949953421312.75, 562949953421313.0);
  TEST_f_f (rint, 1125899906842624.75, 1125899906842625.0);
  TEST_f_f (rint, -70368744177664.75, -70368744177665.0);
  TEST_f_f (rint, -140737488355328.75, -140737488355329.0);
  TEST_f_f (rint, -281474976710656.75, -281474976710657.0);
  TEST_f_f (rint, -562949953421312.75, -562949953421313.0);
  TEST_f_f (rint, -1125899906842624.75, -1125899906842625.0);
#endif
#ifdef TEST_LDOUBLE
  /* The result can only be represented in long double.  */
  TEST_f_f (rint, 4503599627370495.5L, 4503599627370496.0L);
  TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L);
  TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L);
  TEST_f_f (rint, 4503599627370496.75L, 4503599627370497.0L);
  TEST_f_f (rint, 4503599627370497.5L, 4503599627370498.0L);

# if LDBL_MANT_DIG > 100
  TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370495.0L);
  TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370496.0L);
  TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370497.0L);
# endif

  TEST_f_f (rint, -4503599627370495.5L, -4503599627370496.0L);
  TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L);
  TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L);
  TEST_f_f (rint, -4503599627370496.75L, -4503599627370497.0L);
  TEST_f_f (rint, -4503599627370497.5L, -4503599627370498.0L);

# if LDBL_MANT_DIG > 100
  TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370495.0L);
  TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370496.0L);
  TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370497.0L);

  TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L);
  TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L);
  TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L);
  TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740992.0L);
  TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740993.0L);
  TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740994.0L);

  TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L);
  TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L);
  TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L);
  TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740992.0L);
  TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740993.0L);
  TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740994.0L);
# endif

  TEST_f_f (rint, 9007199254740991.5L, 9007199254740992.0L);
  TEST_f_f (rint, 9007199254740992.25L, 9007199254740992.0L);
  TEST_f_f (rint, 9007199254740992.5L, 9007199254740992.0L);
  TEST_f_f (rint, 9007199254740992.75L, 9007199254740993.0L);
  TEST_f_f (rint, 9007199254740993.5L, 9007199254740994.0L);

  TEST_f_f (rint, -9007199254740991.5L, -9007199254740992.0L);
  TEST_f_f (rint, -9007199254740992.25L, -9007199254740992.0L);
  TEST_f_f (rint, -9007199254740992.5L, -9007199254740992.0L);
  TEST_f_f (rint, -9007199254740992.75L, -9007199254740993.0L);
  TEST_f_f (rint, -9007199254740993.5L, -9007199254740994.0L);

  TEST_f_f (rint, 72057594037927935.5L, 72057594037927936.0L);
  TEST_f_f (rint, 72057594037927936.25L, 72057594037927936.0L);
  TEST_f_f (rint, 72057594037927936.5L, 72057594037927936.0L);
  TEST_f_f (rint, 72057594037927936.75L, 72057594037927937.0L);
  TEST_f_f (rint, 72057594037927937.5L, 72057594037927938.0L);

  TEST_f_f (rint, -72057594037927935.5L, -72057594037927936.0L);
  TEST_f_f (rint, -72057594037927936.25L, -72057594037927936.0L);
  TEST_f_f (rint, -72057594037927936.5L, -72057594037927936.0L);
  TEST_f_f (rint, -72057594037927936.75L, -72057594037927937.0L);
  TEST_f_f (rint, -72057594037927937.5L, -72057594037927938.0L);

  TEST_f_f (rint, 10141204801825835211973625643007.5L, 10141204801825835211973625643008.0L);
  TEST_f_f (rint, 10141204801825835211973625643008.25L, 10141204801825835211973625643008.0L);
  TEST_f_f (rint, 10141204801825835211973625643008.5L, 10141204801825835211973625643008.0L);
  TEST_f_f (rint, 10141204801825835211973625643008.75L, 10141204801825835211973625643009.0L);
  TEST_f_f (rint, 10141204801825835211973625643009.5L, 10141204801825835211973625643010.0L);
#endif

  END (rint);
}

static void
rint_test_tonearest (void)
{
  int save_round_mode;
  START (rint_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_f_f (rint, 2.0, 2.0);
      TEST_f_f (rint, 1.5, 2.0);
      TEST_f_f (rint, 1.0, 1.0);
      TEST_f_f (rint, 0.5, 0.0);
      TEST_f_f (rint, 0.0, 0.0);
      TEST_f_f (rint, minus_zero, minus_zero);
      TEST_f_f (rint, -0.5, -0.0);
      TEST_f_f (rint, -1.0, -1.0);
      TEST_f_f (rint, -1.5, -2.0);
      TEST_f_f (rint, -2.0, -2.0);
      TEST_f_f (rint, 0.1, 0.0);
      TEST_f_f (rint, 0.25, 0.0);
      TEST_f_f (rint, 0.625, 1.0);
      TEST_f_f (rint, -0.1, -0.0);
      TEST_f_f (rint, -0.25, -0.0);
      TEST_f_f (rint, -0.625, -1.0);
      TEST_f_f (rint, 1048576.75, 1048577.0);
      TEST_f_f (rint, 2097152.75, 2097153.0);
      TEST_f_f (rint, -1048576.75, -1048577.0);
      TEST_f_f (rint, -2097152.75, -2097153.0);
#ifndef TEST_FLOAT
      TEST_f_f (rint, 70368744177664.75, 70368744177665.0);
      TEST_f_f (rint, 140737488355328.75, 140737488355329.0);
      TEST_f_f (rint, 281474976710656.75, 281474976710657.0);
      TEST_f_f (rint, 562949953421312.75, 562949953421313.0);
      TEST_f_f (rint, 1125899906842624.75, 1125899906842625.0);
      TEST_f_f (rint, -70368744177664.75, -70368744177665.0);
      TEST_f_f (rint, -140737488355328.75, -140737488355329.0);
      TEST_f_f (rint, -281474976710656.75, -281474976710657.0);
      TEST_f_f (rint, -562949953421312.75, -562949953421313.0);
      TEST_f_f (rint, -1125899906842624.75, -1125899906842625.0);
#endif
#ifdef TEST_LDOUBLE
      /* The result can only be represented in long double.  */
      TEST_f_f (rint, 4503599627370495.5L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370496.75L, 4503599627370497.0L);
      TEST_f_f (rint, 4503599627370497.5L, 4503599627370498.0L);
# if LDBL_MANT_DIG > 100
      TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370495.0L);
      TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370497.0L);
# endif
      TEST_f_f (rint, -4503599627370495.5L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370496.75L, -4503599627370497.0L);
      TEST_f_f (rint, -4503599627370497.5L, -4503599627370498.0L);
# if LDBL_MANT_DIG > 100
      TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370495.0L);
      TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370497.0L);

      TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L);
      TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L);
      TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L);
      TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740992.0L);
      TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740993.0L);
      TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740994.0L);

      TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L);
      TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L);
      TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L);
      TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740992.0L);
      TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740993.0L);
      TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740994.0L);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (rint_tonearest);
}

static void
rint_test_towardzero (void)
{
  int save_round_mode;
  START (rint_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_f_f (rint, 2.0, 2.0);
      TEST_f_f (rint, 1.5, 1.0);
      TEST_f_f (rint, 1.0, 1.0);
      TEST_f_f (rint, 0.5, 0.0);
      TEST_f_f (rint, 0.0, 0.0);
      TEST_f_f (rint, minus_zero, minus_zero);
      TEST_f_f (rint, -0.5, -0.0);
      TEST_f_f (rint, -1.0, -1.0);
      TEST_f_f (rint, -1.5, -1.0);
      TEST_f_f (rint, -2.0, -2.0);
      TEST_f_f (rint, 0.1, 0.0);
      TEST_f_f (rint, 0.25, 0.0);
      TEST_f_f (rint, 0.625, 0.0);
      TEST_f_f (rint, -0.1, -0.0);
      TEST_f_f (rint, -0.25, -0.0);
      TEST_f_f (rint, -0.625, -0.0);
      TEST_f_f (rint, 1048576.75, 1048576.0);
      TEST_f_f (rint, 2097152.75, 2097152.0);
      TEST_f_f (rint, -1048576.75, -1048576.0);
      TEST_f_f (rint, -2097152.75, -2097152.0);
#ifndef TEST_FLOAT
      TEST_f_f (rint, 70368744177664.75, 70368744177664.0);
      TEST_f_f (rint, 140737488355328.75, 140737488355328.0);
      TEST_f_f (rint, 281474976710656.75, 281474976710656.0);
      TEST_f_f (rint, 562949953421312.75, 562949953421312.0);
      TEST_f_f (rint, 1125899906842624.75, 1125899906842624.0);
      TEST_f_f (rint, -70368744177664.75, -70368744177664.0);
      TEST_f_f (rint, -140737488355328.75, -140737488355328.0);
      TEST_f_f (rint, -281474976710656.75, -281474976710656.0);
      TEST_f_f (rint, -562949953421312.75, -562949953421312.0);
      TEST_f_f (rint, -1125899906842624.75, -1125899906842624.0);
#endif
#ifdef TEST_LDOUBLE
      /* The result can only be represented in long double.  */
      TEST_f_f (rint, 4503599627370495.5L, 4503599627370495.0L);
      TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370496.75L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370497.5L, 4503599627370497.0L);
# if LDBL_MANT_DIG > 100
      TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370494.0L);
      TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370495.0L);
      TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370496.0L);
# endif
      TEST_f_f (rint, -4503599627370495.5L, -4503599627370495.0L);
      TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370496.75L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370497.5L, -4503599627370497.0L);
# if LDBL_MANT_DIG > 100
      TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370494.0L);
      TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370495.0L);
      TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370496.0L);

      TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L);
      TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L);
      TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L);
      TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740991.0L);
      TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740992.0L);
      TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740993.0L);

      TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L);
      TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L);
      TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L);
      TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740991.0L);
      TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740992.0L);
      TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740993.0L);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (rint_towardzero);
}

static void
rint_test_downward (void)
{
  int save_round_mode;
  START (rint_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_f_f (rint, 2.0, 2.0);
      TEST_f_f (rint, 1.5, 1.0);
      TEST_f_f (rint, 1.0, 1.0);
      TEST_f_f (rint, 0.5, 0.0);
      TEST_f_f (rint, 0.0, 0.0);
      TEST_f_f (rint, minus_zero, minus_zero);
      TEST_f_f (rint, -0.5, -1.0);
      TEST_f_f (rint, -1.0, -1.0);
      TEST_f_f (rint, -1.5, -2.0);
      TEST_f_f (rint, -2.0, -2.0);
      TEST_f_f (rint, 0.1, 0.0);
      TEST_f_f (rint, 0.25, 0.0);
      TEST_f_f (rint, 0.625, 0.0);
      TEST_f_f (rint, -0.1, -1.0);
      TEST_f_f (rint, -0.25, -1.0);
      TEST_f_f (rint, -0.625, -1.0);
      TEST_f_f (rint, 1048576.75, 1048576.0);
      TEST_f_f (rint, 2097152.75, 2097152.0);
      TEST_f_f (rint, -1048576.75, -1048577.0);
      TEST_f_f (rint, -2097152.75, -2097153.0);
#ifndef TEST_FLOAT
      TEST_f_f (rint, 70368744177664.75, 70368744177664.0);
      TEST_f_f (rint, 140737488355328.75, 140737488355328.0);
      TEST_f_f (rint, 281474976710656.75, 281474976710656.0);
      TEST_f_f (rint, 562949953421312.75, 562949953421312.0);
      TEST_f_f (rint, 1125899906842624.75, 1125899906842624.0);
      TEST_f_f (rint, -70368744177664.75, -70368744177665.0);
      TEST_f_f (rint, -140737488355328.75, -140737488355329.0);
      TEST_f_f (rint, -281474976710656.75, -281474976710657.0);
      TEST_f_f (rint, -562949953421312.75, -562949953421313.0);
      TEST_f_f (rint, -1125899906842624.75, -1125899906842625.0);
#endif
#ifdef TEST_LDOUBLE
      /* The result can only be represented in long double.  */
      TEST_f_f (rint, 4503599627370495.5L, 4503599627370495.0L);
      TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370496.75L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370497.5L, 4503599627370497.0L);
# if LDBL_MANT_DIG > 100
      TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370494.0L);
      TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370495.0L);
      TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370496.0L);
# endif
      TEST_f_f (rint, -4503599627370495.5L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370496.25L, -4503599627370497.0L);
      TEST_f_f (rint, -4503599627370496.5L, -4503599627370497.0L);
      TEST_f_f (rint, -4503599627370496.75L, -4503599627370497.0L);
      TEST_f_f (rint, -4503599627370497.5L, -4503599627370498.0L);
# if LDBL_MANT_DIG > 100
      TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370495.0L);
      TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370497.0L);

      TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L);
      TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L);
      TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L);
      TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740991.0L);
      TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740992.0L);
      TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740993.0L);

      TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740992.0L);
      TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740993.0L);
      TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740994.0L);
      TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740992.0L);
      TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740993.0L);
      TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740994.0L);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (rint_downward);
}

static void
rint_test_upward (void)
{
  int save_round_mode;
  START (rint_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_f_f (rint, 2.0, 2.0);
      TEST_f_f (rint, 1.5, 2.0);
      TEST_f_f (rint, 1.0, 1.0);
      TEST_f_f (rint, 0.5, 1.0);
      TEST_f_f (rint, 0.0, 0.0);
      TEST_f_f (rint, minus_zero, minus_zero);
      TEST_f_f (rint, -0.5, -0.0);
      TEST_f_f (rint, -1.0, -1.0);
      TEST_f_f (rint, -1.5, -1.0);
      TEST_f_f (rint, -2.0, -2.0);
      TEST_f_f (rint, 0.1, 1.0);
      TEST_f_f (rint, 0.25, 1.0);
      TEST_f_f (rint, 0.625, 1.0);
      TEST_f_f (rint, -0.1, -0.0);
      TEST_f_f (rint, -0.25, -0.0);
      TEST_f_f (rint, -0.625, -0.0);
      TEST_f_f (rint, 1048576.75, 1048577.0);
      TEST_f_f (rint, 2097152.75, 2097153.0);
      TEST_f_f (rint, -1048576.75, -1048576.0);
      TEST_f_f (rint, -2097152.75, -2097152.0);
#ifndef TEST_FLOAT
      TEST_f_f (rint, 70368744177664.75, 70368744177665.0);
      TEST_f_f (rint, 140737488355328.75, 140737488355329.0);
      TEST_f_f (rint, 281474976710656.75, 281474976710657.0);
      TEST_f_f (rint, 562949953421312.75, 562949953421313.0);
      TEST_f_f (rint, 1125899906842624.75, 1125899906842625.0);
      TEST_f_f (rint, -70368744177664.75, -70368744177664.0);
      TEST_f_f (rint, -140737488355328.75, -140737488355328.0);
      TEST_f_f (rint, -281474976710656.75, -281474976710656.0);
      TEST_f_f (rint, -562949953421312.75, -562949953421312.0);
      TEST_f_f (rint, -1125899906842624.75, -1125899906842624.0);
#endif
#ifdef TEST_LDOUBLE
      /* The result can only be represented in long double.  */
      TEST_f_f (rint, 4503599627370495.5L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370496.25L, 4503599627370497.0L);
      TEST_f_f (rint, 4503599627370496.5L, 4503599627370497.0L);
      TEST_f_f (rint, 4503599627370496.75L, 4503599627370497.0L);
      TEST_f_f (rint, 4503599627370497.5L, 4503599627370498.0L);
# if LDBL_MANT_DIG > 100
      TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370495.0L);
      TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370496.0L);
      TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370497.0L);
# endif
      TEST_f_f (rint, -4503599627370495.5L, -4503599627370495.0L);
      TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370496.75L, -4503599627370496.0L);
      TEST_f_f (rint, -4503599627370497.5L, -4503599627370497.0L);
# if LDBL_MANT_DIG > 100
      TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370494.0L);
      TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370495.0L);
      TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370496.0L);

      TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740992.0L);
      TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740993.0L);
      TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740994.0L);
      TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740992.0L);
      TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740993.0L);
      TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740994.0L);

      TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L);
      TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L);
      TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L);
      TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740991.0L);
      TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740992.0L);
      TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740993.0L);
# endif
#endif
    }

  fesetround (save_round_mode);

  END (rint_upward);
}

static void
round_test (void)
{
  START (round);

  TEST_f_f (round, 0, 0);
  TEST_f_f (round, minus_zero, minus_zero);
  TEST_f_f (round, 0.2L, 0.0);
  TEST_f_f (round, -0.2L, minus_zero);
  TEST_f_f (round, 0.5, 1.0);
  TEST_f_f (round, -0.5, -1.0);
  TEST_f_f (round, 0.8L, 1.0);
  TEST_f_f (round, -0.8L, -1.0);
  TEST_f_f (round, 1.5, 2.0);
  TEST_f_f (round, -1.5, -2.0);
  TEST_f_f (round, 0.1, 0.0);
  TEST_f_f (round, 0.25, 0.0);
  TEST_f_f (round, 0.625, 1.0);
  TEST_f_f (round, -0.1, -0.0);
  TEST_f_f (round, -0.25, -0.0);
  TEST_f_f (round, -0.625, -1.0);
  TEST_f_f (round, 2097152.5, 2097153);
  TEST_f_f (round, -2097152.5, -2097153);

#ifdef TEST_LDOUBLE
  /* The result can only be represented in long double.  */
  TEST_f_f (round, 4503599627370495.5L, 4503599627370496.0L);
  TEST_f_f (round, 4503599627370496.25L, 4503599627370496.0L);
  TEST_f_f (round, 4503599627370496.5L, 4503599627370497.0L);
  TEST_f_f (round, 4503599627370496.75L, 4503599627370497.0L);
  TEST_f_f (round, 4503599627370497.5L, 4503599627370498.0L);
# if LDBL_MANT_DIG > 100
  TEST_f_f (round, 4503599627370494.5000000000001L, 4503599627370495.0L);
  TEST_f_f (round, 4503599627370495.5000000000001L, 4503599627370496.0L);
  TEST_f_f (round, 4503599627370496.5000000000001L, 4503599627370497.0L);
# endif

  TEST_f_f (round, -4503599627370495.5L, -4503599627370496.0L);
  TEST_f_f (round, -4503599627370496.25L, -4503599627370496.0L);
  TEST_f_f (round, -4503599627370496.5L, -4503599627370497.0L);
  TEST_f_f (round, -4503599627370496.75L, -4503599627370497.0L);
  TEST_f_f (round, -4503599627370497.5L, -4503599627370498.0L);
# if LDBL_MANT_DIG > 100
  TEST_f_f (round, -4503599627370494.5000000000001L, -4503599627370495.0L);
  TEST_f_f (round, -4503599627370495.5000000000001L, -4503599627370496.0L);
  TEST_f_f (round, -4503599627370496.5000000000001L, -4503599627370497.0L);
# endif

  TEST_f_f (round, 9007199254740991.5L, 9007199254740992.0L);
  TEST_f_f (round, 9007199254740992.25L, 9007199254740992.0L);
  TEST_f_f (round, 9007199254740992.5L, 9007199254740993.0L);
  TEST_f_f (round, 9007199254740992.75L, 9007199254740993.0L);
  TEST_f_f (round, 9007199254740993.5L, 9007199254740994.0L);

  TEST_f_f (round, -9007199254740991.5L, -9007199254740992.0L);
  TEST_f_f (round, -9007199254740992.25L, -9007199254740992.0L);
  TEST_f_f (round, -9007199254740992.5L, -9007199254740993.0L);
  TEST_f_f (round, -9007199254740992.75L, -9007199254740993.0L);
  TEST_f_f (round, -9007199254740993.5L, -9007199254740994.0L);

# if LDBL_MANT_DIG > 100
  TEST_f_f (round, 9007199254740991.0000000000001L, 9007199254740991.0L);
  TEST_f_f (round, 9007199254740992.0000000000001L, 9007199254740992.0L);
  TEST_f_f (round, 9007199254740993.0000000000001L, 9007199254740993.0L);
  TEST_f_f (round, 9007199254740991.5000000000001L, 9007199254740992.0L);
  TEST_f_f (round, 9007199254740992.5000000000001L, 9007199254740993.0L);
  TEST_f_f (round, 9007199254740993.5000000000001L, 9007199254740994.0L);

  TEST_f_f (round, -9007199254740991.0000000000001L, -9007199254740991.0L);
  TEST_f_f (round, -9007199254740992.0000000000001L, -9007199254740992.0L);
  TEST_f_f (round, -9007199254740993.0000000000001L, -9007199254740993.0L);
  TEST_f_f (round, -9007199254740991.5000000000001L, -9007199254740992.0L);
  TEST_f_f (round, -9007199254740992.5000000000001L, -9007199254740993.0L);
  TEST_f_f (round, -9007199254740993.5000000000001L, -9007199254740994.0L);
# endif

  TEST_f_f (round, 72057594037927935.5L, 72057594037927936.0L);
  TEST_f_f (round, 72057594037927936.25L, 72057594037927936.0L);
  TEST_f_f (round, 72057594037927936.5L, 72057594037927937.0L);
  TEST_f_f (round, 72057594037927936.75L, 72057594037927937.0L);
  TEST_f_f (round, 72057594037927937.5L, 72057594037927938.0L);

  TEST_f_f (round, -72057594037927935.5L, -72057594037927936.0L);
  TEST_f_f (round, -72057594037927936.25L, -72057594037927936.0L);
  TEST_f_f (round, -72057594037927936.5L, -72057594037927937.0L);
  TEST_f_f (round, -72057594037927936.75L, -72057594037927937.0L);
  TEST_f_f (round, -72057594037927937.5L, -72057594037927938.0L);

  TEST_f_f (round, 10141204801825835211973625643007.5L, 10141204801825835211973625643008.0L);
  TEST_f_f (round, 10141204801825835211973625643008.25L, 10141204801825835211973625643008.0L);
  TEST_f_f (round, 10141204801825835211973625643008.5L, 10141204801825835211973625643009.0L);
  TEST_f_f (round, 10141204801825835211973625643008.75L, 10141204801825835211973625643009.0L);
  TEST_f_f (round, 10141204801825835211973625643009.5L, 10141204801825835211973625643010.0L);
#endif

  END (round);
}


static void
scalb_test (void)
{

  START (scalb);

  TEST_ff_f (scalb, 2.0, 0.5, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (scalb, 3.0, -2.5, nan_value, INVALID_EXCEPTION);

  TEST_ff_f (scalb, 0, nan_value, nan_value);
  TEST_ff_f (scalb, 1, nan_value, nan_value);

  TEST_ff_f (scalb, 1, 0, 1);
  TEST_ff_f (scalb, -1, 0, -1);

  TEST_ff_f (scalb, 0, plus_infty, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (scalb, minus_zero, plus_infty, nan_value, INVALID_EXCEPTION);

  TEST_ff_f (scalb, 0, 2, 0);
  TEST_ff_f (scalb, minus_zero, -4, minus_zero);
  TEST_ff_f (scalb, 0, 0, 0);
  TEST_ff_f (scalb, minus_zero, 0, minus_zero);
  TEST_ff_f (scalb, 0, -1, 0);
  TEST_ff_f (scalb, minus_zero, -10, minus_zero);
  TEST_ff_f (scalb, 0, minus_infty, 0);
  TEST_ff_f (scalb, minus_zero, minus_infty, minus_zero);

  TEST_ff_f (scalb, plus_infty, -1, plus_infty);
  TEST_ff_f (scalb, minus_infty, -10, minus_infty);
  TEST_ff_f (scalb, plus_infty, 0, plus_infty);
  TEST_ff_f (scalb, minus_infty, 0, minus_infty);
  TEST_ff_f (scalb, plus_infty, 2, plus_infty);
  TEST_ff_f (scalb, minus_infty, 100, minus_infty);

  TEST_ff_f (scalb, 0.1L, minus_infty, 0.0);
  TEST_ff_f (scalb, -0.1L, minus_infty, minus_zero);

  TEST_ff_f (scalb, 1, plus_infty, plus_infty);
  TEST_ff_f (scalb, -1, plus_infty, minus_infty);
  TEST_ff_f (scalb, plus_infty, plus_infty, plus_infty);
  TEST_ff_f (scalb, minus_infty, plus_infty, minus_infty);

  TEST_ff_f (scalb, plus_infty, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_ff_f (scalb, minus_infty, minus_infty, nan_value, INVALID_EXCEPTION);

  TEST_ff_f (scalb, nan_value, 1, nan_value);
  TEST_ff_f (scalb, 1, nan_value, nan_value);
  TEST_ff_f (scalb, nan_value, 0, nan_value);
  TEST_ff_f (scalb, 0, nan_value, nan_value);
  TEST_ff_f (scalb, nan_value, plus_infty, nan_value);
  TEST_ff_f (scalb, plus_infty, nan_value, nan_value);
  TEST_ff_f (scalb, nan_value, nan_value, nan_value);

  TEST_ff_f (scalb, 0.8L, 4, 12.8L);
  TEST_ff_f (scalb, -0.854375L, 5, -27.34L);

  END (scalb);
}


static void
scalbn_test (void)
{

  START (scalbn);

  TEST_fi_f (scalbn, 0, 0, 0);
  TEST_fi_f (scalbn, minus_zero, 0, minus_zero);

  TEST_fi_f (scalbn, plus_infty, 1, plus_infty);
  TEST_fi_f (scalbn, minus_infty, 1, minus_infty);
  TEST_fi_f (scalbn, nan_value, 1, nan_value);

  TEST_fi_f (scalbn, 0.8L, 4, 12.8L);
  TEST_fi_f (scalbn, -0.854375L, 5, -27.34L);

  TEST_fi_f (scalbn, 1, 0L, 1);

  TEST_fi_f (scalbn, 1, INT_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbn, 1, INT_MIN, plus_zero);
  TEST_fi_f (scalbn, max_value, INT_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbn, max_value, INT_MIN, plus_zero);
  TEST_fi_f (scalbn, min_value, INT_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbn, min_value, INT_MIN, plus_zero);
  TEST_fi_f (scalbn, min_value / 4, INT_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbn, min_value / 4, INT_MIN, plus_zero);

  END (scalbn);
}


static void
scalbln_test (void)
{

  START (scalbln);

  TEST_fl_f (scalbln, 0, 0, 0);
  TEST_fl_f (scalbln, minus_zero, 0, minus_zero);

  TEST_fl_f (scalbln, plus_infty, 1, plus_infty);
  TEST_fl_f (scalbln, minus_infty, 1, minus_infty);
  TEST_fl_f (scalbln, nan_value, 1, nan_value);

  TEST_fl_f (scalbln, 0.8L, 4, 12.8L);
  TEST_fl_f (scalbln, -0.854375L, 5, -27.34L);

  TEST_fl_f (scalbln, 1, 0L, 1);

  TEST_fi_f (scalbln, 1, INT_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, 1, INT_MIN, plus_zero);
  TEST_fi_f (scalbln, max_value, INT_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, max_value, INT_MIN, plus_zero);
  TEST_fi_f (scalbln, min_value, INT_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, min_value, INT_MIN, plus_zero);
  TEST_fi_f (scalbln, min_value / 4, INT_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, min_value / 4, INT_MIN, plus_zero);

  TEST_fi_f (scalbln, 1, LONG_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, 1, LONG_MIN, plus_zero);
  TEST_fi_f (scalbln, max_value, LONG_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, max_value, LONG_MIN, plus_zero);
  TEST_fi_f (scalbln, min_value, LONG_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, min_value, LONG_MIN, plus_zero);
  TEST_fi_f (scalbln, min_value / 4, LONG_MAX, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, min_value / 4, LONG_MIN, plus_zero);

#if LONG_MAX >= 0x100000000
  TEST_fi_f (scalbln, 1, 0x88000000L, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, 1, -0x88000000L, plus_zero);
  TEST_fi_f (scalbln, max_value, 0x88000000L, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, max_value, -0x88000000L, plus_zero);
  TEST_fi_f (scalbln, min_value, 0x88000000L, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, min_value, -0x88000000L, plus_zero);
  TEST_fi_f (scalbln, min_value / 4, 0x88000000L, plus_infty, OVERFLOW_EXCEPTION);
  TEST_fi_f (scalbln, min_value / 4, -0x88000000L, plus_zero);
#endif

  END (scalbn);
}


static void
signbit_test (void)
{

  START (signbit);

  TEST_f_b (signbit, 0, 0);
  TEST_f_b (signbit, minus_zero, 1);
  TEST_f_b (signbit, plus_infty, 0);
  TEST_f_b (signbit, minus_infty, 1);

  /* signbit (x) != 0 for x < 0.  */
  TEST_f_b (signbit, -1, 1);
  /* signbit (x) == 0 for x >= 0.  */
  TEST_f_b (signbit, 1, 0);

  END (signbit);
}


static void
sin_test (void)
{
  errno = 0;
  FUNC(sin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sin);

  TEST_f_f (sin, 0, 0);
  TEST_f_f (sin, minus_zero, minus_zero);
  errno = 0;
  TEST_f_f (sin, plus_infty, nan_value, INVALID_EXCEPTION);
  check_int ("errno for sin(+inf) == EDOM", errno, EDOM, 0, 0, 0);
  errno = 0;
  TEST_f_f (sin, minus_infty, nan_value, INVALID_EXCEPTION);
  check_int ("errno for sin(-inf) == EDOM", errno, EDOM, 0, 0, 0);
  errno = 0;
  TEST_f_f (sin, nan_value, nan_value);
  check_int ("errno for sin(NaN) unchanged", errno, 0, 0, 0, 0);

  TEST_f_f (sin, M_PI_6l, 0.5);
  TEST_f_f (sin, -M_PI_6l, -0.5);
  TEST_f_f (sin, M_PI_2l, 1);
  TEST_f_f (sin, -M_PI_2l, -1);
  TEST_f_f (sin, 0.75L, 0.681638760023334166733241952779893935L);

  TEST_f_f (sin, 0x1p65, -0.047183876212354673805106149805700013943218L);
  TEST_f_f (sin, -0x1p65, 0.047183876212354673805106149805700013943218L);

#ifdef TEST_DOUBLE
  TEST_f_f (sin, 0.80190127184058835, 0.71867942238767868);
  TEST_f_f (sin, 2.522464e-1, 2.4957989804940911e-1);
#endif

#ifndef TEST_FLOAT
  TEST_f_f (sin, 1e22, -0.8522008497671888017727058937530293682618L);
  TEST_f_f (sin, 0x1p1023, 0.5631277798508840134529434079444683477104L);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_f_f (sin, 0x1p16383L, 0.3893629985894208126948115852610595405563L);
#endif

  END (sin);

}


static void
sin_test_tonearest (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(sin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sin_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_f_f (sin, 1, 0.8414709848078965066525023216302989996226L);
      TEST_f_f (sin, 2, 0.9092974268256816953960198659117448427023L);
      TEST_f_f (sin, 3, 0.1411200080598672221007448028081102798469L);
      TEST_f_f (sin, 4, -0.7568024953079282513726390945118290941359L);
      TEST_f_f (sin, 5, -0.9589242746631384688931544061559939733525L);
      TEST_f_f (sin, 6, -0.2794154981989258728115554466118947596280L);
      TEST_f_f (sin, 7, 0.6569865987187890903969990915936351779369L);
      TEST_f_f (sin, 8, 0.9893582466233817778081235982452886721164L);
      TEST_f_f (sin, 9, 0.4121184852417565697562725663524351793439L);
      TEST_f_f (sin, 10, -0.5440211108893698134047476618513772816836L);
    }

  fesetround (save_round_mode);

  END (sin_tonearest);
}


static void
sin_test_towardzero (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(sin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sin_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_f_f (sin, 1, 0.8414709848078965066525023216302989996226L);
      TEST_f_f (sin, 2, 0.9092974268256816953960198659117448427023L);
      TEST_f_f (sin, 3, 0.1411200080598672221007448028081102798469L);
      TEST_f_f (sin, 4, -0.7568024953079282513726390945118290941359L);
      TEST_f_f (sin, 5, -0.9589242746631384688931544061559939733525L);
      TEST_f_f (sin, 6, -0.2794154981989258728115554466118947596280L);
      TEST_f_f (sin, 7, 0.6569865987187890903969990915936351779369L);
      TEST_f_f (sin, 8, 0.9893582466233817778081235982452886721164L);
      TEST_f_f (sin, 9, 0.4121184852417565697562725663524351793439L);
      TEST_f_f (sin, 10, -0.5440211108893698134047476618513772816836L);
    }

  fesetround (save_round_mode);

  END (sin_towardzero);
}


static void
sin_test_downward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(sin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sin_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_f_f (sin, 1, 0.8414709848078965066525023216302989996226L);
      TEST_f_f (sin, 2, 0.9092974268256816953960198659117448427023L);
      TEST_f_f (sin, 3, 0.1411200080598672221007448028081102798469L);
      TEST_f_f (sin, 4, -0.7568024953079282513726390945118290941359L);
      TEST_f_f (sin, 5, -0.9589242746631384688931544061559939733525L);
      TEST_f_f (sin, 6, -0.2794154981989258728115554466118947596280L);
      TEST_f_f (sin, 7, 0.6569865987187890903969990915936351779369L);
      TEST_f_f (sin, 8, 0.9893582466233817778081235982452886721164L);
      TEST_f_f (sin, 9, 0.4121184852417565697562725663524351793439L);
      TEST_f_f (sin, 10, -0.5440211108893698134047476618513772816836L);
    }

  fesetround (save_round_mode);

  END (sin_downward);
}


static void
sin_test_upward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(sin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sin_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_f_f (sin, 1, 0.8414709848078965066525023216302989996226L);
      TEST_f_f (sin, 2, 0.9092974268256816953960198659117448427023L);
      TEST_f_f (sin, 3, 0.1411200080598672221007448028081102798469L);
      TEST_f_f (sin, 4, -0.7568024953079282513726390945118290941359L);
      TEST_f_f (sin, 5, -0.9589242746631384688931544061559939733525L);
      TEST_f_f (sin, 6, -0.2794154981989258728115554466118947596280L);
      TEST_f_f (sin, 7, 0.6569865987187890903969990915936351779369L);
      TEST_f_f (sin, 8, 0.9893582466233817778081235982452886721164L);
      TEST_f_f (sin, 9, 0.4121184852417565697562725663524351793439L);
      TEST_f_f (sin, 10, -0.5440211108893698134047476618513772816836L);
    }

  fesetround (save_round_mode);

  END (sin_upward);
}


static void
sincos_test (void)
{
  FLOAT sin_res, cos_res;

  errno = 0;
  FUNC(sincos) (0, &sin_res, &cos_res);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sincos);

  /* sincos is treated differently because it returns void.  */
  TEST_extra (sincos, 0, 0, 1);

  TEST_extra (sincos, minus_zero, minus_zero, 1);
  TEST_extra (sincos, plus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_extra (sincos, minus_infty, nan_value, nan_value, INVALID_EXCEPTION);
  TEST_extra (sincos, nan_value, nan_value, nan_value);

  TEST_extra (sincos, M_PI_2l, 1, 0);
  TEST_extra (sincos, M_PI_6l, 0.5, 0.86602540378443864676372317075293616L);
  TEST_extra (sincos, M_PI_6l*2.0, 0.86602540378443864676372317075293616L, 0.5);
  TEST_extra (sincos, 0.75L, 0.681638760023334166733241952779893935L, 0.731688868873820886311838753000084544L);

  TEST_extra (sincos, 0x1p65, -0.047183876212354673805106149805700013943218L, 0.99888622066058013610642172179340364209972L);
  TEST_extra (sincos, -0x1p65, 0.047183876212354673805106149805700013943218L, 0.99888622066058013610642172179340364209972L);

#ifdef TEST_DOUBLE
  TEST_extra (sincos, 0.80190127184058835, 0.71867942238767868, 0.69534156199418473);
#endif

#ifndef TEST_FLOAT
  TEST_extra (sincos, 1e22, -0.8522008497671888017727058937530293682618L, 0.5232147853951389454975944733847094921409L);
  TEST_extra (sincos, 0x1p1023, 0.5631277798508840134529434079444683477104L, -0.826369834614147994500785680811743734805L);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_extra (sincos, 0x1p16383L, 0.3893629985894208126948115852610595405563L, 0.9210843909921906206874509522505756251609L);
#endif

  END (sincos);
}

static void
sinh_test (void)
{
  errno = 0;
  FUNC(sinh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sinh);
  TEST_f_f (sinh, 0, 0);
  TEST_f_f (sinh, minus_zero, minus_zero);

#ifndef TEST_INLINE
  TEST_f_f (sinh, plus_infty, plus_infty);
  TEST_f_f (sinh, minus_infty, minus_infty);
#endif
  TEST_f_f (sinh, nan_value, nan_value);

  TEST_f_f (sinh, 0.75L, 0.822316731935829980703661634446913849L);
  TEST_f_f (sinh, 0x8p-32L, 1.86264514923095703232705808926175479e-9L);

  END (sinh);
}


static void
sinh_test_tonearest (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(sinh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sinh_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_f_f (sinh, 22, 1792456423.065795780701106568345764104225L);
      TEST_f_f (sinh, 23, 4872401723.124451299966006944252978187305L);
      TEST_f_f (sinh, 24, 13244561064.92173614705070540368454568168L);
    }

  fesetround (save_round_mode);

  END (sinh_tonearest);
}


static void
sinh_test_towardzero (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(sinh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sinh_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_f_f (sinh, 22, 1792456423.065795780701106568345764104225L);
      TEST_f_f (sinh, 23, 4872401723.124451299966006944252978187305L);
      TEST_f_f (sinh, 24, 13244561064.92173614705070540368454568168L);
    }

  fesetround (save_round_mode);

  END (sinh_towardzero);
}


static void
sinh_test_downward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(sinh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sinh_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_f_f (sinh, 22, 1792456423.065795780701106568345764104225L);
      TEST_f_f (sinh, 23, 4872401723.124451299966006944252978187305L);
      TEST_f_f (sinh, 24, 13244561064.92173614705070540368454568168L);
    }

  fesetround (save_round_mode);

  END (sinh_downward);
}


static void
sinh_test_upward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(sinh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sinh_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_f_f (sinh, 22, 1792456423.065795780701106568345764104225L);
      TEST_f_f (sinh, 23, 4872401723.124451299966006944252978187305L);
      TEST_f_f (sinh, 24, 13244561064.92173614705070540368454568168L);
    }

  fesetround (save_round_mode);

  END (sinh_upward);
}


static void
sqrt_test (void)
{
  errno = 0;
  FUNC(sqrt) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sqrt);

  TEST_f_f (sqrt, 0, 0);
  TEST_f_f (sqrt, nan_value, nan_value);
  TEST_f_f (sqrt, plus_infty, plus_infty);

  TEST_f_f (sqrt, minus_zero, minus_zero);

  /* sqrt (x) == NaN plus invalid exception for x < 0.  */
  TEST_f_f (sqrt, -1, nan_value, INVALID_EXCEPTION);
  TEST_f_f (sqrt, -max_value, nan_value, INVALID_EXCEPTION);
  TEST_f_f (sqrt, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (sqrt, nan_value, nan_value);

  TEST_f_f (sqrt, 2209, 47);
  TEST_f_f (sqrt, 4, 2);
  TEST_f_f (sqrt, 2, M_SQRT2l);
  TEST_f_f (sqrt, 0.25, 0.5);
  TEST_f_f (sqrt, 6642.25, 81.5);
  TEST_f_f (sqrt, 15190.5625L, 123.25L);
  TEST_f_f (sqrt, 0.75L, 0.866025403784438646763723170752936183L);

  END (sqrt);
}


static void
tan_test (void)
{
  errno = 0;
  FUNC(tan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tan);

  TEST_f_f (tan, 0, 0);
  TEST_f_f (tan, minus_zero, minus_zero);
  errno = 0;
  TEST_f_f (tan, plus_infty, nan_value, INVALID_EXCEPTION);
  check_int ("errno for tan(Inf) == EDOM", errno, EDOM, 0, 0, 0);
  errno = 0;
  TEST_f_f (tan, minus_infty, nan_value, INVALID_EXCEPTION);
  check_int ("errno for tan(-Inf) == EDOM", errno, EDOM, 0, 0, 0);
  errno = 0;
  TEST_f_f (tan, nan_value, nan_value);
  check_int ("errno for tan(NaN) == 0", errno, 0, 0, 0, 0);

  TEST_f_f (tan, M_PI_4l, 1);
  TEST_f_f (tan, 0.75L, 0.931596459944072461165202756573936428L);

  TEST_f_f (tan, 0x1p65, -0.0472364872359047946798414219288370688827L);
  TEST_f_f (tan, -0x1p65, 0.0472364872359047946798414219288370688827L);

#ifndef TEST_FLOAT
  TEST_f_f (tan, 1e22, -1.628778225606898878549375936939548513545L);
  TEST_f_f (tan, 0x1p1023, -0.6814476476066215012854144040167365190368L);
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
  TEST_f_f (tan, 0x1p16383L, 0.422722393732022337800504160054440141575L);
#endif

  END (tan);
}


static void
tan_test_tonearest (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(tan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tan_tonearest);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TONEAREST))
    {
      TEST_f_f (tan, 1, 1.5574077246549022305069748074583601730873L);
      TEST_f_f (tan, 2, -2.1850398632615189916433061023136825434320L);
      TEST_f_f (tan, 3, -0.1425465430742778052956354105339134932261L);
      TEST_f_f (tan, 4, 1.1578212823495775831373424182673239231198L);
      TEST_f_f (tan, 5, -3.3805150062465856369827058794473439087096L);
      TEST_f_f (tan, 6, -0.2910061913847491570536995888681755428312L);
      TEST_f_f (tan, 7, 0.8714479827243187364564508896003135663222L);
      TEST_f_f (tan, 8, -6.7997114552203786999252627596086333648814L);
      TEST_f_f (tan, 9, -0.4523156594418098405903708757987855343087L);
      TEST_f_f (tan, 10, 0.6483608274590866712591249330098086768169L);
    }

  fesetround (save_round_mode);

  END (tan_tonearest);
}


static void
tan_test_towardzero (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(tan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tan_towardzero);

  save_round_mode = fegetround ();

  if (!fesetround (FE_TOWARDZERO))
    {
      TEST_f_f (tan, 1, 1.5574077246549022305069748074583601730873L);
      TEST_f_f (tan, 2, -2.1850398632615189916433061023136825434320L);
      TEST_f_f (tan, 3, -0.1425465430742778052956354105339134932261L);
      TEST_f_f (tan, 4, 1.1578212823495775831373424182673239231198L);
      TEST_f_f (tan, 5, -3.3805150062465856369827058794473439087096L);
      TEST_f_f (tan, 6, -0.2910061913847491570536995888681755428312L);
      TEST_f_f (tan, 7, 0.8714479827243187364564508896003135663222L);
      TEST_f_f (tan, 8, -6.7997114552203786999252627596086333648814L);
      TEST_f_f (tan, 9, -0.4523156594418098405903708757987855343087L);
      TEST_f_f (tan, 10, 0.6483608274590866712591249330098086768169L);
    }

  fesetround (save_round_mode);

  END (tan_towardzero);
}


static void
tan_test_downward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(tan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tan_downward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_DOWNWARD))
    {
      TEST_f_f (tan, 1, 1.5574077246549022305069748074583601730873L);
      TEST_f_f (tan, 2, -2.1850398632615189916433061023136825434320L);
      TEST_f_f (tan, 3, -0.1425465430742778052956354105339134932261L);
      TEST_f_f (tan, 4, 1.1578212823495775831373424182673239231198L);
      TEST_f_f (tan, 5, -3.3805150062465856369827058794473439087096L);
      TEST_f_f (tan, 6, -0.2910061913847491570536995888681755428312L);
      TEST_f_f (tan, 7, 0.8714479827243187364564508896003135663222L);
      TEST_f_f (tan, 8, -6.7997114552203786999252627596086333648814L);
      TEST_f_f (tan, 9, -0.4523156594418098405903708757987855343087L);
      TEST_f_f (tan, 10, 0.6483608274590866712591249330098086768169L);
    }

  fesetround (save_round_mode);

  END (tan_downward);
}


static void
tan_test_upward (void)
{
  int save_round_mode;
  errno = 0;
  FUNC(tan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tan_upward);

  save_round_mode = fegetround ();

  if (!fesetround (FE_UPWARD))
    {
      TEST_f_f (tan, 1, 1.5574077246549022305069748074583601730873L);
      TEST_f_f (tan, 2, -2.1850398632615189916433061023136825434320L);
      TEST_f_f (tan, 3, -0.1425465430742778052956354105339134932261L);
      TEST_f_f (tan, 4, 1.1578212823495775831373424182673239231198L);
      TEST_f_f (tan, 5, -3.3805150062465856369827058794473439087096L);
      TEST_f_f (tan, 6, -0.2910061913847491570536995888681755428312L);
      TEST_f_f (tan, 7, 0.8714479827243187364564508896003135663222L);
      TEST_f_f (tan, 8, -6.7997114552203786999252627596086333648814L);
      TEST_f_f (tan, 9, -0.4523156594418098405903708757987855343087L);
      TEST_f_f (tan, 10, 0.6483608274590866712591249330098086768169L);
    }

  fesetround (save_round_mode);

  END (tan_upward);
}


static void
tanh_test (void)
{
  errno = 0;
  FUNC(tanh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tanh);

  TEST_f_f (tanh, 0, 0);
  TEST_f_f (tanh, minus_zero, minus_zero);

#ifndef TEST_INLINE
  TEST_f_f (tanh, plus_infty, 1);
  TEST_f_f (tanh, minus_infty, -1);
#endif
  TEST_f_f (tanh, nan_value, nan_value);

  TEST_f_f (tanh, 0.75L, 0.635148952387287319214434357312496495L);
  TEST_f_f (tanh, -0.75L, -0.635148952387287319214434357312496495L);

  TEST_f_f (tanh, 1.0L, 0.7615941559557648881194582826047935904L);
  TEST_f_f (tanh, -1.0L, -0.7615941559557648881194582826047935904L);

  /* 2^-57  */
  TEST_f_f (tanh, 0x1p-57L, 6.938893903907228377647697925567626953125e-18L);

  END (tanh);
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

  START (tgamma);

  TEST_f_f (tgamma, plus_infty, plus_infty);
  TEST_f_f (tgamma, max_value, plus_infty, OVERFLOW_EXCEPTION);
  TEST_f_f (tgamma, 0, plus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  TEST_f_f (tgamma, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION);
  /* tgamma (x) == NaN plus invalid exception for integer x <= 0.  */
  TEST_f_f (tgamma, -2, nan_value, INVALID_EXCEPTION);
  TEST_f_f (tgamma, -max_value, nan_value, INVALID_EXCEPTION);
  TEST_f_f (tgamma, minus_infty, nan_value, INVALID_EXCEPTION);
  TEST_f_f (tgamma, nan_value, nan_value);

  TEST_f_f (tgamma, 0.5, M_SQRT_PIl);
  TEST_f_f (tgamma, -0.5, -M_2_SQRT_PIl);

  TEST_f_f (tgamma, 1, 1);
  TEST_f_f (tgamma, 4, 6);

  TEST_f_f (tgamma, 0.7L, 1.29805533264755778568117117915281162L);
  TEST_f_f (tgamma, 1.2L, 0.918168742399760610640951655185830401L);

  END (tgamma);
}


static void
trunc_test (void)
{
  START (trunc);

  TEST_f_f (trunc, plus_infty, plus_infty);
  TEST_f_f (trunc, minus_infty, minus_infty);
  TEST_f_f (trunc, nan_value, nan_value);

  TEST_f_f (trunc, 0, 0);
  TEST_f_f (trunc, minus_zero, minus_zero);
  TEST_f_f (trunc, 0.1, 0);
  TEST_f_f (trunc, 0.25, 0);
  TEST_f_f (trunc, 0.625, 0);
  TEST_f_f (trunc, -0.1, minus_zero);
  TEST_f_f (trunc, -0.25, minus_zero);
  TEST_f_f (trunc, -0.625, minus_zero);
  TEST_f_f (trunc, 1, 1);
  TEST_f_f (trunc, -1, -1);
  TEST_f_f (trunc, 1.625, 1);
  TEST_f_f (trunc, -1.625, -1);

  TEST_f_f (trunc, 1048580.625L, 1048580L);
  TEST_f_f (trunc, -1048580.625L, -1048580L);

  TEST_f_f (trunc, 8388610.125L, 8388610.0L);
  TEST_f_f (trunc, -8388610.125L, -8388610.0L);

  TEST_f_f (trunc, 4294967296.625L, 4294967296.0L);
  TEST_f_f (trunc, -4294967296.625L, -4294967296.0L);

#ifdef TEST_LDOUBLE
  /* The result can only be represented in long double.  */
  TEST_f_f (trunc, 4503599627370495.5L, 4503599627370495.0L);
  TEST_f_f (trunc, 4503599627370496.25L, 4503599627370496.0L);
  TEST_f_f (trunc, 4503599627370496.5L, 4503599627370496.0L);
  TEST_f_f (trunc, 4503599627370496.75L, 4503599627370496.0L);
  TEST_f_f (trunc, 4503599627370497.5L, 4503599627370497.0L);

# if LDBL_MANT_DIG > 100
  TEST_f_f (trunc, 4503599627370494.5000000000001L, 4503599627370494.0L);
  TEST_f_f (trunc, 4503599627370495.5000000000001L, 4503599627370495.0L);
  TEST_f_f (trunc, 4503599627370496.5000000000001L, 4503599627370496.0L);
# endif

  TEST_f_f (trunc, -4503599627370495.5L, -4503599627370495.0L);
  TEST_f_f (trunc, -4503599627370496.25L, -4503599627370496.0L);
  TEST_f_f (trunc, -4503599627370496.5L, -4503599627370496.0L);
  TEST_f_f (trunc, -4503599627370496.75L, -4503599627370496.0L);
  TEST_f_f (trunc, -4503599627370497.5L, -4503599627370497.0L);

# if LDBL_MANT_DIG > 100
  TEST_f_f (trunc, -4503599627370494.5000000000001L, -4503599627370494.0L);
  TEST_f_f (trunc, -4503599627370495.5000000000001L, -4503599627370495.0L);
  TEST_f_f (trunc, -4503599627370496.5000000000001L, -4503599627370496.0L);
# endif

  TEST_f_f (trunc, 9007199254740991.5L, 9007199254740991.0L);
  TEST_f_f (trunc, 9007199254740992.25L, 9007199254740992.0L);
  TEST_f_f (trunc, 9007199254740992.5L, 9007199254740992.0L);
  TEST_f_f (trunc, 9007199254740992.75L, 9007199254740992.0L);
  TEST_f_f (trunc, 9007199254740993.5L, 9007199254740993.0L);

# if LDBL_MANT_DIG > 100
  TEST_f_f (trunc, 9007199254740991.0000000000001L, 9007199254740991.0L);
  TEST_f_f (trunc, 9007199254740992.0000000000001L, 9007199254740992.0L);
  TEST_f_f (trunc, 9007199254740993.0000000000001L, 9007199254740993.0L);
  TEST_f_f (trunc, 9007199254740991.5000000000001L, 9007199254740991.0L);
  TEST_f_f (trunc, 9007199254740992.5000000000001L, 9007199254740992.0L);
  TEST_f_f (trunc, 9007199254740993.5000000000001L, 9007199254740993.0L);
# endif

  TEST_f_f (trunc, -9007199254740991.5L, -9007199254740991.0L);
  TEST_f_f (trunc, -9007199254740992.25L, -9007199254740992.0L);
  TEST_f_f (trunc, -9007199254740992.5L, -9007199254740992.0L);
  TEST_f_f (trunc, -9007199254740992.75L, -9007199254740992.0L);
  TEST_f_f (trunc, -9007199254740993.5L, -9007199254740993.0L);

# if LDBL_MANT_DIG > 100
  TEST_f_f (trunc, -9007199254740991.0000000000001L, -9007199254740991.0L);
  TEST_f_f (trunc, -9007199254740992.0000000000001L, -9007199254740992.0L);
  TEST_f_f (trunc, -9007199254740993.0000000000001L, -9007199254740993.0L);
  TEST_f_f (trunc, -9007199254740991.5000000000001L, -9007199254740991.0L);
  TEST_f_f (trunc, -9007199254740992.5000000000001L, -9007199254740992.0L);
  TEST_f_f (trunc, -9007199254740993.5000000000001L, -9007199254740993.0L);
# endif

  TEST_f_f (trunc, 72057594037927935.5L, 72057594037927935.0L);
  TEST_f_f (trunc, 72057594037927936.25L, 72057594037927936.0L);
  TEST_f_f (trunc, 72057594037927936.5L, 72057594037927936.0L);
  TEST_f_f (trunc, 72057594037927936.75L, 72057594037927936.0L);
  TEST_f_f (trunc, 72057594037927937.5L, 72057594037927937.0L);

  TEST_f_f (trunc, -72057594037927935.5L, -72057594037927935.0L);
  TEST_f_f (trunc, -72057594037927936.25L, -72057594037927936.0L);
  TEST_f_f (trunc, -72057594037927936.5L, -72057594037927936.0L);
  TEST_f_f (trunc, -72057594037927936.75L, -72057594037927936.0L);
  TEST_f_f (trunc, -72057594037927937.5L, -72057594037927937.0L);

  TEST_f_f (trunc, 10141204801825835211973625643007.5L, 10141204801825835211973625643007.0L);
  TEST_f_f (trunc, 10141204801825835211973625643008.25L, 10141204801825835211973625643008.0L);
  TEST_f_f (trunc, 10141204801825835211973625643008.5L, 10141204801825835211973625643008.0L);
  TEST_f_f (trunc, 10141204801825835211973625643008.75L, 10141204801825835211973625643008.0L);
  TEST_f_f (trunc, 10141204801825835211973625643009.5L, 10141204801825835211973625643009.0L);
#endif

  END (trunc);
}

static void
y0_test (void)
{
  FLOAT s, c;
  errno = 0;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
  FUNC(y0) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* y0 is the Bessel function of the second kind of order 0 */
  START (y0);

  TEST_f_f (y0, -1.0, minus_infty, INVALID_EXCEPTION);
  TEST_f_f (y0, -max_value, minus_infty, INVALID_EXCEPTION);
  TEST_f_f (y0, 0.0, minus_infty);
  TEST_f_f (y0, nan_value, nan_value);
  TEST_f_f (y0, plus_infty, 0);

  TEST_f_f (y0, 0.125L, -1.38968062514384052915582277745018693L);
  TEST_f_f (y0, 0.75L, -0.137172769385772397522814379396581855L);
  TEST_f_f (y0, 1.0, 0.0882569642156769579829267660235151628L);
  TEST_f_f (y0, 1.5, 0.382448923797758843955068554978089862L);
  TEST_f_f (y0, 2.0, 0.510375672649745119596606592727157873L);
  TEST_f_f (y0, 8.0, 0.223521489387566220527323400498620359L);
  TEST_f_f (y0, 10.0, 0.0556711672835993914244598774101900481L);

  TEST_f_f (y0, 0x1.3ffp+74L, 1.818984347516051243459467456433028748678e-12L);

#ifndef TEST_FLOAT
  TEST_f_f (y0, 0x1.ff00000000002p+840L, 1.846591691699331493194965158699937660696e-127L);
#endif

  END (y0);
}


static void
y1_test (void)
{
  FLOAT s, c;
  errno = 0;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
  FUNC(y1) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* y1 is the Bessel function of the second kind of order 1 */
  START (y1);

  TEST_f_f (y1, -1.0, minus_infty, INVALID_EXCEPTION);
  TEST_f_f (y1, -max_value, minus_infty, INVALID_EXCEPTION);
  TEST_f_f (y1, 0.0, minus_infty);
  TEST_f_f (y1, plus_infty, 0);
  TEST_f_f (y1, nan_value, nan_value);

  TEST_f_f (y1, 0.125L, -5.19993611253477499595928744876579921L);
  TEST_f_f (y1, 0.75L, -1.03759455076928541973767132140642198L);
  TEST_f_f (y1, 1.0, -0.781212821300288716547150000047964821L);
  TEST_f_f (y1, 1.5, -0.412308626973911295952829820633445323L);
  TEST_f_f (y1, 2.0, -0.107032431540937546888370772277476637L);
  TEST_f_f (y1, 8.0, -0.158060461731247494255555266187483550L);
  TEST_f_f (y1, 10.0, 0.249015424206953883923283474663222803L);

  TEST_f_f (y1, 0x1.27e204p+99L, -8.881610148467797208469612080785210013461e-16L);

#ifndef TEST_FLOAT
  TEST_f_f (y1, 0x1.001000001p+593L, 3.927269966354206207832593635798954916263e-90L);
#endif

  END (y1);
}


static void
yn_test (void)
{
  FLOAT s, c;
  errno = 0;
  FUNC (sincos) (0, &s, &c);
  if (errno == ENOSYS)
    /* Required function not implemented.  */
    return;
  FUNC(yn) (1, 1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  /* yn is the Bessel function of the second kind of order n */
  START (yn);

  /* yn (0, x) == y0 (x)  */
  TEST_ff_f (yn, 0, -1.0, minus_infty, INVALID_EXCEPTION);
  TEST_ff_f (yn, 0, -max_value, minus_infty, INVALID_EXCEPTION);
  TEST_ff_f (yn, 0, 0.0, minus_infty);
  TEST_ff_f (yn, 0, nan_value, nan_value);
  TEST_ff_f (yn, 0, plus_infty, 0);

  TEST_ff_f (yn, 0, 0.125L, -1.38968062514384052915582277745018693L);
  TEST_ff_f (yn, 0, 0.75L, -0.137172769385772397522814379396581855L);
  TEST_ff_f (yn, 0, 1.0, 0.0882569642156769579829267660235151628L);
  TEST_ff_f (yn, 0, 1.5, 0.382448923797758843955068554978089862L);
  TEST_ff_f (yn, 0, 2.0, 0.510375672649745119596606592727157873L);
  TEST_ff_f (yn, 0, 8.0, 0.223521489387566220527323400498620359L);
  TEST_ff_f (yn, 0, 10.0, 0.0556711672835993914244598774101900481L);

  /* yn (1, x) == y1 (x)  */
  TEST_ff_f (yn, 1, -1.0, minus_infty, INVALID_EXCEPTION);
  TEST_ff_f (yn, 1, 0.0, minus_infty);
  TEST_ff_f (yn, 1, plus_infty, 0);
  TEST_ff_f (yn, 1, nan_value, nan_value);

  TEST_ff_f (yn, 1, 0.125L, -5.19993611253477499595928744876579921L);
  TEST_ff_f (yn, 1, 0.75L, -1.03759455076928541973767132140642198L);
  TEST_ff_f (yn, 1, 1.0, -0.781212821300288716547150000047964821L);
  TEST_ff_f (yn, 1, 1.5, -0.412308626973911295952829820633445323L);
  TEST_ff_f (yn, 1, 2.0, -0.107032431540937546888370772277476637L);
  TEST_ff_f (yn, 1, 8.0, -0.158060461731247494255555266187483550L);
  TEST_ff_f (yn, 1, 10.0, 0.249015424206953883923283474663222803L);

  /* yn (3, x)  */
  TEST_ff_f (yn, 3, plus_infty, 0);
  TEST_ff_f (yn, 3, nan_value, nan_value);

  TEST_ff_f (yn, 3, 0.125L, -2612.69757350066712600220955744091741L);
  TEST_ff_f (yn, 3, 0.75L, -12.9877176234475433186319774484809207L);
  TEST_ff_f (yn, 3, 1.0, -5.82151760596472884776175706442981440L);
  TEST_ff_f (yn, 3, 2.0, -1.12778377684042778608158395773179238L);
  TEST_ff_f (yn, 3, 10.0, -0.251362657183837329779204747654240998L);

  /* yn (10, x)  */
  TEST_ff_f (yn, 10, plus_infty, 0);
  TEST_ff_f (yn, 10, nan_value, nan_value);

  TEST_ff_f (yn, 10, 0.125L, -127057845771019398.252538486899753195L);
  TEST_ff_f (yn, 10, 0.75L, -2133501638.90573424452445412893839236L);
  TEST_ff_f (yn, 10, 1.0, -121618014.278689189288130426667971145L);
  TEST_ff_f (yn, 10, 2.0, -129184.542208039282635913145923304214L);
  TEST_ff_f (yn, 10, 10.0, -0.359814152183402722051986577343560609L);

  END (yn);

}


static void
significand_test (void)
{
  /* significand returns the mantissa of the exponential representation.  */
  START (significand);

  TEST_f_f (significand, 4.0, 1.0);
  TEST_f_f (significand, 6.0, 1.5);
  TEST_f_f (significand, 8.0, 1.0);

  END (significand);
}


static void
initialize (void)
{
  fpstack_test ("start *init*");
  plus_zero = 0.0;
  nan_value = plus_zero / plus_zero;	/* Suppress GCC warning */

  minus_zero = FUNC(copysign) (0.0, -1.0);
  plus_infty = CHOOSE (HUGE_VALL, HUGE_VAL, HUGE_VALF,
		       HUGE_VALL, HUGE_VAL, HUGE_VALF);
  minus_infty = CHOOSE (-HUGE_VALL, -HUGE_VAL, -HUGE_VALF,
			-HUGE_VALL, -HUGE_VAL, -HUGE_VALF);
  max_value = CHOOSE (LDBL_MAX, DBL_MAX, FLT_MAX,
		      LDBL_MAX, DBL_MAX, FLT_MAX);
  min_value = CHOOSE (LDBL_MIN, DBL_MIN, FLT_MIN,
		      LDBL_MIN, DBL_MIN, FLT_MIN);

  (void) &plus_zero;
  (void) &nan_value;
  (void) &minus_zero;
  (void) &plus_infty;
  (void) &minus_infty;
  (void) &max_value;
  (void) &min_value;

  /* Clear all exceptions.  From now on we must not get random exceptions.  */
  feclearexcept (FE_ALL_EXCEPT);

  /* Test to make sure we start correctly.  */
  fpstack_test ("end *init*");
}

/* Definitions of arguments for argp functions.  */
static const struct argp_option options[] =
{
  { "verbose", 'v', "NUMBER", 0, "Level of verbosity (0..3)"},
  { "ulps-file", 'u', NULL, 0, "Output ulps to file ULPs"},
  { "no-max-error", 'f', NULL, 0,
    "Don't output maximal errors of functions"},
  { "no-points", 'p', NULL, 0,
    "Don't output results of functions invocations"},
  { "ignore-max-ulp", 'i', "yes/no", 0,
    "Ignore given maximal errors"},
  { NULL, 0, NULL, 0, NULL }
};

/* Short description of program.  */
static const char doc[] = "Math test suite: " TEST_MSG ;

/* Prototype for option handler.  */
static error_t parse_opt (int key, char *arg, struct argp_state *state);

/* Data structure to communicate with argp functions.  */
static struct argp argp =
{
  options, parse_opt, NULL, doc,
};


/* Handle program arguments.  */
static error_t
parse_opt (int key, char *arg, struct argp_state *state)
{
  switch (key)
    {
    case 'f':
      output_max_error = 0;
      break;
    case 'i':
      if (strcmp (arg, "yes") == 0)
	ignore_max_ulp = 1;
      else if (strcmp (arg, "no") == 0)
	ignore_max_ulp = 0;
      break;
    case 'p':
      output_points = 0;
      break;
    case 'u':
      output_ulps = 1;
      break;
    case 'v':
      if (optarg)
	verbose = (unsigned int) strtoul (optarg, NULL, 0);
      else
	verbose = 3;
      break;
    default:
      return ARGP_ERR_UNKNOWN;
    }
  return 0;
}

#if 0
/* function to check our ulp calculation.  */
void
check_ulp (void)
{
  int i;

  FLOAT u, diff, ulp;
  /* This gives one ulp.  */
  u = FUNC(nextafter) (10, 20);
  check_equal (10.0, u, 1, &diff, &ulp);
  printf ("One ulp: % .4" PRINTF_NEXPR "\n", ulp);

  /* This gives one more ulp.  */
  u = FUNC(nextafter) (u, 20);
  check_equal (10.0, u, 2, &diff, &ulp);
  printf ("two ulp: % .4" PRINTF_NEXPR "\n", ulp);

  /* And now calculate 100 ulp.  */
  for (i = 2; i < 100; i++)
    u = FUNC(nextafter) (u, 20);
  check_equal (10.0, u, 100, &diff, &ulp);
  printf ("100 ulp: % .4" PRINTF_NEXPR "\n", ulp);
}
#endif

int
main (int argc, char **argv)
{

  int remaining;

  verbose = 1;
  output_ulps = 0;
  output_max_error = 1;
  output_points = 1;
  /* XXX set to 0 for releases.  */
  ignore_max_ulp = 0;

  /* Parse and process arguments.  */
  argp_parse (&argp, argc, argv, 0, &remaining, NULL);

  if (remaining != argc)
    {
      fprintf (stderr, "wrong number of arguments");
      argp_help (&argp, stdout, ARGP_HELP_SEE, program_invocation_short_name);
      exit (EXIT_FAILURE);
    }

  if (output_ulps)
    {
      ulps_file = fopen ("ULPs", "a");
      if (ulps_file == NULL)
	{
	  perror ("can't open file `ULPs' for writing: ");
	  exit (1);
	}
    }


  initialize ();
  printf (TEST_MSG);

#if 0
  check_ulp ();
#endif

  /* Keep the tests a wee bit ordered (according to ISO C99).  */
  /* Classification macros:  */
  fpclassify_test ();
  isfinite_test ();
  isnormal_test ();
  signbit_test ();

  /* Trigonometric functions:  */
  acos_test ();
  acos_test_tonearest ();
  acos_test_towardzero ();
  acos_test_downward ();
  acos_test_upward ();
  asin_test ();
  asin_test_tonearest ();
  asin_test_towardzero ();
  asin_test_downward ();
  asin_test_upward ();
  atan_test ();
  atan2_test ();
  cos_test ();
  cos_test_tonearest ();
  cos_test_towardzero ();
  cos_test_downward ();
  cos_test_upward ();
  sin_test ();
  sin_test_tonearest ();
  sin_test_towardzero ();
  sin_test_downward ();
  sin_test_upward ();
  sincos_test ();
  tan_test ();
  tan_test_tonearest ();
  tan_test_towardzero ();
  tan_test_downward ();
  tan_test_upward ();

  /* Hyperbolic functions:  */
  acosh_test ();
  asinh_test ();
  atanh_test ();
  cosh_test ();
  cosh_test_tonearest ();
  cosh_test_towardzero ();
  cosh_test_downward ();
  cosh_test_upward ();
  sinh_test ();
  sinh_test_tonearest ();
  sinh_test_towardzero ();
  sinh_test_downward ();
  sinh_test_upward ();
  tanh_test ();

  /* Exponential and logarithmic functions:  */
  exp_test ();
  exp_test_tonearest ();
  exp_test_towardzero ();
  exp_test_downward ();
  exp_test_upward ();
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
  logb_test_downward ();
  modf_test ();
  ilogb_test ();
  scalb_test ();
  scalbn_test ();
  scalbln_test ();
  significand_test ();

  /* Power and absolute value functions:  */
  cbrt_test ();
  fabs_test ();
  hypot_test ();
  pow_test ();
  pow_test_tonearest ();
  pow_test_towardzero ();
  pow_test_downward ();
  pow_test_upward ();
  sqrt_test ();

  /* Error and gamma functions:  */
  erf_test ();
  erfc_test ();
  gamma_test ();
  lgamma_test ();
  tgamma_test ();

  /* Nearest integer functions:  */
  ceil_test ();
  floor_test ();
  nearbyint_test ();
  rint_test ();
  rint_test_tonearest ();
  rint_test_towardzero ();
  rint_test_downward ();
  rint_test_upward ();
  lrint_test ();
  lrint_test_tonearest ();
  lrint_test_towardzero ();
  lrint_test_downward ();
  lrint_test_upward ();
  llrint_test ();
  llrint_test_tonearest ();
  llrint_test_towardzero ();
  llrint_test_downward ();
  llrint_test_upward ();
  round_test ();
  lround_test ();
  llround_test ();
  trunc_test ();

  /* Remainder functions:  */
  fmod_test ();
  remainder_test ();
  remquo_test ();

  /* Manipulation functions:  */
  copysign_test ();
  nextafter_test ();
  nexttoward_test ();

  /* maximum, minimum and positive difference functions */
  fdim_test ();
  fmax_test ();
  fmin_test ();

  /* Multiply and add:  */
  fma_test ();

  /* Complex functions:  */
  cabs_test ();
  cacos_test ();
  cacosh_test ();
  carg_test ();
  casin_test ();
  casinh_test ();
  catan_test ();
  catanh_test ();
  ccos_test ();
  ccosh_test ();
  cexp_test ();
  cimag_test ();
  clog10_test ();
  clog_test ();
  conj_test ();
  cpow_test ();
  cproj_test ();
  creal_test ();
  csin_test ();
  csinh_test ();
  csqrt_test ();
  ctan_test ();
  ctanh_test ();

  /* Bessel functions:  */
  j0_test ();
  j1_test ();
  jn_test ();
  y0_test ();
  y1_test ();
  yn_test ();

  if (output_ulps)
    fclose (ulps_file);

  printf ("\nTest suite completed:\n");
  printf ("  %d test cases plus %d tests for exception flags executed.\n",
	  noTests, noExcTests);
  if (noXFails)
    printf ("  %d expected failures occurred.\n", noXFails);
  if (noXPasses)
    printf ("  %d unexpected passes occurred.\n", noXPasses);
  if (noErrors)
    {
      printf ("  %d errors occurred.\n", noErrors);
      return 1;
    }
  printf ("  All tests passed successfully.\n");

  return 0;
}

/*
 * Local Variables:
 * mode:c
 * End:
 */
