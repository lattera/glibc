/* Copyright (C) 1997-2013 Free Software Foundation, Inc.
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
   fabs, fdim, finite, floor, fma, fmax, fmin, fmod, fpclassify,
   frexp, gamma, hypot,
   ilogb, isfinite, isinf, isnan, isnormal, issignaling,
   isless, islessequal, isgreater, isgreaterequal, islessgreater, isunordered,
   j0, j1, jn,
   ldexp, lgamma, log, log10, log1p, log2, logb,
   modf, nearbyint, nextafter, nexttoward,
   pow, remainder, remquo, rint, lrint, llrint,
   round, lround, llround,
   scalb, scalbn, scalbln, signbit, sin, sincos, sinh, sqrt, tan, tanh, tgamma, trunc,
   y0, y1, yn, significand

   and for the following complex math functions:
   cabs, cacos, cacosh, carg, casin, casinh, catan, catanh,
   ccos, ccosh, cexp, cimag, clog, clog10, conj, cpow, cproj, creal,
   csin, csinh, csqrt, ctan, ctanh.

   At the moment the following functions and macros aren't tested:
   drem (alias for remainder),
   lgamma_r,
   nan,
   pow10 (alias for exp10).

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

   Exception testing: At the moment only divide-by-zero, invalid,
   overflow and underflow exceptions are tested.  Inexact exceptions
   aren't checked at the moment.

   NaN values: There exist signalling and quiet NaNs.  This implementation
   only uses quiet NaN as parameter.  Where the sign of a NaN is
   significant, this is not tested.  The payload of NaNs is not examined.

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
#include <tininess.h>

/* Allow platforms without all rounding modes to test properly,
   assuming they provide an __FE_UNDEFINED in <bits/fenv.h> which
   causes fesetround() to return failure.  */
#ifndef FE_TONEAREST
# define FE_TONEAREST	__FE_UNDEFINED
#endif
#ifndef FE_TOWARDZERO
# define FE_TOWARDZERO	__FE_UNDEFINED
#endif
#ifndef FE_UPWARD
# define FE_UPWARD	__FE_UNDEFINED
#endif
#ifndef FE_DOWNWARD
# define FE_DOWNWARD	__FE_UNDEFINED
#endif

/* Possible exceptions */
#define NO_EXCEPTION			0x0
#define INVALID_EXCEPTION		0x1
#define DIVIDE_BY_ZERO_EXCEPTION	0x2
#define OVERFLOW_EXCEPTION		0x4
#define UNDERFLOW_EXCEPTION		0x8
/* The next flags signals that those exceptions are allowed but not required.   */
#define INVALID_EXCEPTION_OK		0x10
#define DIVIDE_BY_ZERO_EXCEPTION_OK	0x20
#define OVERFLOW_EXCEPTION_OK		0x40
#define UNDERFLOW_EXCEPTION_OK		0x80
#define EXCEPTIONS_OK INVALID_EXCEPTION_OK+DIVIDE_BY_ZERO_EXCEPTION_OK
/* Some special test flags, passed together with exceptions.  */
#define IGNORE_ZERO_INF_SIGN		0x100
/* Indicate errno settings required or disallowed.  */
#define ERRNO_UNCHANGED			0x200
#define ERRNO_EDOM			0x400
#define ERRNO_ERANGE			0x800

/* Values underflowing only for float.  */
#ifdef TEST_FLOAT
# define UNDERFLOW_EXCEPTION_FLOAT	UNDERFLOW_EXCEPTION
# define UNDERFLOW_EXCEPTION_OK_FLOAT	UNDERFLOW_EXCEPTION_OK
#else
# define UNDERFLOW_EXCEPTION_FLOAT	0
# define UNDERFLOW_EXCEPTION_OK_FLOAT	0
#endif
/* Values underflowing only for double or types with a larger least
   positive normal value.  */
#if defined TEST_FLOAT || defined TEST_DOUBLE \
  || (defined TEST_LDOUBLE && LDBL_MIN_EXP >= DBL_MIN_EXP)
# define UNDERFLOW_EXCEPTION_DOUBLE	UNDERFLOW_EXCEPTION
# define UNDERFLOW_EXCEPTION_OK_DOUBLE	UNDERFLOW_EXCEPTION_OK
#else
# define UNDERFLOW_EXCEPTION_DOUBLE	0
# define UNDERFLOW_EXCEPTION_OK_DOUBLE	0
#endif
/* Values underflowing only for IBM long double or types with a larger least
   positive normal value.  */
#if defined TEST_FLOAT || (defined TEST_LDOUBLE && LDBL_MIN_EXP > DBL_MIN_EXP)
# define UNDERFLOW_EXCEPTION_LDOUBLE_IBM	UNDERFLOW_EXCEPTION
#else
# define UNDERFLOW_EXCEPTION_LDOUBLE_IBM	0
#endif
/* Values underflowing on architectures detecting tininess before
   rounding, but not on those detecting tininess after rounding.  */
#define UNDERFLOW_EXCEPTION_BEFORE_ROUNDING	(TININESS_AFTER_ROUNDING \
						 ? 0			\
						 : UNDERFLOW_EXCEPTION)

/* Various constants (we must supply them precalculated for accuracy).  */
#define M_PI_6l			.52359877559829887307710723054658383L
#define M_E2l			7.389056098930650227230427460575008L
#define M_E3l			20.085536923187667740928529654581719L
#define M_2_SQRT_PIl		3.5449077018110320545963349666822903L	/* 2 sqrt (M_PIl)  */
#define M_SQRT_PIl		1.7724538509055160272981674833411451L	/* sqrt (M_PIl)  */
#define M_LOG_SQRT_PIl		0.57236494292470008707171367567652933L	/* log(sqrt(M_PIl))  */
#define M_LOG_2_SQRT_PIl	1.265512123484645396488945797134706L	/* log(2*sqrt(M_PIl))  */
#define M_PI_34l		2.356194490192344928846982537459627163L	/* 3*pi/4 */
#define M_PI_34_LOG10El		1.023282265381381010614337719073516828L
#define M_PI2_LOG10El		0.682188176920920673742891812715677885L
#define M_PI4_LOG10El		0.341094088460460336871445906357838943L
#define M_PI_LOG10El		1.364376353841841347485783625431355770L
#define M_1_DIV_El              0.367879441171442321595523770161460867L /* 1 div e  */

#define ulps_file_name "ULPs"	/* Name of the ULPs file.  */
static FILE *ulps_file;		/* File to document difference.  */
static int output_ulps;		/* Should ulps printed?  */
static char *output_dir;	/* Directory where generated files will be written.  */

static int noErrors;	/* number of errors */
static int noTests;	/* number of tests (without testing exceptions) */
static int noExcTests;	/* number of tests for exception flags */
static int noErrnoTests;/* number of tests for errno values */

static int verbose;
static int output_max_error;	/* Should the maximal errors printed?  */
static int output_points;	/* Should the single function results printed?  */
static int ignore_max_ulp;	/* Should we ignore max_ulp?  */

#define PLUS_ZERO_INIT	CHOOSE (0.0L, 0.0, 0.0f,	\
				0.0L, 0.0, 0.0f)
#define MINUS_ZERO_INIT	CHOOSE (-0.0L, -0.0, -0.0f,	\
				-0.0L, -0.0, -0.0f)
#define PLUS_INFTY_INIT	CHOOSE (HUGE_VALL, HUGE_VAL, HUGE_VALF, \
				HUGE_VALL, HUGE_VAL, HUGE_VALF)
#define MINUS_INFTY_INIT	CHOOSE (-HUGE_VALL, -HUGE_VAL, -HUGE_VALF, \
					-HUGE_VALL, -HUGE_VAL, -HUGE_VALF)
#define QNAN_VALUE_INIT	FUNC (__builtin_nan) ("")
#define MAX_VALUE_INIT	CHOOSE (LDBL_MAX, DBL_MAX, FLT_MAX,	\
				LDBL_MAX, DBL_MAX, FLT_MAX)
#define MIN_VALUE_INIT	CHOOSE (LDBL_MIN, DBL_MIN, FLT_MIN,	\
				LDBL_MIN, DBL_MIN, FLT_MIN)
#define MIN_SUBNORM_VALUE_INIT	CHOOSE (__LDBL_DENORM_MIN__,	\
					__DBL_DENORM_MIN__,	\
					__FLT_DENORM_MIN__,	\
					__LDBL_DENORM_MIN__,	\
					__DBL_DENORM_MIN__,	\
					__FLT_DENORM_MIN__)

static const FLOAT plus_zero = PLUS_ZERO_INIT;
static const FLOAT minus_zero = MINUS_ZERO_INIT;
static const FLOAT plus_infty = PLUS_INFTY_INIT;
static const FLOAT minus_infty = MINUS_INFTY_INIT;
static const FLOAT qnan_value = QNAN_VALUE_INIT;
static const FLOAT max_value = MAX_VALUE_INIT;
static const FLOAT min_value = MIN_VALUE_INIT;
static const FLOAT min_subnorm_value = MIN_SUBNORM_VALUE_INIT;

static FLOAT max_error, real_max_error, imag_max_error;


#define BUILD_COMPLEX(real, imag) \
  ({ __complex__ FLOAT __retval;					      \
     __real__ __retval = (real);					      \
     __imag__ __retval = (imag);					      \
     __retval; })
#define BUILD_COMPLEX_ULP(real, imag) ((real) + I * (imag))

#define MANT_DIG CHOOSE ((LDBL_MANT_DIG-1), (DBL_MANT_DIG-1), (FLT_MANT_DIG-1),  \
			 (LDBL_MANT_DIG-1), (DBL_MANT_DIG-1), (FLT_MANT_DIG-1))

static void
init_max_error (void)
{
  max_error = 0;
  real_max_error = 0;
  imag_max_error = 0;
  feclearexcept (FE_ALL_EXCEPT);
  errno = 0;
}

static void
set_max_error (FLOAT current, FLOAT *curr_max_error)
{
  if (current > *curr_max_error)
    *curr_max_error = current;
}


/* Print a FLOAT.  */
static void
print_float (FLOAT f)
{
  /* As printf doesn't differ between a sNaN and a qNaN, do this manually.  */
  if (issignaling (f))
    printf ("sNaN\n");
  else if (isnan (f))
    printf ("qNaN\n");
  else
    printf ("% .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR "\n", f, f);
}

/* Should the message print to screen?  This depends on the verbose flag,
   and the test status.  */
static int
print_screen (int ok)
{
  if (output_points
      && (verbose > 1
	  || (verbose == 1 && ok == 0)))
    return 1;
  return 0;
}


/* Should the message print to screen?  This depends on the verbose flag,
   and the test status.  */
static int
print_screen_max_error (int ok)
{
  if (output_max_error
      && (verbose > 1
	  || ((verbose == 1) && (ok == 0))))
    return 1;
  return 0;
}

/* Update statistic counters.  */
static void
update_stats (int ok)
{
  ++noTests;
  if (!ok)
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
#if defined (__i386__) || defined (__x86_64__)
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
print_max_error (const char *func_name, FLOAT allowed)
{
  int ok = 0;

  if (max_error == 0.0 || (max_error <= allowed && !ignore_max_ulp))
    {
      ok = 1;
    }

  if (!ok)
    print_function_ulps (func_name, max_error);


  if (print_screen_max_error (ok))
    {
      printf ("Maximal error of `%s'\n", func_name);
      printf (" is      : %.0" PRINTF_NEXPR " ulp\n", FUNC(ceil) (max_error));
      printf (" accepted: %.0" PRINTF_NEXPR " ulp\n", FUNC(ceil) (allowed));
    }

  update_stats (ok);
}


static void
print_complex_max_error (const char *func_name, __complex__ FLOAT allowed)
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


  if (print_screen_max_error (ok))
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

  update_stats (ok);
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
	  if (print_screen (1))
	    printf ("Pass: %s: Exception \"%s\" set\n", test_name, flag_name);
	}
      else
	{
	  ok = 0;
	  if (print_screen (0))
	    printf ("Failure: %s: Exception \"%s\" not set\n",
		    test_name, flag_name);
	}
    }
  else
    {
      if (fetestexcept (fe_flag))
	{
	  ok = 0;
	  if (print_screen (0))
	    printf ("Failure: %s: Exception \"%s\" set\n",
		    test_name, flag_name);
	}
      else
	{
	  if (print_screen (1))
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
#ifdef FE_UNDERFLOW
  if ((exception & UNDERFLOW_EXCEPTION_OK) == 0)
    test_single_exception (test_name, exception, UNDERFLOW_EXCEPTION,
			   FE_UNDERFLOW, "Underflow");
#endif
  feclearexcept (FE_ALL_EXCEPT);
}

/* Test whether errno for TEST_NAME, set to ERRNO_VALUE, has value
   EXPECTED_VALUE (description EXPECTED_NAME).  */
static void
test_single_errno (const char *test_name, int errno_value,
		   int expected_value, const char *expected_name)
{
  if (errno_value == expected_value)
    {
      if (print_screen (1))
	printf ("Pass: %s: errno set to %d (%s)\n", test_name, errno_value,
		expected_name);
    }
  else
    {
      ++noErrors;
      if (print_screen (0))
	printf ("Failure: %s: errno set to %d, expected %d (%s)\n",
		test_name, errno_value, expected_value, expected_name);
    }
}

/* Test whether errno (value ERRNO_VALUE) has been for TEST_NAME set
   as required by EXCEPTIONS.  */
static void
test_errno (const char *test_name, int errno_value, int exceptions)
{
  ++noErrnoTests;
  if (exceptions & ERRNO_UNCHANGED)
    test_single_errno (test_name, errno_value, 0, "unchanged");
  if (exceptions & ERRNO_EDOM)
    test_single_errno (test_name, errno_value, EDOM, "EDOM");
  if (exceptions & ERRNO_ERANGE)
    test_single_errno (test_name, errno_value, ERANGE, "ERANGE");
}

static void
check_float_internal (const char *test_name, FLOAT computed, FLOAT expected,
		      FLOAT max_ulp, int exceptions,
		      FLOAT *curr_max_error)
{
  int ok = 0;
  int print_diff = 0;
  FLOAT diff = 0;
  FLOAT ulp = 0;
  int errno_value = errno;

  test_exceptions (test_name, exceptions);
  test_errno (test_name, errno_value, exceptions);
  if (issignaling (computed) && issignaling (expected))
    ok = 1;
  else if (issignaling (computed) || issignaling (expected))
    ok = 0;
  else if (isnan (computed) && isnan (expected))
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
  /* Don't calculate ULPs for infinities or any kind of NaNs.  */
  else if (isinf (computed) || isnan (computed)
	   || isinf (expected) || isnan (expected))
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
	  /* 1ulp for a subnormal value, shifted by MANT_DIG, is the
	     least normal value.  */
	  ulp = (FUNC(ldexp) (diff, MANT_DIG) / min_value);
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
  if (print_screen (ok))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         ");
      print_float (computed);
      printf (" should be:  ");
      print_float (expected);
      if (print_diff)
	{
	  printf (" difference: % .20" PRINTF_EXPR "  % .20" PRINTF_XEXPR
		  "\n", diff, diff);
	  printf (" ulp       : % .4" PRINTF_NEXPR "\n", ulp);
	  printf (" max.ulp   : % .4" PRINTF_NEXPR "\n", max_ulp);
	}
    }
  update_stats (ok);

  fpstack_test (test_name);
  errno = 0;
}


static void
check_float (const char *test_name, FLOAT computed, FLOAT expected,
	     FLOAT max_ulp, int exceptions)
{
  check_float_internal (test_name, computed, expected, max_ulp,
			exceptions, &max_error);
}


static void
check_complex (const char *test_name, __complex__ FLOAT computed,
	       __complex__ FLOAT expected,
	       __complex__ FLOAT max_ulp,
	       int exception)
{
  FLOAT part_comp, part_exp, part_max_ulp;
  char *str;

  if (asprintf (&str, "Real part of: %s", test_name) == -1)
    abort ();

  part_comp = __real__ computed;
  part_exp = __real__ expected;
  part_max_ulp = __real__ max_ulp;

  check_float_internal (str, part_comp, part_exp, part_max_ulp,
			exception, &real_max_error);
  free (str);

  if (asprintf (&str, "Imaginary part of: %s", test_name) == -1)
    abort ();

  part_comp = __imag__ computed;
  part_exp = __imag__ expected;
  part_max_ulp = __imag__ max_ulp;

  /* Don't check again for exceptions or errno, just pass through the
     zero/inf sign test.  */
  check_float_internal (str, part_comp, part_exp, part_max_ulp,
			exception & IGNORE_ZERO_INF_SIGN,
			&imag_max_error);
  free (str);
}


/* Check that computed and expected values are equal (int values).  */
static void
check_int (const char *test_name, int computed, int expected, int max_ulp,
	   int exceptions)
{
  int diff = computed - expected;
  int ok = 0;
  int errno_value = errno;

  test_exceptions (test_name, exceptions);
  test_errno (test_name, errno_value, exceptions);
  noTests++;
  if (abs (diff) <= max_ulp)
    ok = 1;

  if (!ok)
    print_ulps (test_name, diff);

  if (print_screen (ok))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %d\n", computed);
      printf (" should be:  %d\n", expected);
    }

  update_stats (ok);
  fpstack_test (test_name);
  errno = 0;
}


/* Check that computed and expected values are equal (long int values).  */
static void
check_long (const char *test_name, long int computed, long int expected,
	    long int max_ulp, int exceptions)
{
  long int diff = computed - expected;
  int ok = 0;
  int errno_value = errno;

  test_exceptions (test_name, exceptions);
  test_errno (test_name, errno_value, exceptions);
  noTests++;
  if (labs (diff) <= max_ulp)
    ok = 1;

  if (!ok)
    print_ulps (test_name, diff);

  if (print_screen (ok))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %ld\n", computed);
      printf (" should be:  %ld\n", expected);
    }

  update_stats (ok);
  fpstack_test (test_name);
  errno = 0;
}


/* Check that computed value is true/false.  */
static void
check_bool (const char *test_name, int computed, int expected,
	    long int max_ulp, int exceptions)
{
  int ok = 0;
  int errno_value = errno;

  test_exceptions (test_name, exceptions);
  test_errno (test_name, errno_value, exceptions);
  noTests++;
  if ((computed == 0) == (expected == 0))
    ok = 1;

  if (print_screen (ok))
    {
      if (!ok)
	printf ("Failure: ");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %d\n", computed);
      printf (" should be:  %d\n", expected);
    }

  update_stats (ok);
  fpstack_test (test_name);
  errno = 0;
}


/* check that computed and expected values are equal (long int values) */
static void
check_longlong (const char *test_name, long long int computed,
		long long int expected,
		long long int max_ulp,
		int exceptions)
{
  long long int diff = computed - expected;
  int ok = 0;
  int errno_value = errno;

  test_exceptions (test_name, exceptions);
  test_errno (test_name, errno_value, exceptions);
  noTests++;
  if (llabs (diff) <= max_ulp)
    ok = 1;

  if (!ok)
    print_ulps (test_name, diff);

  if (print_screen (ok))
    {
      if (!ok)
	printf ("Failure:");
      printf ("Test: %s\n", test_name);
      printf ("Result:\n");
      printf (" is:         %lld\n", computed);
      printf (" should be:  %lld\n", expected);
    }

  update_stats (ok);
  fpstack_test (test_name);
  errno = 0;
}

/* Structures for each kind of test.  */
struct test_f_f_data
{
  const char *test_name;
  FLOAT arg;
  FLOAT expected;
  FLOAT max_ulp;
  int exceptions;
};
struct test_ff_f_data
{
  const char *test_name;
  FLOAT arg1, arg2;
  FLOAT expected;
  FLOAT max_ulp;
  int exceptions;
};
struct test_ff_f_data_nexttoward
{
  const char *test_name;
  FLOAT arg1;
  long double arg2;
  FLOAT expected;
  FLOAT max_ulp;
  int exceptions;
};
struct test_fi_f_data
{
  const char *test_name;
  FLOAT arg1;
  int arg2;
  FLOAT expected;
  FLOAT max_ulp;
  int exceptions;
};
struct test_fl_f_data
{
  const char *test_name;
  FLOAT arg1;
  long int arg2;
  FLOAT expected;
  FLOAT max_ulp;
  int exceptions;
};
struct test_if_f_data
{
  const char *test_name;
  int arg1;
  FLOAT arg2;
  FLOAT expected;
  FLOAT max_ulp;
  int exceptions;
};
struct test_fff_f_data
{
  const char *test_name;
  FLOAT arg1, arg2, arg3;
  FLOAT expected;
  FLOAT max_ulp;
  int exceptions;
};
struct test_c_f_data
{
  const char *test_name;
  FLOAT argr, argc;
  FLOAT expected;
  FLOAT max_ulp;
  int exceptions;
};
/* Used for both RUN_TEST_LOOP_f_f1 and RUN_TEST_LOOP_fI_f1.  */
struct test_f_f1_data
{
  const char *test_name;
  FLOAT arg;
  FLOAT expected;
  FLOAT max_ulp;
  int exceptions;
  const char *extra_name;
  int extra_init;
  int extra_test;
  int extra_expected;
  int extra_ulp;
};
struct test_fF_f1_data
{
  const char *test_name;
  FLOAT arg;
  FLOAT expected;
  FLOAT max_ulp;
  int exceptions;
  const char *extra_name;
  FLOAT extra_init;
  FLOAT extra_test;
  FLOAT extra_expected;
  FLOAT extra_ulp;
};
struct test_c_c_data
{
  const char *test_name;
  FLOAT argr, argc;
  FLOAT expr, expc;
  __complex__ FLOAT max_ulp;
  int exceptions;
};
struct test_f_i_data
{
  const char *test_name;
  FLOAT arg;
  int expected;
  FLOAT max_ulp;
  int exceptions;
};

/* Set the rounding mode, or restore the saved value.  */
#define IF_ROUND_INIT_	/* Empty.  */
#define IF_ROUND_INIT_FE_DOWNWARD		\
  int save_round_mode = fegetround ();		\
  if (fesetround (FE_DOWNWARD) == 0)
#define IF_ROUND_INIT_FE_TONEAREST		\
  int save_round_mode = fegetround ();		\
  if (fesetround (FE_TONEAREST) == 0)
#define IF_ROUND_INIT_FE_TOWARDZERO		\
  int save_round_mode = fegetround ();		\
  if (fesetround (FE_TOWARDZERO) == 0)
#define IF_ROUND_INIT_FE_UPWARD			\
  int save_round_mode = fegetround ();		\
  if (fesetround (FE_UPWARD) == 0)
#define ROUND_RESTORE_	/* Empty.  */
#define ROUND_RESTORE_FE_DOWNWARD		\
  fesetround (save_round_mode)
#define ROUND_RESTORE_FE_TONEAREST		\
  fesetround (save_round_mode)
#define ROUND_RESTORE_FE_TOWARDZERO		\
  fesetround (save_round_mode)
#define ROUND_RESTORE_FE_UPWARD			\
  fesetround (save_round_mode)

/* Run an individual test, including any required setup and checking
   of results, or loop over all tests in an array.  */
#define RUN_TEST_f_f(TEST_NAME, FUNC_NAME, ARG, EXPECTED,	\
		     MAX_ULP, EXCEPTIONS)			\
  check_float (TEST_NAME, FUNC (FUNC_NAME) (ARG), EXPECTED,	\
	       MAX_ULP, EXCEPTIONS)
#define RUN_TEST_LOOP_f_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_f ((ARRAY)[i].test_name, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].expected, (ARRAY)[i].max_ulp,		\
		    (ARRAY)[i].exceptions);				\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_2_f(TEST_NAME, FUNC_NAME, ARG1, ARG2, EXPECTED,	\
		     MAX_ULP, EXCEPTIONS)				\
  check_float (TEST_NAME, FUNC (FUNC_NAME) (ARG1, ARG2), EXPECTED,	\
	       MAX_ULP, EXCEPTIONS)
#define RUN_TEST_LOOP_2_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_2_f ((ARRAY)[i].test_name, FUNC_NAME, (ARRAY)[i].arg1,	\
		    (ARRAY)[i].arg2, (ARRAY)[i].expected,		\
		    (ARRAY)[i].max_ulp,	(ARRAY)[i].exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_ff_f RUN_TEST_2_f
#define RUN_TEST_LOOP_ff_f RUN_TEST_LOOP_2_f
#define RUN_TEST_fi_f RUN_TEST_2_f
#define RUN_TEST_LOOP_fi_f RUN_TEST_LOOP_2_f
#define RUN_TEST_fl_f RUN_TEST_2_f
#define RUN_TEST_LOOP_fl_f RUN_TEST_LOOP_2_f
#define RUN_TEST_if_f RUN_TEST_2_f
#define RUN_TEST_LOOP_if_f RUN_TEST_LOOP_2_f
#define RUN_TEST_fff_f(TEST_NAME, FUNC_NAME, ARG1, ARG2, ARG3,	\
		       EXPECTED, MAX_ULP, EXCEPTIONS)		\
  check_float (TEST_NAME, FUNC (FUNC_NAME) (ARG1, ARG2, ARG3),	\
	       EXPECTED, MAX_ULP, EXCEPTIONS)
#define RUN_TEST_LOOP_fff_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_fff_f ((ARRAY)[i].test_name, FUNC_NAME, (ARRAY)[i].arg1,	\
		      (ARRAY)[i].arg2, (ARRAY)[i].arg3,			\
		      (ARRAY)[i].expected, (ARRAY)[i].max_ulp,		\
		      (ARRAY)[i].exceptions);				\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_c_f(TEST_NAME, FUNC_NAME, ARG1, ARG2, EXPECTED,	\
		     MAX_ULP, EXCEPTIONS)				\
  check_float (TEST_NAME,						\
	       FUNC (FUNC_NAME) (BUILD_COMPLEX (ARG1, ARG2)),		\
	       EXPECTED, MAX_ULP, EXCEPTIONS)
#define RUN_TEST_LOOP_c_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_c_f ((ARRAY)[i].test_name, FUNC_NAME, (ARRAY)[i].argr,	\
		    (ARRAY)[i].argc, (ARRAY)[i].expected,		\
		    (ARRAY)[i].max_ulp, (ARRAY)[i].exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_f1(TEST_NAME, FUNC_NAME, ARG, EXPECTED,		\
		      MAX_ULP, EXCEPTIONS,				\
		      EXTRA_NAME, EXTRA_VAR, EXTRA_INIT, EXTRA_TEST,	\
		      EXTRA_EXPECTED, EXTRA_ULP)			\
  do									\
    {									\
      (EXTRA_VAR) = (EXTRA_INIT);					\
      check_float (TEST_NAME, FUNC (FUNC_NAME) (ARG), EXPECTED,		\
		   MAX_ULP, EXCEPTIONS);				\
      if (EXTRA_TEST)							\
	check_int (EXTRA_NAME, EXTRA_VAR, EXTRA_EXPECTED,		\
		   EXTRA_ULP, 0);					\
    }									\
  while (0)
#define RUN_TEST_LOOP_f_f1(FUNC_NAME, ARRAY, ROUNDING_MODE, EXTRA_VAR)	\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_f1 ((ARRAY)[i].test_name, FUNC_NAME, (ARRAY)[i].arg,	\
		     (ARRAY)[i].expected, (ARRAY)[i].max_ulp,		\
		     (ARRAY)[i].exceptions, (ARRAY)[i].extra_name,	\
		     EXTRA_VAR, (ARRAY)[i].extra_init,			\
		     (ARRAY)[i].extra_test, (ARRAY)[i].extra_expected,	\
		     (ARRAY)[i].extra_ulp);				\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_fF_f1(TEST_NAME, FUNC_NAME, ARG, EXPECTED,		\
		       MAX_ULP, EXCEPTIONS,				\
		       EXTRA_NAME, EXTRA_VAR, EXTRA_INIT, EXTRA_TEST,	\
		       EXTRA_EXPECTED, EXTRA_ULP)			\
  do									\
    {									\
      (EXTRA_VAR) = (EXTRA_INIT);					\
      check_float (TEST_NAME, FUNC (FUNC_NAME) (ARG, &(EXTRA_VAR)),	\
		   EXPECTED, MAX_ULP, EXCEPTIONS);			\
      if (EXTRA_TEST)							\
	check_float (EXTRA_NAME, EXTRA_VAR, EXTRA_EXPECTED,		\
		     EXTRA_ULP, 0);					\
    }									\
  while (0)
#define RUN_TEST_LOOP_fF_f1(FUNC_NAME, ARRAY, ROUNDING_MODE, EXTRA_VAR)	\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_fF_f1 ((ARRAY)[i].test_name, FUNC_NAME, (ARRAY)[i].arg,	\
		      (ARRAY)[i].expected, (ARRAY)[i].max_ulp,		\
		      (ARRAY)[i].exceptions, (ARRAY)[i].extra_name,	\
		      EXTRA_VAR, (ARRAY)[i].extra_init,			\
		      (ARRAY)[i].extra_test, (ARRAY)[i].extra_expected,	\
		      (ARRAY)[i].extra_ulp);				\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_fI_f1(TEST_NAME, FUNC_NAME, ARG, EXPECTED,		\
		       MAX_ULP, EXCEPTIONS,				\
		       EXTRA_NAME, EXTRA_VAR, EXTRA_INIT, EXTRA_TEST,	\
		       EXTRA_EXPECTED, EXTRA_ULP)			\
  do									\
    {									\
      (EXTRA_VAR) = (EXTRA_INIT);					\
      check_float (TEST_NAME, FUNC (FUNC_NAME) (ARG, &(EXTRA_VAR)),	\
		   EXPECTED, MAX_ULP, EXCEPTIONS);			\
      if (EXTRA_TEST)							\
	check_int (EXTRA_NAME, EXTRA_VAR, EXTRA_EXPECTED,		\
		   EXTRA_ULP, 0);					\
    }									\
  while (0)
#define RUN_TEST_LOOP_fI_f1(FUNC_NAME, ARRAY, ROUNDING_MODE, EXTRA_VAR)	\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_fI_f1 ((ARRAY)[i].test_name, FUNC_NAME, (ARRAY)[i].arg,	\
		      (ARRAY)[i].expected, (ARRAY)[i].max_ulp,		\
		      (ARRAY)[i].exceptions, (ARRAY)[i].extra_name,	\
		      EXTRA_VAR, (ARRAY)[i].extra_init,			\
		      (ARRAY)[i].extra_test, (ARRAY)[i].extra_expected,	\
		      (ARRAY)[i].extra_ulp);				\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_ffI_f1(TEST_NAME, FUNC_NAME, ARG1, ARG2, EXPECTED,	\
			MAX_ULP, EXCEPTIONS,				\
			EXTRA_NAME, EXTRA_VAR, EXTRA_INIT, EXTRA_TEST,	\
			EXTRA_EXPECTED, EXTRA_ULP)			\
  do									\
    {									\
      (EXTRA_VAR) = (EXTRA_INIT);					\
      check_float (TEST_NAME,						\
		   FUNC (FUNC_NAME) (ARG1, ARG2, &(EXTRA_VAR)),		\
		   EXPECTED, MAX_ULP, EXCEPTIONS);			\
      if (EXTRA_TEST)							\
	check_int (EXTRA_NAME, EXTRA_VAR, EXTRA_EXPECTED,		\
		   EXTRA_ULP, 0);					\
    }									\
  while (0)
#define RUN_TEST_c_c(TEST_NAME, FUNC_NAME, ARGR, ARGC, EXPR, EXPC,	\
		     MAX_ULP, EXCEPTIONS)				\
  check_complex (TEST_NAME,						\
		 FUNC (FUNC_NAME) (BUILD_COMPLEX (ARGR, ARGC)),		\
		 BUILD_COMPLEX (EXPR, EXPC),				\
		 MAX_ULP, EXCEPTIONS)
#define RUN_TEST_LOOP_c_c(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_c_c ((ARRAY)[i].test_name, FUNC_NAME, (ARRAY)[i].argr,	\
		    (ARRAY)[i].argc, (ARRAY)[i].expr, (ARRAY)[i].expc,	\
		    (ARRAY)[i].max_ulp, (ARRAY)[i].exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_cc_c(TEST_NAME, FUNC_NAME, ARG1R, ARG1C, ARG2R, ARG2C,	\
		      EXPR, EXPC, MAX_ULP, EXCEPTIONS)			\
  check_complex (TEST_NAME,						\
		 FUNC (FUNC_NAME) (BUILD_COMPLEX (ARG1R, ARG1C),	\
				   BUILD_COMPLEX (ARG2R, ARG2C)),	\
		 BUILD_COMPLEX (EXPR, EXPC),				\
		 MAX_ULP, EXCEPTIONS)
#define RUN_TEST_f_i(TEST_NAME, FUNC_NAME, ARG, EXPECTED,	\
		     MAX_ULP, EXCEPTIONS)			\
  check_int (TEST_NAME, FUNC (FUNC_NAME) (ARG), EXPECTED,	\
	     MAX_ULP, EXCEPTIONS)
#define RUN_TEST_LOOP_f_i(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_i ((ARRAY)[i].test_name, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].expected, (ARRAY)[i].max_ulp,		\
		    (ARRAY)[i].exceptions);				\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_i_tg(TEST_NAME, FUNC_NAME, ARG, EXPECTED,	\
			MAX_ULP, EXCEPTIONS)			\
  check_int (TEST_NAME, FUNC_NAME (ARG), EXPECTED,		\
	     MAX_ULP, EXCEPTIONS)
#define RUN_TEST_LOOP_f_i_tg(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_i_tg ((ARRAY)[i].test_name, FUNC_NAME, (ARRAY)[i].arg,	\
		       (ARRAY)[i].expected, (ARRAY)[i].max_ulp,		\
		       (ARRAY)[i].exceptions);				\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_ff_i_tg(TEST_NAME, FUNC_NAME, ARG1, ARG2, EXPECTED,	\
			 MAX_ULP, EXCEPTIONS)				\
  check_int (TEST_NAME, FUNC_NAME (ARG1, ARG2), EXPECTED,		\
	     MAX_ULP, EXCEPTIONS)
#define RUN_TEST_f_b(TEST_NAME, FUNC_NAME, ARG, EXPECTED,	\
		     MAX_ULP, EXCEPTIONS)			\
  check_bool (TEST_NAME, FUNC (FUNC_NAME) (ARG), EXPECTED,	\
	      MAX_ULP, EXCEPTIONS)
#define RUN_TEST_f_b_tg(TEST_NAME, FUNC_NAME, ARG, EXPECTED,	\
			MAX_ULP, EXCEPTIONS)			\
  check_bool (TEST_NAME, FUNC_NAME (ARG), EXPECTED,		\
	      MAX_ULP, EXCEPTIONS)
#define RUN_TEST_f_l(TEST_NAME, FUNC_NAME, ARG, EXPECTED,	\
		     MAX_ULP, EXCEPTIONS)			\
  check_long (TEST_NAME, FUNC (FUNC_NAME) (ARG), EXPECTED,	\
	      MAX_ULP, EXCEPTIONS)
#define RUN_TEST_f_L(TEST_NAME, FUNC_NAME, ARG, EXPECTED,	\
		     MAX_ULP, EXCEPTIONS)			\
  check_longlong (TEST_NAME, FUNC (FUNC_NAME) (ARG), EXPECTED,	\
		  MAX_ULP, EXCEPTIONS)
#define RUN_TEST_sincos(ARG, TEST_NAME_SIN, SIN_RES_VAR, EXPECTED_SIN,	\
			MAX_ULP_SIN, TEST_NAME_COS, COS_RES_VAR,	\
			EXPECTED_COS, MAX_ULP_COS, EXCEPTIONS)		\
  do									\
    {									\
      FUNC (sincos) (ARG, &(SIN_RES_VAR), &(COS_RES_VAR));		\
      check_float (TEST_NAME_SIN, SIN_RES_VAR,				\
		   EXPECTED_SIN, MAX_ULP_SIN, EXCEPTIONS);		\
      check_float (TEST_NAME_COS, COS_RES_VAR,				\
		   EXPECTED_COS, MAX_ULP_COS, 0);			\
    }									\
  while (0)



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

static const struct test_f_f_data acos_test_data[] =
  {
    START_DATA (acos),
    TEST_f_f (acos, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (acos, minus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (acos, qnan_value, qnan_value),

    /* |x| > 1: */
    TEST_f_f (acos, 1.125L, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (acos, -1.125L, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (acos, max_value, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (acos, -max_value, qnan_value, INVALID_EXCEPTION),

    TEST_f_f (acos, 0, M_PI_2l),
    TEST_f_f (acos, minus_zero, M_PI_2l),
    TEST_f_f (acos, 1, 0),
    TEST_f_f (acos, -1, M_PIl),
    TEST_f_f (acos, 0.5, M_PI_6l*2.0),
    TEST_f_f (acos, -0.5, M_PI_6l*4.0),
    TEST_f_f (acos, 0.75L, 0.722734247813415611178377352641333362L),
    TEST_f_f (acos, 2e-17L, 1.57079632679489659923132169163975144L),
    TEST_f_f (acos, 0.0625L, 1.50825556499840522843072005474337068L),
    TEST_f_f (acos, 0x0.ffffffp0L, 3.4526698471620358760324948263873649728491e-4L),
    TEST_f_f (acos, -0x0.ffffffp0L, 3.1412473866050770348750401337968641476999L),
#ifndef TEST_FLOAT
    TEST_f_f (acos, 0x0.ffffffff8p0L, 1.5258789062648029736620564947844627548516e-5L),
    TEST_f_f (acos, -0x0.ffffffff8p0L, 3.1415773948007305904329067627145550395696L),
    TEST_f_f (acos, 0x0.ffffffffffffp0L, 8.4293697021788088529885473244391795127130e-8L),
    TEST_f_f (acos, -0x0.ffffffffffffp0L, 3.1415925692960962166745548533940296398054L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_f_f (acos, 0x0.ffffffffffffffffp0L, 3.2927225399135962333718255320079907245059e-10L),
    TEST_f_f (acos, -0x0.ffffffffffffffffp0L, 3.1415926532605209844712837599423203309964L),
#endif
    END_DATA (acos)
  };

static void
acos_test (void)
{
  errno = 0;
  FUNC(acos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acos);
  RUN_TEST_LOOP_f_f (acos, acos_test_data, );
  END (acos);
}


static const struct test_f_f_data acos_tonearest_test_data[] =
  {
    START_DATA (acos_tonearest),
    TEST_f_f (acos, 0, M_PI_2l),
    TEST_f_f (acos, minus_zero, M_PI_2l),
    TEST_f_f (acos, 1, 0),
    TEST_f_f (acos, -1, M_PIl),
    TEST_f_f (acos, 0.5, M_PI_6l*2.0),
    TEST_f_f (acos, -0.5, M_PI_6l*4.0),
    END_DATA (acos_tonearest)
  };

static void
acos_test_tonearest (void)
{
  errno = 0;
  FUNC(acos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acos_tonearest);
  RUN_TEST_LOOP_f_f (acos, acos_tonearest_test_data, FE_TONEAREST);
  END (acos_tonearest);
}


static const struct test_f_f_data acos_towardzero_test_data[] =
  {
    START_DATA (acos_towardzero),
    TEST_f_f (acos, 0, M_PI_2l),
    TEST_f_f (acos, minus_zero, M_PI_2l),
    TEST_f_f (acos, 1, 0),
    TEST_f_f (acos, -1, M_PIl),
    TEST_f_f (acos, 0.5, M_PI_6l*2.0),
    TEST_f_f (acos, -0.5, M_PI_6l*4.0),
    END_DATA (acos_towardzero)
  };

static void
acos_test_towardzero (void)
{
  errno = 0;
  FUNC(acos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acos_towardzero);
  RUN_TEST_LOOP_f_f (acos, acos_towardzero_test_data, FE_TOWARDZERO);
  END (acos_towardzero);
}


static const struct test_f_f_data acos_downward_test_data[] =
  {
    START_DATA (acos_downward),
    TEST_f_f (acos, 0, M_PI_2l),
    TEST_f_f (acos, minus_zero, M_PI_2l),
    TEST_f_f (acos, 1, 0),
    TEST_f_f (acos, -1, M_PIl),
    TEST_f_f (acos, 0.5, M_PI_6l*2.0),
    TEST_f_f (acos, -0.5, M_PI_6l*4.0),
    END_DATA (acos_downward)
  };

static void
acos_test_downward (void)
{
  errno = 0;
  FUNC(acos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acos_downward);
  RUN_TEST_LOOP_f_f (acos, acos_downward_test_data, FE_DOWNWARD);
  END (acos_downward);
}


static const struct test_f_f_data acos_upward_test_data[] =
  {
    START_DATA (acos_upward),
    TEST_f_f (acos, 0, M_PI_2l),
    TEST_f_f (acos, minus_zero, M_PI_2l),
    TEST_f_f (acos, 1, 0),
    TEST_f_f (acos, -1, M_PIl),
    TEST_f_f (acos, 0.5, M_PI_6l*2.0),
    TEST_f_f (acos, -0.5, M_PI_6l*4.0),
    END_DATA (acos_upward)
  };

static void
acos_test_upward (void)
{
  errno = 0;
  FUNC(acos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acos_upward);
  RUN_TEST_LOOP_f_f (acos, acos_upward_test_data, FE_UPWARD);
  END (acos_upward);
}

static const struct test_f_f_data acosh_test_data[] =
  {
    START_DATA (acosh),
    TEST_f_f (acosh, plus_infty, plus_infty),
    TEST_f_f (acosh, minus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (acosh, qnan_value, qnan_value),

    /* x < 1:  */
    TEST_f_f (acosh, -1.125L, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (acosh, -max_value, qnan_value, INVALID_EXCEPTION),

    TEST_f_f (acosh, 1, 0),
    TEST_f_f (acosh, 7, 2.63391579384963341725009269461593689L),
    END_DATA (acosh)
  };

static void
acosh_test (void)
{
  errno = 0;
  FUNC(acosh) (7);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (acosh);
  RUN_TEST_LOOP_f_f (acosh, acosh_test_data, );
  END (acosh);
}

static const struct test_f_f_data asin_test_data[] =
  {
    START_DATA (asin),
    TEST_f_f (asin, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (asin, minus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (asin, qnan_value, qnan_value),

    /* asin x == qNaN plus invalid exception for |x| > 1.  */
    TEST_f_f (asin, 1.125L, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (asin, -1.125L, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (asin, max_value, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (asin, -max_value, qnan_value, INVALID_EXCEPTION),

    TEST_f_f (asin, 0, 0),
    TEST_f_f (asin, minus_zero, minus_zero),
    TEST_f_f (asin, 0.5, M_PI_6l),
    TEST_f_f (asin, -0.5, -M_PI_6l),
    TEST_f_f (asin, 1.0, M_PI_2l),
    TEST_f_f (asin, -1.0, -M_PI_2l),
    TEST_f_f (asin, 0.75L, 0.848062078981481008052944338998418080L),
    TEST_f_f (asin, 0x0.ffffffp0L, 1.5704510598101804156437184421571127056013L),
    TEST_f_f (asin, -0x0.ffffffp0L, -1.5704510598101804156437184421571127056013L),
#ifndef TEST_FLOAT
    TEST_f_f (asin, 0x0.ffffffff8p0L, 1.5707810680058339712015850710748035974710L),
    TEST_f_f (asin, -0x0.ffffffff8p0L, -1.5707810680058339712015850710748035974710L),
    TEST_f_f (asin, 0x0.ffffffffffffp0L, 1.5707962425011995974432331617542781977068L),
    TEST_f_f (asin, -0x0.ffffffffffffp0L, -1.5707962425011995974432331617542781977068L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_f_f (asin, 0x0.ffffffffffffffffp0L, 1.5707963264656243652399620683025688888978L),
    TEST_f_f (asin, -0x0.ffffffffffffffffp0L, -1.5707963264656243652399620683025688888978L),
#endif
    END_DATA (asin)
  };

static void
asin_test (void)
{
  errno = 0;
  FUNC(asin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asin);
  RUN_TEST_LOOP_f_f (asin, asin_test_data, );
  END (asin);
}


static const struct test_f_f_data asin_tonearest_test_data[] =
  {
    START_DATA (asin_tonearest),
    TEST_f_f (asin, 0, 0),
    TEST_f_f (asin, minus_zero, minus_zero),
    TEST_f_f (asin, 0.5, M_PI_6l),
    TEST_f_f (asin, -0.5, -M_PI_6l),
    TEST_f_f (asin, 1.0, M_PI_2l),
    TEST_f_f (asin, -1.0, -M_PI_2l),
    END_DATA (asin_tonearest)
  };

static void
asin_test_tonearest (void)
{
  errno = 0;
  FUNC(asin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asin_tonearest);
  RUN_TEST_LOOP_f_f (asin, asin_tonearest_test_data, FE_TONEAREST);
  END (asin_tonearest);
}


static const struct test_f_f_data asin_towardzero_test_data[] =
  {
    START_DATA (asin_towardzero),
    TEST_f_f (asin, 0, 0),
    TEST_f_f (asin, minus_zero, minus_zero),
    TEST_f_f (asin, 0.5, M_PI_6l),
    TEST_f_f (asin, -0.5, -M_PI_6l),
    TEST_f_f (asin, 1.0, M_PI_2l),
    TEST_f_f (asin, -1.0, -M_PI_2l),
    END_DATA (asin_towardzero)
  };

static void
asin_test_towardzero (void)
{
  errno = 0;
  FUNC(asin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asin_towardzero);
  RUN_TEST_LOOP_f_f (asin, asin_towardzero_test_data, FE_TOWARDZERO);
  END (asin_towardzero);
}


static const struct test_f_f_data asin_downward_test_data[] =
  {
    START_DATA (asin_downward),
    TEST_f_f (asin, 0, 0),
    TEST_f_f (asin, minus_zero, minus_zero),
    TEST_f_f (asin, 0.5, M_PI_6l),
    TEST_f_f (asin, -0.5, -M_PI_6l),
    TEST_f_f (asin, 1.0, M_PI_2l),
    TEST_f_f (asin, -1.0, -M_PI_2l),
    END_DATA (asin_downward)
  };

static void
asin_test_downward (void)
{
  errno = 0;
  FUNC(asin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asin_downward);
  RUN_TEST_LOOP_f_f (asin, asin_downward_test_data, FE_DOWNWARD);
  END (asin_downward);
}


static const struct test_f_f_data asin_upward_test_data[] =
  {
    START_DATA (asin_upward),
    TEST_f_f (asin, 0, 0),
    TEST_f_f (asin, minus_zero, minus_zero),
    TEST_f_f (asin, 0.5, M_PI_6l),
    TEST_f_f (asin, -0.5, -M_PI_6l),
    TEST_f_f (asin, 1.0, M_PI_2l),
    TEST_f_f (asin, -1.0, -M_PI_2l),
    END_DATA (asin_upward)
  };

static void
asin_test_upward (void)
{
  errno = 0;
  FUNC(asin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asin_upward);
  RUN_TEST_LOOP_f_f (asin, asin_upward_test_data, FE_UPWARD);
  END (asin_upward);
}

static const struct test_f_f_data asinh_test_data[] =
  {
    START_DATA (asinh),
    TEST_f_f (asinh, 0, 0),
    TEST_f_f (asinh, minus_zero, minus_zero),
#ifndef TEST_INLINE
    TEST_f_f (asinh, plus_infty, plus_infty),
    TEST_f_f (asinh, minus_infty, minus_infty),
#endif
    TEST_f_f (asinh, qnan_value, qnan_value),
    TEST_f_f (asinh, 0.75L, 0.693147180559945309417232121458176568L),
    END_DATA (asinh)
  };

static void
asinh_test (void)
{
  errno = 0;
  FUNC(asinh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (asinh);
  RUN_TEST_LOOP_f_f (asinh, asinh_test_data, );
  END (asinh);
}

static const struct test_f_f_data atan_test_data[] =
  {
    START_DATA (atan),
    TEST_f_f (atan, 0, 0),
    TEST_f_f (atan, minus_zero, minus_zero),

    TEST_f_f (atan, plus_infty, M_PI_2l),
    TEST_f_f (atan, minus_infty, -M_PI_2l),
    TEST_f_f (atan, qnan_value, qnan_value),
    TEST_f_f (atan, max_value, M_PI_2l),
    TEST_f_f (atan, -max_value, -M_PI_2l),

    TEST_f_f (atan, 1, M_PI_4l),
    TEST_f_f (atan, -1, -M_PI_4l),

    TEST_f_f (atan, 0.75L, 0.643501108793284386802809228717322638L),

    TEST_f_f (atan, 0x1p-100L, 0x1p-100L),
#ifndef TEST_FLOAT
    TEST_f_f (atan, 0x1p-600L, 0x1p-600L),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_f_f (atan, 0x1p-10000L, 0x1p-10000L),
#endif
    END_DATA (atan)
  };

static void
atan_test (void)
{
  errno = 0;
  FUNC(atan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (atan);
  RUN_TEST_LOOP_f_f (atan, atan_test_data, );
  END (atan);
}



static const struct test_f_f_data atanh_test_data[] =
  {
    START_DATA (atanh),
    TEST_f_f (atanh, 0, 0),
    TEST_f_f (atanh, minus_zero, minus_zero),

    TEST_f_f (atanh, 1, plus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f (atanh, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f (atanh, qnan_value, qnan_value),

    /* atanh (x) == qNaN plus invalid exception if |x| > 1.  */
    TEST_f_f (atanh, 1.125L, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (atanh, -1.125L, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (atanh, max_value, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (atanh, -max_value, qnan_value, INVALID_EXCEPTION),

    TEST_f_f (atanh, 0.75L, 0.972955074527656652552676371721589865L),
    END_DATA (atanh)
  };

static void
atanh_test (void)
{
  errno = 0;
  FUNC(atanh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (atanh);
  RUN_TEST_LOOP_f_f (atanh, atanh_test_data, );
  END (atanh);
}

static const struct test_ff_f_data atan2_test_data[] =
  {
    START_DATA (atan2),
    /* atan2 (0,x) == 0 for x > 0.  */
    TEST_ff_f (atan2, 0, 1, 0),

    /* atan2 (-0,x) == -0 for x > 0.  */
    TEST_ff_f (atan2, minus_zero, 1, minus_zero),

    TEST_ff_f (atan2, 0, 0, 0),
    TEST_ff_f (atan2, minus_zero, 0, minus_zero),

    /* atan2 (+0,x) == +pi for x < 0.  */
    TEST_ff_f (atan2, 0, -1, M_PIl),

    /* atan2 (-0,x) == -pi for x < 0.  */
    TEST_ff_f (atan2, minus_zero, -1, -M_PIl),

    TEST_ff_f (atan2, 0, minus_zero, M_PIl),
    TEST_ff_f (atan2, minus_zero, minus_zero, -M_PIl),

    /* atan2 (y,+0) == pi/2 for y > 0.  */
    TEST_ff_f (atan2, 1, 0, M_PI_2l),

    /* atan2 (y,-0) == pi/2 for y > 0.  */
    TEST_ff_f (atan2, 1, minus_zero, M_PI_2l),

    /* atan2 (y,+0) == -pi/2 for y < 0.  */
    TEST_ff_f (atan2, -1, 0, -M_PI_2l),

    /* atan2 (y,-0) == -pi/2 for y < 0.  */
    TEST_ff_f (atan2, -1, minus_zero, -M_PI_2l),

    /* atan2 (y,inf) == +0 for finite y > 0.  */
    TEST_ff_f (atan2, 1, plus_infty, 0),

    /* atan2 (y,inf) == -0 for finite y < 0.  */
    TEST_ff_f (atan2, -1, plus_infty, minus_zero),

    /* atan2(+inf, x) == pi/2 for finite x.  */
    TEST_ff_f (atan2, plus_infty, -1, M_PI_2l),

    /* atan2(-inf, x) == -pi/2 for finite x.  */
    TEST_ff_f (atan2, minus_infty, 1, -M_PI_2l),

    /* atan2 (y,-inf) == +pi for finite y > 0.  */
    TEST_ff_f (atan2, 1, minus_infty, M_PIl),

    /* atan2 (y,-inf) == -pi for finite y < 0.  */
    TEST_ff_f (atan2, -1, minus_infty, -M_PIl),

    TEST_ff_f (atan2, plus_infty, plus_infty, M_PI_4l),
    TEST_ff_f (atan2, minus_infty, plus_infty, -M_PI_4l),
    TEST_ff_f (atan2, plus_infty, minus_infty, M_PI_34l),
    TEST_ff_f (atan2, minus_infty, minus_infty, -M_PI_34l),
    TEST_ff_f (atan2, qnan_value, qnan_value, qnan_value),

    TEST_ff_f (atan2, max_value, max_value, M_PI_4l),

    TEST_ff_f (atan2, max_value, min_value, M_PI_2l),
    TEST_ff_f (atan2, -max_value, -min_value, -M_PI_2l),

    TEST_ff_f (atan2, 0.75L, 1, 0.643501108793284386802809228717322638L),
    TEST_ff_f (atan2, -0.75L, 1.0L, -0.643501108793284386802809228717322638L),
    TEST_ff_f (atan2, 0.75L, -1.0L, 2.49809154479650885165983415456218025L),
    TEST_ff_f (atan2, -0.75L, -1.0L, -2.49809154479650885165983415456218025L),
    TEST_ff_f (atan2, 0.390625L, .00029L, 1.57005392693128974780151246612928941L),
    TEST_ff_f (atan2, 1.390625L, 0.9296875L, 0.981498387184244311516296577615519772L),

    TEST_ff_f (atan2, -0.00756827042671106339L, -.001792735857538728036L, -1.80338464113663849327153994379639112L),
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_ff_f (atan2, 0x1.00000000000001p0L, 0x1.00000000000001p0L, M_PI_4l),
#endif
    END_DATA (atan2)
  };

static void
atan2_test (void)
{
  errno = 0;
  FUNC(atan2) (-0, 1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (atan2);
  RUN_TEST_LOOP_ff_f (atan2, atan2_test_data, );
  END (atan2);
}

static const struct test_c_f_data cabs_test_data[] =
  {
    START_DATA (cabs);
    /* cabs (x + iy) is specified as hypot (x,y) */

    /* cabs (+inf + i x) == +inf.  */
    TEST_c_f (cabs, plus_infty, 1.0, plus_infty),
    /* cabs (-inf + i x) == +inf.  */
    TEST_c_f (cabs, minus_infty, 1.0, plus_infty),

    TEST_c_f (cabs, minus_infty, qnan_value, plus_infty),
    TEST_c_f (cabs, minus_infty, qnan_value, plus_infty),

    TEST_c_f (cabs, qnan_value, qnan_value, qnan_value),

    /* cabs (x,y) == cabs (y,x).  */
    TEST_c_f (cabs, 0.75L, 12.390625L, 12.4133028598606664302388810868156657L),
    /* cabs (x,y) == cabs (-x,y).  */
    TEST_c_f (cabs, -12.390625L, 0.75L, 12.4133028598606664302388810868156657L),
    /* cabs (x,y) == cabs (-y,x).  */
    TEST_c_f (cabs, -0.75L, 12.390625L, 12.4133028598606664302388810868156657L),
    /* cabs (x,y) == cabs (-x,-y).  */
    TEST_c_f (cabs, -12.390625L, -0.75L, 12.4133028598606664302388810868156657L),
    /* cabs (x,y) == cabs (-y,-x).  */
    TEST_c_f (cabs, -0.75L, -12.390625L, 12.4133028598606664302388810868156657L),
    /* cabs (x,0) == fabs (x).  */
    TEST_c_f (cabs, -0.75L, 0, 0.75L),
    TEST_c_f (cabs, 0.75L, 0, 0.75L),
    TEST_c_f (cabs, -1.0L, 0, 1.0L),
    TEST_c_f (cabs, 1.0L, 0, 1.0L),
    TEST_c_f (cabs, -5.7e7L, 0, 5.7e7L),
    TEST_c_f (cabs, 5.7e7L, 0, 5.7e7L),

    TEST_c_f (cabs, 0.75L, 1.25L, 1.45773797371132511771853821938639577L),
    END_DATA (cabs);
  };

static void
cabs_test (void)
{
  errno = 0;
  FUNC(cabs) (BUILD_COMPLEX (0.7L, 12.4L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cabs);
  RUN_TEST_LOOP_c_f (cabs, cabs_test_data, );
  END (cabs);
}


static const struct test_c_c_data cacos_test_data[] =
  {
    START_DATA (cacos),
    TEST_c_c (cacos, 0, 0, M_PI_2l, minus_zero),
    TEST_c_c (cacos, minus_zero, 0, M_PI_2l, minus_zero),
    TEST_c_c (cacos, minus_zero, minus_zero, M_PI_2l, 0.0),
    TEST_c_c (cacos, 0, minus_zero, M_PI_2l, 0.0),

    TEST_c_c (cacos, minus_infty, plus_infty, M_PI_34l, minus_infty),
    TEST_c_c (cacos, minus_infty, minus_infty, M_PI_34l, plus_infty),

    TEST_c_c (cacos, plus_infty, plus_infty, M_PI_4l, minus_infty),
    TEST_c_c (cacos, plus_infty, minus_infty, M_PI_4l, plus_infty),

    TEST_c_c (cacos, -10.0, plus_infty, M_PI_2l, minus_infty),
    TEST_c_c (cacos, -10.0, minus_infty, M_PI_2l, plus_infty),
    TEST_c_c (cacos, 0, plus_infty, M_PI_2l, minus_infty),
    TEST_c_c (cacos, 0, minus_infty, M_PI_2l, plus_infty),
    TEST_c_c (cacos, 0.1L, plus_infty, M_PI_2l, minus_infty),
    TEST_c_c (cacos, 0.1L, minus_infty, M_PI_2l, plus_infty),

    TEST_c_c (cacos, minus_infty, 0, M_PIl, minus_infty),
    TEST_c_c (cacos, minus_infty, minus_zero, M_PIl, plus_infty),
    TEST_c_c (cacos, minus_infty, 100, M_PIl, minus_infty),
    TEST_c_c (cacos, minus_infty, -100, M_PIl, plus_infty),

    TEST_c_c (cacos, plus_infty, 0, 0.0, minus_infty),
    TEST_c_c (cacos, plus_infty, minus_zero, 0.0, plus_infty),
    TEST_c_c (cacos, plus_infty, 0.5, 0.0, minus_infty),
    TEST_c_c (cacos, plus_infty, -0.5, 0.0, plus_infty),

    TEST_c_c (cacos, plus_infty, qnan_value, qnan_value, plus_infty, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (cacos, minus_infty, qnan_value, qnan_value, plus_infty, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (cacos, 0, qnan_value, M_PI_2l, qnan_value),
    TEST_c_c (cacos, minus_zero, qnan_value, M_PI_2l, qnan_value),

    TEST_c_c (cacos, qnan_value, plus_infty, qnan_value, minus_infty),
    TEST_c_c (cacos, qnan_value, minus_infty, qnan_value, plus_infty),

    TEST_c_c (cacos, 10.5, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (cacos, -10.5, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (cacos, qnan_value, 0.75, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (cacos, qnan_value, -0.75, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (cacos, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (cacos, plus_zero, -1.5L, M_PI_2l, 1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, minus_zero, -1.5L, M_PI_2l, 1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, plus_zero, -1.0L, M_PI_2l, 0.8813735870195430252326093249797923090282L),
    TEST_c_c (cacos, minus_zero, -1.0L, M_PI_2l, 0.8813735870195430252326093249797923090282L),
    TEST_c_c (cacos, plus_zero, -0.5L, M_PI_2l, 0.4812118250596034474977589134243684231352L),
    TEST_c_c (cacos, minus_zero, -0.5L, M_PI_2l, 0.4812118250596034474977589134243684231352L),
    TEST_c_c (cacos, plus_zero, 0.5L, M_PI_2l, -0.4812118250596034474977589134243684231352L),
    TEST_c_c (cacos, minus_zero, 0.5L, M_PI_2l, -0.4812118250596034474977589134243684231352L),
    TEST_c_c (cacos, plus_zero, 1.0L, M_PI_2l, -0.8813735870195430252326093249797923090282L),
    TEST_c_c (cacos, minus_zero, 1.0L, M_PI_2l, -0.8813735870195430252326093249797923090282L),
    TEST_c_c (cacos, plus_zero, 1.5L, M_PI_2l, -1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, minus_zero, 1.5L, M_PI_2l, -1.194763217287109304111930828519090523536L),

    TEST_c_c (cacos, -1.5L, plus_zero, M_PIl, -0.9624236501192068949955178268487368462704L),
    TEST_c_c (cacos, -1.5L, minus_zero, M_PIl, 0.9624236501192068949955178268487368462704L),
    TEST_c_c (cacos, -1.0L, plus_zero, M_PIl, minus_zero),
    TEST_c_c (cacos, -1.0L, minus_zero, M_PIl, plus_zero),
    TEST_c_c (cacos, -0.5L, plus_zero, 2.094395102393195492308428922186335256131L, minus_zero),
    TEST_c_c (cacos, -0.5L, minus_zero, 2.094395102393195492308428922186335256131L, plus_zero),
    TEST_c_c (cacos, 0.5L, plus_zero, 1.047197551196597746154214461093167628066L, minus_zero),
    TEST_c_c (cacos, 0.5L, minus_zero, 1.047197551196597746154214461093167628066L, plus_zero),
    TEST_c_c (cacos, 1.0L, plus_zero, plus_zero, minus_zero),
    TEST_c_c (cacos, 1.0L, minus_zero, plus_zero, plus_zero),
    TEST_c_c (cacos, 1.5L, plus_zero, plus_zero, -0.9624236501192068949955178268487368462704L),
    TEST_c_c (cacos, 1.5L, minus_zero, plus_zero, 0.9624236501192068949955178268487368462704L),

    TEST_c_c (cacos, 0x1p50L, 1.0L, 8.881784197001252323389053344727730248720e-16L, -3.535050620855721078027883819436720218708e1L),
    TEST_c_c (cacos, 0x1p50L, -1.0L, 8.881784197001252323389053344727730248720e-16L, 3.535050620855721078027883819436720218708e1L),
    TEST_c_c (cacos, -0x1p50L, 1.0L, 3.141592653589792350284223683154270545292L, -3.535050620855721078027883819436720218708e1L),
    TEST_c_c (cacos, -0x1p50L, -1.0L, 3.141592653589792350284223683154270545292L, 3.535050620855721078027883819436720218708e1L),
    TEST_c_c (cacos, 1.0L, 0x1p50L, 1.570796326794895731052901991514519103193L, -3.535050620855721078027883819436759661753e1L),
    TEST_c_c (cacos, -1.0L, 0x1p50L, 1.570796326794897507409741391764983781004L, -3.535050620855721078027883819436759661753e1L),
    TEST_c_c (cacos, 1.0L, -0x1p50L, 1.570796326794895731052901991514519103193L, 3.535050620855721078027883819436759661753e1L),
    TEST_c_c (cacos, -1.0L, -0x1p50L, 1.570796326794897507409741391764983781004L, 3.535050620855721078027883819436759661753e1L),
#ifndef TEST_FLOAT
    TEST_c_c (cacos, 0x1p500L, 1.0L, 3.054936363499604682051979393213617699789e-151L, -3.472667374605326000180332928505464606058e2L),
    TEST_c_c (cacos, 0x1p500L, -1.0L, 3.054936363499604682051979393213617699789e-151L, 3.472667374605326000180332928505464606058e2L),
    TEST_c_c (cacos, -0x1p500L, 1.0L, 3.141592653589793238462643383279502884197L, -3.472667374605326000180332928505464606058e2L),
    TEST_c_c (cacos, -0x1p500L, -1.0L, 3.141592653589793238462643383279502884197L, 3.472667374605326000180332928505464606058e2L),
    TEST_c_c (cacos, 1.0L, 0x1p500L, 1.570796326794896619231321691639751442099L, -3.472667374605326000180332928505464606058e2L),
    TEST_c_c (cacos, -1.0L, 0x1p500L, 1.570796326794896619231321691639751442099L, -3.472667374605326000180332928505464606058e2L),
    TEST_c_c (cacos, 1.0L, -0x1p500L, 1.570796326794896619231321691639751442099L, 3.472667374605326000180332928505464606058e2L),
    TEST_c_c (cacos, -1.0L, -0x1p500L, 1.570796326794896619231321691639751442099L, 3.472667374605326000180332928505464606058e2L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (cacos, 0x1p5000L, 1.0L, 7.079811261048172892385615158694057552948e-1506L, -3.466429049980286492395577839412341016946e3L),
    TEST_c_c (cacos, 0x1p5000L, -1.0L, 7.079811261048172892385615158694057552948e-1506L, 3.466429049980286492395577839412341016946e3L),
    TEST_c_c (cacos, -0x1p5000L, 1.0L, 3.141592653589793238462643383279502884197L, -3.466429049980286492395577839412341016946e3L),
    TEST_c_c (cacos, -0x1p5000L, -1.0L, 3.141592653589793238462643383279502884197L, 3.466429049980286492395577839412341016946e3L),
    TEST_c_c (cacos, 1.0L, 0x1p5000L, 1.570796326794896619231321691639751442099L, -3.466429049980286492395577839412341016946e3L),
    TEST_c_c (cacos, -1.0L, 0x1p5000L, 1.570796326794896619231321691639751442099L, -3.466429049980286492395577839412341016946e3L),
    TEST_c_c (cacos, 1.0L, -0x1p5000L, 1.570796326794896619231321691639751442099L, 3.466429049980286492395577839412341016946e3L),
    TEST_c_c (cacos, -1.0L, -0x1p5000L, 1.570796326794896619231321691639751442099L, 3.466429049980286492395577839412341016946e3L),
#endif

    TEST_c_c (cacos, 0x1.fp127L, 0x1.fp127L, 7.853981633974483096156608458198757210493e-1L, -8.973081118419833726837456344608533993585e1L),
#ifndef TEST_FLOAT
    TEST_c_c (cacos, 0x1.fp1023L, 0x1.fp1023L, 7.853981633974483096156608458198757210493e-1L, -7.107906849659093345062145442726115449315e2L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (cacos, 0x1.fp16383L, 0x1.fp16383L, 7.853981633974483096156608458198757210493e-1L, -1.135753137836666928715489992987020363057e4L),
#endif

    TEST_c_c (cacos, 0x1.fp-129L, 1.5L, 1.570796326794896619231321691639751442097L, -1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, 0x1.fp-129L, -1.5L, 1.570796326794896619231321691639751442097L, 1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, -0x1.fp-129L, 1.5L, 1.570796326794896619231321691639751442100L, -1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, -0x1.fp-129L, -1.5L, 1.570796326794896619231321691639751442100L, 1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, 1.5L, 0x1.fp-129L, 2.546345110742945032959687790021055102355e-39L, -9.624236501192068949955178268487368462704e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, -1.5L, 0x1.fp-129L, 3.141592653589793238462643383279502884195L, -9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (cacos, 1.5L, -0x1.fp-129L, 2.546345110742945032959687790021055102355e-39L, 9.624236501192068949955178268487368462704e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, -1.5L, -0x1.fp-129L, 3.141592653589793238462643383279502884195L, 9.624236501192068949955178268487368462704e-1L),
#ifndef TEST_FLOAT
    TEST_c_c (cacos, 0x1.fp-1025L, 1.5L, 1.570796326794896619231321691639751442099L, -1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, 0x1.fp-1025L, -1.5L, 1.570796326794896619231321691639751442099L, 1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, -0x1.fp-1025L, 1.5L, 1.570796326794896619231321691639751442099L, -1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, -0x1.fp-1025L, -1.5L, 1.570796326794896619231321691639751442099L, 1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, 1.5L, 0x1.fp-1025L, 4.819934639999230680322935210539402497827e-309L, -9.624236501192068949955178268487368462704e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, -1.5L, 0x1.fp-1025L, 3.141592653589793238462643383279502884197L, -9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (cacos, 1.5L, -0x1.fp-1025L, 4.819934639999230680322935210539402497827e-309L, 9.624236501192068949955178268487368462704e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, -1.5L, -0x1.fp-1025L, 3.141592653589793238462643383279502884197L, 9.624236501192068949955178268487368462704e-1L),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacos, 0x1.fp-16385L, 1.5L, 1.570796326794896619231321691639751442099L, -1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, 0x1.fp-16385L, -1.5L, 1.570796326794896619231321691639751442099L, 1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, -0x1.fp-16385L, 1.5L, 1.570796326794896619231321691639751442099L, -1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, -0x1.fp-16385L, -1.5L, 1.570796326794896619231321691639751442099L, 1.194763217287109304111930828519090523536L),
    TEST_c_c (cacos, 1.5L, 0x1.fp-16385L, 7.282957076134209141226696333885150260319e-4933L, -9.624236501192068949955178268487368462704e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, -1.5L, 0x1.fp-16385L, 3.141592653589793238462643383279502884197L, -9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (cacos, 1.5L, -0x1.fp-16385L, 7.282957076134209141226696333885150260319e-4933L, 9.624236501192068949955178268487368462704e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, -1.5L, -0x1.fp-16385L, 3.141592653589793238462643383279502884197L, 9.624236501192068949955178268487368462704e-1L),
#endif

    TEST_c_c (cacos, 0.5L, 1.0L, 1.221357263937683325603909865564381489366L, -9.261330313501824245501244453057873152694e-1L),
    TEST_c_c (cacos, 0.5L, -1.0L, 1.221357263937683325603909865564381489366L, 9.261330313501824245501244453057873152694e-1L),
    TEST_c_c (cacos, -0.5L, 1.0L, 1.920235389652109912858733517715121394831L, -9.261330313501824245501244453057873152694e-1L),
    TEST_c_c (cacos, -0.5L, -1.0L, 1.920235389652109912858733517715121394831L, 9.261330313501824245501244453057873152694e-1L),
    TEST_c_c (cacos, 1.0L, 0.5L, 6.748888455860063801646649673121744318756e-1L, -7.328576759736452608886724437653071523305e-1L),
    TEST_c_c (cacos, -1.0L, 0.5L, 2.466703808003786858297978415967328452322L, -7.328576759736452608886724437653071523305e-1L),
    TEST_c_c (cacos, 1.0L, -0.5L, 6.748888455860063801646649673121744318756e-1L, 7.328576759736452608886724437653071523305e-1L),
    TEST_c_c (cacos, -1.0L, -0.5L, 2.466703808003786858297978415967328452322L, 7.328576759736452608886724437653071523305e-1L),
    TEST_c_c (cacos, 0.25L, 1.0L, 1.394493894017929688812643125003661339452L, -8.924633639033482359562124741744951972772e-1L),
    TEST_c_c (cacos, 0.25L, -1.0L, 1.394493894017929688812643125003661339452L, 8.924633639033482359562124741744951972772e-1L),
    TEST_c_c (cacos, -0.25L, 1.0L, 1.747098759571863549650000258275841544745L, -8.924633639033482359562124741744951972772e-1L),
    TEST_c_c (cacos, -0.25L, -1.0L, 1.747098759571863549650000258275841544745L, 8.924633639033482359562124741744951972772e-1L),
    TEST_c_c (cacos, 1.0L, 0.25L, 4.890443302710802929202843732146540079124e-1L, -5.097911466811016354623559941115413499164e-1L),
    TEST_c_c (cacos, -1.0L, 0.25L, 2.652548323318712945542359010064848876285L, -5.097911466811016354623559941115413499164e-1L),
    TEST_c_c (cacos, 1.0L, -0.25L, 4.890443302710802929202843732146540079124e-1L, 5.097911466811016354623559941115413499164e-1L),
    TEST_c_c (cacos, -1.0L, -0.25L, 2.652548323318712945542359010064848876285L, 5.097911466811016354623559941115413499164e-1L),
    TEST_c_c (cacos, 0x1.fp-10L, 1.0L, 1.569458417435338878318763342108699202986L, -8.813742198809567991336704287826445879025e-1L),
    TEST_c_c (cacos, 0x1.fp-10L, -1.0L, 1.569458417435338878318763342108699202986L, 8.813742198809567991336704287826445879025e-1L),
    TEST_c_c (cacos, -0x1.fp-10L, 1.0L, 1.572134236154454360143880041170803681211L, -8.813742198809567991336704287826445879025e-1L),
    TEST_c_c (cacos, -0x1.fp-10L, -1.0L, 1.572134236154454360143880041170803681211L, 8.813742198809567991336704287826445879025e-1L),
    TEST_c_c (cacos, 1.0L, 0x1.fp-10L, 4.349129763101882771258049954181971959031e-2L, -4.350501469856803800217957402220976497152e-2L),
    TEST_c_c (cacos, -1.0L, 0x1.fp-10L, 3.098101355958774410750062883737683164607L, -4.350501469856803800217957402220976497152e-2L),
    TEST_c_c (cacos, 1.0L, -0x1.fp-10L, 4.349129763101882771258049954181971959031e-2L, 4.350501469856803800217957402220976497152e-2L),
    TEST_c_c (cacos, -1.0L, -0x1.fp-10L, 3.098101355958774410750062883737683164607L, 4.350501469856803800217957402220976497152e-2L),
    TEST_c_c (cacos, 0x1.fp-30L, 1.0L, 1.570796325518966635014803151387033957091L, -8.813735870195430258081932989769495326854e-1L),
    TEST_c_c (cacos, 0x1.fp-30L, -1.0L, 1.570796325518966635014803151387033957091L, 8.813735870195430258081932989769495326854e-1L),
    TEST_c_c (cacos, -0x1.fp-30L, 1.0L, 1.570796328070826603447840231892468927106L, -8.813735870195430258081932989769495326854e-1L),
    TEST_c_c (cacos, -0x1.fp-30L, -1.0L, 1.570796328070826603447840231892468927106L, 8.813735870195430258081932989769495326854e-1L),
    TEST_c_c (cacos, 1.0L, 0x1.fp-30L, 4.247867097467650115899790787875186617316e-5L, -4.247867098745151888768727039216644758847e-5L),
    TEST_c_c (cacos, -1.0L, 0x1.fp-30L, 3.141550174918818561961484385371624132331L, -4.247867098745151888768727039216644758847e-5L),
    TEST_c_c (cacos, 1.0L, -0x1.fp-30L, 4.247867097467650115899790787875186617316e-5L, 4.247867098745151888768727039216644758847e-5L),
    TEST_c_c (cacos, -1.0L, -0x1.fp-30L, 3.141550174918818561961484385371624132331L, 4.247867098745151888768727039216644758847e-5L),
    TEST_c_c (cacos, 0x1.fp-100L, 1.0L, 1.570796326794896619231321691638670687364L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 0x1.fp-100L, -1.0L, 1.570796326794896619231321691638670687364L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-100L, 1.0L, 1.570796326794896619231321691640832196834L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-100L, -1.0L, 1.570796326794896619231321691640832196834L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 1.0L, 0x1.fp-100L, 1.236292038260260888664514866456887257525e-15L, -1.236292038260260888664514866457202186027e-15L),
    TEST_c_c (cacos, -1.0L, 0x1.fp-100L, 3.141592653589792002170605123018614219682L, -1.236292038260260888664514866457202186027e-15L),
    TEST_c_c (cacos, 1.0L, -0x1.fp-100L, 1.236292038260260888664514866456887257525e-15L, 1.236292038260260888664514866457202186027e-15L),
    TEST_c_c (cacos, -1.0L, -0x1.fp-100L, 3.141592653589792002170605123018614219682L, 1.236292038260260888664514866457202186027e-15L),
    TEST_c_c (cacos, 0x1.fp-129L, 1.0L, 1.570796326794896619231321691639751442097L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 0x1.fp-129L, -1.0L, 1.570796326794896619231321691639751442097L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-129L, 1.0L, 1.570796326794896619231321691639751442101L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-129L, -1.0L, 1.570796326794896619231321691639751442101L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 1.0L, 0x1.fp-129L, 5.335635276982233498398987585285818977930e-20L, -5.335635276982233498398987585285818977933e-20L),
    TEST_c_c (cacos, -1.0L, 0x1.fp-129L, 3.141592653589793238409287030509680549213L, -5.335635276982233498398987585285818977933e-20L),
    TEST_c_c (cacos, 1.0L, -0x1.fp-129L, 5.335635276982233498398987585285818977930e-20L, 5.335635276982233498398987585285818977933e-20L),
    TEST_c_c (cacos, -1.0L, -0x1.fp-129L, 3.141592653589793238409287030509680549213L, 5.335635276982233498398987585285818977933e-20L),
#ifndef TEST_FLOAT
    TEST_c_c (cacos, 0x1.fp-1000L, 1.0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 0x1.fp-1000L, -1.0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-1000L, 1.0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-1000L, -1.0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 1.0L, 0x1.fp-1000L, 4.252291453851660175550490409247739011867e-151L, -4.252291453851660175550490409247739011867e-151L),
    TEST_c_c (cacos, -1.0L, 0x1.fp-1000L, 3.141592653589793238462643383279502884197L, -4.252291453851660175550490409247739011867e-151L),
    TEST_c_c (cacos, 1.0L, -0x1.fp-1000L, 4.252291453851660175550490409247739011867e-151L, 4.252291453851660175550490409247739011867e-151L),
    TEST_c_c (cacos, -1.0L, -0x1.fp-1000L, 3.141592653589793238462643383279502884197L, 4.252291453851660175550490409247739011867e-151L),
    TEST_c_c (cacos, 0x1.fp-1025L, 1.0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 0x1.fp-1025L, -1.0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, 1.0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, -1.0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 1.0L, 0x1.fp-1025L, 7.340879205566679497036857179189356754017e-155L, -7.340879205566679497036857179189356754017e-155L),
    TEST_c_c (cacos, -1.0L, 0x1.fp-1025L, 3.141592653589793238462643383279502884197L, -7.340879205566679497036857179189356754017e-155L),
    TEST_c_c (cacos, 1.0L, -0x1.fp-1025L, 7.340879205566679497036857179189356754017e-155L, 7.340879205566679497036857179189356754017e-155L),
    TEST_c_c (cacos, -1.0L, -0x1.fp-1025L, 3.141592653589793238462643383279502884197L, 7.340879205566679497036857179189356754017e-155L),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacos, 0x1.fp-10000L, 1.0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 0x1.fp-10000L, -1.0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-10000L, 1.0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-10000L, -1.0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 1.0L, 0x1.fp-10000L, 9.854680208706673586644342922051388714633e-1506L, -9.854680208706673586644342922051388714633e-1506L),
    TEST_c_c (cacos, -1.0L, 0x1.fp-10000L, 3.141592653589793238462643383279502884197L, -9.854680208706673586644342922051388714633e-1506L),
    TEST_c_c (cacos, 1.0L, -0x1.fp-10000L, 9.854680208706673586644342922051388714633e-1506L, 9.854680208706673586644342922051388714633e-1506L),
    TEST_c_c (cacos, -1.0L, -0x1.fp-10000L, 3.141592653589793238462643383279502884197L, 9.854680208706673586644342922051388714633e-1506L),
    TEST_c_c (cacos, 0x1.fp-16385L, 1.0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 0x1.fp-16385L, -1.0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, 1.0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, -1.0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (cacos, 1.0L, 0x1.fp-16385L, 9.023632056840860275214893047597614177639e-2467L, -9.023632056840860275214893047597614177639e-2467L),
    TEST_c_c (cacos, -1.0L, 0x1.fp-16385L, 3.141592653589793238462643383279502884197L, -9.023632056840860275214893047597614177639e-2467L),
    TEST_c_c (cacos, 1.0L, -0x1.fp-16385L, 9.023632056840860275214893047597614177639e-2467L, 9.023632056840860275214893047597614177639e-2467L),
    TEST_c_c (cacos, -1.0L, -0x1.fp-16385L, 3.141592653589793238462643383279502884197L, 9.023632056840860275214893047597614177639e-2467L),
#endif

    TEST_c_c (cacos, 0x1p-23L, 0x1.000002p0L, 1.570796242501204621739026081991856762292L, -8.813736713132400470205730751186547909968e-1L),
    TEST_c_c (cacos, 0x1p-23L, -0x1.000002p0L, 1.570796242501204621739026081991856762292L, 8.813736713132400470205730751186547909968e-1L),
    TEST_c_c (cacos, -0x1p-23L, 0x1.000002p0L, 1.570796411088588616723617301287646121905L, -8.813736713132400470205730751186547909968e-1L),
    TEST_c_c (cacos, -0x1p-23L, -0x1.000002p0L, 1.570796411088588616723617301287646121905L, 8.813736713132400470205730751186547909968e-1L),
    TEST_c_c (cacos, 0x1.000002p0L, 0x1p-23L, 2.222118384408546368406374049167636760903e-4L, -5.364668491573609633134147164031476452679e-4L),
    TEST_c_c (cacos, -0x1.000002p0L, 0x1p-23L, 3.141370441751352383825802745874586120521L, -5.364668491573609633134147164031476452679e-4L),
    TEST_c_c (cacos, 0x1.000002p0L, -0x1p-23L, 2.222118384408546368406374049167636760903e-4L, 5.364668491573609633134147164031476452679e-4L),
    TEST_c_c (cacos, -0x1.000002p0L, -0x1p-23L, 3.141370441751352383825802745874586120521L, 5.364668491573609633134147164031476452679e-4L),
    TEST_c_c (cacos, 0x1.fp-129L, 0x1.000002p0L, 1.570796326794896619231321691639751442097L, -8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (cacos, 0x1.fp-129L, -0x1.000002p0L, 1.570796326794896619231321691639751442097L, 8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (cacos, -0x1.fp-129L, 0x1.000002p0L, 1.570796326794896619231321691639751442101L, -8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (cacos, -0x1.fp-129L, -0x1.000002p0L, 1.570796326794896619231321691639751442101L, 8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (cacos, 0x1.000002p0L, 0x1.fp-129L, 5.830451806317544230969669308596361881467e-36L, -4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (cacos, -0x1.000002p0L, 0x1.fp-129L, 3.141592653589793238462643383279502878367L, -4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (cacos, 0x1.000002p0L, -0x1.fp-129L, 5.830451806317544230969669308596361881467e-36L, 4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (cacos, -0x1.000002p0L, -0x1.fp-129L, 3.141592653589793238462643383279502878367L, 4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (cacos, 0.0L, 0x1.000002p0L, 1.570796326794896619231321691639751442099L, -8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (cacos, 0.0L, -0x1.000002p0L, 1.570796326794896619231321691639751442099L, 8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (cacos, -0.0L, 0x1.000002p0L, 1.570796326794896619231321691639751442099L, -8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (cacos, -0.0L, -0x1.000002p0L, 1.570796326794896619231321691639751442099L, 8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (cacos, 0x1.000002p0L, 0.0L, 0.0L, -4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (cacos, -0x1.000002p0L, 0.0L, 3.141592653589793238462643383279502884197L, -4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (cacos, 0x1.000002p0L, -0.0L, 0.0L, 4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (cacos, -0x1.000002p0L, -0.0L, 3.141592653589793238462643383279502884197L, 4.882812451493617206486388134172712975070e-4L),
#ifndef TEST_FLOAT
    TEST_c_c (cacos, 0x1p-52L, 0x1.0000000000001p0L, 1.570796326794896462222075823262262934288L, -8.813735870195431822418551933572982483664e-1L),
    TEST_c_c (cacos, 0x1p-52L, -0x1.0000000000001p0L, 1.570796326794896462222075823262262934288L, 8.813735870195431822418551933572982483664e-1L),
    TEST_c_c (cacos, -0x1p-52L, 0x1.0000000000001p0L, 1.570796326794896776240567560017239949909L, -8.813735870195431822418551933572982483664e-1L),
    TEST_c_c (cacos, -0x1p-52L, -0x1.0000000000001p0L, 1.570796326794896776240567560017239949909L, 8.813735870195431822418551933572982483664e-1L),
    TEST_c_c (cacos, 0x1.0000000000001p0L, 0x1p-52L, 9.590301705980041385828904092662391018164e-9L, -2.315303644582684770975188768022139415020e-8L),
    TEST_c_c (cacos, -0x1.0000000000001p0L, 0x1p-52L, 3.141592643999491532482601997450598791535L, -2.315303644582684770975188768022139415020e-8L),
    TEST_c_c (cacos, 0x1.0000000000001p0L, -0x1p-52L, 9.590301705980041385828904092662391018164e-9L, 2.315303644582684770975188768022139415020e-8L),
    TEST_c_c (cacos, -0x1.0000000000001p0L, -0x1p-52L, 3.141592643999491532482601997450598791535L, 2.315303644582684770975188768022139415020e-8L),
    TEST_c_c (cacos, 0x1.fp-1025L, 0x1.0000000000001p0L, 1.570796326794896619231321691639751442099L, -8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (cacos, 0x1.fp-1025L, -0x1.0000000000001p0L, 1.570796326794896619231321691639751442099L, 8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, 0x1.0000000000001p0L, 1.570796326794896619231321691639751442099L, -8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, -0x1.0000000000001p0L, 1.570796326794896619231321691639751442099L, 8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (cacos, 0x1.0000000000001p0L, 0x1.fp-1025L, 2.557178503953494342609835913586108008322e-301L, -2.107342425544701550354780375182800088393e-8L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacos, -0x1.0000000000001p0L, 0x1.fp-1025L, 3.141592653589793238462643383279502884197L, -2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (cacos, 0x1.0000000000001p0L, -0x1.fp-1025L, 2.557178503953494342609835913586108008322e-301L, 2.107342425544701550354780375182800088393e-8L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacos, -0x1.0000000000001p0L, -0x1.fp-1025L, 3.141592653589793238462643383279502884197L, 2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (cacos, 0.0L, 0x1.0000000000001p0L, 1.570796326794896619231321691639751442099L, -8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (cacos, 0.0L, -0x1.0000000000001p0L, 1.570796326794896619231321691639751442099L, 8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (cacos, -0.0L, 0x1.0000000000001p0L, 1.570796326794896619231321691639751442099L, -8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (cacos, -0.0L, -0x1.0000000000001p0L, 1.570796326794896619231321691639751442099L, 8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (cacos, 0x1.0000000000001p0L, 0.0L, 0.0L, -2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (cacos, -0x1.0000000000001p0L, 0.0L, 3.141592653589793238462643383279502884197L, -2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (cacos, 0x1.0000000000001p0L, -0.0L, 0.0L, 2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (cacos, -0x1.0000000000001p0L, -0.0L, 3.141592653589793238462643383279502884197L, 2.107342425544701550354780375182800088393e-8L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (cacos, 0x1p-63L, 0x1.0000000000000002p0L, 1.570796326794896619154657020805582738031L, -8.813735870195430253092739958139610131001e-1L),
    TEST_c_c (cacos, 0x1p-63L, -0x1.0000000000000002p0L, 1.570796326794896619154657020805582738031L, 8.813735870195430253092739958139610131001e-1L),
    TEST_c_c (cacos, -0x1p-63L, 0x1.0000000000000002p0L, 1.570796326794896619307986362473920146166L, -8.813735870195430253092739958139610131001e-1L),
    TEST_c_c (cacos, -0x1p-63L, -0x1.0000000000000002p0L, 1.570796326794896619307986362473920146166L, 8.813735870195430253092739958139610131001e-1L),
    TEST_c_c (cacos, 0x1.0000000000000002p0L, 0x1p-63L, 2.119177303101063432592523199680782317447e-10L, -5.116146586219826555037807251857670783420e-10L),
    TEST_c_c (cacos, -0x1.0000000000000002p0L, 0x1p-63L, 3.141592653377875508152537040020250564229L, -5.116146586219826555037807251857670783420e-10L),
    TEST_c_c (cacos, 0x1.0000000000000002p0L, -0x1p-63L, 2.119177303101063432592523199680782317447e-10L, 5.116146586219826555037807251857670783420e-10L),
    TEST_c_c (cacos, -0x1.0000000000000002p0L, -0x1p-63L, 3.141592653377875508152537040020250564229L, 5.116146586219826555037807251857670783420e-10L),
# if LDBL_MIN_EXP <= -16381
    TEST_c_c (cacos, 0x1.fp-16385L, 0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (cacos, 0x1.fp-16385L, -0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, 0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, -0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (cacos, 0x1.0000000000000002p0L, 0x1.fp-16385L, 1.748608650034385653922359120438227480943e-4923L, -4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (cacos, -0x1.0000000000000002p0L, 0x1.fp-16385L, 3.141592653589793238462643383279502884197L, -4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (cacos, 0x1.0000000000000002p0L, -0x1.fp-16385L, 1.748608650034385653922359120438227480943e-4923L, 4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (cacos, -0x1.0000000000000002p0L, -0x1.fp-16385L, 3.141592653589793238462643383279502884197L, 4.656612873077392578082927418388212703712e-10L),
# endif
    TEST_c_c (cacos, 0.0L, 0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (cacos, 0.0L, -0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (cacos, -0.0L, 0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (cacos, -0.0L, -0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (cacos, 0x1.0000000000000002p0L, 0.0L, 0.0L, -4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (cacos, -0x1.0000000000000002p0L, 0.0L, 3.141592653589793238462643383279502884197L, -4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (cacos, 0x1.0000000000000002p0L, -0.0L, 0.0L, 4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (cacos, -0x1.0000000000000002p0L, -0.0L, 3.141592653589793238462643383279502884197L, 4.656612873077392578082927418388212703712e-10L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (cacos, 0x1p-106L, 0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639742726335L, -8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, 0x1p-106L, -0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639742726335L, 8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, -0x1p-106L, 0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639760157863L, -8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, -0x1p-106L, -0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639760157863L, 8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, 0x1.000000000000000000000000008p0L, 0x1p-106L, 5.394221422390606848017034778914096659726e-17L, -2.285028863093221674154232933662774454211e-16L),
    TEST_c_c (cacos, -0x1.000000000000000000000000008p0L, 0x1p-106L, 3.141592653589793184520429159373434404027L, -2.285028863093221674154232933662774454211e-16L),
    TEST_c_c (cacos, 0x1.000000000000000000000000008p0L, -0x1p-106L, 5.394221422390606848017034778914096659726e-17L, 2.285028863093221674154232933662774454211e-16L),
    TEST_c_c (cacos, -0x1.000000000000000000000000008p0L, -0x1p-106L, 3.141592653589793184520429159373434404027L, 2.285028863093221674154232933662774454211e-16L),
    TEST_c_c (cacos, 0x1.fp-1025L, 0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, 0x1.fp-1025L, -0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, 0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, -0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, 0x1.000000000000000000000000008p0L, 0x1.fp-1025L, 2.426922623448365473354662093431821897807e-293L, -2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (cacos, -0x1.000000000000000000000000008p0L, 0x1.fp-1025L, 3.141592653589793238462643383279502884197L, -2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (cacos, 0x1.000000000000000000000000008p0L, -0x1.fp-1025L, 2.426922623448365473354662093431821897807e-293L, 2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (cacos, -0x1.000000000000000000000000008p0L, -0x1.fp-1025L, 3.141592653589793238462643383279502884197L, 2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (cacos, 0.0L, 0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, 0.0L, -0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, -0.0L, 0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, -0.0L, -0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (cacos, 0x1.000000000000000000000000008p0L, 0.0L, 0.0L, -2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (cacos, -0x1.000000000000000000000000008p0L, 0.0L, 3.141592653589793238462643383279502884197L, -2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (cacos, 0x1.000000000000000000000000008p0L, -0.0L, 0.0L, 2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (cacos, -0x1.000000000000000000000000008p0L, -0.0L, 3.141592653589793238462643383279502884197L, 2.220446049250313080847263336181636063482e-16L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (cacos, 0x1p-113L, 0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751374007L, -8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, 0x1p-113L, -0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751374007L, 8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, -0x1p-113L, 0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751510190L, -8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, -0x1p-113L, -0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751510190L, 8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, 0x1.0000000000000000000000000001p0L, 0x1p-113L, 4.767863183742677481693563511435642755521e-18L, -2.019699255375255198156433710951064632386e-17L),
    TEST_c_c (cacos, -0x1.0000000000000000000000000001p0L, 0x1p-113L, 3.141592653589793233694780199536825402504L, -2.019699255375255198156433710951064632386e-17L),
    TEST_c_c (cacos, 0x1.0000000000000000000000000001p0L, -0x1p-113L, 4.767863183742677481693563511435642755521e-18L, 2.019699255375255198156433710951064632386e-17L),
    TEST_c_c (cacos, -0x1.0000000000000000000000000001p0L, -0x1p-113L, 3.141592653589793233694780199536825402504L, 2.019699255375255198156433710951064632386e-17L),
    TEST_c_c (cacos, 0x1.fp-16385L, 0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, 0x1.fp-16385L, -0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, 0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, -0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, 0x1.0000000000000000000000000001p0L, 0x1.fp-16385L, 4.148847925325683229178506116378864361396e-4916L, -1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (cacos, -0x1.0000000000000000000000000001p0L, 0x1.fp-16385L, 3.141592653589793238462643383279502884197L, -1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (cacos, 0x1.0000000000000000000000000001p0L, -0x1.fp-16385L, 4.148847925325683229178506116378864361396e-4916L, 1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (cacos, -0x1.0000000000000000000000000001p0L, -0x1.fp-16385L, 3.141592653589793238462643383279502884197L, 1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (cacos, 0.0L, 0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, 0.0L, -0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, -0.0L, 0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, -0.0L, -0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (cacos, 0x1.0000000000000000000000000001p0L, 0.0L, 0.0L, -1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (cacos, -0x1.0000000000000000000000000001p0L, 0.0L, 3.141592653589793238462643383279502884197L, -1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (cacos, 0x1.0000000000000000000000000001p0L, -0.0L, 0.0L, 1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (cacos, -0x1.0000000000000000000000000001p0L, -0.0L, 3.141592653589793238462643383279502884197L, 1.962615573354718824241727964954454332780e-17L),
#endif

    TEST_c_c (cacos, 0x1p-23L, 0x0.ffffffp0L, 1.570796242501197085295336701632142060969L, -8.813735448726963984495965873956465777250e-1L),
    TEST_c_c (cacos, 0x1p-23L, -0x0.ffffffp0L, 1.570796242501197085295336701632142060969L, 8.813735448726963984495965873956465777250e-1L),
    TEST_c_c (cacos, -0x1p-23L, 0x0.ffffffp0L, 1.570796411088596153167306681647360823228L, -8.813735448726963984495965873956465777250e-1L),
    TEST_c_c (cacos, -0x1p-23L, -0x0.ffffffp0L, 1.570796411088596153167306681647360823228L, 8.813735448726963984495965873956465777250e-1L),
    TEST_c_c (cacos, 0x0.ffffffp0L, 0x1p-23L, 4.391863861910487109445187743978204002614e-4L, -2.714321200917194650737217746780928423385e-4L),
    TEST_c_c (cacos, -0x0.ffffffp0L, 0x1p-23L, 3.141153467203602189751698864505105063797L, -2.714321200917194650737217746780928423385e-4L),
    TEST_c_c (cacos, 0x0.ffffffp0L, -0x1p-23L, 4.391863861910487109445187743978204002614e-4L, 2.714321200917194650737217746780928423385e-4L),
    TEST_c_c (cacos, -0x0.ffffffp0L, -0x1p-23L, 3.141153467203602189751698864505105063797L, 2.714321200917194650737217746780928423385e-4L),
    TEST_c_c (cacos, 0x1.fp-129L, 0x0.ffffffp0L, 1.570796326794896619231321691639751442097L, -8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (cacos, 0x1.fp-129L, -0x0.ffffffp0L, 1.570796326794896619231321691639751442097L, 8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (cacos, -0x1.fp-129L, 0x0.ffffffp0L, 1.570796326794896619231321691639751442101L, -8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (cacos, -0x1.fp-129L, -0x0.ffffffp0L, 1.570796326794896619231321691639751442101L, 8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (cacos, 0x0.ffffffp0L, 0x1.fp-129L, 3.452669847162035876032494826387364972849e-4L, -8.245504387859737323891843603996428534945e-36L),
    TEST_c_c (cacos, -0x0.ffffffp0L, 0x1.fp-129L, 3.141247386605077034875040133796864147700L, -8.245504387859737323891843603996428534945e-36L),
    TEST_c_c (cacos, 0x0.ffffffp0L, -0x1.fp-129L, 3.452669847162035876032494826387364972849e-4L, 8.245504387859737323891843603996428534945e-36L),
    TEST_c_c (cacos, -0x0.ffffffp0L, -0x1.fp-129L, 3.141247386605077034875040133796864147700L, 8.245504387859737323891843603996428534945e-36L),
    TEST_c_c (cacos, 0.0L, 0x0.ffffffp0L, 1.570796326794896619231321691639751442099L, -8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (cacos, 0.0L, -0x0.ffffffp0L, 1.570796326794896619231321691639751442099L, 8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (cacos, -0.0L, 0x0.ffffffp0L, 1.570796326794896619231321691639751442099L, -8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (cacos, -0.0L, -0x0.ffffffp0L, 1.570796326794896619231321691639751442099L, 8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (cacos, 0x0.ffffffp0L, 0.0L, 3.452669847162035876032494826387364972849e-4L, -0.0L),
    TEST_c_c (cacos, -0x0.ffffffp0L, 0.0L, 3.141247386605077034875040133796864147700L, -0.0L),
    TEST_c_c (cacos, 0x0.ffffffp0L, -0.0L, 3.452669847162035876032494826387364972849e-4L, 0.0L),
    TEST_c_c (cacos, -0x0.ffffffp0L, -0.0L, 3.141247386605077034875040133796864147700L, 0.0L),
    TEST_c_c (cacos, 0x1p-23L, 0.5L, 1.570796220170866625230343643673321150378L, -4.812118250596059896127318386463676808798e-1L),
    TEST_c_c (cacos, 0x1p-23L, -0.5L, 1.570796220170866625230343643673321150378L, 4.812118250596059896127318386463676808798e-1L),
    TEST_c_c (cacos, -0x1p-23L, 0.5L, 1.570796433418926613232299739606181733819L, -4.812118250596059896127318386463676808798e-1L),
    TEST_c_c (cacos, -0x1p-23L, -0.5L, 1.570796433418926613232299739606181733819L, 4.812118250596059896127318386463676808798e-1L),
    TEST_c_c (cacos, 0.5L, 0x1p-23L, 1.047197551196603215914744618665204532273L, -1.376510308240943236356088341381173571841e-7L),
    TEST_c_c (cacos, -0.5L, 0x1p-23L, 2.094395102393190022547898764614298351924L, -1.376510308240943236356088341381173571841e-7L),
    TEST_c_c (cacos, 0.5L, -0x1p-23L, 1.047197551196603215914744618665204532273L, 1.376510308240943236356088341381173571841e-7L),
    TEST_c_c (cacos, -0.5L, -0x1p-23L, 2.094395102393190022547898764614298351924L, 1.376510308240943236356088341381173571841e-7L),
    TEST_c_c (cacos, 0x1.fp-129L, 0.5L, 1.570796326794896619231321691639751442096L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, 0x1.fp-129L, -0.5L, 1.570796326794896619231321691639751442096L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, -0x1.fp-129L, 0.5L, 1.570796326794896619231321691639751442101L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, -0x1.fp-129L, -0.5L, 1.570796326794896619231321691639751442101L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, 0.5L, 0x1.fp-129L, 1.047197551196597746154214461093167628066L, -3.287317402534702257036015056278368274737e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, -0.5L, 0x1.fp-129L, 2.094395102393195492308428922186335256131L, -3.287317402534702257036015056278368274737e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, 0.5L, -0x1.fp-129L, 1.047197551196597746154214461093167628066L, 3.287317402534702257036015056278368274737e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, -0.5L, -0x1.fp-129L, 2.094395102393195492308428922186335256131L, 3.287317402534702257036015056278368274737e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, 0x1p-23L, 0x1p-23L, 1.570796207585607068450636380271254316214L, -1.192092895507818146886315028596704749235e-7L),
    TEST_c_c (cacos, 0x1p-23L, -0x1p-23L, 1.570796207585607068450636380271254316214L, 1.192092895507818146886315028596704749235e-7L),
    TEST_c_c (cacos, -0x1p-23L, 0x1p-23L, 1.570796446004186170012007003008248567984L, -1.192092895507818146886315028596704749235e-7L),
    TEST_c_c (cacos, -0x1p-23L, -0x1p-23L, 1.570796446004186170012007003008248567984L, 1.192092895507818146886315028596704749235e-7L),
    TEST_c_c (cacos, 0x1.fp-129L, 0x1p-23L, 1.570796326794896619231321691639751442096L, -1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (cacos, 0x1.fp-129L, -0x1p-23L, 1.570796326794896619231321691639751442096L, 1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (cacos, -0x1.fp-129L, 0x1p-23L, 1.570796326794896619231321691639751442101L, -1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (cacos, -0x1.fp-129L, -0x1p-23L, 1.570796326794896619231321691639751442101L, 1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (cacos, 0x1p-23L, 0x1.fp-129L, 1.570796207585607068449789347324000006847L, -2.846900380897747786805634596726756660388e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, -0x1p-23L, 0x1.fp-129L, 1.570796446004186170012854035955502877351L, -2.846900380897747786805634596726756660388e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, 0x1p-23L, -0x1.fp-129L, 1.570796207585607068449789347324000006847L, 2.846900380897747786805634596726756660388e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, -0x1p-23L, -0x1.fp-129L, 1.570796446004186170012854035955502877351L, 2.846900380897747786805634596726756660388e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, 0.0L, 0x1p-23L, 1.570796326794896619231321691639751442099L, -1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (cacos, 0.0L, -0x1p-23L, 1.570796326794896619231321691639751442099L, 1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (cacos, -0.0L, 0x1p-23L, 1.570796326794896619231321691639751442099L, -1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (cacos, -0.0L, -0x1p-23L, 1.570796326794896619231321691639751442099L, 1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (cacos, 0x1p-23L, 0.0L, 1.570796207585607068449789347324000006847L, -0.0L),
    TEST_c_c (cacos, -0x1p-23L, 0.0L, 1.570796446004186170012854035955502877351L, -0.0L),
    TEST_c_c (cacos, 0x1p-23L, -0.0L, 1.570796207585607068449789347324000006847L, 0.0L),
    TEST_c_c (cacos, -0x1p-23L, -0.0L, 1.570796446004186170012854035955502877351L, 0.0L),
    TEST_c_c (cacos, 0x1.fp-129L, 0x1.fp-129L, 1.570796326794896619231321691639751442096L, -2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, 0x1.fp-129L, -0x1.fp-129L, 1.570796326794896619231321691639751442096L, 2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, -0x1.fp-129L, 0x1.fp-129L, 1.570796326794896619231321691639751442101L, -2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacos, -0x1.fp-129L, -0x1.fp-129L, 1.570796326794896619231321691639751442101L, 2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (cacos, 0x1p-52L, 0x0.fffffffffffff8p0L, 1.570796326794896462222075823262236786996L, -8.813735870195429467279863907910458761820e-1L),
    TEST_c_c (cacos, 0x1p-52L, -0x0.fffffffffffff8p0L, 1.570796326794896462222075823262236786996L, 8.813735870195429467279863907910458761820e-1L),
    TEST_c_c (cacos, -0x1p-52L, 0x0.fffffffffffff8p0L, 1.570796326794896776240567560017266097201L, -8.813735870195429467279863907910458761820e-1L),
    TEST_c_c (cacos, -0x1p-52L, -0x0.fffffffffffff8p0L, 1.570796326794896776240567560017266097201L, 8.813735870195429467279863907910458761820e-1L),
    TEST_c_c (cacos, 0x0.fffffffffffff8p0L, 0x1p-52L, 1.895456983915074112227925127005564372844e-8L, -1.171456840272878582596796205397918831268e-8L),
    TEST_c_c (cacos, -0x0.fffffffffffff8p0L, 0x1p-52L, 3.141592634635223399311902261000251614142L, -1.171456840272878582596796205397918831268e-8L),
    TEST_c_c (cacos, 0x0.fffffffffffff8p0L, -0x1p-52L, 1.895456983915074112227925127005564372844e-8L, 1.171456840272878582596796205397918831268e-8L),
    TEST_c_c (cacos, -0x0.fffffffffffff8p0L, -0x1p-52L, 3.141592634635223399311902261000251614142L, 1.171456840272878582596796205397918831268e-8L),
    TEST_c_c (cacos, 0x1.fp-1025L, 0x0.fffffffffffff8p0L, 1.570796326794896619231321691639751442099L, -8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (cacos, 0x1.fp-1025L, -0x0.fffffffffffff8p0L, 1.570796326794896619231321691639751442099L, 8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, 0x0.fffffffffffff8p0L, 1.570796326794896619231321691639751442099L, -8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, -0x0.fffffffffffff8p0L, 1.570796326794896619231321691639751442099L, 8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (cacos, 0x0.fffffffffffff8p0L, 0x1.fp-1025L, 1.490116119384765638786343542550461592240e-8L, -3.616396521699973256461764099945789620604e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacos, -0x0.fffffffffffff8p0L, 0x1.fp-1025L, 3.141592638688632044614986995416067458693L, -3.616396521699973256461764099945789620604e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacos, 0x0.fffffffffffff8p0L, -0x1.fp-1025L, 1.490116119384765638786343542550461592240e-8L, 3.616396521699973256461764099945789620604e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacos, -0x0.fffffffffffff8p0L, -0x1.fp-1025L, 3.141592638688632044614986995416067458693L, 3.616396521699973256461764099945789620604e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacos, 0.0L, 0x0.fffffffffffff8p0L, 1.570796326794896619231321691639751442099L, -8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (cacos, 0.0L, -0x0.fffffffffffff8p0L, 1.570796326794896619231321691639751442099L, 8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (cacos, -0.0L, 0x0.fffffffffffff8p0L, 1.570796326794896619231321691639751442099L, -8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (cacos, -0.0L, -0x0.fffffffffffff8p0L, 1.570796326794896619231321691639751442099L, 8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (cacos, 0x0.fffffffffffff8p0L, 0.0L, 1.490116119384765638786343542550461592240e-8L, -0.0L),
    TEST_c_c (cacos, -0x0.fffffffffffff8p0L, 0.0L, 3.141592638688632044614986995416067458693L, -0.0L),
    TEST_c_c (cacos, 0x0.fffffffffffff8p0L, -0.0L, 1.490116119384765638786343542550461592240e-8L, 0.0L),
    TEST_c_c (cacos, -0x0.fffffffffffff8p0L, -0.0L, 3.141592638688632044614986995416067458693L, 0.0L),
#endif
    TEST_c_c (cacos, 0x1p-52L, 0.5L, 1.570796326794896420628589431857911675917L, -4.812118250596034474977589134243772428682e-1L),
    TEST_c_c (cacos, 0x1p-52L, -0.5L, 1.570796326794896420628589431857911675917L, 4.812118250596034474977589134243772428682e-1L),
    TEST_c_c (cacos, -0x1p-52L, 0.5L, 1.570796326794896817834053951421591208280L, -4.812118250596034474977589134243772428682e-1L),
    TEST_c_c (cacos, -0x1p-52L, -0.5L, 1.570796326794896817834053951421591208280L, 4.812118250596034474977589134243772428682e-1L),
    TEST_c_c (cacos, 0.5L, 0x1p-52L, 1.047197551196597746154214461093186605110L, -2.563950248511418570403591756798643804971e-16L),
    TEST_c_c (cacos, -0.5L, 0x1p-52L, 2.094395102393195492308428922186316279087L, -2.563950248511418570403591756798643804971e-16L),
    TEST_c_c (cacos, 0.5L, -0x1p-52L, 1.047197551196597746154214461093186605110L, 2.563950248511418570403591756798643804971e-16L),
    TEST_c_c (cacos, -0.5L, -0x1p-52L, 2.094395102393195492308428922186316279087L, 2.563950248511418570403591756798643804971e-16L),
#ifndef TEST_FLOAT
    TEST_c_c (cacos, 0x1.fp-1025L, 0.5L, 1.570796326794896619231321691639751442099L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, 0x1.fp-1025L, -0.5L, 1.570796326794896619231321691639751442099L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, 0.5L, 1.570796326794896619231321691639751442099L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, -0.5L, 1.570796326794896619231321691639751442099L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, 0.5L, 0x1.fp-1025L, 1.047197551196597746154214461093167628066L, -6.222508863508420569166420770843207333493e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, -0.5L, 0x1.fp-1025L, 2.094395102393195492308428922186335256131L, -6.222508863508420569166420770843207333493e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, 0.5L, -0x1.fp-1025L, 1.047197551196597746154214461093167628066L, 6.222508863508420569166420770843207333493e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, -0.5L, -0x1.fp-1025L, 2.094395102393195492308428922186335256131L, 6.222508863508420569166420770843207333493e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
    TEST_c_c (cacos, 0x1p-52L, 0x1p-52L, 1.570796326794896397186716766608443357372L, -2.220446049250313080847263336181677117148e-16L),
    TEST_c_c (cacos, 0x1p-52L, -0x1p-52L, 1.570796326794896397186716766608443357372L, 2.220446049250313080847263336181677117148e-16L),
    TEST_c_c (cacos, -0x1p-52L, 0x1p-52L, 1.570796326794896841275926616671059526825L, -2.220446049250313080847263336181677117148e-16L),
    TEST_c_c (cacos, -0x1p-52L, -0x1p-52L, 1.570796326794896841275926616671059526825L, 2.220446049250313080847263336181677117148e-16L),
#ifndef TEST_FLOAT
    TEST_c_c (cacos, 0x1.fp-1025L, 0x1p-52L, 1.570796326794896619231321691639751442099L, -2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (cacos, 0x1.fp-1025L, -0x1p-52L, 1.570796326794896619231321691639751442099L, 2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (cacos, -0x1.fp-1025L, 0x1p-52L, 1.570796326794896619231321691639751442099L, -2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (cacos, -0x1.fp-1025L, -0x1p-52L, 1.570796326794896619231321691639751442099L, 2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (cacos, 0x1p-52L, 0x1.fp-1025L, 1.570796326794896397186716766608443357372L, -5.388850751072128349671657362289548938458e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, -0x1p-52L, 0x1.fp-1025L, 1.570796326794896841275926616671059526825L, -5.388850751072128349671657362289548938458e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, 0x1p-52L, -0x1.fp-1025L, 1.570796326794896397186716766608443357372L, 5.388850751072128349671657362289548938458e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, -0x1p-52L, -0x1.fp-1025L, 1.570796326794896841275926616671059526825L, 5.388850751072128349671657362289548938458e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
    TEST_c_c (cacos, 0.0L, 0x1p-52L, 1.570796326794896619231321691639751442099L, -2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (cacos, 0.0L, -0x1p-52L, 1.570796326794896619231321691639751442099L, 2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (cacos, -0.0L, 0x1p-52L, 1.570796326794896619231321691639751442099L, -2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (cacos, -0.0L, -0x1p-52L, 1.570796326794896619231321691639751442099L, 2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (cacos, 0x1p-52L, 0.0L, 1.570796326794896397186716766608443357372L, -0.0L),
    TEST_c_c (cacos, -0x1p-52L, 0.0L, 1.570796326794896841275926616671059526825L, -0.0L),
    TEST_c_c (cacos, 0x1p-52L, -0.0L, 1.570796326794896397186716766608443357372L, 0.0L),
    TEST_c_c (cacos, -0x1p-52L, -0.0L, 1.570796326794896841275926616671059526825L, 0.0L),
#ifndef TEST_FLOAT
    TEST_c_c (cacos, 0x1.fp-1025L, 0x1.fp-1025L, 1.570796326794896619231321691639751442099L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, 0x1.fp-1025L, -0x1.fp-1025L, 1.570796326794896619231321691639751442099L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, -0x1.fp-1025L, 0x1.fp-1025L, 1.570796326794896619231321691639751442099L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, -0x1.fp-1025L, -0x1.fp-1025L, 1.570796326794896619231321691639751442099L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (cacos, 0x1p-63L, 0x0.ffffffffffffffffp0L, 1.570796326794896619154657020805582738025L, -8.813735870195430251942769895627079569937e-1L),
    TEST_c_c (cacos, 0x1p-63L, -0x0.ffffffffffffffffp0L, 1.570796326794896619154657020805582738025L, 8.813735870195430251942769895627079569937e-1L),
    TEST_c_c (cacos, -0x1p-63L, 0x0.ffffffffffffffffp0L, 1.570796326794896619307986362473920146173L, -8.813735870195430251942769895627079569937e-1L),
    TEST_c_c (cacos, -0x1p-63L, -0x0.ffffffffffffffffp0L, 1.570796326794896619307986362473920146173L, 8.813735870195430251942769895627079569937e-1L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffp0L, 0x1p-63L, 4.188407771167967636741951941902992986043e-10L, -2.588578361325995866221775673638805081337e-10L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffp0L, 0x1p-63L, 3.141592653170952461345846619605307690007L, -2.588578361325995866221775673638805081337e-10L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffp0L, -0x1p-63L, 4.188407771167967636741951941902992986043e-10L, 2.588578361325995866221775673638805081337e-10L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffp0L, -0x1p-63L, 3.141592653170952461345846619605307690007L, 2.588578361325995866221775673638805081337e-10L),
# if LDBL_MIN_EXP <= -16381
    TEST_c_c (cacos, 0x1.fp-16385L, 0x0.ffffffffffffffffp0L, 1.570796326794896619231321691639751442099L, -8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (cacos, 0x1.fp-16385L, -0x0.ffffffffffffffffp0L, 1.570796326794896619231321691639751442099L, 8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, 0x0.ffffffffffffffffp0L, 1.570796326794896619231321691639751442099L, -8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, -0x0.ffffffffffffffffp0L, 1.570796326794896619231321691639751442099L, 8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffp0L, 0x1.fp-16385L, 3.292722539913596233371825532007990724506e-10L, -2.472906068161537187835415298076415423459e-4923L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffp0L, 0x1.fp-16385L, 3.141592653260520984471283759942320330996L, -2.472906068161537187835415298076415423459e-4923L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffp0L, -0x1.fp-16385L, 3.292722539913596233371825532007990724506e-10L, 2.472906068161537187835415298076415423459e-4923L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffp0L, -0x1.fp-16385L, 3.141592653260520984471283759942320330996L, 2.472906068161537187835415298076415423459e-4923L),
# endif
    TEST_c_c (cacos, 0.0L, 0x0.ffffffffffffffffp0L, 1.570796326794896619231321691639751442099L, -8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (cacos, 0.0L, -0x0.ffffffffffffffffp0L, 1.570796326794896619231321691639751442099L, 8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (cacos, -0.0L, 0x0.ffffffffffffffffp0L, 1.570796326794896619231321691639751442099L, -8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (cacos, -0.0L, -0x0.ffffffffffffffffp0L, 1.570796326794896619231321691639751442099L, 8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffp0L, 0.0L, 3.292722539913596233371825532007990724506e-10L, -0.0L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffp0L, 0.0L, 3.141592653260520984471283759942320330996L, -0.0L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffp0L, -0.0L, 3.292722539913596233371825532007990724506e-10L, 0.0L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffp0L, -0.0L, 3.141592653260520984471283759942320330996L, 0.0L),
#endif
    TEST_c_c (cacos, 0x1p-63L, 0.5L, 1.570796326794896619134347701278529840650L, -4.812118250596034474977589134243684231373e-1L),
    TEST_c_c (cacos, 0x1p-63L, -0.5L, 1.570796326794896619134347701278529840650L, 4.812118250596034474977589134243684231373e-1L),
    TEST_c_c (cacos, -0x1p-63L, 0.5L, 1.570796326794896619328295682000973043547L, -4.812118250596034474977589134243684231373e-1L),
    TEST_c_c (cacos, -0x1p-63L, -0.5L, 1.570796326794896619328295682000973043547L, 4.812118250596034474977589134243684231373e-1L),
    TEST_c_c (cacos, 0.5L, 0x1p-63L, 1.047197551196597746154214461093167628070L, -1.251928832280966098829878787499365228659e-19L),
    TEST_c_c (cacos, -0.5L, 0x1p-63L, 2.094395102393195492308428922186335256127L, -1.251928832280966098829878787499365228659e-19L),
    TEST_c_c (cacos, 0.5L, -0x1p-63L, 1.047197551196597746154214461093167628070L, 1.251928832280966098829878787499365228659e-19L),
    TEST_c_c (cacos, -0.5L, -0x1p-63L, 2.094395102393195492308428922186335256127L, 1.251928832280966098829878787499365228659e-19L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacos, 0x1.fp-16385L, 0.5L, 1.570796326794896619231321691639751442099L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, 0x1.fp-16385L, -0.5L, 1.570796326794896619231321691639751442099L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, 0.5L, 1.570796326794896619231321691639751442099L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, -0.5L, 1.570796326794896619231321691639751442099L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, 0.5L, 0x1.fp-16385L, 1.047197551196597746154214461093167628066L, -9.402257155670418112159334875199821342890e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, -0.5L, 0x1.fp-16385L, 2.094395102393195492308428922186335256131L, -9.402257155670418112159334875199821342890e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, 0.5L, -0x1.fp-16385L, 1.047197551196597746154214461093167628066L, 9.402257155670418112159334875199821342890e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, -0.5L, -0x1.fp-16385L, 2.094395102393195492308428922186335256131L, 9.402257155670418112159334875199821342890e-4933L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (cacos, 0x1p-63L, 0x1p-63L, 1.570796326794896619122901474391200998698L, -1.084202172485504434007452800869941711430e-19L),
    TEST_c_c (cacos, 0x1p-63L, -0x1p-63L, 1.570796326794896619122901474391200998698L, 1.084202172485504434007452800869941711430e-19L),
    TEST_c_c (cacos, -0x1p-63L, 0x1p-63L, 1.570796326794896619339741908888301885499L, -1.084202172485504434007452800869941711430e-19L),
    TEST_c_c (cacos, -0x1p-63L, -0x1p-63L, 1.570796326794896619339741908888301885499L, 1.084202172485504434007452800869941711430e-19L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacos, 0x1.fp-16385L, 0x1p-63L, 1.570796326794896619231321691639751442099L, -1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (cacos, 0x1.fp-16385L, -0x1p-63L, 1.570796326794896619231321691639751442099L, 1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (cacos, -0x1.fp-16385L, 0x1p-63L, 1.570796326794896619231321691639751442099L, -1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (cacos, -0x1.fp-16385L, -0x1p-63L, 1.570796326794896619231321691639751442099L, 1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (cacos, 0x1p-63L, 0x1.fp-16385L, 1.570796326794896619122901474391200998698L, -8.142593549724601460479922838826119584465e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, -0x1p-63L, 0x1.fp-16385L, 1.570796326794896619339741908888301885499L, -8.142593549724601460479922838826119584465e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, 0x1p-63L, -0x1.fp-16385L, 1.570796326794896619122901474391200998698L, 8.142593549724601460479922838826119584465e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, -0x1p-63L, -0x1.fp-16385L, 1.570796326794896619339741908888301885499L, 8.142593549724601460479922838826119584465e-4933L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (cacos, 0.0L, 0x1p-63L, 1.570796326794896619231321691639751442099L, -1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (cacos, 0.0L, -0x1p-63L, 1.570796326794896619231321691639751442099L, 1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (cacos, -0.0L, 0x1p-63L, 1.570796326794896619231321691639751442099L, -1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (cacos, -0.0L, -0x1p-63L, 1.570796326794896619231321691639751442099L, 1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (cacos, 0x1p-63L, 0.0L, 1.570796326794896619122901474391200998698L, -0.0L),
    TEST_c_c (cacos, -0x1p-63L, 0.0L, 1.570796326794896619339741908888301885499L, -0.0L),
    TEST_c_c (cacos, 0x1p-63L, -0.0L, 1.570796326794896619122901474391200998698L, 0.0L),
    TEST_c_c (cacos, -0x1p-63L, -0.0L, 1.570796326794896619339741908888301885499L, 0.0L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacos, 0x1.fp-16385L, 0x1.fp-16385L, 1.570796326794896619231321691639751442099L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, 0x1.fp-16385L, -0x1.fp-16385L, 1.570796326794896619231321691639751442099L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, -0x1.fp-16385L, 0x1.fp-16385L, 1.570796326794896619231321691639751442099L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, -0x1.fp-16385L, -0x1.fp-16385L, 1.570796326794896619231321691639751442099L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (cacos, 0x1p-105L, 0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639734010571L, -8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, 0x1p-105L, -0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639734010571L, 8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, -0x1p-105L, 0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639768873627L, -8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, -0x1p-105L, -0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639768873627L, 8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffcp0L, 0x1p-105L, 1.997188458999618357060632049675702684031e-16L, -1.234330349600789959989661887846005659983e-16L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffcp0L, 0x1p-105L, 3.141592653589793038743797483317667178134L, -1.234330349600789959989661887846005659983e-16L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffcp0L, -0x1p-105L, 1.997188458999618357060632049675702684031e-16L, 1.234330349600789959989661887846005659983e-16L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffcp0L, -0x1p-105L, 3.141592653589793038743797483317667178134L, 1.234330349600789959989661887846005659983e-16L),
    TEST_c_c (cacos, 0x1.fp-1025L, 0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, 0x1.fp-1025L, -0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, 0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, -0x1.fp-1025L, -0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffcp0L, 0x1.fp-1025L, 1.570092458683775059393382371963565104164e-16L, -3.432186888910770473784103112009399360940e-293L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffcp0L, 0x1.fp-1025L, 3.141592653589793081453397514901996944859L, -3.432186888910770473784103112009399360940e-293L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffcp0L, -0x1.fp-1025L, 1.570092458683775059393382371963565104164e-16L, 3.432186888910770473784103112009399360940e-293L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffcp0L, -0x1.fp-1025L, 3.141592653589793081453397514901996944859L, 3.432186888910770473784103112009399360940e-293L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacos, 0.0L, 0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, 0.0L, -0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, -0.0L, 0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, -0.0L, -0x0.ffffffffffffffffffffffffffcp0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffcp0L, 0.0L, 1.570092458683775059393382371963565104164e-16L, -0.0L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffcp0L, 0.0L, 3.141592653589793081453397514901996944859L, -0.0L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffcp0L, -0.0L, 1.570092458683775059393382371963565104164e-16L, 0.0L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffcp0L, -0.0L, 3.141592653589793081453397514901996944859L, 0.0L),
#endif
    TEST_c_c (cacos, 0x1p-105L, 0.5L, 1.570796326794896619231321691639729392766L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, 0x1p-105L, -0.5L, 1.570796326794896619231321691639729392766L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, -0x1p-105L, 0.5L, 1.570796326794896619231321691639773491431L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, -0x1p-105L, -0.5L, 1.570796326794896619231321691639773491431L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, 0.5L, 0x1p-105L, 1.047197551196597746154214461093167628066L, -2.846556599890768890940941645338546615370e-32L),
    TEST_c_c (cacos, -0.5L, 0x1p-105L, 2.094395102393195492308428922186335256131L, -2.846556599890768890940941645338546615370e-32L),
    TEST_c_c (cacos, 0.5L, -0x1p-105L, 1.047197551196597746154214461093167628066L, 2.846556599890768890940941645338546615370e-32L),
    TEST_c_c (cacos, -0.5L, -0x1p-105L, 2.094395102393195492308428922186335256131L, 2.846556599890768890940941645338546615370e-32L),
    TEST_c_c (cacos, 0x1p-105L, 0x1p-105L, 1.570796326794896619231321691639726790195L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (cacos, 0x1p-105L, -0x1p-105L, 1.570796326794896619231321691639726790195L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (cacos, -0x1p-105L, 0x1p-105L, 1.570796326794896619231321691639776094002L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (cacos, -0x1p-105L, -0x1p-105L, 1.570796326794896619231321691639776094002L, 2.465190328815661891911651766508706967729e-32L),
#ifndef TEST_FLOAT
    TEST_c_c (cacos, 0x1.fp-1025L, 0x1p-105L, 1.570796326794896619231321691639751442099L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (cacos, 0x1.fp-1025L, -0x1p-105L, 1.570796326794896619231321691639751442099L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (cacos, -0x1.fp-1025L, 0x1p-105L, 1.570796326794896619231321691639751442099L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (cacos, -0x1.fp-1025L, -0x1p-105L, 1.570796326794896619231321691639751442099L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (cacos, 0x1p-105L, 0x1.fp-1025L, 1.570796326794896619231321691639726790195L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, -0x1p-105L, 0x1.fp-1025L, 1.570796326794896619231321691639776094002L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, 0x1p-105L, -0x1.fp-1025L, 1.570796326794896619231321691639726790195L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacos, -0x1p-105L, -0x1.fp-1025L, 1.570796326794896619231321691639776094002L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
    TEST_c_c (cacos, 0.0L, 0x1p-105L, 1.570796326794896619231321691639751442099L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (cacos, 0.0L, -0x1p-105L, 1.570796326794896619231321691639751442099L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (cacos, -0.0L, 0x1p-105L, 1.570796326794896619231321691639751442099L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (cacos, -0.0L, -0x1p-105L, 1.570796326794896619231321691639751442099L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (cacos, 0x1p-105L, 0.0L, 1.570796326794896619231321691639726790195L, -0.0L),
    TEST_c_c (cacos, -0x1p-105L, 0.0L, 1.570796326794896619231321691639776094002L, -0.0L),
    TEST_c_c (cacos, 0x1p-105L, -0.0L, 1.570796326794896619231321691639726790195L, 0.0L),
    TEST_c_c (cacos, -0x1p-105L, -0.0L, 1.570796326794896619231321691639776094002L, 0.0L),
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (cacos, 0x1p-112L, 0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751305915L, -8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, 0x1p-112L, -0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751305915L, 8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, -0x1p-112L, 0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751578282L, -8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, -0x1p-112L, -0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751578282L, 8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-112L, 1.765281878332676474347460257221369818872e-17L, -1.091004200533850618345330909857103457781e-17L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-112L, 3.141592653589793220809824599952738140723L, -1.091004200533850618345330909857103457781e-17L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-112L, 1.765281878332676474347460257221369818872e-17L, 1.091004200533850618345330909857103457781e-17L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-112L, 3.141592653589793220809824599952738140723L, 1.091004200533850618345330909857103457781e-17L),
    TEST_c_c (cacos, 0x1.fp-16385L, 0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, 0x1.fp-16385L, -0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, 0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, -0x1.fp-16385L, -0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1.fp-16385L, 1.387778780781445675529539585113525401762e-17L, -5.867357004219059108790368268900490184505e-4916L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1.fp-16385L, 3.141592653589793224584855575465046128902L, -5.867357004219059108790368268900490184505e-4916L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1.fp-16385L, 1.387778780781445675529539585113525401762e-17L, 5.867357004219059108790368268900490184505e-4916L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1.fp-16385L, 3.141592653589793224584855575465046128902L, 5.867357004219059108790368268900490184505e-4916L),
    TEST_c_c (cacos, 0.0L, 0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, 0.0L, -0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, -0.0L, 0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751442099L, -8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, -0.0L, -0x0.ffffffffffffffffffffffffffff8p0L, 1.570796326794896619231321691639751442099L, 8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, 1.387778780781445675529539585113525401762e-17L, -0.0L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, 3.141592653589793224584855575465046128902L, -0.0L),
    TEST_c_c (cacos, 0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, 1.387778780781445675529539585113525401762e-17L, 0.0L),
    TEST_c_c (cacos, -0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, 3.141592653589793224584855575465046128902L, 0.0L),
#endif
    TEST_c_c (cacos, 0x1p-112L, 0.5L, 1.570796326794896619231321691639751269838L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, 0x1p-112L, -0.5L, 1.570796326794896619231321691639751269838L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, -0x1p-112L, 0.5L, 1.570796326794896619231321691639751614359L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, -0x1p-112L, -0.5L, 1.570796326794896619231321691639751614359L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (cacos, 0.5L, 0x1p-112L, 1.047197551196597746154214461093167628066L, -2.223872343664663196047610660420739543258e-34L),
    TEST_c_c (cacos, -0.5L, 0x1p-112L, 2.094395102393195492308428922186335256131L, -2.223872343664663196047610660420739543258e-34L),
    TEST_c_c (cacos, 0.5L, -0x1p-112L, 1.047197551196597746154214461093167628066L, 2.223872343664663196047610660420739543258e-34L),
    TEST_c_c (cacos, -0.5L, -0x1p-112L, 2.094395102393195492308428922186335256131L, 2.223872343664663196047610660420739543258e-34L),
    TEST_c_c (cacos, 0x1p-112L, 0x1p-112L, 1.570796326794896619231321691639751249506L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (cacos, 0x1p-112L, -0x1p-112L, 1.570796326794896619231321691639751249506L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (cacos, -0x1p-112L, 0x1p-112L, 1.570796326794896619231321691639751634692L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (cacos, -0x1p-112L, -0x1p-112L, 1.570796326794896619231321691639751634692L, 1.925929944387235853055977942584927318538e-34L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacos, 0x1.fp-16385L, 0x1p-112L, 1.570796326794896619231321691639751442099L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (cacos, 0x1.fp-16385L, -0x1p-112L, 1.570796326794896619231321691639751442099L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (cacos, -0x1.fp-16385L, 0x1p-112L, 1.570796326794896619231321691639751442099L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (cacos, -0x1.fp-16385L, -0x1p-112L, 1.570796326794896619231321691639751442099L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (cacos, 0x1p-112L, 0x1.fp-16385L, 1.570796326794896619231321691639751249506L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, -0x1p-112L, 0x1.fp-16385L, 1.570796326794896619231321691639751634692L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, 0x1p-112L, -0x1.fp-16385L, 1.570796326794896619231321691639751249506L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacos, -0x1p-112L, -0x1.fp-16385L, 1.570796326794896619231321691639751634692L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (cacos, 0.0L, 0x1p-112L, 1.570796326794896619231321691639751442099L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (cacos, 0.0L, -0x1p-112L, 1.570796326794896619231321691639751442099L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (cacos, -0.0L, 0x1p-112L, 1.570796326794896619231321691639751442099L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (cacos, -0.0L, -0x1p-112L, 1.570796326794896619231321691639751442099L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (cacos, 0x1p-112L, 0.0L, 1.570796326794896619231321691639751249506L, -0.0L),
    TEST_c_c (cacos, -0x1p-112L, 0.0L, 1.570796326794896619231321691639751634692L, -0.0L),
    TEST_c_c (cacos, 0x1p-112L, -0.0L, 1.570796326794896619231321691639751249506L, 0.0L),
    TEST_c_c (cacos, -0x1p-112L, -0.0L, 1.570796326794896619231321691639751634692L, 0.0L),

    TEST_c_c (cacos, 0.75L, 1.25L, 1.11752014915610270578240049553777969L, -1.13239363160530819522266333696834467L),
    TEST_c_c (cacos, -2, -3, 2.1414491111159960199416055713254211L, 1.9833870299165354323470769028940395L),
    END_DATA (cacos)
  };

static void
cacos_test (void)
{
  errno = 0;
  FUNC(cacos) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cacos);
  RUN_TEST_LOOP_c_c (cacos, cacos_test_data, );
  END (cacos, complex);
}

static const struct test_c_c_data cacosh_test_data[] =
  {
    START_DATA (cacosh),
    TEST_c_c (cacosh, 0, 0, 0.0, M_PI_2l),
    TEST_c_c (cacosh, minus_zero, 0, 0.0, M_PI_2l),
    TEST_c_c (cacosh, 0, minus_zero, 0.0, -M_PI_2l),
    TEST_c_c (cacosh, minus_zero, minus_zero, 0.0, -M_PI_2l),
    TEST_c_c (cacosh, minus_infty, plus_infty, plus_infty, M_PI_34l),
    TEST_c_c (cacosh, minus_infty, minus_infty, plus_infty, -M_PI_34l),

    TEST_c_c (cacosh, plus_infty, plus_infty, plus_infty, M_PI_4l),
    TEST_c_c (cacosh, plus_infty, minus_infty, plus_infty, -M_PI_4l),

    TEST_c_c (cacosh, -10.0, plus_infty, plus_infty, M_PI_2l),
    TEST_c_c (cacosh, -10.0, minus_infty, plus_infty, -M_PI_2l),
    TEST_c_c (cacosh, 0, plus_infty, plus_infty, M_PI_2l),
    TEST_c_c (cacosh, 0, minus_infty, plus_infty, -M_PI_2l),
    TEST_c_c (cacosh, 0.1L, plus_infty, plus_infty, M_PI_2l),
    TEST_c_c (cacosh, 0.1L, minus_infty, plus_infty, -M_PI_2l),

    TEST_c_c (cacosh, minus_infty, 0, plus_infty, M_PIl),
    TEST_c_c (cacosh, minus_infty, minus_zero, plus_infty, -M_PIl),
    TEST_c_c (cacosh, minus_infty, 100, plus_infty, M_PIl),
    TEST_c_c (cacosh, minus_infty, -100, plus_infty, -M_PIl),

    TEST_c_c (cacosh, plus_infty, 0, plus_infty, 0.0),
    TEST_c_c (cacosh, plus_infty, minus_zero, plus_infty, minus_zero),
    TEST_c_c (cacosh, plus_infty, 0.5, plus_infty, 0.0),
    TEST_c_c (cacosh, plus_infty, -0.5, plus_infty, minus_zero),

    TEST_c_c (cacosh, plus_infty, qnan_value, plus_infty, qnan_value),
    TEST_c_c (cacosh, minus_infty, qnan_value, plus_infty, qnan_value),

    TEST_c_c (cacosh, 0, qnan_value, qnan_value, qnan_value),
    TEST_c_c (cacosh, minus_zero, qnan_value, qnan_value, qnan_value),

    TEST_c_c (cacosh, qnan_value, plus_infty, plus_infty, qnan_value),
    TEST_c_c (cacosh, qnan_value, minus_infty, plus_infty, qnan_value),

    TEST_c_c (cacosh, 10.5, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (cacosh, -10.5, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (cacosh, qnan_value, 0.75, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (cacosh, qnan_value, -0.75, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (cacosh, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (cacosh, plus_zero, -1.5L, 1.194763217287109304111930828519090523536L, -M_PI_2l),
    TEST_c_c (cacosh, minus_zero, -1.5L, 1.194763217287109304111930828519090523536L, -M_PI_2l),
    TEST_c_c (cacosh, plus_zero, -1.0L, 0.8813735870195430252326093249797923090282L, -M_PI_2l),
    TEST_c_c (cacosh, minus_zero, -1.0L, 0.8813735870195430252326093249797923090282L, -M_PI_2l),
    TEST_c_c (cacosh, plus_zero, -0.5L, 0.4812118250596034474977589134243684231352L, -M_PI_2l),
    TEST_c_c (cacosh, minus_zero, -0.5L, 0.4812118250596034474977589134243684231352L, -M_PI_2l),
    TEST_c_c (cacosh, plus_zero, 0.5L, 0.4812118250596034474977589134243684231352L, M_PI_2l),
    TEST_c_c (cacosh, minus_zero, 0.5L, 0.4812118250596034474977589134243684231352L, M_PI_2l),
    TEST_c_c (cacosh, plus_zero, 1.0L, 0.8813735870195430252326093249797923090282L, M_PI_2l),
    TEST_c_c (cacosh, minus_zero, 1.0L, 0.8813735870195430252326093249797923090282L, M_PI_2l),
    TEST_c_c (cacosh, plus_zero, 1.5L, 1.194763217287109304111930828519090523536L, M_PI_2l),
    TEST_c_c (cacosh, minus_zero, 1.5L, 1.194763217287109304111930828519090523536L, M_PI_2l),

    TEST_c_c (cacosh, -1.5L, plus_zero, 0.9624236501192068949955178268487368462704L, M_PIl),
    TEST_c_c (cacosh, -1.5L, minus_zero, 0.9624236501192068949955178268487368462704L, -M_PIl),
    TEST_c_c (cacosh, -1.0L, plus_zero, plus_zero, M_PIl),
    TEST_c_c (cacosh, -1.0L, minus_zero, plus_zero, -M_PIl),
    TEST_c_c (cacosh, -0.5L, plus_zero, plus_zero, 2.094395102393195492308428922186335256131L),
    TEST_c_c (cacosh, -0.5L, minus_zero, plus_zero, -2.094395102393195492308428922186335256131L),
    TEST_c_c (cacosh, 0.5L, plus_zero, plus_zero, 1.047197551196597746154214461093167628066L),
    TEST_c_c (cacosh, 0.5L, minus_zero, plus_zero, -1.047197551196597746154214461093167628066L),
    TEST_c_c (cacosh, 1.0L, plus_zero, plus_zero, plus_zero),
    TEST_c_c (cacosh, 1.0L, minus_zero, plus_zero, minus_zero),
    TEST_c_c (cacosh, 1.5L, plus_zero, 0.9624236501192068949955178268487368462704L, plus_zero),
    TEST_c_c (cacosh, 1.5L, minus_zero, 0.9624236501192068949955178268487368462704L, minus_zero),

    TEST_c_c (cacosh, 0x1p50L, 1.0L, 3.535050620855721078027883819436720218708e1L, 8.881784197001252323389053344727730248720e-16L),
    TEST_c_c (cacosh, 0x1p50L, -1.0L, 3.535050620855721078027883819436720218708e1L, -8.881784197001252323389053344727730248720e-16L),
    TEST_c_c (cacosh, -0x1p50L, 1.0L, 3.535050620855721078027883819436720218708e1L, 3.141592653589792350284223683154270545292L),
    TEST_c_c (cacosh, -0x1p50L, -1.0L, 3.535050620855721078027883819436720218708e1L, -3.141592653589792350284223683154270545292L),
    TEST_c_c (cacosh, 1.0L, 0x1p50L, 3.535050620855721078027883819436759661753e1L, 1.570796326794895731052901991514519103193L),
    TEST_c_c (cacosh, -1.0L, 0x1p50L, 3.535050620855721078027883819436759661753e1L, 1.570796326794897507409741391764983781004L),
    TEST_c_c (cacosh, 1.0L, -0x1p50L, 3.535050620855721078027883819436759661753e1L, -1.570796326794895731052901991514519103193L),
    TEST_c_c (cacosh, -1.0L, -0x1p50L, 3.535050620855721078027883819436759661753e1L, -1.570796326794897507409741391764983781004L),
#ifndef TEST_FLOAT
    TEST_c_c (cacosh, 0x1p500L, 1.0L, 3.472667374605326000180332928505464606058e2L, 3.054936363499604682051979393213617699789e-151L),
    TEST_c_c (cacosh, 0x1p500L, -1.0L, 3.472667374605326000180332928505464606058e2L, -3.054936363499604682051979393213617699789e-151L),
    TEST_c_c (cacosh, -0x1p500L, 1.0L, 3.472667374605326000180332928505464606058e2L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, -0x1p500L, -1.0L, 3.472667374605326000180332928505464606058e2L, -3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 1.0L, 0x1p500L, 3.472667374605326000180332928505464606058e2L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -1.0L, 0x1p500L, 3.472667374605326000180332928505464606058e2L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 1.0L, -0x1p500L, 3.472667374605326000180332928505464606058e2L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -1.0L, -0x1p500L, 3.472667374605326000180332928505464606058e2L, -1.570796326794896619231321691639751442099L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (cacosh, 0x1p5000L, 1.0L, 3.466429049980286492395577839412341016946e3L, 7.079811261048172892385615158694057552948e-1506L),
    TEST_c_c (cacosh, 0x1p5000L, -1.0L, 3.466429049980286492395577839412341016946e3L, -7.079811261048172892385615158694057552948e-1506L),
    TEST_c_c (cacosh, -0x1p5000L, 1.0L, 3.466429049980286492395577839412341016946e3L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, -0x1p5000L, -1.0L, 3.466429049980286492395577839412341016946e3L, -3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 1.0L, 0x1p5000L, 3.466429049980286492395577839412341016946e3L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -1.0L, 0x1p5000L, 3.466429049980286492395577839412341016946e3L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 1.0L, -0x1p5000L, 3.466429049980286492395577839412341016946e3L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -1.0L, -0x1p5000L, 3.466429049980286492395577839412341016946e3L, -1.570796326794896619231321691639751442099L),
#endif
    TEST_c_c (cacosh, 0x1.fp127L, 0x1.fp127L, 8.973081118419833726837456344608533993585e1L, 7.853981633974483096156608458198757210493e-1L),
#ifndef TEST_FLOAT
    TEST_c_c (cacosh, 0x1.fp1023L, 0x1.fp1023L, 7.107906849659093345062145442726115449315e2L, 7.853981633974483096156608458198757210493e-1L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (cacosh, 0x1.fp16383L, 0x1.fp16383L, 1.135753137836666928715489992987020363057e4L, 7.853981633974483096156608458198757210493e-1L),
#endif
    TEST_c_c (cacosh, 0x1.fp-129L, 1.5L, 1.194763217287109304111930828519090523536L, 1.570796326794896619231321691639751442097L),
    TEST_c_c (cacosh, 0x1.fp-129L, -1.5L, 1.194763217287109304111930828519090523536L, -1.570796326794896619231321691639751442097L),
    TEST_c_c (cacosh, -0x1.fp-129L, 1.5L, 1.194763217287109304111930828519090523536L, 1.570796326794896619231321691639751442100L),
    TEST_c_c (cacosh, -0x1.fp-129L, -1.5L, 1.194763217287109304111930828519090523536L, -1.570796326794896619231321691639751442100L),
    TEST_c_c (cacosh, 1.5L, 0x1.fp-129L, 9.624236501192068949955178268487368462704e-1L, 2.546345110742945032959687790021055102355e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, -1.5L, 0x1.fp-129L, 9.624236501192068949955178268487368462704e-1L, 3.141592653589793238462643383279502884195L),
    TEST_c_c (cacosh, 1.5L, -0x1.fp-129L, 9.624236501192068949955178268487368462704e-1L, -2.546345110742945032959687790021055102355e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, -1.5L, -0x1.fp-129L, 9.624236501192068949955178268487368462704e-1L, -3.141592653589793238462643383279502884195L),
#ifndef TEST_FLOAT
    TEST_c_c (cacosh, 0x1.fp-1025L, 1.5L, 1.194763217287109304111930828519090523536L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-1025L, -1.5L, 1.194763217287109304111930828519090523536L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, 1.5L, 1.194763217287109304111930828519090523536L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, -1.5L, 1.194763217287109304111930828519090523536L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 1.5L, 0x1.fp-1025L, 9.624236501192068949955178268487368462704e-1L, 4.819934639999230680322935210539402497827e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, -1.5L, 0x1.fp-1025L, 9.624236501192068949955178268487368462704e-1L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 1.5L, -0x1.fp-1025L, 9.624236501192068949955178268487368462704e-1L, -4.819934639999230680322935210539402497827e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, -1.5L, -0x1.fp-1025L, 9.624236501192068949955178268487368462704e-1L, -3.141592653589793238462643383279502884197L),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacosh, 0x1.fp-16385L, 1.5L, 1.194763217287109304111930828519090523536L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-16385L, -1.5L, 1.194763217287109304111930828519090523536L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, 1.5L, 1.194763217287109304111930828519090523536L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, -1.5L, 1.194763217287109304111930828519090523536L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 1.5L, 0x1.fp-16385L, 9.624236501192068949955178268487368462704e-1L, 7.282957076134209141226696333885150260319e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, -1.5L, 0x1.fp-16385L, 9.624236501192068949955178268487368462704e-1L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 1.5L, -0x1.fp-16385L, 9.624236501192068949955178268487368462704e-1L, -7.282957076134209141226696333885150260319e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, -1.5L, -0x1.fp-16385L, 9.624236501192068949955178268487368462704e-1L, -3.141592653589793238462643383279502884197L),
#endif
    TEST_c_c (cacosh, 0.5L, 1.0L, 9.261330313501824245501244453057873152694e-1L, 1.221357263937683325603909865564381489366L),
    TEST_c_c (cacosh, 0.5L, -1.0L, 9.261330313501824245501244453057873152694e-1L, -1.221357263937683325603909865564381489366L),
    TEST_c_c (cacosh, -0.5L, 1.0L, 9.261330313501824245501244453057873152694e-1L, 1.920235389652109912858733517715121394831L),
    TEST_c_c (cacosh, -0.5L, -1.0L, 9.261330313501824245501244453057873152694e-1L, -1.920235389652109912858733517715121394831L),
    TEST_c_c (cacosh, 1.0L, 0.5L, 7.328576759736452608886724437653071523305e-1L, 6.748888455860063801646649673121744318756e-1L),
    TEST_c_c (cacosh, -1.0L, 0.5L, 7.328576759736452608886724437653071523305e-1L, 2.466703808003786858297978415967328452322L),
    TEST_c_c (cacosh, 1.0L, -0.5L, 7.328576759736452608886724437653071523305e-1L, -6.748888455860063801646649673121744318756e-1L),
    TEST_c_c (cacosh, -1.0L, -0.5L, 7.328576759736452608886724437653071523305e-1L, -2.466703808003786858297978415967328452322L),
    TEST_c_c (cacosh, 0.25L, 1.0L, 8.924633639033482359562124741744951972772e-1L, 1.394493894017929688812643125003661339452L),
    TEST_c_c (cacosh, 0.25L, -1.0L, 8.924633639033482359562124741744951972772e-1L, -1.394493894017929688812643125003661339452L),
    TEST_c_c (cacosh, -0.25L, 1.0L, 8.924633639033482359562124741744951972772e-1L, 1.747098759571863549650000258275841544745L),
    TEST_c_c (cacosh, -0.25L, -1.0L, 8.924633639033482359562124741744951972772e-1L, -1.747098759571863549650000258275841544745L),
    TEST_c_c (cacosh, 1.0L, 0.25L, 5.097911466811016354623559941115413499164e-1L, 4.890443302710802929202843732146540079124e-1L),
    TEST_c_c (cacosh, -1.0L, 0.25L, 5.097911466811016354623559941115413499164e-1L, 2.652548323318712945542359010064848876285L),
    TEST_c_c (cacosh, 1.0L, -0.25L, 5.097911466811016354623559941115413499164e-1L, -4.890443302710802929202843732146540079124e-1L),
    TEST_c_c (cacosh, -1.0L, -0.25L, 5.097911466811016354623559941115413499164e-1L, -2.652548323318712945542359010064848876285L),
    TEST_c_c (cacosh, 0x1.fp-10L, 1.0L, 8.813742198809567991336704287826445879025e-1L, 1.569458417435338878318763342108699202986L),
    TEST_c_c (cacosh, 0x1.fp-10L, -1.0L, 8.813742198809567991336704287826445879025e-1L, -1.569458417435338878318763342108699202986L),
    TEST_c_c (cacosh, -0x1.fp-10L, 1.0L, 8.813742198809567991336704287826445879025e-1L, 1.572134236154454360143880041170803681211L),
    TEST_c_c (cacosh, -0x1.fp-10L, -1.0L, 8.813742198809567991336704287826445879025e-1L, -1.572134236154454360143880041170803681211L),
    TEST_c_c (cacosh, 1.0L, 0x1.fp-10L, 4.350501469856803800217957402220976497152e-2L, 4.349129763101882771258049954181971959031e-2L),
    TEST_c_c (cacosh, -1.0L, 0x1.fp-10L, 4.350501469856803800217957402220976497152e-2L, 3.098101355958774410750062883737683164607L),
    TEST_c_c (cacosh, 1.0L, -0x1.fp-10L, 4.350501469856803800217957402220976497152e-2L, -4.349129763101882771258049954181971959031e-2L),
    TEST_c_c (cacosh, -1.0L, -0x1.fp-10L, 4.350501469856803800217957402220976497152e-2L, -3.098101355958774410750062883737683164607L),
    TEST_c_c (cacosh, 0x1.fp-30L, 1.0L, 8.813735870195430258081932989769495326854e-1L, 1.570796325518966635014803151387033957091L),
    TEST_c_c (cacosh, 0x1.fp-30L, -1.0L, 8.813735870195430258081932989769495326854e-1L, -1.570796325518966635014803151387033957091L),
    TEST_c_c (cacosh, -0x1.fp-30L, 1.0L, 8.813735870195430258081932989769495326854e-1L, 1.570796328070826603447840231892468927106L),
    TEST_c_c (cacosh, -0x1.fp-30L, -1.0L, 8.813735870195430258081932989769495326854e-1L, -1.570796328070826603447840231892468927106L),
    TEST_c_c (cacosh, 1.0L, 0x1.fp-30L, 4.247867098745151888768727039216644758847e-5L, 4.247867097467650115899790787875186617316e-5L),
    TEST_c_c (cacosh, -1.0L, 0x1.fp-30L, 4.247867098745151888768727039216644758847e-5L, 3.141550174918818561961484385371624132331L),
    TEST_c_c (cacosh, 1.0L, -0x1.fp-30L, 4.247867098745151888768727039216644758847e-5L, -4.247867097467650115899790787875186617316e-5L),
    TEST_c_c (cacosh, -1.0L, -0x1.fp-30L, 4.247867098745151888768727039216644758847e-5L, -3.141550174918818561961484385371624132331L),
    TEST_c_c (cacosh, 0x1.fp-100L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691638670687364L),
    TEST_c_c (cacosh, 0x1.fp-100L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691638670687364L),
    TEST_c_c (cacosh, -0x1.fp-100L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691640832196834L),
    TEST_c_c (cacosh, -0x1.fp-100L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691640832196834L),
    TEST_c_c (cacosh, 1.0L, 0x1.fp-100L, 1.236292038260260888664514866457202186027e-15L, 1.236292038260260888664514866456887257525e-15L),
    TEST_c_c (cacosh, -1.0L, 0x1.fp-100L, 1.236292038260260888664514866457202186027e-15L, 3.141592653589792002170605123018614219682L),
    TEST_c_c (cacosh, 1.0L, -0x1.fp-100L, 1.236292038260260888664514866457202186027e-15L, -1.236292038260260888664514866456887257525e-15L),
    TEST_c_c (cacosh, -1.0L, -0x1.fp-100L, 1.236292038260260888664514866457202186027e-15L, -3.141592653589792002170605123018614219682L),
    TEST_c_c (cacosh, 0x1.fp-129L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691639751442097L),
    TEST_c_c (cacosh, 0x1.fp-129L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691639751442097L),
    TEST_c_c (cacosh, -0x1.fp-129L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691639751442101L),
    TEST_c_c (cacosh, -0x1.fp-129L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691639751442101L),
    TEST_c_c (cacosh, 1.0L, 0x1.fp-129L, 5.335635276982233498398987585285818977933e-20L, 5.335635276982233498398987585285818977930e-20L),
    TEST_c_c (cacosh, -1.0L, 0x1.fp-129L, 5.335635276982233498398987585285818977933e-20L, 3.141592653589793238409287030509680549213L),
    TEST_c_c (cacosh, 1.0L, -0x1.fp-129L, 5.335635276982233498398987585285818977933e-20L, -5.335635276982233498398987585285818977930e-20L),
    TEST_c_c (cacosh, -1.0L, -0x1.fp-129L, 5.335635276982233498398987585285818977933e-20L, -3.141592653589793238409287030509680549213L),
#ifndef TEST_FLOAT
    TEST_c_c (cacosh, 0x1.fp-1000L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-1000L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1000L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1000L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 1.0L, 0x1.fp-1000L, 4.252291453851660175550490409247739011867e-151L, 4.252291453851660175550490409247739011867e-151L),
    TEST_c_c (cacosh, -1.0L, 0x1.fp-1000L, 4.252291453851660175550490409247739011867e-151L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 1.0L, -0x1.fp-1000L, 4.252291453851660175550490409247739011867e-151L, -4.252291453851660175550490409247739011867e-151L),
    TEST_c_c (cacosh, -1.0L, -0x1.fp-1000L, 4.252291453851660175550490409247739011867e-151L, -3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0x1.fp-1025L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-1025L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 1.0L, 0x1.fp-1025L, 7.340879205566679497036857179189356754017e-155L, 7.340879205566679497036857179189356754017e-155L),
    TEST_c_c (cacosh, -1.0L, 0x1.fp-1025L, 7.340879205566679497036857179189356754017e-155L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 1.0L, -0x1.fp-1025L, 7.340879205566679497036857179189356754017e-155L, -7.340879205566679497036857179189356754017e-155L),
    TEST_c_c (cacosh, -1.0L, -0x1.fp-1025L, 7.340879205566679497036857179189356754017e-155L, -3.141592653589793238462643383279502884197L),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacosh, 0x1.fp-10000L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-10000L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-10000L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-10000L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 1.0L, 0x1.fp-10000L, 9.854680208706673586644342922051388714633e-1506L, 9.854680208706673586644342922051388714633e-1506L),
    TEST_c_c (cacosh, -1.0L, 0x1.fp-10000L, 9.854680208706673586644342922051388714633e-1506L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 1.0L, -0x1.fp-10000L, 9.854680208706673586644342922051388714633e-1506L, -9.854680208706673586644342922051388714633e-1506L),
    TEST_c_c (cacosh, -1.0L, -0x1.fp-10000L, 9.854680208706673586644342922051388714633e-1506L, -3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0x1.fp-16385L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-16385L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, 1.0L, 8.813735870195430252326093249797923090282e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, -1.0L, 8.813735870195430252326093249797923090282e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 1.0L, 0x1.fp-16385L, 9.023632056840860275214893047597614177639e-2467L, 9.023632056840860275214893047597614177639e-2467L),
    TEST_c_c (cacosh, -1.0L, 0x1.fp-16385L, 9.023632056840860275214893047597614177639e-2467L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 1.0L, -0x1.fp-16385L, 9.023632056840860275214893047597614177639e-2467L, -9.023632056840860275214893047597614177639e-2467L),
    TEST_c_c (cacosh, -1.0L, -0x1.fp-16385L, 9.023632056840860275214893047597614177639e-2467L, -3.141592653589793238462643383279502884197L),
#endif
    TEST_c_c (cacosh, 0x1p-23L, 0x1.000002p0L, 8.813736713132400470205730751186547909968e-1L, 1.570796242501204621739026081991856762292L),
    TEST_c_c (cacosh, 0x1p-23L, -0x1.000002p0L, 8.813736713132400470205730751186547909968e-1L, -1.570796242501204621739026081991856762292L),
    TEST_c_c (cacosh, -0x1p-23L, 0x1.000002p0L, 8.813736713132400470205730751186547909968e-1L, 1.570796411088588616723617301287646121905L),
    TEST_c_c (cacosh, -0x1p-23L, -0x1.000002p0L, 8.813736713132400470205730751186547909968e-1L, -1.570796411088588616723617301287646121905L),
    TEST_c_c (cacosh, 0x1.000002p0L, 0x1p-23L, 5.364668491573609633134147164031476452679e-4L, 2.222118384408546368406374049167636760903e-4L),
    TEST_c_c (cacosh, -0x1.000002p0L, 0x1p-23L, 5.364668491573609633134147164031476452679e-4L, 3.141370441751352383825802745874586120521L),
    TEST_c_c (cacosh, 0x1.000002p0L, -0x1p-23L, 5.364668491573609633134147164031476452679e-4L, -2.222118384408546368406374049167636760903e-4L),
    TEST_c_c (cacosh, -0x1.000002p0L, -0x1p-23L, 5.364668491573609633134147164031476452679e-4L, -3.141370441751352383825802745874586120521L),
    TEST_c_c (cacosh, 0x1.fp-129L, 0x1.000002p0L, 8.813736713132375348727889167749389235161e-1L, 1.570796326794896619231321691639751442097L),
    TEST_c_c (cacosh, 0x1.fp-129L, -0x1.000002p0L, 8.813736713132375348727889167749389235161e-1L, -1.570796326794896619231321691639751442097L),
    TEST_c_c (cacosh, -0x1.fp-129L, 0x1.000002p0L, 8.813736713132375348727889167749389235161e-1L, 1.570796326794896619231321691639751442101L),
    TEST_c_c (cacosh, -0x1.fp-129L, -0x1.000002p0L, 8.813736713132375348727889167749389235161e-1L, -1.570796326794896619231321691639751442101L),
    TEST_c_c (cacosh, 0x1.000002p0L, 0x1.fp-129L, 4.882812451493617206486388134172712975070e-4L, 5.830451806317544230969669308596361881467e-36L),
    TEST_c_c (cacosh, -0x1.000002p0L, 0x1.fp-129L, 4.882812451493617206486388134172712975070e-4L, 3.141592653589793238462643383279502878367L),
    TEST_c_c (cacosh, 0x1.000002p0L, -0x1.fp-129L, 4.882812451493617206486388134172712975070e-4L, -5.830451806317544230969669308596361881467e-36L),
    TEST_c_c (cacosh, -0x1.000002p0L, -0x1.fp-129L, 4.882812451493617206486388134172712975070e-4L, -3.141592653589793238462643383279502878367L),
    TEST_c_c (cacosh, 0.0L, 0x1.000002p0L, 8.813736713132375348727889167749389235161e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x1.000002p0L, 8.813736713132375348727889167749389235161e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x1.000002p0L, 8.813736713132375348727889167749389235161e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x1.000002p0L, 8.813736713132375348727889167749389235161e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.000002p0L, 0.0L, 4.882812451493617206486388134172712975070e-4L, 0.0L),
    TEST_c_c (cacosh, -0x1.000002p0L, 0.0L, 4.882812451493617206486388134172712975070e-4L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0x1.000002p0L, -0.0L, 4.882812451493617206486388134172712975070e-4L, -0.0L),
    TEST_c_c (cacosh, -0x1.000002p0L, -0.0L, 4.882812451493617206486388134172712975070e-4L, -3.141592653589793238462643383279502884197L),
#ifndef TEST_FLOAT
    TEST_c_c (cacosh, 0x1p-52L, 0x1.0000000000001p0L, 8.813735870195431822418551933572982483664e-1L, 1.570796326794896462222075823262262934288L),
    TEST_c_c (cacosh, 0x1p-52L, -0x1.0000000000001p0L, 8.813735870195431822418551933572982483664e-1L, -1.570796326794896462222075823262262934288L),
    TEST_c_c (cacosh, -0x1p-52L, 0x1.0000000000001p0L, 8.813735870195431822418551933572982483664e-1L, 1.570796326794896776240567560017239949909L),
    TEST_c_c (cacosh, -0x1p-52L, -0x1.0000000000001p0L, 8.813735870195431822418551933572982483664e-1L, -1.570796326794896776240567560017239949909L),
    TEST_c_c (cacosh, 0x1.0000000000001p0L, 0x1p-52L, 2.315303644582684770975188768022139415020e-8L, 9.590301705980041385828904092662391018164e-9L),
    TEST_c_c (cacosh, -0x1.0000000000001p0L, 0x1p-52L, 2.315303644582684770975188768022139415020e-8L, 3.141592643999491532482601997450598791535L),
    TEST_c_c (cacosh, 0x1.0000000000001p0L, -0x1p-52L, 2.315303644582684770975188768022139415020e-8L, -9.590301705980041385828904092662391018164e-9L),
    TEST_c_c (cacosh, -0x1.0000000000001p0L, -0x1p-52L, 2.315303644582684770975188768022139415020e-8L, -3.141592643999491532482601997450598791535L),
    TEST_c_c (cacosh, 0x1.fp-1025L, 0x1.0000000000001p0L, 8.813735870195431822418551933572895326024e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-1025L, -0x1.0000000000001p0L, 8.813735870195431822418551933572895326024e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, 0x1.0000000000001p0L, 8.813735870195431822418551933572895326024e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, -0x1.0000000000001p0L, 8.813735870195431822418551933572895326024e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.0000000000001p0L, 0x1.fp-1025L, 2.107342425544701550354780375182800088393e-8L, 2.557178503953494342609835913586108008322e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, -0x1.0000000000001p0L, 0x1.fp-1025L, 2.107342425544701550354780375182800088393e-8L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0x1.0000000000001p0L, -0x1.fp-1025L, 2.107342425544701550354780375182800088393e-8L, -2.557178503953494342609835913586108008322e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, -0x1.0000000000001p0L, -0x1.fp-1025L, 2.107342425544701550354780375182800088393e-8L, -3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0.0L, 0x1.0000000000001p0L, 8.813735870195431822418551933572895326024e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x1.0000000000001p0L, 8.813735870195431822418551933572895326024e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x1.0000000000001p0L, 8.813735870195431822418551933572895326024e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x1.0000000000001p0L, 8.813735870195431822418551933572895326024e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.0000000000001p0L, 0.0L, 2.107342425544701550354780375182800088393e-8L, 0.0L),
    TEST_c_c (cacosh, -0x1.0000000000001p0L, 0.0L, 2.107342425544701550354780375182800088393e-8L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0x1.0000000000001p0L, -0.0L, 2.107342425544701550354780375182800088393e-8L, -0.0L),
    TEST_c_c (cacosh, -0x1.0000000000001p0L, -0.0L, 2.107342425544701550354780375182800088393e-8L, -3.141592653589793238462643383279502884197L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (cacosh, 0x1p-63L, 0x1.0000000000000002p0L, 8.813735870195430253092739958139610131001e-1L, 1.570796326794896619154657020805582738031L),
    TEST_c_c (cacosh, 0x1p-63L, -0x1.0000000000000002p0L, 8.813735870195430253092739958139610131001e-1L, -1.570796326794896619154657020805582738031L),
    TEST_c_c (cacosh, -0x1p-63L, 0x1.0000000000000002p0L, 8.813735870195430253092739958139610131001e-1L, 1.570796326794896619307986362473920146166L),
    TEST_c_c (cacosh, -0x1p-63L, -0x1.0000000000000002p0L, 8.813735870195430253092739958139610131001e-1L, -1.570796326794896619307986362473920146166L),
    TEST_c_c (cacosh, 0x1.0000000000000002p0L, 0x1p-63L, 5.116146586219826555037807251857670783420e-10L, 2.119177303101063432592523199680782317447e-10L),
    TEST_c_c (cacosh, -0x1.0000000000000002p0L, 0x1p-63L, 5.116146586219826555037807251857670783420e-10L, 3.141592653377875508152537040020250564229L),
    TEST_c_c (cacosh, 0x1.0000000000000002p0L, -0x1p-63L, 5.116146586219826555037807251857670783420e-10L, -2.119177303101063432592523199680782317447e-10L),
    TEST_c_c (cacosh, -0x1.0000000000000002p0L, -0x1p-63L, 5.116146586219826555037807251857670783420e-10L, -3.141592653377875508152537040020250564229L),
# if LDBL_MIN_EXP <= -16381
    TEST_c_c (cacosh, 0x1.fp-16385L, 0x1.0000000000000002p0L, 8.813735870195430253092739958139610130980e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-16385L, -0x1.0000000000000002p0L, 8.813735870195430253092739958139610130980e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, 0x1.0000000000000002p0L, 8.813735870195430253092739958139610130980e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, -0x1.0000000000000002p0L, 8.813735870195430253092739958139610130980e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.0000000000000002p0L, 0x1.fp-16385L, 4.656612873077392578082927418388212703712e-10L, 1.748608650034385653922359120438227480943e-4923L),
    TEST_c_c (cacosh, -0x1.0000000000000002p0L, 0x1.fp-16385L, 4.656612873077392578082927418388212703712e-10L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0x1.0000000000000002p0L, -0x1.fp-16385L, 4.656612873077392578082927418388212703712e-10L, -1.748608650034385653922359120438227480943e-4923L),
    TEST_c_c (cacosh, -0x1.0000000000000002p0L, -0x1.fp-16385L, 4.656612873077392578082927418388212703712e-10L, -3.141592653589793238462643383279502884197L),
# endif
    TEST_c_c (cacosh, 0.0L, 0x1.0000000000000002p0L, 8.813735870195430253092739958139610130980e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x1.0000000000000002p0L, 8.813735870195430253092739958139610130980e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x1.0000000000000002p0L, 8.813735870195430253092739958139610130980e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x1.0000000000000002p0L, 8.813735870195430253092739958139610130980e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.0000000000000002p0L, 0.0L, 4.656612873077392578082927418388212703712e-10L, 0.0L),
    TEST_c_c (cacosh, -0x1.0000000000000002p0L, 0.0L, 4.656612873077392578082927418388212703712e-10L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0x1.0000000000000002p0L, -0.0L, 4.656612873077392578082927418388212703712e-10L, -0.0L),
    TEST_c_c (cacosh, -0x1.0000000000000002p0L, -0.0L, 4.656612873077392578082927418388212703712e-10L, -3.141592653589793238462643383279502884197L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (cacosh, 0x1p-106L, 0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, 1.570796326794896619231321691639742726335L),
    TEST_c_c (cacosh, 0x1p-106L, -0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, -1.570796326794896619231321691639742726335L),
    TEST_c_c (cacosh, -0x1p-106L, 0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, 1.570796326794896619231321691639760157863L),
    TEST_c_c (cacosh, -0x1p-106L, -0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, -1.570796326794896619231321691639760157863L),
    TEST_c_c (cacosh, 0x1.000000000000000000000000008p0L, 0x1p-106L, 2.285028863093221674154232933662774454211e-16L, 5.394221422390606848017034778914096659726e-17L),
    TEST_c_c (cacosh, -0x1.000000000000000000000000008p0L, 0x1p-106L, 2.285028863093221674154232933662774454211e-16L, 3.141592653589793184520429159373434404027L),
    TEST_c_c (cacosh, 0x1.000000000000000000000000008p0L, -0x1p-106L, 2.285028863093221674154232933662774454211e-16L, -5.394221422390606848017034778914096659726e-17L),
    TEST_c_c (cacosh, -0x1.000000000000000000000000008p0L, -0x1p-106L, 2.285028863093221674154232933662774454211e-16L, -3.141592653589793184520429159373434404027L),
    TEST_c_c (cacosh, 0x1.fp-1025L, 0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-1025L, -0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, 0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, -0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.000000000000000000000000008p0L, 0x1.fp-1025L, 2.220446049250313080847263336181636063482e-16L, 2.426922623448365473354662093431821897807e-293L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, -0x1.000000000000000000000000008p0L, 0x1.fp-1025L, 2.220446049250313080847263336181636063482e-16L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0x1.000000000000000000000000008p0L, -0x1.fp-1025L, 2.220446049250313080847263336181636063482e-16L, -2.426922623448365473354662093431821897807e-293L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, -0x1.000000000000000000000000008p0L, -0x1.fp-1025L, 2.220446049250313080847263336181636063482e-16L, -3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0.0L, 0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x1.000000000000000000000000008p0L, 8.813735870195430252326093249798097405561e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.000000000000000000000000008p0L, 0.0L, 2.220446049250313080847263336181636063482e-16L, 0.0L),
    TEST_c_c (cacosh, -0x1.000000000000000000000000008p0L, 0.0L, 2.220446049250313080847263336181636063482e-16L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0x1.000000000000000000000000008p0L, -0.0L, 2.220446049250313080847263336181636063482e-16L, -0.0L),
    TEST_c_c (cacosh, -0x1.000000000000000000000000008p0L, -0.0L, 2.220446049250313080847263336181636063482e-16L, -3.141592653589793238462643383279502884197L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (cacosh, 0x1p-113L, 0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, 1.570796326794896619231321691639751374007L),
    TEST_c_c (cacosh, 0x1p-113L, -0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, -1.570796326794896619231321691639751374007L),
    TEST_c_c (cacosh, -0x1p-113L, 0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, 1.570796326794896619231321691639751510190L),
    TEST_c_c (cacosh, -0x1p-113L, -0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, -1.570796326794896619231321691639751510190L),
    TEST_c_c (cacosh, 0x1.0000000000000000000000000001p0L, 0x1p-113L, 2.019699255375255198156433710951064632386e-17L, 4.767863183742677481693563511435642755521e-18L),
    TEST_c_c (cacosh, -0x1.0000000000000000000000000001p0L, 0x1p-113L, 2.019699255375255198156433710951064632386e-17L, 3.141592653589793233694780199536825402504L),
    TEST_c_c (cacosh, 0x1.0000000000000000000000000001p0L, -0x1p-113L, 2.019699255375255198156433710951064632386e-17L, -4.767863183742677481693563511435642755521e-18L),
    TEST_c_c (cacosh, -0x1.0000000000000000000000000001p0L, -0x1p-113L, 2.019699255375255198156433710951064632386e-17L, -3.141592653589793233694780199536825402504L),
    TEST_c_c (cacosh, 0x1.fp-16385L, 0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-16385L, -0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, 0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, -0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.0000000000000000000000000001p0L, 0x1.fp-16385L, 1.962615573354718824241727964954454332780e-17L, 4.148847925325683229178506116378864361396e-4916L),
    TEST_c_c (cacosh, -0x1.0000000000000000000000000001p0L, 0x1.fp-16385L, 1.962615573354718824241727964954454332780e-17L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0x1.0000000000000000000000000001p0L, -0x1.fp-16385L, 1.962615573354718824241727964954454332780e-17L, -4.148847925325683229178506116378864361396e-4916L),
    TEST_c_c (cacosh, -0x1.0000000000000000000000000001p0L, -0x1.fp-16385L, 1.962615573354718824241727964954454332780e-17L, -3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0.0L, 0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x1.0000000000000000000000000001p0L, 8.813735870195430252326093249797924452120e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.0000000000000000000000000001p0L, 0.0L, 1.962615573354718824241727964954454332780e-17L, 0.0L),
    TEST_c_c (cacosh, -0x1.0000000000000000000000000001p0L, 0.0L, 1.962615573354718824241727964954454332780e-17L, 3.141592653589793238462643383279502884197L),
    TEST_c_c (cacosh, 0x1.0000000000000000000000000001p0L, -0.0L, 1.962615573354718824241727964954454332780e-17L, -0.0L),
    TEST_c_c (cacosh, -0x1.0000000000000000000000000001p0L, -0.0L, 1.962615573354718824241727964954454332780e-17L, -3.141592653589793238462643383279502884197L),
#endif
    TEST_c_c (cacosh, 0x1p-23L, 0x0.ffffffp0L, 8.813735448726963984495965873956465777250e-1L, 1.570796242501197085295336701632142060969L),
    TEST_c_c (cacosh, 0x1p-23L, -0x0.ffffffp0L, 8.813735448726963984495965873956465777250e-1L, -1.570796242501197085295336701632142060969L),
    TEST_c_c (cacosh, -0x1p-23L, 0x0.ffffffp0L, 8.813735448726963984495965873956465777250e-1L, 1.570796411088596153167306681647360823228L),
    TEST_c_c (cacosh, -0x1p-23L, -0x0.ffffffp0L, 8.813735448726963984495965873956465777250e-1L, -1.570796411088596153167306681647360823228L),
    TEST_c_c (cacosh, 0x0.ffffffp0L, 0x1p-23L, 2.714321200917194650737217746780928423385e-4L, 4.391863861910487109445187743978204002614e-4L),
    TEST_c_c (cacosh, -0x0.ffffffp0L, 0x1p-23L, 2.714321200917194650737217746780928423385e-4L, 3.141153467203602189751698864505105063797L),
    TEST_c_c (cacosh, 0x0.ffffffp0L, -0x1p-23L, 2.714321200917194650737217746780928423385e-4L, -4.391863861910487109445187743978204002614e-4L),
    TEST_c_c (cacosh, -0x0.ffffffp0L, -0x1p-23L, 2.714321200917194650737217746780928423385e-4L, -3.141153467203602189751698864505105063797L),
    TEST_c_c (cacosh, 0x1.fp-129L, 0x0.ffffffp0L, 8.813735448726938863015878255140556727969e-1L, 1.570796326794896619231321691639751442097L),
    TEST_c_c (cacosh, 0x1.fp-129L, -0x0.ffffffp0L, 8.813735448726938863015878255140556727969e-1L, -1.570796326794896619231321691639751442097L),
    TEST_c_c (cacosh, -0x1.fp-129L, 0x0.ffffffp0L, 8.813735448726938863015878255140556727969e-1L, 1.570796326794896619231321691639751442101L),
    TEST_c_c (cacosh, -0x1.fp-129L, -0x0.ffffffp0L, 8.813735448726938863015878255140556727969e-1L, -1.570796326794896619231321691639751442101L),
    TEST_c_c (cacosh, 0x0.ffffffp0L, 0x1.fp-129L, 8.245504387859737323891843603996428534945e-36L, 3.452669847162035876032494826387364972849e-4L),
    TEST_c_c (cacosh, -0x0.ffffffp0L, 0x1.fp-129L, 8.245504387859737323891843603996428534945e-36L, 3.141247386605077034875040133796864147700L),
    TEST_c_c (cacosh, 0x0.ffffffp0L, -0x1.fp-129L, 8.245504387859737323891843603996428534945e-36L, -3.452669847162035876032494826387364972849e-4L),
    TEST_c_c (cacosh, -0x0.ffffffp0L, -0x1.fp-129L, 8.245504387859737323891843603996428534945e-36L, -3.141247386605077034875040133796864147700L),
    TEST_c_c (cacosh, 0.0L, 0x0.ffffffp0L, 8.813735448726938863015878255140556727969e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x0.ffffffp0L, 8.813735448726938863015878255140556727969e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x0.ffffffp0L, 8.813735448726938863015878255140556727969e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x0.ffffffp0L, 8.813735448726938863015878255140556727969e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x0.ffffffp0L, 0.0L, 0.0L, 3.452669847162035876032494826387364972849e-4L),
    TEST_c_c (cacosh, -0x0.ffffffp0L, 0.0L, 0.0L, 3.141247386605077034875040133796864147700L),
    TEST_c_c (cacosh, 0x0.ffffffp0L, -0.0L, 0.0L, -3.452669847162035876032494826387364972849e-4L),
    TEST_c_c (cacosh, -0x0.ffffffp0L, -0.0L, 0.0L, -3.141247386605077034875040133796864147700L),
    TEST_c_c (cacosh, 0x1p-23L, 0.5L, 4.812118250596059896127318386463676808798e-1L, 1.570796220170866625230343643673321150378L),
    TEST_c_c (cacosh, 0x1p-23L, -0.5L, 4.812118250596059896127318386463676808798e-1L, -1.570796220170866625230343643673321150378L),
    TEST_c_c (cacosh, -0x1p-23L, 0.5L, 4.812118250596059896127318386463676808798e-1L, 1.570796433418926613232299739606181733819L),
    TEST_c_c (cacosh, -0x1p-23L, -0.5L, 4.812118250596059896127318386463676808798e-1L, -1.570796433418926613232299739606181733819L),
    TEST_c_c (cacosh, 0.5L, 0x1p-23L, 1.376510308240943236356088341381173571841e-7L, 1.047197551196603215914744618665204532273L),
    TEST_c_c (cacosh, -0.5L, 0x1p-23L, 1.376510308240943236356088341381173571841e-7L, 2.094395102393190022547898764614298351924L),
    TEST_c_c (cacosh, 0.5L, -0x1p-23L, 1.376510308240943236356088341381173571841e-7L, -1.047197551196603215914744618665204532273L),
    TEST_c_c (cacosh, -0.5L, -0x1p-23L, 1.376510308240943236356088341381173571841e-7L, -2.094395102393190022547898764614298351924L),
    TEST_c_c (cacosh, 0x1.fp-129L, 0.5L, 4.812118250596034474977589134243684231352e-1L, 1.570796326794896619231321691639751442096L),
    TEST_c_c (cacosh, 0x1.fp-129L, -0.5L, 4.812118250596034474977589134243684231352e-1L, -1.570796326794896619231321691639751442096L),
    TEST_c_c (cacosh, -0x1.fp-129L, 0.5L, 4.812118250596034474977589134243684231352e-1L, 1.570796326794896619231321691639751442101L),
    TEST_c_c (cacosh, -0x1.fp-129L, -0.5L, 4.812118250596034474977589134243684231352e-1L, -1.570796326794896619231321691639751442101L),
    TEST_c_c (cacosh, 0.5L, 0x1.fp-129L, 3.287317402534702257036015056278368274737e-39L, 1.047197551196597746154214461093167628066L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, -0.5L, 0x1.fp-129L, 3.287317402534702257036015056278368274737e-39L, 2.094395102393195492308428922186335256131L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, 0.5L, -0x1.fp-129L, 3.287317402534702257036015056278368274737e-39L, -1.047197551196597746154214461093167628066L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, -0.5L, -0x1.fp-129L, 3.287317402534702257036015056278368274737e-39L, -2.094395102393195492308428922186335256131L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, 0x1p-23L, 0x1p-23L, 1.192092895507818146886315028596704749235e-7L, 1.570796207585607068450636380271254316214L),
    TEST_c_c (cacosh, 0x1p-23L, -0x1p-23L, 1.192092895507818146886315028596704749235e-7L, -1.570796207585607068450636380271254316214L),
    TEST_c_c (cacosh, -0x1p-23L, 0x1p-23L, 1.192092895507818146886315028596704749235e-7L, 1.570796446004186170012007003008248567984L),
    TEST_c_c (cacosh, -0x1p-23L, -0x1p-23L, 1.192092895507818146886315028596704749235e-7L, -1.570796446004186170012007003008248567984L),
    TEST_c_c (cacosh, 0x1.fp-129L, 0x1p-23L, 1.192092895507809676556842485683592032154e-7L, 1.570796326794896619231321691639751442096L),
    TEST_c_c (cacosh, 0x1.fp-129L, -0x1p-23L, 1.192092895507809676556842485683592032154e-7L, -1.570796326794896619231321691639751442096L),
    TEST_c_c (cacosh, -0x1.fp-129L, 0x1p-23L, 1.192092895507809676556842485683592032154e-7L, 1.570796326794896619231321691639751442101L),
    TEST_c_c (cacosh, -0x1.fp-129L, -0x1p-23L, 1.192092895507809676556842485683592032154e-7L, -1.570796326794896619231321691639751442101L),
    TEST_c_c (cacosh, 0x1p-23L, 0x1.fp-129L, 2.846900380897747786805634596726756660388e-39L, 1.570796207585607068449789347324000006847L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, -0x1p-23L, 0x1.fp-129L, 2.846900380897747786805634596726756660388e-39L, 1.570796446004186170012854035955502877351L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, 0x1p-23L, -0x1.fp-129L, 2.846900380897747786805634596726756660388e-39L, -1.570796207585607068449789347324000006847L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, -0x1p-23L, -0x1.fp-129L, 2.846900380897747786805634596726756660388e-39L, -1.570796446004186170012854035955502877351L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, 0.0L, 0x1p-23L, 1.192092895507809676556842485683592032154e-7L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x1p-23L, 1.192092895507809676556842485683592032154e-7L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x1p-23L, 1.192092895507809676556842485683592032154e-7L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x1p-23L, 1.192092895507809676556842485683592032154e-7L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1p-23L, 0.0L, 0.0L, 1.570796207585607068449789347324000006847L),
    TEST_c_c (cacosh, -0x1p-23L, 0.0L, 0.0L, 1.570796446004186170012854035955502877351L),
    TEST_c_c (cacosh, 0x1p-23L, -0.0L, 0.0L, -1.570796207585607068449789347324000006847L),
    TEST_c_c (cacosh, -0x1p-23L, -0.0L, 0.0L, -1.570796446004186170012854035955502877351L),
    TEST_c_c (cacosh, 0x1.fp-129L, 0x1.fp-129L, 2.846900380897727558361783801085126250967e-39L, 1.570796326794896619231321691639751442096L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, 0x1.fp-129L, -0x1.fp-129L, 2.846900380897727558361783801085126250967e-39L, -1.570796326794896619231321691639751442096L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, -0x1.fp-129L, 0x1.fp-129L, 2.846900380897727558361783801085126250967e-39L, 1.570796326794896619231321691639751442101L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (cacosh, -0x1.fp-129L, -0x1.fp-129L, 2.846900380897727558361783801085126250967e-39L, -1.570796326794896619231321691639751442101L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (cacosh, 0x1p-52L, 0x0.fffffffffffff8p0L, 8.813735870195429467279863907910458761820e-1L, 1.570796326794896462222075823262236786996L),
    TEST_c_c (cacosh, 0x1p-52L, -0x0.fffffffffffff8p0L, 8.813735870195429467279863907910458761820e-1L, -1.570796326794896462222075823262236786996L),
    TEST_c_c (cacosh, -0x1p-52L, 0x0.fffffffffffff8p0L, 8.813735870195429467279863907910458761820e-1L, 1.570796326794896776240567560017266097201L),
    TEST_c_c (cacosh, -0x1p-52L, -0x0.fffffffffffff8p0L, 8.813735870195429467279863907910458761820e-1L, -1.570796326794896776240567560017266097201L),
    TEST_c_c (cacosh, 0x0.fffffffffffff8p0L, 0x1p-52L, 1.171456840272878582596796205397918831268e-8L, 1.895456983915074112227925127005564372844e-8L),
    TEST_c_c (cacosh, -0x0.fffffffffffff8p0L, 0x1p-52L, 1.171456840272878582596796205397918831268e-8L, 3.141592634635223399311902261000251614142L),
    TEST_c_c (cacosh, 0x0.fffffffffffff8p0L, -0x1p-52L, 1.171456840272878582596796205397918831268e-8L, -1.895456983915074112227925127005564372844e-8L),
    TEST_c_c (cacosh, -0x0.fffffffffffff8p0L, -0x1p-52L, 1.171456840272878582596796205397918831268e-8L, -3.141592634635223399311902261000251614142L),
    TEST_c_c (cacosh, 0x1.fp-1025L, 0x0.fffffffffffff8p0L, 8.813735870195429467279863907910371604180e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-1025L, -0x0.fffffffffffff8p0L, 8.813735870195429467279863907910371604180e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, 0x0.fffffffffffff8p0L, 8.813735870195429467279863907910371604180e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, -0x0.fffffffffffff8p0L, 8.813735870195429467279863907910371604180e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x0.fffffffffffff8p0L, 0x1.fp-1025L, 3.616396521699973256461764099945789620604e-301L, 1.490116119384765638786343542550461592240e-8L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, -0x0.fffffffffffff8p0L, 0x1.fp-1025L, 3.616396521699973256461764099945789620604e-301L, 3.141592638688632044614986995416067458693L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, 0x0.fffffffffffff8p0L, -0x1.fp-1025L, 3.616396521699973256461764099945789620604e-301L, -1.490116119384765638786343542550461592240e-8L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, -0x0.fffffffffffff8p0L, -0x1.fp-1025L, 3.616396521699973256461764099945789620604e-301L, -3.141592638688632044614986995416067458693L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, 0.0L, 0x0.fffffffffffff8p0L, 8.813735870195429467279863907910371604180e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x0.fffffffffffff8p0L, 8.813735870195429467279863907910371604180e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x0.fffffffffffff8p0L, 8.813735870195429467279863907910371604180e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x0.fffffffffffff8p0L, 8.813735870195429467279863907910371604180e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x0.fffffffffffff8p0L, 0.0L, 0.0L, 1.490116119384765638786343542550461592240e-8L),
    TEST_c_c (cacosh, -0x0.fffffffffffff8p0L, 0.0L, 0.0L, 3.141592638688632044614986995416067458693L),
    TEST_c_c (cacosh, 0x0.fffffffffffff8p0L, -0.0L, 0.0L, -1.490116119384765638786343542550461592240e-8L),
    TEST_c_c (cacosh, -0x0.fffffffffffff8p0L, -0.0L, 0.0L, -3.141592638688632044614986995416067458693L),
#endif
    TEST_c_c (cacosh, 0x1p-52L, 0.5L, 4.812118250596034474977589134243772428682e-1L, 1.570796326794896420628589431857911675917L),
    TEST_c_c (cacosh, 0x1p-52L, -0.5L, 4.812118250596034474977589134243772428682e-1L, -1.570796326794896420628589431857911675917L),
    TEST_c_c (cacosh, -0x1p-52L, 0.5L, 4.812118250596034474977589134243772428682e-1L, 1.570796326794896817834053951421591208280L),
    TEST_c_c (cacosh, -0x1p-52L, -0.5L, 4.812118250596034474977589134243772428682e-1L, -1.570796326794896817834053951421591208280L),
    TEST_c_c (cacosh, 0.5L, 0x1p-52L, 2.563950248511418570403591756798643804971e-16L, 1.047197551196597746154214461093186605110L),
    TEST_c_c (cacosh, -0.5L, 0x1p-52L, 2.563950248511418570403591756798643804971e-16L, 2.094395102393195492308428922186316279087L),
    TEST_c_c (cacosh, 0.5L, -0x1p-52L, 2.563950248511418570403591756798643804971e-16L, -1.047197551196597746154214461093186605110L),
    TEST_c_c (cacosh, -0.5L, -0x1p-52L, 2.563950248511418570403591756798643804971e-16L, -2.094395102393195492308428922186316279087L),
#ifndef TEST_FLOAT
    TEST_c_c (cacosh, 0x1.fp-1025L, 0.5L, 4.812118250596034474977589134243684231352e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-1025L, -0.5L, 4.812118250596034474977589134243684231352e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, 0.5L, 4.812118250596034474977589134243684231352e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, -0.5L, 4.812118250596034474977589134243684231352e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.5L, 0x1.fp-1025L, 6.222508863508420569166420770843207333493e-309L, 1.047197551196597746154214461093167628066L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, -0.5L, 0x1.fp-1025L, 6.222508863508420569166420770843207333493e-309L, 2.094395102393195492308428922186335256131L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, 0.5L, -0x1.fp-1025L, 6.222508863508420569166420770843207333493e-309L, -1.047197551196597746154214461093167628066L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, -0.5L, -0x1.fp-1025L, 6.222508863508420569166420770843207333493e-309L, -2.094395102393195492308428922186335256131L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
    TEST_c_c (cacosh, 0x1p-52L, 0x1p-52L, 2.220446049250313080847263336181677117148e-16L, 1.570796326794896397186716766608443357372L),
    TEST_c_c (cacosh, 0x1p-52L, -0x1p-52L, 2.220446049250313080847263336181677117148e-16L, -1.570796326794896397186716766608443357372L),
    TEST_c_c (cacosh, -0x1p-52L, 0x1p-52L, 2.220446049250313080847263336181677117148e-16L, 1.570796326794896841275926616671059526825L),
    TEST_c_c (cacosh, -0x1p-52L, -0x1p-52L, 2.220446049250313080847263336181677117148e-16L, -1.570796326794896841275926616671059526825L),
#ifndef TEST_FLOAT
    TEST_c_c (cacosh, 0x1.fp-1025L, 0x1p-52L, 2.220446049250313080847263336181622378926e-16L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-1025L, -0x1p-52L, 2.220446049250313080847263336181622378926e-16L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, 0x1p-52L, 2.220446049250313080847263336181622378926e-16L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, -0x1p-52L, 2.220446049250313080847263336181622378926e-16L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1p-52L, 0x1.fp-1025L, 5.388850751072128349671657362289548938458e-309L, 1.570796326794896397186716766608443357372L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, -0x1p-52L, 0x1.fp-1025L, 5.388850751072128349671657362289548938458e-309L, 1.570796326794896841275926616671059526825L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, 0x1p-52L, -0x1.fp-1025L, 5.388850751072128349671657362289548938458e-309L, -1.570796326794896397186716766608443357372L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, -0x1p-52L, -0x1.fp-1025L, 5.388850751072128349671657362289548938458e-309L, -1.570796326794896841275926616671059526825L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
    TEST_c_c (cacosh, 0.0L, 0x1p-52L, 2.220446049250313080847263336181622378926e-16L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x1p-52L, 2.220446049250313080847263336181622378926e-16L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x1p-52L, 2.220446049250313080847263336181622378926e-16L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x1p-52L, 2.220446049250313080847263336181622378926e-16L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1p-52L, 0.0L, 0.0L, 1.570796326794896397186716766608443357372L),
    TEST_c_c (cacosh, -0x1p-52L, 0.0L, 0.0L, 1.570796326794896841275926616671059526825L),
    TEST_c_c (cacosh, 0x1p-52L, -0.0L, 0.0L, -1.570796326794896397186716766608443357372L),
    TEST_c_c (cacosh, -0x1p-52L, -0.0L, 0.0L, -1.570796326794896841275926616671059526825L),
#ifndef TEST_FLOAT
    TEST_c_c (cacosh, 0x1.fp-1025L, 0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, 0x1.fp-1025L, -0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, -0x1.fp-1025L, 0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, -0x1.fp-1025L, -0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (cacosh, 0x1p-63L, 0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569937e-1L, 1.570796326794896619154657020805582738025L),
    TEST_c_c (cacosh, 0x1p-63L, -0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569937e-1L, -1.570796326794896619154657020805582738025L),
    TEST_c_c (cacosh, -0x1p-63L, 0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569937e-1L, 1.570796326794896619307986362473920146173L),
    TEST_c_c (cacosh, -0x1p-63L, -0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569937e-1L, -1.570796326794896619307986362473920146173L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffp0L, 0x1p-63L, 2.588578361325995866221775673638805081337e-10L, 4.188407771167967636741951941902992986043e-10L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffp0L, 0x1p-63L, 2.588578361325995866221775673638805081337e-10L, 3.141592653170952461345846619605307690007L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffp0L, -0x1p-63L, 2.588578361325995866221775673638805081337e-10L, -4.188407771167967636741951941902992986043e-10L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffp0L, -0x1p-63L, 2.588578361325995866221775673638805081337e-10L, -3.141592653170952461345846619605307690007L),
# if LDBL_MIN_EXP <= -16381
    TEST_c_c (cacosh, 0x1.fp-16385L, 0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569917e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-16385L, -0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569917e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, 0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569917e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, -0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569917e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffp0L, 0x1.fp-16385L, 2.472906068161537187835415298076415423459e-4923L, 3.292722539913596233371825532007990724506e-10L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffp0L, 0x1.fp-16385L, 2.472906068161537187835415298076415423459e-4923L, 3.141592653260520984471283759942320330996L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffp0L, -0x1.fp-16385L, 2.472906068161537187835415298076415423459e-4923L, -3.292722539913596233371825532007990724506e-10L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffp0L, -0x1.fp-16385L, 2.472906068161537187835415298076415423459e-4923L, -3.141592653260520984471283759942320330996L),
# endif
    TEST_c_c (cacosh, 0.0L, 0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569917e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569917e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569917e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x0.ffffffffffffffffp0L, 8.813735870195430251942769895627079569917e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffp0L, 0.0L, 0.0L, 3.292722539913596233371825532007990724506e-10L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffp0L, 0.0L, 0.0L, 3.141592653260520984471283759942320330996L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffp0L, -0.0L, 0.0L, -3.292722539913596233371825532007990724506e-10L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffp0L, -0.0L, 0.0L, -3.141592653260520984471283759942320330996L),
#endif
    TEST_c_c (cacosh, 0x1p-63L, 0.5L, 4.812118250596034474977589134243684231373e-1L, 1.570796326794896619134347701278529840650L),
    TEST_c_c (cacosh, 0x1p-63L, -0.5L, 4.812118250596034474977589134243684231373e-1L, -1.570796326794896619134347701278529840650L),
    TEST_c_c (cacosh, -0x1p-63L, 0.5L, 4.812118250596034474977589134243684231373e-1L, 1.570796326794896619328295682000973043547L),
    TEST_c_c (cacosh, -0x1p-63L, -0.5L, 4.812118250596034474977589134243684231373e-1L, -1.570796326794896619328295682000973043547L),
    TEST_c_c (cacosh, 0.5L, 0x1p-63L, 1.251928832280966098829878787499365228659e-19L, 1.047197551196597746154214461093167628070L),
    TEST_c_c (cacosh, -0.5L, 0x1p-63L, 1.251928832280966098829878787499365228659e-19L, 2.094395102393195492308428922186335256127L),
    TEST_c_c (cacosh, 0.5L, -0x1p-63L, 1.251928832280966098829878787499365228659e-19L, -1.047197551196597746154214461093167628070L),
    TEST_c_c (cacosh, -0.5L, -0x1p-63L, 1.251928832280966098829878787499365228659e-19L, -2.094395102393195492308428922186335256127L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacosh, 0x1.fp-16385L, 0.5L, 4.812118250596034474977589134243684231352e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-16385L, -0.5L, 4.812118250596034474977589134243684231352e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, 0.5L, 4.812118250596034474977589134243684231352e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, -0.5L, 4.812118250596034474977589134243684231352e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.5L, 0x1.fp-16385L, 9.402257155670418112159334875199821342890e-4933L, 1.047197551196597746154214461093167628066L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, -0.5L, 0x1.fp-16385L, 9.402257155670418112159334875199821342890e-4933L, 2.094395102393195492308428922186335256131L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, 0.5L, -0x1.fp-16385L, 9.402257155670418112159334875199821342890e-4933L, -1.047197551196597746154214461093167628066L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, -0.5L, -0x1.fp-16385L, 9.402257155670418112159334875199821342890e-4933L, -2.094395102393195492308428922186335256131L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (cacosh, 0x1p-63L, 0x1p-63L, 1.084202172485504434007452800869941711430e-19L, 1.570796326794896619122901474391200998698L),
    TEST_c_c (cacosh, 0x1p-63L, -0x1p-63L, 1.084202172485504434007452800869941711430e-19L, -1.570796326794896619122901474391200998698L),
    TEST_c_c (cacosh, -0x1p-63L, 0x1p-63L, 1.084202172485504434007452800869941711430e-19L, 1.570796326794896619339741908888301885499L),
    TEST_c_c (cacosh, -0x1p-63L, -0x1p-63L, 1.084202172485504434007452800869941711430e-19L, -1.570796326794896619339741908888301885499L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacosh, 0x1.fp-16385L, 0x1p-63L, 1.084202172485504434007452800869941711424e-19L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-16385L, -0x1p-63L, 1.084202172485504434007452800869941711424e-19L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, 0x1p-63L, 1.084202172485504434007452800869941711424e-19L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, -0x1p-63L, 1.084202172485504434007452800869941711424e-19L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1p-63L, 0x1.fp-16385L, 8.142593549724601460479922838826119584465e-4933L, 1.570796326794896619122901474391200998698L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, -0x1p-63L, 0x1.fp-16385L, 8.142593549724601460479922838826119584465e-4933L, 1.570796326794896619339741908888301885499L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, 0x1p-63L, -0x1.fp-16385L, 8.142593549724601460479922838826119584465e-4933L, -1.570796326794896619122901474391200998698L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, -0x1p-63L, -0x1.fp-16385L, 8.142593549724601460479922838826119584465e-4933L, -1.570796326794896619339741908888301885499L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (cacosh, 0.0L, 0x1p-63L, 1.084202172485504434007452800869941711424e-19L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x1p-63L, 1.084202172485504434007452800869941711424e-19L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x1p-63L, 1.084202172485504434007452800869941711424e-19L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x1p-63L, 1.084202172485504434007452800869941711424e-19L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1p-63L, 0.0L, 0.0L, 1.570796326794896619122901474391200998698L),
    TEST_c_c (cacosh, -0x1p-63L, 0.0L, 0.0L, 1.570796326794896619339741908888301885499L),
    TEST_c_c (cacosh, 0x1p-63L, -0.0L, 0.0L, -1.570796326794896619122901474391200998698L),
    TEST_c_c (cacosh, -0x1p-63L, -0.0L, 0.0L, -1.570796326794896619339741908888301885499L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacosh, 0x1.fp-16385L, 0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, 0x1.fp-16385L, -0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, -0x1.fp-16385L, 0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, -0x1.fp-16385L, -0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (cacosh, 0x1p-105L, 0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, 1.570796326794896619231321691639734010571L),
    TEST_c_c (cacosh, 0x1p-105L, -0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, -1.570796326794896619231321691639734010571L),
    TEST_c_c (cacosh, -0x1p-105L, 0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, 1.570796326794896619231321691639768873627L),
    TEST_c_c (cacosh, -0x1p-105L, -0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, -1.570796326794896619231321691639768873627L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffcp0L, 0x1p-105L, 1.234330349600789959989661887846005659983e-16L, 1.997188458999618357060632049675702684031e-16L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffcp0L, 0x1p-105L, 1.234330349600789959989661887846005659983e-16L, 3.141592653589793038743797483317667178134L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffcp0L, -0x1p-105L, 1.234330349600789959989661887846005659983e-16L, -1.997188458999618357060632049675702684031e-16L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffcp0L, -0x1p-105L, 1.234330349600789959989661887846005659983e-16L, -3.141592653589793038743797483317667178134L),
    TEST_c_c (cacosh, 0x1.fp-1025L, 0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-1025L, -0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, 0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, -0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffcp0L, 0x1.fp-1025L, 3.432186888910770473784103112009399360940e-293L, 1.570092458683775059393382371963565104164e-16L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffcp0L, 0x1.fp-1025L, 3.432186888910770473784103112009399360940e-293L, 3.141592653589793081453397514901996944859L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffcp0L, -0x1.fp-1025L, 3.432186888910770473784103112009399360940e-293L, -1.570092458683775059393382371963565104164e-16L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffcp0L, -0x1.fp-1025L, 3.432186888910770473784103112009399360940e-293L, -3.141592653589793081453397514901996944859L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (cacosh, 0.0L, 0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x0.ffffffffffffffffffffffffffcp0L, 8.813735870195430252326093249797835932642e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffcp0L, 0.0L, 0.0L, 1.570092458683775059393382371963565104164e-16L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffcp0L, 0.0L, 0.0L, 3.141592653589793081453397514901996944859L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffcp0L, -0.0L, 0.0L, -1.570092458683775059393382371963565104164e-16L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffcp0L, -0.0L, 0.0L, -3.141592653589793081453397514901996944859L),
#endif
    TEST_c_c (cacosh, 0x1p-105L, 0.5L, 4.812118250596034474977589134243684231352e-1L, 1.570796326794896619231321691639729392766L),
    TEST_c_c (cacosh, 0x1p-105L, -0.5L, 4.812118250596034474977589134243684231352e-1L, -1.570796326794896619231321691639729392766L),
    TEST_c_c (cacosh, -0x1p-105L, 0.5L, 4.812118250596034474977589134243684231352e-1L, 1.570796326794896619231321691639773491431L),
    TEST_c_c (cacosh, -0x1p-105L, -0.5L, 4.812118250596034474977589134243684231352e-1L, -1.570796326794896619231321691639773491431L),
    TEST_c_c (cacosh, 0.5L, 0x1p-105L, 2.846556599890768890940941645338546615370e-32L, 1.047197551196597746154214461093167628066L),
    TEST_c_c (cacosh, -0.5L, 0x1p-105L, 2.846556599890768890940941645338546615370e-32L, 2.094395102393195492308428922186335256131L),
    TEST_c_c (cacosh, 0.5L, -0x1p-105L, 2.846556599890768890940941645338546615370e-32L, -1.047197551196597746154214461093167628066L),
    TEST_c_c (cacosh, -0.5L, -0x1p-105L, 2.846556599890768890940941645338546615370e-32L, -2.094395102393195492308428922186335256131L),
    TEST_c_c (cacosh, 0x1p-105L, 0x1p-105L, 2.465190328815661891911651766508706967729e-32L, 1.570796326794896619231321691639726790195L),
    TEST_c_c (cacosh, 0x1p-105L, -0x1p-105L, 2.465190328815661891911651766508706967729e-32L, -1.570796326794896619231321691639726790195L),
    TEST_c_c (cacosh, -0x1p-105L, 0x1p-105L, 2.465190328815661891911651766508706967729e-32L, 1.570796326794896619231321691639776094002L),
    TEST_c_c (cacosh, -0x1p-105L, -0x1p-105L, 2.465190328815661891911651766508706967729e-32L, -1.570796326794896619231321691639776094002L),
#ifndef TEST_FLOAT
    TEST_c_c (cacosh, 0x1.fp-1025L, 0x1p-105L, 2.465190328815661891911651766508706967729e-32L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-1025L, -0x1p-105L, 2.465190328815661891911651766508706967729e-32L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, 0x1p-105L, 2.465190328815661891911651766508706967729e-32L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-1025L, -0x1p-105L, 2.465190328815661891911651766508706967729e-32L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1p-105L, 0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, 1.570796326794896619231321691639726790195L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, -0x1p-105L, 0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, 1.570796326794896619231321691639776094002L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, 0x1p-105L, -0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, -1.570796326794896619231321691639726790195L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (cacosh, -0x1p-105L, -0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, -1.570796326794896619231321691639776094002L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
    TEST_c_c (cacosh, 0.0L, 0x1p-105L, 2.465190328815661891911651766508706967729e-32L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x1p-105L, 2.465190328815661891911651766508706967729e-32L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x1p-105L, 2.465190328815661891911651766508706967729e-32L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x1p-105L, 2.465190328815661891911651766508706967729e-32L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1p-105L, 0.0L, 0.0L, 1.570796326794896619231321691639726790195L),
    TEST_c_c (cacosh, -0x1p-105L, 0.0L, 0.0L, 1.570796326794896619231321691639776094002L),
    TEST_c_c (cacosh, 0x1p-105L, -0.0L, 0.0L, -1.570796326794896619231321691639726790195L),
    TEST_c_c (cacosh, -0x1p-105L, -0.0L, 0.0L, -1.570796326794896619231321691639776094002L),
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (cacosh, 0x1p-112L, 0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, 1.570796326794896619231321691639751305915L),
    TEST_c_c (cacosh, 0x1p-112L, -0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, -1.570796326794896619231321691639751305915L),
    TEST_c_c (cacosh, -0x1p-112L, 0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, 1.570796326794896619231321691639751578282L),
    TEST_c_c (cacosh, -0x1p-112L, -0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, -1.570796326794896619231321691639751578282L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-112L, 1.091004200533850618345330909857103457781e-17L, 1.765281878332676474347460257221369818872e-17L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-112L, 1.091004200533850618345330909857103457781e-17L, 3.141592653589793220809824599952738140723L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-112L, 1.091004200533850618345330909857103457781e-17L, -1.765281878332676474347460257221369818872e-17L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-112L, 1.091004200533850618345330909857103457781e-17L, -3.141592653589793220809824599952738140723L),
    TEST_c_c (cacosh, 0x1.fp-16385L, 0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-16385L, -0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, 0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, -0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1.fp-16385L, 5.867357004219059108790368268900490184505e-4916L, 1.387778780781445675529539585113525401762e-17L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1.fp-16385L, 5.867357004219059108790368268900490184505e-4916L, 3.141592653589793224584855575465046128902L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1.fp-16385L, 5.867357004219059108790368268900490184505e-4916L, -1.387778780781445675529539585113525401762e-17L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1.fp-16385L, 5.867357004219059108790368268900490184505e-4916L, -3.141592653589793224584855575465046128902L),
    TEST_c_c (cacosh, 0.0L, 0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x0.ffffffffffffffffffffffffffff8p0L, 8.813735870195430252326093249797922409363e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, 0.0L, 1.387778780781445675529539585113525401762e-17L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, 0.0L, 3.141592653589793224584855575465046128902L),
    TEST_c_c (cacosh, 0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, 0.0L, -1.387778780781445675529539585113525401762e-17L),
    TEST_c_c (cacosh, -0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, 0.0L, -3.141592653589793224584855575465046128902L),
#endif
    TEST_c_c (cacosh, 0x1p-112L, 0.5L, 4.812118250596034474977589134243684231352e-1L, 1.570796326794896619231321691639751269838L),
    TEST_c_c (cacosh, 0x1p-112L, -0.5L, 4.812118250596034474977589134243684231352e-1L, -1.570796326794896619231321691639751269838L),
    TEST_c_c (cacosh, -0x1p-112L, 0.5L, 4.812118250596034474977589134243684231352e-1L, 1.570796326794896619231321691639751614359L),
    TEST_c_c (cacosh, -0x1p-112L, -0.5L, 4.812118250596034474977589134243684231352e-1L, -1.570796326794896619231321691639751614359L),
    TEST_c_c (cacosh, 0.5L, 0x1p-112L, 2.223872343664663196047610660420739543258e-34L, 1.047197551196597746154214461093167628066L),
    TEST_c_c (cacosh, -0.5L, 0x1p-112L, 2.223872343664663196047610660420739543258e-34L, 2.094395102393195492308428922186335256131L),
    TEST_c_c (cacosh, 0.5L, -0x1p-112L, 2.223872343664663196047610660420739543258e-34L, -1.047197551196597746154214461093167628066L),
    TEST_c_c (cacosh, -0.5L, -0x1p-112L, 2.223872343664663196047610660420739543258e-34L, -2.094395102393195492308428922186335256131L),
    TEST_c_c (cacosh, 0x1p-112L, 0x1p-112L, 1.925929944387235853055977942584927318538e-34L, 1.570796326794896619231321691639751249506L),
    TEST_c_c (cacosh, 0x1p-112L, -0x1p-112L, 1.925929944387235853055977942584927318538e-34L, -1.570796326794896619231321691639751249506L),
    TEST_c_c (cacosh, -0x1p-112L, 0x1p-112L, 1.925929944387235853055977942584927318538e-34L, 1.570796326794896619231321691639751634692L),
    TEST_c_c (cacosh, -0x1p-112L, -0x1p-112L, 1.925929944387235853055977942584927318538e-34L, -1.570796326794896619231321691639751634692L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (cacosh, 0x1.fp-16385L, 0x1p-112L, 1.925929944387235853055977942584927318538e-34L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1.fp-16385L, -0x1p-112L, 1.925929944387235853055977942584927318538e-34L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, 0x1p-112L, 1.925929944387235853055977942584927318538e-34L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0x1.fp-16385L, -0x1p-112L, 1.925929944387235853055977942584927318538e-34L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1p-112L, 0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, 1.570796326794896619231321691639751249506L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, -0x1p-112L, 0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, 1.570796326794896619231321691639751634692L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, 0x1p-112L, -0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, -1.570796326794896619231321691639751249506L, UNDERFLOW_EXCEPTION),
    TEST_c_c (cacosh, -0x1p-112L, -0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, -1.570796326794896619231321691639751634692L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (cacosh, 0.0L, 0x1p-112L, 1.925929944387235853055977942584927318538e-34L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0.0L, -0x1p-112L, 1.925929944387235853055977942584927318538e-34L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, 0x1p-112L, 1.925929944387235853055977942584927318538e-34L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, -0.0L, -0x1p-112L, 1.925929944387235853055977942584927318538e-34L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (cacosh, 0x1p-112L, 0.0L, 0.0L, 1.570796326794896619231321691639751249506L),
    TEST_c_c (cacosh, -0x1p-112L, 0.0L, 0.0L, 1.570796326794896619231321691639751634692L),
    TEST_c_c (cacosh, 0x1p-112L, -0.0L, 0.0L, -1.570796326794896619231321691639751249506L),
    TEST_c_c (cacosh, -0x1p-112L, -0.0L, 0.0L, -1.570796326794896619231321691639751634692L),

    TEST_c_c (cacosh, 0.75L, 1.25L, 1.13239363160530819522266333696834467L, 1.11752014915610270578240049553777969L),
    TEST_c_c (cacosh, -2, -3, 1.9833870299165354323470769028940395L, -2.1414491111159960199416055713254211L),
    END_DATA (cacosh)
  };

static void
cacosh_test (void)
{
  errno = 0;
  FUNC(cacosh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cacosh);
  RUN_TEST_LOOP_c_c (cacosh, cacosh_test_data, );
  END (cacosh, complex);
}


static const struct test_c_f_data carg_test_data[] =
  {
    START_DATA (carg);
    /* carg (x + iy) is specified as atan2 (y, x) */

    /* carg (x + i 0) == 0 for x > 0.  */
    TEST_c_f (carg, 2.0, 0, 0),
    /* carg (x - i 0) == -0 for x > 0.  */
    TEST_c_f (carg, 2.0, minus_zero, minus_zero),

    TEST_c_f (carg, 0, 0, 0),
    TEST_c_f (carg, 0, minus_zero, minus_zero),

    /* carg (x + i 0) == +pi for x < 0.  */
    TEST_c_f (carg, -2.0, 0, M_PIl),

    /* carg (x - i 0) == -pi for x < 0.  */
    TEST_c_f (carg, -2.0, minus_zero, -M_PIl),

    TEST_c_f (carg, minus_zero, 0, M_PIl),
    TEST_c_f (carg, minus_zero, minus_zero, -M_PIl),

    /* carg (+0 + i y) == pi/2 for y > 0.  */
    TEST_c_f (carg, 0, 2.0, M_PI_2l),

    /* carg (-0 + i y) == pi/2 for y > 0.  */
    TEST_c_f (carg, minus_zero, 2.0, M_PI_2l),

    /* carg (+0 + i y) == -pi/2 for y < 0.  */
    TEST_c_f (carg, 0, -2.0, -M_PI_2l),

    /* carg (-0 + i y) == -pi/2 for y < 0.  */
    TEST_c_f (carg, minus_zero, -2.0, -M_PI_2l),

    /* carg (inf + i y) == +0 for finite y > 0.  */
    TEST_c_f (carg, plus_infty, 2.0, 0),

    /* carg (inf + i y) == -0 for finite y < 0.  */
    TEST_c_f (carg, plus_infty, -2.0, minus_zero),

    /* carg(x + i inf) == pi/2 for finite x.  */
    TEST_c_f (carg, 10.0, plus_infty, M_PI_2l),

    /* carg(x - i inf) == -pi/2 for finite x.  */
    TEST_c_f (carg, 10.0, minus_infty, -M_PI_2l),

    /* carg (-inf + i y) == +pi for finite y > 0.  */
    TEST_c_f (carg, minus_infty, 10.0, M_PIl),

    /* carg (-inf + i y) == -pi for finite y < 0.  */
    TEST_c_f (carg, minus_infty, -10.0, -M_PIl),

    TEST_c_f (carg, plus_infty, plus_infty, M_PI_4l),

    TEST_c_f (carg, plus_infty, minus_infty, -M_PI_4l),

    TEST_c_f (carg, minus_infty, plus_infty, M_PI_34l),

    TEST_c_f (carg, minus_infty, minus_infty, -M_PI_34l),

    TEST_c_f (carg, qnan_value, qnan_value, qnan_value),
    END_DATA (carg);
  };

static void
carg_test (void)
{
  START (carg);
  RUN_TEST_LOOP_c_f (carg, carg_test_data, );
  END (carg);
}

static const struct test_c_c_data casin_test_data[] =
  {
    START_DATA (casin),
    TEST_c_c (casin, 0, 0, 0.0, 0.0),
    TEST_c_c (casin, minus_zero, 0, minus_zero, 0.0),
    TEST_c_c (casin, 0, minus_zero, 0.0, minus_zero),
    TEST_c_c (casin, minus_zero, minus_zero, minus_zero, minus_zero),

    TEST_c_c (casin, plus_infty, plus_infty, M_PI_4l, plus_infty),
    TEST_c_c (casin, plus_infty, minus_infty, M_PI_4l, minus_infty),
    TEST_c_c (casin, minus_infty, plus_infty, -M_PI_4l, plus_infty),
    TEST_c_c (casin, minus_infty, minus_infty, -M_PI_4l, minus_infty),

    TEST_c_c (casin, -10.0, plus_infty, minus_zero, plus_infty),
    TEST_c_c (casin, -10.0, minus_infty, minus_zero, minus_infty),
    TEST_c_c (casin, 0, plus_infty, 0.0, plus_infty),
    TEST_c_c (casin, 0, minus_infty, 0.0, minus_infty),
    TEST_c_c (casin, minus_zero, plus_infty, minus_zero, plus_infty),
    TEST_c_c (casin, minus_zero, minus_infty, minus_zero, minus_infty),
    TEST_c_c (casin, 0.1L, plus_infty, 0.0, plus_infty),
    TEST_c_c (casin, 0.1L, minus_infty, 0.0, minus_infty),

    TEST_c_c (casin, minus_infty, 0, -M_PI_2l, plus_infty),
    TEST_c_c (casin, minus_infty, minus_zero, -M_PI_2l, minus_infty),
    TEST_c_c (casin, minus_infty, 100, -M_PI_2l, plus_infty),
    TEST_c_c (casin, minus_infty, -100, -M_PI_2l, minus_infty),

    TEST_c_c (casin, plus_infty, 0, M_PI_2l, plus_infty),
    TEST_c_c (casin, plus_infty, minus_zero, M_PI_2l, minus_infty),
    TEST_c_c (casin, plus_infty, 0.5, M_PI_2l, plus_infty),
    TEST_c_c (casin, plus_infty, -0.5, M_PI_2l, minus_infty),

    TEST_c_c (casin, qnan_value, plus_infty, qnan_value, plus_infty),
    TEST_c_c (casin, qnan_value, minus_infty, qnan_value, minus_infty),

    TEST_c_c (casin, 0.0, qnan_value, 0.0, qnan_value),
    TEST_c_c (casin, minus_zero, qnan_value, minus_zero, qnan_value),

    TEST_c_c (casin, plus_infty, qnan_value, qnan_value, plus_infty, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (casin, minus_infty, qnan_value, qnan_value, plus_infty, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (casin, qnan_value, 10.5, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (casin, qnan_value, -10.5, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (casin, 0.75, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (casin, -0.75, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (casin, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (casin, plus_zero, -1.5L, plus_zero, -1.194763217287109304111930828519090523536L),
    TEST_c_c (casin, minus_zero, -1.5L, minus_zero, -1.194763217287109304111930828519090523536L),
    TEST_c_c (casin, plus_zero, -1.0L, plus_zero, -0.8813735870195430252326093249797923090282L),
    TEST_c_c (casin, minus_zero, -1.0L, minus_zero, -0.8813735870195430252326093249797923090282L),
    TEST_c_c (casin, plus_zero, -0.5L, plus_zero, -0.4812118250596034474977589134243684231352L),
    TEST_c_c (casin, minus_zero, -0.5L, minus_zero, -0.4812118250596034474977589134243684231352L),
    TEST_c_c (casin, plus_zero, 0.5L, plus_zero, 0.4812118250596034474977589134243684231352L),
    TEST_c_c (casin, minus_zero, 0.5L, minus_zero, 0.4812118250596034474977589134243684231352L),
    TEST_c_c (casin, plus_zero, 1.0L, plus_zero, 0.8813735870195430252326093249797923090282L),
    TEST_c_c (casin, minus_zero, 1.0L, minus_zero, 0.8813735870195430252326093249797923090282L),
    TEST_c_c (casin, plus_zero, 1.5L, plus_zero, 1.194763217287109304111930828519090523536L),
    TEST_c_c (casin, minus_zero, 1.5L, minus_zero, 1.194763217287109304111930828519090523536L),

    TEST_c_c (casin, -1.5L, plus_zero, -M_PI_2l, 0.9624236501192068949955178268487368462704L),
    TEST_c_c (casin, -1.5L, minus_zero, -M_PI_2l, -0.9624236501192068949955178268487368462704L),
    TEST_c_c (casin, -1.0L, plus_zero, -M_PI_2l, plus_zero),
    TEST_c_c (casin, -1.0L, minus_zero, -M_PI_2l, minus_zero),
    TEST_c_c (casin, -0.5L, plus_zero, -0.5235987755982988730771072305465838140329L, plus_zero),
    TEST_c_c (casin, -0.5L, minus_zero, -0.5235987755982988730771072305465838140329L, minus_zero),
    TEST_c_c (casin, 0.5L, plus_zero, 0.5235987755982988730771072305465838140329L, plus_zero),
    TEST_c_c (casin, 0.5L, minus_zero, 0.5235987755982988730771072305465838140329L, minus_zero),
    TEST_c_c (casin, 1.0L, plus_zero, M_PI_2l, plus_zero),
    TEST_c_c (casin, 1.0L, minus_zero, M_PI_2l, minus_zero),
    TEST_c_c (casin, 1.5L, plus_zero, M_PI_2l, 0.9624236501192068949955178268487368462704L),
    TEST_c_c (casin, 1.5L, minus_zero, M_PI_2l, -0.9624236501192068949955178268487368462704L),

    TEST_c_c (casin, 0x1p50L, 1.0L, 1.570796326794895731052901991514519103193L, 3.535050620855721078027883819436720218708e1L),
    TEST_c_c (casin, 0x1p50L, -1.0L, 1.570796326794895731052901991514519103193L, -3.535050620855721078027883819436720218708e1L),
    TEST_c_c (casin, -0x1p50L, 1.0L, -1.570796326794895731052901991514519103193L, 3.535050620855721078027883819436720218708e1L),
    TEST_c_c (casin, -0x1p50L, -1.0L, -1.570796326794895731052901991514519103193L, -3.535050620855721078027883819436720218708e1L),
    TEST_c_c (casin, 1.0L, 0x1p50L, 8.881784197001252323389053344720723756399e-16L, 3.535050620855721078027883819436759661753e1L),
    TEST_c_c (casin, -1.0L, 0x1p50L, -8.881784197001252323389053344720723756399e-16L, 3.535050620855721078027883819436759661753e1L),
    TEST_c_c (casin, 1.0L, -0x1p50L, 8.881784197001252323389053344720723756399e-16L, -3.535050620855721078027883819436759661753e1L),
    TEST_c_c (casin, -1.0L, -0x1p50L, -8.881784197001252323389053344720723756399e-16L, -3.535050620855721078027883819436759661753e1L),
#ifndef TEST_FLOAT
    TEST_c_c (casin, 0x1p500L, 1.0L, 1.570796326794896619231321691639751442099L, 3.472667374605326000180332928505464606058e2L),
    TEST_c_c (casin, 0x1p500L, -1.0L, 1.570796326794896619231321691639751442099L, -3.472667374605326000180332928505464606058e2L),
    TEST_c_c (casin, -0x1p500L, 1.0L, -1.570796326794896619231321691639751442099L, 3.472667374605326000180332928505464606058e2L),
    TEST_c_c (casin, -0x1p500L, -1.0L, -1.570796326794896619231321691639751442099L, -3.472667374605326000180332928505464606058e2L),
    TEST_c_c (casin, 1.0L, 0x1p500L, 3.054936363499604682051979393213617699789e-151L, 3.472667374605326000180332928505464606058e2L),
    TEST_c_c (casin, -1.0L, 0x1p500L, -3.054936363499604682051979393213617699789e-151L, 3.472667374605326000180332928505464606058e2L),
    TEST_c_c (casin, 1.0L, -0x1p500L, 3.054936363499604682051979393213617699789e-151L, -3.472667374605326000180332928505464606058e2L),
    TEST_c_c (casin, -1.0L, -0x1p500L, -3.054936363499604682051979393213617699789e-151L, -3.472667374605326000180332928505464606058e2L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (casin, 0x1p5000L, 1.0L, 1.570796326794896619231321691639751442099L, 3.466429049980286492395577839412341016946e3L),
    TEST_c_c (casin, 0x1p5000L, -1.0L, 1.570796326794896619231321691639751442099L, -3.466429049980286492395577839412341016946e3L),
    TEST_c_c (casin, -0x1p5000L, 1.0L, -1.570796326794896619231321691639751442099L, 3.466429049980286492395577839412341016946e3L),
    TEST_c_c (casin, -0x1p5000L, -1.0L, -1.570796326794896619231321691639751442099L, -3.466429049980286492395577839412341016946e3L),
    TEST_c_c (casin, 1.0L, 0x1p5000L, 7.079811261048172892385615158694057552948e-1506L, 3.466429049980286492395577839412341016946e3L),
    TEST_c_c (casin, -1.0L, 0x1p5000L, -7.079811261048172892385615158694057552948e-1506L, 3.466429049980286492395577839412341016946e3L),
    TEST_c_c (casin, 1.0L, -0x1p5000L, 7.079811261048172892385615158694057552948e-1506L, -3.466429049980286492395577839412341016946e3L),
    TEST_c_c (casin, -1.0L, -0x1p5000L, -7.079811261048172892385615158694057552948e-1506L, -3.466429049980286492395577839412341016946e3L),
#endif

    TEST_c_c (casin, 0x1.fp127L, 0x1.fp127L, 7.853981633974483096156608458198757210493e-1L, 8.973081118419833726837456344608533993585e1L),
#ifndef TEST_FLOAT
    TEST_c_c (casin, 0x1.fp1023L, 0x1.fp1023L, 7.853981633974483096156608458198757210493e-1L, 7.107906849659093345062145442726115449315e2L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (casin, 0x1.fp16383L, 0x1.fp16383L, 7.853981633974483096156608458198757210493e-1L, 1.135753137836666928715489992987020363057e4L),
#endif

    TEST_c_c (casin, 0x1.fp-129L, 1.5L, 1.579176199917649005841160751101628985741e-39L, 1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0x1.fp-129L, -1.5L, 1.579176199917649005841160751101628985741e-39L, -1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, 1.5L, -1.579176199917649005841160751101628985741e-39L, 1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, -1.5L, -1.579176199917649005841160751101628985741e-39L, -1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 1.5L, 0x1.fp-129L, 1.570796326794896619231321691639751442096L, 9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (casin, -1.5L, 0x1.fp-129L, -1.570796326794896619231321691639751442096L, 9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (casin, 1.5L, -0x1.fp-129L, 1.570796326794896619231321691639751442096L, -9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (casin, -1.5L, -0x1.fp-129L, -1.570796326794896619231321691639751442096L, -9.624236501192068949955178268487368462704e-1L),
#ifndef TEST_FLOAT
    TEST_c_c (casin, 0x1.fp-1025L, 1.5L, 2.989196569048182929051881765490354365918e-309L, 1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1.fp-1025L, -1.5L, 2.989196569048182929051881765490354365918e-309L, -1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, 1.5L, -2.989196569048182929051881765490354365918e-309L, 1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, -1.5L, -2.989196569048182929051881765490354365918e-309L, -1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 1.5L, 0x1.fp-1025L, 1.570796326794896619231321691639751442099L, 9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (casin, -1.5L, 0x1.fp-1025L, -1.570796326794896619231321691639751442099L, 9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (casin, 1.5L, -0x1.fp-1025L, 1.570796326794896619231321691639751442099L, -9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (casin, -1.5L, -0x1.fp-1025L, -1.570796326794896619231321691639751442099L, -9.624236501192068949955178268487368462704e-1L),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casin, 0x1.fp-16385L, 1.5L, 4.516698239814521372306784062043266700598e-4933L, 1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.fp-16385L, -1.5L, 4.516698239814521372306784062043266700598e-4933L, -1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, 1.5L, -4.516698239814521372306784062043266700598e-4933L, 1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, -1.5L, -4.516698239814521372306784062043266700598e-4933L, -1.194763217287109304111930828519090523536L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 1.5L, 0x1.fp-16385L, 1.570796326794896619231321691639751442099L, 9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (casin, -1.5L, 0x1.fp-16385L, -1.570796326794896619231321691639751442099L, 9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (casin, 1.5L, -0x1.fp-16385L, 1.570796326794896619231321691639751442099L, -9.624236501192068949955178268487368462704e-1L),
    TEST_c_c (casin, -1.5L, -0x1.fp-16385L, -1.570796326794896619231321691639751442099L, -9.624236501192068949955178268487368462704e-1L),
#endif

    TEST_c_c (casin, 0.5L, 1.0L, 3.494390628572132936274118260753699527325e-1L, 9.261330313501824245501244453057873152694e-1L),
    TEST_c_c (casin, 0.5L, -1.0L, 3.494390628572132936274118260753699527325e-1L, -9.261330313501824245501244453057873152694e-1L),
    TEST_c_c (casin, -0.5L, 1.0L, -3.494390628572132936274118260753699527325e-1L, 9.261330313501824245501244453057873152694e-1L),
    TEST_c_c (casin, -0.5L, -1.0L, -3.494390628572132936274118260753699527325e-1L, -9.261330313501824245501244453057873152694e-1L),
    TEST_c_c (casin, 1.0L, 0.5L, 8.959074812088902390666567243275770102229e-1L, 7.328576759736452608886724437653071523305e-1L),
    TEST_c_c (casin, -1.0L, 0.5L, -8.959074812088902390666567243275770102229e-1L, 7.328576759736452608886724437653071523305e-1L),
    TEST_c_c (casin, 1.0L, -0.5L, 8.959074812088902390666567243275770102229e-1L, -7.328576759736452608886724437653071523305e-1L),
    TEST_c_c (casin, -1.0L, -0.5L, -8.959074812088902390666567243275770102229e-1L, -7.328576759736452608886724437653071523305e-1L),
    TEST_c_c (casin, 0.25L, 1.0L, 1.763024327769669304186785666360901026468e-1L, 8.924633639033482359562124741744951972772e-1L),
    TEST_c_c (casin, 0.25L, -1.0L, 1.763024327769669304186785666360901026468e-1L, -8.924633639033482359562124741744951972772e-1L),
    TEST_c_c (casin, -0.25L, 1.0L, -1.763024327769669304186785666360901026468e-1L, 8.924633639033482359562124741744951972772e-1L),
    TEST_c_c (casin, -0.25L, -1.0L, -1.763024327769669304186785666360901026468e-1L, -8.924633639033482359562124741744951972772e-1L),
    TEST_c_c (casin, 1.0L, 0.25L, 1.081751996523816326311037318425097434186L, 5.097911466811016354623559941115413499164e-1L),
    TEST_c_c (casin, -1.0L, 0.25L, -1.081751996523816326311037318425097434186L, 5.097911466811016354623559941115413499164e-1L),
    TEST_c_c (casin, 1.0L, -0.25L, 1.081751996523816326311037318425097434186L, -5.097911466811016354623559941115413499164e-1L),
    TEST_c_c (casin, -1.0L, -0.25L, -1.081751996523816326311037318425097434186L, -5.097911466811016354623559941115413499164e-1L),
    TEST_c_c (casin, 0x1.fp-10L, 1.0L, 1.337909359557740912558349531052239112857e-3L, 8.813742198809567991336704287826445879025e-1L),
    TEST_c_c (casin, 0x1.fp-10L, -1.0L, 1.337909359557740912558349531052239112857e-3L, -8.813742198809567991336704287826445879025e-1L),
    TEST_c_c (casin, -0x1.fp-10L, 1.0L, -1.337909359557740912558349531052239112857e-3L, 8.813742198809567991336704287826445879025e-1L),
    TEST_c_c (casin, -0x1.fp-10L, -1.0L, -1.337909359557740912558349531052239112857e-3L, -8.813742198809567991336704287826445879025e-1L),
    TEST_c_c (casin, 1.0L, 0x1.fp-10L, 1.527305029163877791518741192097931722508L, 4.350501469856803800217957402220976497152e-2L),
    TEST_c_c (casin, -1.0L, 0x1.fp-10L, -1.527305029163877791518741192097931722508L, 4.350501469856803800217957402220976497152e-2L),
    TEST_c_c (casin, 1.0L, -0x1.fp-10L, 1.527305029163877791518741192097931722508L, -4.350501469856803800217957402220976497152e-2L),
    TEST_c_c (casin, -1.0L, -0x1.fp-10L, -1.527305029163877791518741192097931722508L, -4.350501469856803800217957402220976497152e-2L),
    TEST_c_c (casin, 0x1.fp-30L, 1.0L, 1.275929984216518540252717485007112529021e-9L, 8.813735870195430258081932989769495326854e-1L),
    TEST_c_c (casin, 0x1.fp-30L, -1.0L, 1.275929984216518540252717485007112529021e-9L, -8.813735870195430258081932989769495326854e-1L),
    TEST_c_c (casin, -0x1.fp-30L, 1.0L, -1.275929984216518540252717485007112529021e-9L, 8.813735870195430258081932989769495326854e-1L),
    TEST_c_c (casin, -0x1.fp-30L, -1.0L, -1.275929984216518540252717485007112529021e-9L, -8.813735870195430258081932989769495326854e-1L),
    TEST_c_c (casin, 1.0L, 0x1.fp-30L, 1.570753848123921942730162693731872690232L, 4.247867098745151888768727039216644758847e-5L),
    TEST_c_c (casin, -1.0L, 0x1.fp-30L, -1.570753848123921942730162693731872690232L, 4.247867098745151888768727039216644758847e-5L),
    TEST_c_c (casin, 1.0L, -0x1.fp-30L, 1.570753848123921942730162693731872690232L, -4.247867098745151888768727039216644758847e-5L),
    TEST_c_c (casin, -1.0L, -0x1.fp-30L, -1.570753848123921942730162693731872690232L, -4.247867098745151888768727039216644758847e-5L),
    TEST_c_c (casin, 0x1.fp-100L, 1.0L, 1.080754735021050612990719608916167354321e-30L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (casin, 0x1.fp-100L, -1.0L, 1.080754735021050612990719608916167354321e-30L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (casin, -0x1.fp-100L, 1.0L, -1.080754735021050612990719608916167354321e-30L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (casin, -0x1.fp-100L, -1.0L, -1.080754735021050612990719608916167354321e-30L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (casin, 1.0L, 0x1.fp-100L, 1.570796326794895382939283431378862777584L, 1.236292038260260888664514866457202186027e-15L),
    TEST_c_c (casin, -1.0L, 0x1.fp-100L, -1.570796326794895382939283431378862777584L, 1.236292038260260888664514866457202186027e-15L),
    TEST_c_c (casin, 1.0L, -0x1.fp-100L, 1.570796326794895382939283431378862777584L, -1.236292038260260888664514866457202186027e-15L),
    TEST_c_c (casin, -1.0L, -0x1.fp-100L, -1.570796326794895382939283431378862777584L, -1.236292038260260888664514866457202186027e-15L),
    TEST_c_c (casin, 0x1.fp-129L, 1.0L, 2.013062564695348242280482517399205554874e-39L, 8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0x1.fp-129L, -1.0L, 2.013062564695348242280482517399205554874e-39L, -8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, 1.0L, -2.013062564695348242280482517399205554874e-39L, 8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, -1.0L, -2.013062564695348242280482517399205554874e-39L, -8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 1.0L, 0x1.fp-129L, 1.570796326794896619177965338869929107115L, 5.335635276982233498398987585285818977933e-20L),
    TEST_c_c (casin, -1.0L, 0x1.fp-129L, -1.570796326794896619177965338869929107115L, 5.335635276982233498398987585285818977933e-20L),
    TEST_c_c (casin, 1.0L, -0x1.fp-129L, 1.570796326794896619177965338869929107115L, -5.335635276982233498398987585285818977933e-20L),
    TEST_c_c (casin, -1.0L, -0x1.fp-129L, -1.570796326794896619177965338869929107115L, -5.335635276982233498398987585285818977933e-20L),
#ifndef TEST_FLOAT
    TEST_c_c (casin, 0x1.fp-1000L, 1.0L, 1.278589251976747242280879285935084814093e-301L, 8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, 0x1.fp-1000L, -1.0L, 1.278589251976747242280879285935084814093e-301L, -8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, -0x1.fp-1000L, 1.0L, -1.278589251976747242280879285935084814093e-301L, 8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, -0x1.fp-1000L, -1.0L, -1.278589251976747242280879285935084814093e-301L, -8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, 1.0L, 0x1.fp-1000L, 1.570796326794896619231321691639751442099L, 4.252291453851660175550490409247739011867e-151L),
    TEST_c_c (casin, -1.0L, 0x1.fp-1000L, -1.570796326794896619231321691639751442099L, 4.252291453851660175550490409247739011867e-151L),
    TEST_c_c (casin, 1.0L, -0x1.fp-1000L, 1.570796326794896619231321691639751442099L, -4.252291453851660175550490409247739011867e-151L),
    TEST_c_c (casin, -1.0L, -0x1.fp-1000L, -1.570796326794896619231321691639751442099L, -4.252291453851660175550490409247739011867e-151L),
    TEST_c_c (casin, 0x1.fp-1025L, 1.0L, 3.810492908885321743133304375216617626230e-309L, 8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1.fp-1025L, -1.0L, 3.810492908885321743133304375216617626230e-309L, -8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, 1.0L, -3.810492908885321743133304375216617626230e-309L, 8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, -1.0L, -3.810492908885321743133304375216617626230e-309L, -8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 1.0L, 0x1.fp-1025L, 1.570796326794896619231321691639751442099L, 7.340879205566679497036857179189356754017e-155L),
    TEST_c_c (casin, -1.0L, 0x1.fp-1025L, -1.570796326794896619231321691639751442099L, 7.340879205566679497036857179189356754017e-155L),
    TEST_c_c (casin, 1.0L, -0x1.fp-1025L, 1.570796326794896619231321691639751442099L, -7.340879205566679497036857179189356754017e-155L),
    TEST_c_c (casin, -1.0L, -0x1.fp-1025L, -1.570796326794896619231321691639751442099L, -7.340879205566679497036857179189356754017e-155L),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casin, 0x1.fp-10000L, 1.0L, 6.867047849047171855399183659351043150871e-3011L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (casin, 0x1.fp-10000L, -1.0L, 6.867047849047171855399183659351043150871e-3011L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (casin, -0x1.fp-10000L, 1.0L, -6.867047849047171855399183659351043150871e-3011L, 8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (casin, -0x1.fp-10000L, -1.0L, -6.867047849047171855399183659351043150871e-3011L, -8.813735870195430252326093249797923090282e-1L),
    TEST_c_c (casin, 1.0L, 0x1.fp-10000L, 1.570796326794896619231321691639751442099L, 9.854680208706673586644342922051388714633e-1506L),
    TEST_c_c (casin, -1.0L, 0x1.fp-10000L, -1.570796326794896619231321691639751442099L, 9.854680208706673586644342922051388714633e-1506L),
    TEST_c_c (casin, 1.0L, -0x1.fp-10000L, 1.570796326794896619231321691639751442099L, -9.854680208706673586644342922051388714633e-1506L),
    TEST_c_c (casin, -1.0L, -0x1.fp-10000L, -1.570796326794896619231321691639751442099L, -9.854680208706673586644342922051388714633e-1506L),
    TEST_c_c (casin, 0x1.fp-16385L, 1.0L, 5.757683115456107044131264955348448954458e-4933L, 8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.fp-16385L, -1.0L, 5.757683115456107044131264955348448954458e-4933L, -8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, 1.0L, -5.757683115456107044131264955348448954458e-4933L, 8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, -1.0L, -5.757683115456107044131264955348448954458e-4933L, -8.813735870195430252326093249797923090282e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 1.0L, 0x1.fp-16385L, 1.570796326794896619231321691639751442099L, 9.023632056840860275214893047597614177639e-2467L),
    TEST_c_c (casin, -1.0L, 0x1.fp-16385L, -1.570796326794896619231321691639751442099L, 9.023632056840860275214893047597614177639e-2467L),
    TEST_c_c (casin, 1.0L, -0x1.fp-16385L, 1.570796326794896619231321691639751442099L, -9.023632056840860275214893047597614177639e-2467L),
    TEST_c_c (casin, -1.0L, -0x1.fp-16385L, -1.570796326794896619231321691639751442099L, -9.023632056840860275214893047597614177639e-2467L),
#endif

    TEST_c_c (casin, 0x1p-23L, 0x1.000002p0L, 8.429369199749229560964789467980644296420e-8L, 8.813736713132400470205730751186547909968e-1L),
    TEST_c_c (casin, 0x1p-23L, -0x1.000002p0L, 8.429369199749229560964789467980644296420e-8L, -8.813736713132400470205730751186547909968e-1L),
    TEST_c_c (casin, -0x1p-23L, 0x1.000002p0L, -8.429369199749229560964789467980644296420e-8L, 8.813736713132400470205730751186547909968e-1L),
    TEST_c_c (casin, -0x1p-23L, -0x1.000002p0L, -8.429369199749229560964789467980644296420e-8L, -8.813736713132400470205730751186547909968e-1L),
    TEST_c_c (casin, 0x1.000002p0L, 0x1p-23L, 1.570574114956455764594481054234834678422L, 5.364668491573609633134147164031476452679e-4L),
    TEST_c_c (casin, -0x1.000002p0L, 0x1p-23L, -1.570574114956455764594481054234834678422L, 5.364668491573609633134147164031476452679e-4L),
    TEST_c_c (casin, 0x1.000002p0L, -0x1p-23L, 1.570574114956455764594481054234834678422L, -5.364668491573609633134147164031476452679e-4L),
    TEST_c_c (casin, -0x1.000002p0L, -0x1p-23L, -1.570574114956455764594481054234834678422L, -5.364668491573609633134147164031476452679e-4L),
    TEST_c_c (casin, 0x1.fp-129L, 0x1.000002p0L, 2.013062444707472738895109955455676357057e-39L, 8.813736713132375348727889167749389235161e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0x1.fp-129L, -0x1.000002p0L, 2.013062444707472738895109955455676357057e-39L, -8.813736713132375348727889167749389235161e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, 0x1.000002p0L, -2.013062444707472738895109955455676357057e-39L, 8.813736713132375348727889167749389235161e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, -0x1.000002p0L, -2.013062444707472738895109955455676357057e-39L, -8.813736713132375348727889167749389235161e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0x1.000002p0L, 0x1.fp-129L, 1.570796326794896619231321691639751436268L, 4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (casin, -0x1.000002p0L, 0x1.fp-129L, -1.570796326794896619231321691639751436268L, 4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (casin, 0x1.000002p0L, -0x1.fp-129L, 1.570796326794896619231321691639751436268L, -4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (casin, -0x1.000002p0L, -0x1.fp-129L, -1.570796326794896619231321691639751436268L, -4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (casin, 0.0L, 0x1.000002p0L, 0.0L, 8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (casin, 0.0L, -0x1.000002p0L, 0.0L, -8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (casin, -0.0L, 0x1.000002p0L, -0.0L, 8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (casin, -0.0L, -0x1.000002p0L, -0.0L, -8.813736713132375348727889167749389235161e-1L),
    TEST_c_c (casin, 0x1.000002p0L, 0.0L, 1.570796326794896619231321691639751442099L, 4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (casin, -0x1.000002p0L, 0.0L, -1.570796326794896619231321691639751442099L, 4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (casin, 0x1.000002p0L, -0.0L, 1.570796326794896619231321691639751442099L, -4.882812451493617206486388134172712975070e-4L),
    TEST_c_c (casin, -0x1.000002p0L, -0.0L, -1.570796326794896619231321691639751442099L, -4.882812451493617206486388134172712975070e-4L),
#ifndef TEST_FLOAT
    TEST_c_c (casin, 0x1p-52L, 0x1.0000000000001p0L, 1.570092458683774885078102529858632363236e-16L, 8.813735870195431822418551933572982483664e-1L),
    TEST_c_c (casin, 0x1p-52L, -0x1.0000000000001p0L, 1.570092458683774885078102529858632363236e-16L, -8.813735870195431822418551933572982483664e-1L),
    TEST_c_c (casin, -0x1p-52L, 0x1.0000000000001p0L, -1.570092458683774885078102529858632363236e-16L, 8.813735870195431822418551933572982483664e-1L),
    TEST_c_c (casin, -0x1p-52L, -0x1.0000000000001p0L, -1.570092458683774885078102529858632363236e-16L, -8.813735870195431822418551933572982483664e-1L),
    TEST_c_c (casin, 0x1.0000000000001p0L, 0x1p-52L, 1.570796317204594913251280305810847349436L, 2.315303644582684770975188768022139415020e-8L),
    TEST_c_c (casin, -0x1.0000000000001p0L, 0x1p-52L, -1.570796317204594913251280305810847349436L, 2.315303644582684770975188768022139415020e-8L),
    TEST_c_c (casin, 0x1.0000000000001p0L, -0x1p-52L, 1.570796317204594913251280305810847349436L, -2.315303644582684770975188768022139415020e-8L),
    TEST_c_c (casin, -0x1.0000000000001p0L, -0x1p-52L, -1.570796317204594913251280305810847349436L, -2.315303644582684770975188768022139415020e-8L),
    TEST_c_c (casin, 0x1.fp-1025L, 0x1.0000000000001p0L, 3.810492908885321320083608113679347200012e-309L, 8.813735870195431822418551933572895326024e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1.fp-1025L, -0x1.0000000000001p0L, 3.810492908885321320083608113679347200012e-309L, -8.813735870195431822418551933572895326024e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, 0x1.0000000000001p0L, -3.810492908885321320083608113679347200012e-309L, 8.813735870195431822418551933572895326024e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, -0x1.0000000000001p0L, -3.810492908885321320083608113679347200012e-309L, -8.813735870195431822418551933572895326024e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1.0000000000001p0L, 0x1.fp-1025L, 1.570796326794896619231321691639751442099L, 2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (casin, -0x1.0000000000001p0L, 0x1.fp-1025L, -1.570796326794896619231321691639751442099L, 2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (casin, 0x1.0000000000001p0L, -0x1.fp-1025L, 1.570796326794896619231321691639751442099L, -2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (casin, -0x1.0000000000001p0L, -0x1.fp-1025L, -1.570796326794896619231321691639751442099L, -2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (casin, 0.0L, 0x1.0000000000001p0L, 0.0L, 8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (casin, 0.0L, -0x1.0000000000001p0L, 0.0L, -8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (casin, -0.0L, 0x1.0000000000001p0L, -0.0L, 8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (casin, -0.0L, -0x1.0000000000001p0L, -0.0L, -8.813735870195431822418551933572895326024e-1L),
    TEST_c_c (casin, 0x1.0000000000001p0L, 0.0L, 1.570796326794896619231321691639751442099L, 2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (casin, -0x1.0000000000001p0L, 0.0L, -1.570796326794896619231321691639751442099L, 2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (casin, 0x1.0000000000001p0L, -0.0L, 1.570796326794896619231321691639751442099L, -2.107342425544701550354780375182800088393e-8L),
    TEST_c_c (casin, -0x1.0000000000001p0L, -0.0L, -1.570796326794896619231321691639751442099L, -2.107342425544701550354780375182800088393e-8L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (casin, 0x1p-63L, 0x1.0000000000000002p0L, 7.666467083416870406778649849746878368519e-20L, 8.813735870195430253092739958139610131001e-1L),
    TEST_c_c (casin, 0x1p-63L, -0x1.0000000000000002p0L, 7.666467083416870406778649849746878368519e-20L, -8.813735870195430253092739958139610131001e-1L),
    TEST_c_c (casin, -0x1p-63L, 0x1.0000000000000002p0L, -7.666467083416870406778649849746878368519e-20L, 8.813735870195430253092739958139610131001e-1L),
    TEST_c_c (casin, -0x1p-63L, -0x1.0000000000000002p0L, -7.666467083416870406778649849746878368519e-20L, -8.813735870195430253092739958139610131001e-1L),
    TEST_c_c (casin, 0x1.0000000000000002p0L, 0x1p-63L, 1.570796326582978888921215348380499122131L, 5.116146586219826555037807251857670783420e-10L),
    TEST_c_c (casin, -0x1.0000000000000002p0L, 0x1p-63L, -1.570796326582978888921215348380499122131L, 5.116146586219826555037807251857670783420e-10L),
    TEST_c_c (casin, 0x1.0000000000000002p0L, -0x1p-63L, 1.570796326582978888921215348380499122131L, -5.116146586219826555037807251857670783420e-10L),
    TEST_c_c (casin, -0x1.0000000000000002p0L, -0x1p-63L, -1.570796326582978888921215348380499122131L, -5.116146586219826555037807251857670783420e-10L),
# if LDBL_MIN_EXP <= -16381
    TEST_c_c (casin, 0x1.fp-16385L, 0x1.0000000000000002p0L, 5.757683115456107043819140328235418018963e-4933L, 8.813735870195430253092739958139610130980e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.fp-16385L, -0x1.0000000000000002p0L, 5.757683115456107043819140328235418018963e-4933L, -8.813735870195430253092739958139610130980e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, 0x1.0000000000000002p0L, -5.757683115456107043819140328235418018963e-4933L, 8.813735870195430253092739958139610130980e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, -0x1.0000000000000002p0L, -5.757683115456107043819140328235418018963e-4933L, -8.813735870195430253092739958139610130980e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.0000000000000002p0L, 0x1.fp-16385L, 1.570796326794896619231321691639751442099L, 4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (casin, -0x1.0000000000000002p0L, 0x1.fp-16385L, -1.570796326794896619231321691639751442099L, 4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (casin, 0x1.0000000000000002p0L, -0x1.fp-16385L, 1.570796326794896619231321691639751442099L, -4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (casin, -0x1.0000000000000002p0L, -0x1.fp-16385L, -1.570796326794896619231321691639751442099L, -4.656612873077392578082927418388212703712e-10L),
# endif
    TEST_c_c (casin, 0.0L, 0x1.0000000000000002p0L, 0.0L, 8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (casin, 0.0L, -0x1.0000000000000002p0L, 0.0L, -8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (casin, -0.0L, 0x1.0000000000000002p0L, -0.0L, 8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (casin, -0.0L, -0x1.0000000000000002p0L, -0.0L, -8.813735870195430253092739958139610130980e-1L),
    TEST_c_c (casin, 0x1.0000000000000002p0L, 0.0L, 1.570796326794896619231321691639751442099L, 4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (casin, -0x1.0000000000000002p0L, 0.0L, -1.570796326794896619231321691639751442099L, 4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (casin, 0x1.0000000000000002p0L, -0.0L, 1.570796326794896619231321691639751442099L, -4.656612873077392578082927418388212703712e-10L),
    TEST_c_c (casin, -0x1.0000000000000002p0L, -0.0L, -1.570796326794896619231321691639751442099L, -4.656612873077392578082927418388212703712e-10L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (casin, 0x1p-106L, 0x1.000000000000000000000000008p0L, 8.715763992105246878957416200936726072500e-33L, 8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (casin, 0x1p-106L, -0x1.000000000000000000000000008p0L, 8.715763992105246878957416200936726072500e-33L, -8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (casin, -0x1p-106L, 0x1.000000000000000000000000008p0L, -8.715763992105246878957416200936726072500e-33L, 8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (casin, -0x1p-106L, -0x1.000000000000000000000000008p0L, -8.715763992105246878957416200936726072500e-33L, -8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (casin, 0x1.000000000000000000000000008p0L, 0x1p-106L, 1.570796326794896565289107467733682961928L, 2.285028863093221674154232933662774454211e-16L),
    TEST_c_c (casin, -0x1.000000000000000000000000008p0L, 0x1p-106L, -1.570796326794896565289107467733682961928L, 2.285028863093221674154232933662774454211e-16L),
    TEST_c_c (casin, 0x1.000000000000000000000000008p0L, -0x1p-106L, 1.570796326794896565289107467733682961928L, -2.285028863093221674154232933662774454211e-16L),
    TEST_c_c (casin, -0x1.000000000000000000000000008p0L, -0x1p-106L, -1.570796326794896565289107467733682961928L, -2.285028863093221674154232933662774454211e-16L),
    TEST_c_c (casin, 0x1.fp-1025L, 0x1.000000000000000000000000008p0L, 3.810492908885321743133304375216570658278e-309L, 8.813735870195430252326093249798097405561e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1.fp-1025L, -0x1.000000000000000000000000008p0L, 3.810492908885321743133304375216570658278e-309L, -8.813735870195430252326093249798097405561e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, 0x1.000000000000000000000000008p0L, -3.810492908885321743133304375216570658278e-309L, 8.813735870195430252326093249798097405561e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, -0x1.000000000000000000000000008p0L, -3.810492908885321743133304375216570658278e-309L, -8.813735870195430252326093249798097405561e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1.000000000000000000000000008p0L, 0x1.fp-1025L, 1.570796326794896619231321691639751442099L, 2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (casin, -0x1.000000000000000000000000008p0L, 0x1.fp-1025L, -1.570796326794896619231321691639751442099L, 2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (casin, 0x1.000000000000000000000000008p0L, -0x1.fp-1025L, 1.570796326794896619231321691639751442099L, -2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (casin, -0x1.000000000000000000000000008p0L, -0x1.fp-1025L, -1.570796326794896619231321691639751442099L, -2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (casin, 0.0L, 0x1.000000000000000000000000008p0L, 0.0L, 8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (casin, 0.0L, -0x1.000000000000000000000000008p0L, 0.0L, -8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (casin, -0.0L, 0x1.000000000000000000000000008p0L, -0.0L, 8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (casin, -0.0L, -0x1.000000000000000000000000008p0L, -0.0L, -8.813735870195430252326093249798097405561e-1L),
    TEST_c_c (casin, 0x1.000000000000000000000000008p0L, 0.0L, 1.570796326794896619231321691639751442099L, 2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (casin, -0x1.000000000000000000000000008p0L, 0.0L, -1.570796326794896619231321691639751442099L, 2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (casin, 0x1.000000000000000000000000008p0L, -0.0L, 1.570796326794896619231321691639751442099L, -2.220446049250313080847263336181636063482e-16L),
    TEST_c_c (casin, -0x1.000000000000000000000000008p0L, -0.0L, -1.570796326794896619231321691639751442099L, -2.220446049250313080847263336181636063482e-16L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (casin, 0x1p-113L, 0x1.0000000000000000000000000001p0L, 6.809190618832224124185481406981900518193e-35L, 8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (casin, 0x1p-113L, -0x1.0000000000000000000000000001p0L, 6.809190618832224124185481406981900518193e-35L, -8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (casin, -0x1p-113L, 0x1.0000000000000000000000000001p0L, -6.809190618832224124185481406981900518193e-35L, 8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (casin, -0x1p-113L, -0x1.0000000000000000000000000001p0L, -6.809190618832224124185481406981900518193e-35L, -8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (casin, 0x1.0000000000000000000000000001p0L, 0x1p-113L, 1.570796326794896614463458507897073960405L, 2.019699255375255198156433710951064632386e-17L),
    TEST_c_c (casin, -0x1.0000000000000000000000000001p0L, 0x1p-113L, -1.570796326794896614463458507897073960405L, 2.019699255375255198156433710951064632386e-17L),
    TEST_c_c (casin, 0x1.0000000000000000000000000001p0L, -0x1p-113L, 1.570796326794896614463458507897073960405L, -2.019699255375255198156433710951064632386e-17L),
    TEST_c_c (casin, -0x1.0000000000000000000000000001p0L, -0x1p-113L, -1.570796326794896614463458507897073960405L, -2.019699255375255198156433710951064632386e-17L),
    TEST_c_c (casin, 0x1.fp-16385L, 0x1.0000000000000000000000000001p0L, 5.757683115456107044131264955348448400014e-4933L, 8.813735870195430252326093249797924452120e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.fp-16385L, -0x1.0000000000000000000000000001p0L, 5.757683115456107044131264955348448400014e-4933L, -8.813735870195430252326093249797924452120e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, 0x1.0000000000000000000000000001p0L, -5.757683115456107044131264955348448400014e-4933L, 8.813735870195430252326093249797924452120e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, -0x1.0000000000000000000000000001p0L, -5.757683115456107044131264955348448400014e-4933L, -8.813735870195430252326093249797924452120e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.0000000000000000000000000001p0L, 0x1.fp-16385L, 1.570796326794896619231321691639751442099L, 1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (casin, -0x1.0000000000000000000000000001p0L, 0x1.fp-16385L, -1.570796326794896619231321691639751442099L, 1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (casin, 0x1.0000000000000000000000000001p0L, -0x1.fp-16385L, 1.570796326794896619231321691639751442099L, -1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (casin, -0x1.0000000000000000000000000001p0L, -0x1.fp-16385L, -1.570796326794896619231321691639751442099L, -1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (casin, 0.0L, 0x1.0000000000000000000000000001p0L, 0.0L, 8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (casin, 0.0L, -0x1.0000000000000000000000000001p0L, 0.0L, -8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (casin, -0.0L, 0x1.0000000000000000000000000001p0L, -0.0L, 8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (casin, -0.0L, -0x1.0000000000000000000000000001p0L, -0.0L, -8.813735870195430252326093249797924452120e-1L),
    TEST_c_c (casin, 0x1.0000000000000000000000000001p0L, 0.0L, 1.570796326794896619231321691639751442099L, 1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (casin, -0x1.0000000000000000000000000001p0L, 0.0L, -1.570796326794896619231321691639751442099L, 1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (casin, 0x1.0000000000000000000000000001p0L, -0.0L, 1.570796326794896619231321691639751442099L, -1.962615573354718824241727964954454332780e-17L),
    TEST_c_c (casin, -0x1.0000000000000000000000000001p0L, -0.0L, -1.570796326794896619231321691639751442099L, -1.962615573354718824241727964954454332780e-17L),
#endif

    TEST_c_c (casin, 0x1p-23L, 0x0.ffffffp0L, 8.429369953393598499000760938112968755257e-8L, 8.813735448726963984495965873956465777250e-1L),
    TEST_c_c (casin, 0x1p-23L, -0x0.ffffffp0L, 8.429369953393598499000760938112968755257e-8L, -8.813735448726963984495965873956465777250e-1L),
    TEST_c_c (casin, -0x1p-23L, 0x0.ffffffp0L, -8.429369953393598499000760938112968755257e-8L, 8.813735448726963984495965873956465777250e-1L),
    TEST_c_c (casin, -0x1p-23L, -0x0.ffffffp0L, -8.429369953393598499000760938112968755257e-8L, -8.813735448726963984495965873956465777250e-1L),
    TEST_c_c (casin, 0x0.ffffffp0L, 0x1p-23L, 1.570357140408705570520377172865353621698L, 2.714321200917194650737217746780928423385e-4L),
    TEST_c_c (casin, -0x0.ffffffp0L, 0x1p-23L, -1.570357140408705570520377172865353621698L, 2.714321200917194650737217746780928423385e-4L),
    TEST_c_c (casin, 0x0.ffffffp0L, -0x1p-23L, 1.570357140408705570520377172865353621698L, -2.714321200917194650737217746780928423385e-4L),
    TEST_c_c (casin, -0x0.ffffffp0L, -0x1p-23L, -1.570357140408705570520377172865353621698L, -2.714321200917194650737217746780928423385e-4L),
    TEST_c_c (casin, 0x1.fp-129L, 0x0.ffffffp0L, 2.013062624689288675911339929224634593425e-39L, 8.813735448726938863015878255140556727969e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0x1.fp-129L, -0x0.ffffffp0L, 2.013062624689288675911339929224634593425e-39L, -8.813735448726938863015878255140556727969e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, 0x0.ffffffp0L, -2.013062624689288675911339929224634593425e-39L, 8.813735448726938863015878255140556727969e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, -0x0.ffffffp0L, -2.013062624689288675911339929224634593425e-39L, -8.813735448726938863015878255140556727969e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0x0.ffffffp0L, 0x1.fp-129L, 1.570451059810180415643718442157112705601L, 8.245504387859737323891843603996428534945e-36L),
    TEST_c_c (casin, -0x0.ffffffp0L, 0x1.fp-129L, -1.570451059810180415643718442157112705601L, 8.245504387859737323891843603996428534945e-36L),
    TEST_c_c (casin, 0x0.ffffffp0L, -0x1.fp-129L, 1.570451059810180415643718442157112705601L, -8.245504387859737323891843603996428534945e-36L),
    TEST_c_c (casin, -0x0.ffffffp0L, -0x1.fp-129L, -1.570451059810180415643718442157112705601L, -8.245504387859737323891843603996428534945e-36L),
    TEST_c_c (casin, 0.0L, 0x0.ffffffp0L, 0.0L, 8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (casin, 0.0L, -0x0.ffffffp0L, 0.0L, -8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (casin, -0.0L, 0x0.ffffffp0L, -0.0L, 8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (casin, -0.0L, -0x0.ffffffp0L, -0.0L, -8.813735448726938863015878255140556727969e-1L),
    TEST_c_c (casin, 0x0.ffffffp0L, 0.0L, 1.570451059810180415643718442157112705601L, 0.0L),
    TEST_c_c (casin, -0x0.ffffffp0L, 0.0L, -1.570451059810180415643718442157112705601L, 0.0L),
    TEST_c_c (casin, 0x0.ffffffp0L, -0.0L, 1.570451059810180415643718442157112705601L, -0.0L),
    TEST_c_c (casin, -0x0.ffffffp0L, -0.0L, -1.570451059810180415643718442157112705601L, -0.0L),
    TEST_c_c (casin, 0x1p-23L, 0.5L, 1.066240299940009780479664302917201696886e-7L, 4.812118250596059896127318386463676808798e-1L),
    TEST_c_c (casin, 0x1p-23L, -0.5L, 1.066240299940009780479664302917201696886e-7L, -4.812118250596059896127318386463676808798e-1L),
    TEST_c_c (casin, -0x1p-23L, 0.5L, -1.066240299940009780479664302917201696886e-7L, 4.812118250596059896127318386463676808798e-1L),
    TEST_c_c (casin, -0x1p-23L, -0.5L, -1.066240299940009780479664302917201696886e-7L, -4.812118250596059896127318386463676808798e-1L),
    TEST_c_c (casin, 0.5L, 0x1p-23L, 5.235987755982934033165770729745469098254e-1L, 1.376510308240943236356088341381173571841e-7L),
    TEST_c_c (casin, -0.5L, 0x1p-23L, -5.235987755982934033165770729745469098254e-1L, 1.376510308240943236356088341381173571841e-7L),
    TEST_c_c (casin, 0.5L, -0x1p-23L, 5.235987755982934033165770729745469098254e-1L, -1.376510308240943236356088341381173571841e-7L),
    TEST_c_c (casin, -0.5L, -0x1p-23L, -5.235987755982934033165770729745469098254e-1L, -1.376510308240943236356088341381173571841e-7L),
    TEST_c_c (casin, 0x1.fp-129L, 0.5L, 2.546345110742945032959687790021055102355e-39L, 4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0x1.fp-129L, -0.5L, 2.546345110742945032959687790021055102355e-39L, -4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, 0.5L, -2.546345110742945032959687790021055102355e-39L, 4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, -0.5L, -2.546345110742945032959687790021055102355e-39L, -4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0.5L, 0x1.fp-129L, 5.235987755982988730771072305465838140329e-1L, 3.287317402534702257036015056278368274737e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0.5L, 0x1.fp-129L, -5.235987755982988730771072305465838140329e-1L, 3.287317402534702257036015056278368274737e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0.5L, -0x1.fp-129L, 5.235987755982988730771072305465838140329e-1L, -3.287317402534702257036015056278368274737e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0.5L, -0x1.fp-129L, -5.235987755982988730771072305465838140329e-1L, -3.287317402534702257036015056278368274737e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0x1p-23L, 0x1p-23L, 1.192092895507806853113684971258850504935e-7L, 1.192092895507818146886315028596704749235e-7L),
    TEST_c_c (casin, 0x1p-23L, -0x1p-23L, 1.192092895507806853113684971258850504935e-7L, -1.192092895507818146886315028596704749235e-7L),
    TEST_c_c (casin, -0x1p-23L, 0x1p-23L, -1.192092895507806853113684971258850504935e-7L, 1.192092895507818146886315028596704749235e-7L),
    TEST_c_c (casin, -0x1p-23L, -0x1p-23L, -1.192092895507806853113684971258850504935e-7L, -1.192092895507818146886315028596704749235e-7L),
    /* Bug 15319: underflow exception may be missing.  */
    TEST_c_c (casin, 0x1.fp-129L, 0x1p-23L, 2.846900380897707329917933005874691056564e-39L, 1.192092895507809676556842485683592032154e-7L, UNDERFLOW_EXCEPTION_OK_FLOAT),
    TEST_c_c (casin, 0x1.fp-129L, -0x1p-23L, 2.846900380897707329917933005874691056564e-39L, -1.192092895507809676556842485683592032154e-7L, UNDERFLOW_EXCEPTION_OK_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, 0x1p-23L, -2.846900380897707329917933005874691056564e-39L, 1.192092895507809676556842485683592032154e-7L, UNDERFLOW_EXCEPTION_OK_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, -0x1p-23L, -2.846900380897707329917933005874691056564e-39L, -1.192092895507809676556842485683592032154e-7L, UNDERFLOW_EXCEPTION_OK_FLOAT),
    TEST_c_c (casin, 0x1p-23L, 0x1.fp-129L, 1.192092895507815323443157514352519154304e-7L, 2.846900380897747786805634596726756660388e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1p-23L, 0x1.fp-129L, -1.192092895507815323443157514352519154304e-7L, 2.846900380897747786805634596726756660388e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0x1p-23L, -0x1.fp-129L, 1.192092895507815323443157514352519154304e-7L, -2.846900380897747786805634596726756660388e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1p-23L, -0x1.fp-129L, -1.192092895507815323443157514352519154304e-7L, -2.846900380897747786805634596726756660388e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0.0L, 0x1p-23L, 0.0L, 1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (casin, 0.0L, -0x1p-23L, 0.0L, -1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (casin, -0.0L, 0x1p-23L, -0.0L, 1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (casin, -0.0L, -0x1p-23L, -0.0L, -1.192092895507809676556842485683592032154e-7L),
    TEST_c_c (casin, 0x1p-23L, 0.0L, 1.192092895507815323443157514352519154304e-7L, 0.0L),
    TEST_c_c (casin, -0x1p-23L, 0.0L, -1.192092895507815323443157514352519154304e-7L, 0.0L),
    TEST_c_c (casin, 0x1p-23L, -0.0L, 1.192092895507815323443157514352519154304e-7L, -0.0L),
    TEST_c_c (casin, -0x1p-23L, -0.0L, -1.192092895507815323443157514352519154304e-7L, -0.0L),
    TEST_c_c (casin, 0x1.fp-129L, 0x1.fp-129L, 2.846900380897727558361783801085126250967e-39L, 2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, 0x1.fp-129L, -0x1.fp-129L, 2.846900380897727558361783801085126250967e-39L, -2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, 0x1.fp-129L, -2.846900380897727558361783801085126250967e-39L, 2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casin, -0x1.fp-129L, -0x1.fp-129L, -2.846900380897727558361783801085126250967e-39L, -2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (casin, 0x1p-52L, 0x0.fffffffffffff8p0L, 1.570092458683775146551022293016031474627e-16L, 8.813735870195429467279863907910458761820e-1L),
    TEST_c_c (casin, 0x1p-52L, -0x0.fffffffffffff8p0L, 1.570092458683775146551022293016031474627e-16L, -8.813735870195429467279863907910458761820e-1L),
    TEST_c_c (casin, -0x1p-52L, 0x0.fffffffffffff8p0L, -1.570092458683775146551022293016031474627e-16L, 8.813735870195429467279863907910458761820e-1L),
    TEST_c_c (casin, -0x1p-52L, -0x0.fffffffffffff8p0L, -1.570092458683775146551022293016031474627e-16L, -8.813735870195429467279863907910458761820e-1L),
    TEST_c_c (casin, 0x0.fffffffffffff8p0L, 0x1p-52L, 1.570796307840326780080580569360500172043L, 1.171456840272878582596796205397918831268e-8L),
    TEST_c_c (casin, -0x0.fffffffffffff8p0L, 0x1p-52L, -1.570796307840326780080580569360500172043L, 1.171456840272878582596796205397918831268e-8L),
    TEST_c_c (casin, 0x0.fffffffffffff8p0L, -0x1p-52L, 1.570796307840326780080580569360500172043L, -1.171456840272878582596796205397918831268e-8L),
    TEST_c_c (casin, -0x0.fffffffffffff8p0L, -0x1p-52L, -1.570796307840326780080580569360500172043L, -1.171456840272878582596796205397918831268e-8L),
    TEST_c_c (casin, 0x1.fp-1025L, 0x0.fffffffffffff8p0L, 3.810492908885321954658152505985270452320e-309L, 8.813735870195429467279863907910371604180e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1.fp-1025L, -0x0.fffffffffffff8p0L, 3.810492908885321954658152505985270452320e-309L, -8.813735870195429467279863907910371604180e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, 0x0.fffffffffffff8p0L, -3.810492908885321954658152505985270452320e-309L, 8.813735870195429467279863907910371604180e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, -0x0.fffffffffffff8p0L, -3.810492908885321954658152505985270452320e-309L, -8.813735870195429467279863907910371604180e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x0.fffffffffffff8p0L, 0x1.fp-1025L, 1.570796311893735425383665303776316016594L, 3.616396521699973256461764099945789620604e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, -0x0.fffffffffffff8p0L, 0x1.fp-1025L, -1.570796311893735425383665303776316016594L, 3.616396521699973256461764099945789620604e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, 0x0.fffffffffffff8p0L, -0x1.fp-1025L, 1.570796311893735425383665303776316016594L, -3.616396521699973256461764099945789620604e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, -0x0.fffffffffffff8p0L, -0x1.fp-1025L, -1.570796311893735425383665303776316016594L, -3.616396521699973256461764099945789620604e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, 0.0L, 0x0.fffffffffffff8p0L, 0.0L, 8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (casin, 0.0L, -0x0.fffffffffffff8p0L, 0.0L, -8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (casin, -0.0L, 0x0.fffffffffffff8p0L, -0.0L, 8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (casin, -0.0L, -0x0.fffffffffffff8p0L, -0.0L, -8.813735870195429467279863907910371604180e-1L),
    TEST_c_c (casin, 0x0.fffffffffffff8p0L, 0.0L, 1.570796311893735425383665303776316016594L, 0.0L),
    TEST_c_c (casin, -0x0.fffffffffffff8p0L, 0.0L, -1.570796311893735425383665303776316016594L, 0.0L),
    TEST_c_c (casin, 0x0.fffffffffffff8p0L, -0.0L, 1.570796311893735425383665303776316016594L, -0.0L),
    TEST_c_c (casin, -0x0.fffffffffffff8p0L, -0.0L, -1.570796311893735425383665303776316016594L, -0.0L),
#endif
    TEST_c_c (casin, 0x1p-52L, 0.5L, 1.986027322597818397661816253158024383707e-16L, 4.812118250596034474977589134243772428682e-1L),
    TEST_c_c (casin, 0x1p-52L, -0.5L, 1.986027322597818397661816253158024383707e-16L, -4.812118250596034474977589134243772428682e-1L),
    TEST_c_c (casin, -0x1p-52L, 0.5L, -1.986027322597818397661816253158024383707e-16L, 4.812118250596034474977589134243772428682e-1L),
    TEST_c_c (casin, -0x1p-52L, -0.5L, -1.986027322597818397661816253158024383707e-16L, -4.812118250596034474977589134243772428682e-1L),
    TEST_c_c (casin, 0.5L, 0x1p-52L, 5.235987755982988730771072305465648369889e-1L, 2.563950248511418570403591756798643804971e-16L),
    TEST_c_c (casin, -0.5L, 0x1p-52L, -5.235987755982988730771072305465648369889e-1L, 2.563950248511418570403591756798643804971e-16L),
    TEST_c_c (casin, 0.5L, -0x1p-52L, 5.235987755982988730771072305465648369889e-1L, -2.563950248511418570403591756798643804971e-16L),
    TEST_c_c (casin, -0.5L, -0x1p-52L, -5.235987755982988730771072305465648369889e-1L, -2.563950248511418570403591756798643804971e-16L),
#ifndef TEST_FLOAT
    TEST_c_c (casin, 0x1.fp-1025L, 0.5L, 4.819934639999230680322935210539402497827e-309L, 4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1.fp-1025L, -0.5L, 4.819934639999230680322935210539402497827e-309L, -4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, 0.5L, -4.819934639999230680322935210539402497827e-309L, 4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, -0.5L, -4.819934639999230680322935210539402497827e-309L, -4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0.5L, 0x1.fp-1025L, 5.235987755982988730771072305465838140329e-1L, 6.222508863508420569166420770843207333493e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0.5L, 0x1.fp-1025L, -5.235987755982988730771072305465838140329e-1L, 6.222508863508420569166420770843207333493e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0.5L, -0x1.fp-1025L, 5.235987755982988730771072305465838140329e-1L, -6.222508863508420569166420770843207333493e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0.5L, -0x1.fp-1025L, -5.235987755982988730771072305465838140329e-1L, -6.222508863508420569166420770843207333493e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
    TEST_c_c (casin, 0x1p-52L, 0x1p-52L, 2.220446049250313080847263336181604132852e-16L, 2.220446049250313080847263336181677117148e-16L),
    TEST_c_c (casin, 0x1p-52L, -0x1p-52L, 2.220446049250313080847263336181604132852e-16L, -2.220446049250313080847263336181677117148e-16L),
    TEST_c_c (casin, -0x1p-52L, 0x1p-52L, -2.220446049250313080847263336181604132852e-16L, 2.220446049250313080847263336181677117148e-16L),
    TEST_c_c (casin, -0x1p-52L, -0x1p-52L, -2.220446049250313080847263336181604132852e-16L, -2.220446049250313080847263336181677117148e-16L),
#ifndef TEST_FLOAT
    /* Bug 15319: underflow exception may be missing.  */
    TEST_c_c (casin, 0x1.fp-1025L, 0x1p-52L, 5.388850751072128349671657362289283247603e-309L, 2.220446049250313080847263336181622378926e-16L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casin, 0x1.fp-1025L, -0x1p-52L, 5.388850751072128349671657362289283247603e-309L, -2.220446049250313080847263336181622378926e-16L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, 0x1p-52L, -5.388850751072128349671657362289283247603e-309L, 2.220446049250313080847263336181622378926e-16L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, -0x1p-52L, -5.388850751072128349671657362289283247603e-309L, -2.220446049250313080847263336181622378926e-16L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casin, 0x1p-52L, 0x1.fp-1025L, 2.220446049250313080847263336181658871074e-16L, 5.388850751072128349671657362289548938458e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1p-52L, 0x1.fp-1025L, -2.220446049250313080847263336181658871074e-16L, 5.388850751072128349671657362289548938458e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1p-52L, -0x1.fp-1025L, 2.220446049250313080847263336181658871074e-16L, -5.388850751072128349671657362289548938458e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1p-52L, -0x1.fp-1025L, -2.220446049250313080847263336181658871074e-16L, -5.388850751072128349671657362289548938458e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
    TEST_c_c (casin, 0.0L, 0x1p-52L, 0.0L, 2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (casin, 0.0L, -0x1p-52L, 0.0L, -2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (casin, -0.0L, 0x1p-52L, -0.0L, 2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (casin, -0.0L, -0x1p-52L, -0.0L, -2.220446049250313080847263336181622378926e-16L),
    TEST_c_c (casin, 0x1p-52L, 0.0L, 2.220446049250313080847263336181658871074e-16L, 0.0L),
    TEST_c_c (casin, -0x1p-52L, 0.0L, -2.220446049250313080847263336181658871074e-16L, 0.0L),
    TEST_c_c (casin, 0x1p-52L, -0.0L, 2.220446049250313080847263336181658871074e-16L, -0.0L),
    TEST_c_c (casin, -0x1p-52L, -0.0L, -2.220446049250313080847263336181658871074e-16L, -0.0L),
#ifndef TEST_FLOAT
    TEST_c_c (casin, 0x1.fp-1025L, 0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1.fp-1025L, -0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, 0x1.fp-1025L, -5.388850751072128349671657362289416093031e-309L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, -0x1.fp-1025L, -5.388850751072128349671657362289416093031e-309L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (casin, 0x1p-63L, 0x0.ffffffffffffffffp0L, 7.666467083416870407402049869781566856442e-20L, 8.813735870195430251942769895627079569937e-1L),
    TEST_c_c (casin, 0x1p-63L, -0x0.ffffffffffffffffp0L, 7.666467083416870407402049869781566856442e-20L, -8.813735870195430251942769895627079569937e-1L),
    TEST_c_c (casin, -0x1p-63L, 0x0.ffffffffffffffffp0L, -7.666467083416870407402049869781566856442e-20L, 8.813735870195430251942769895627079569937e-1L),
    TEST_c_c (casin, -0x1p-63L, -0x0.ffffffffffffffffp0L, -7.666467083416870407402049869781566856442e-20L, -8.813735870195430251942769895627079569937e-1L),
    TEST_c_c (casin, 0x0.ffffffffffffffffp0L, 0x1p-63L, 1.570796326376055842114524927965556247908L, 2.588578361325995866221775673638805081337e-10L),
    TEST_c_c (casin, -0x0.ffffffffffffffffp0L, 0x1p-63L, -1.570796326376055842114524927965556247908L, 2.588578361325995866221775673638805081337e-10L),
    TEST_c_c (casin, 0x0.ffffffffffffffffp0L, -0x1p-63L, 1.570796326376055842114524927965556247908L, -2.588578361325995866221775673638805081337e-10L),
    TEST_c_c (casin, -0x0.ffffffffffffffffp0L, -0x1p-63L, -1.570796326376055842114524927965556247908L, -2.588578361325995866221775673638805081337e-10L),
# if LDBL_MIN_EXP <= -16381
    TEST_c_c (casin, 0x1.fp-16385L, 0x0.ffffffffffffffffp0L, 5.757683115456107044287327268904964422212e-4933L, 8.813735870195430251942769895627079569917e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.fp-16385L, -0x0.ffffffffffffffffp0L, 5.757683115456107044287327268904964422212e-4933L, -8.813735870195430251942769895627079569917e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, 0x0.ffffffffffffffffp0L, -5.757683115456107044287327268904964422212e-4933L, 8.813735870195430251942769895627079569917e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, -0x0.ffffffffffffffffp0L, -5.757683115456107044287327268904964422212e-4933L, -8.813735870195430251942769895627079569917e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x0.ffffffffffffffffp0L, 0x1.fp-16385L, 1.570796326465624365239962068302568888898L, 2.472906068161537187835415298076415423459e-4923L),
    TEST_c_c (casin, -0x0.ffffffffffffffffp0L, 0x1.fp-16385L, -1.570796326465624365239962068302568888898L, 2.472906068161537187835415298076415423459e-4923L),
    TEST_c_c (casin, 0x0.ffffffffffffffffp0L, -0x1.fp-16385L, 1.570796326465624365239962068302568888898L, -2.472906068161537187835415298076415423459e-4923L),
    TEST_c_c (casin, -0x0.ffffffffffffffffp0L, -0x1.fp-16385L, -1.570796326465624365239962068302568888898L, -2.472906068161537187835415298076415423459e-4923L),
# endif
    TEST_c_c (casin, 0.0L, 0x0.ffffffffffffffffp0L, 0.0L, 8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (casin, 0.0L, -0x0.ffffffffffffffffp0L, 0.0L, -8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (casin, -0.0L, 0x0.ffffffffffffffffp0L, -0.0L, 8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (casin, -0.0L, -0x0.ffffffffffffffffp0L, -0.0L, -8.813735870195430251942769895627079569917e-1L),
    TEST_c_c (casin, 0x0.ffffffffffffffffp0L, 0.0L, 1.570796326465624365239962068302568888898L, 0.0L),
    TEST_c_c (casin, -0x0.ffffffffffffffffp0L, 0.0L, -1.570796326465624365239962068302568888898L, 0.0L),
    TEST_c_c (casin, 0x0.ffffffffffffffffp0L, -0.0L, 1.570796326465624365239962068302568888898L, -0.0L),
    TEST_c_c (casin, -0x0.ffffffffffffffffp0L, -0.0L, -1.570796326465624365239962068302568888898L, -0.0L),
#endif
    TEST_c_c (casin, 0x1p-63L, 0.5L, 9.697399036122160144833087173623140436413e-20L, 4.812118250596034474977589134243684231373e-1L),
    TEST_c_c (casin, 0x1p-63L, -0.5L, 9.697399036122160144833087173623140436413e-20L, -4.812118250596034474977589134243684231373e-1L),
    TEST_c_c (casin, -0x1p-63L, 0.5L, -9.697399036122160144833087173623140436413e-20L, 4.812118250596034474977589134243684231373e-1L),
    TEST_c_c (casin, -0x1p-63L, -0.5L, -9.697399036122160144833087173623140436413e-20L, -4.812118250596034474977589134243684231373e-1L),
    TEST_c_c (casin, 0.5L, 0x1p-63L, 5.235987755982988730771072305465838140283e-1L, 1.251928832280966098829878787499365228659e-19L),
    TEST_c_c (casin, -0.5L, 0x1p-63L, -5.235987755982988730771072305465838140283e-1L, 1.251928832280966098829878787499365228659e-19L),
    TEST_c_c (casin, 0.5L, -0x1p-63L, 5.235987755982988730771072305465838140283e-1L, -1.251928832280966098829878787499365228659e-19L),
    TEST_c_c (casin, -0.5L, -0x1p-63L, -5.235987755982988730771072305465838140283e-1L, -1.251928832280966098829878787499365228659e-19L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casin, 0x1.fp-16385L, 0.5L, 7.282957076134209141226696333885150260319e-4933L, 4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.fp-16385L, -0.5L, 7.282957076134209141226696333885150260319e-4933L, -4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, 0.5L, -7.282957076134209141226696333885150260319e-4933L, 4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, -0.5L, -7.282957076134209141226696333885150260319e-4933L, -4.812118250596034474977589134243684231352e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0.5L, 0x1.fp-16385L, 5.235987755982988730771072305465838140329e-1L, 9.402257155670418112159334875199821342890e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0.5L, 0x1.fp-16385L, -5.235987755982988730771072305465838140329e-1L, 9.402257155670418112159334875199821342890e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0.5L, -0x1.fp-16385L, 5.235987755982988730771072305465838140329e-1L, -9.402257155670418112159334875199821342890e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0.5L, -0x1.fp-16385L, -5.235987755982988730771072305465838140329e-1L, -9.402257155670418112159334875199821342890e-4933L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (casin, 0x1p-63L, 0x1p-63L, 1.084202172485504434007452800869941711422e-19L, 1.084202172485504434007452800869941711430e-19L),
    TEST_c_c (casin, 0x1p-63L, -0x1p-63L, 1.084202172485504434007452800869941711422e-19L, -1.084202172485504434007452800869941711430e-19L),
    TEST_c_c (casin, -0x1p-63L, 0x1p-63L, -1.084202172485504434007452800869941711422e-19L, 1.084202172485504434007452800869941711430e-19L),
    TEST_c_c (casin, -0x1p-63L, -0x1p-63L, -1.084202172485504434007452800869941711422e-19L, -1.084202172485504434007452800869941711430e-19L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casin, 0x1.fp-16385L, 0x1p-63L, 8.142593549724601460479922838826119584369e-4933L, 1.084202172485504434007452800869941711424e-19L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.fp-16385L, -0x1p-63L, 8.142593549724601460479922838826119584369e-4933L, -1.084202172485504434007452800869941711424e-19L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, 0x1p-63L, -8.142593549724601460479922838826119584369e-4933L, 1.084202172485504434007452800869941711424e-19L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, -0x1p-63L, -8.142593549724601460479922838826119584369e-4933L, -1.084202172485504434007452800869941711424e-19L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1p-63L, 0x1.fp-16385L, 1.084202172485504434007452800869941711428e-19L, 8.142593549724601460479922838826119584465e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1p-63L, 0x1.fp-16385L, -1.084202172485504434007452800869941711428e-19L, 8.142593549724601460479922838826119584465e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1p-63L, -0x1.fp-16385L, 1.084202172485504434007452800869941711428e-19L, -8.142593549724601460479922838826119584465e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1p-63L, -0x1.fp-16385L, -1.084202172485504434007452800869941711428e-19L, -8.142593549724601460479922838826119584465e-4933L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (casin, 0.0L, 0x1p-63L, 0.0L, 1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (casin, 0.0L, -0x1p-63L, 0.0L, -1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (casin, -0.0L, 0x1p-63L, -0.0L, 1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (casin, -0.0L, -0x1p-63L, -0.0L, -1.084202172485504434007452800869941711424e-19L),
    TEST_c_c (casin, 0x1p-63L, 0.0L, 1.084202172485504434007452800869941711428e-19L, 0.0L),
    TEST_c_c (casin, -0x1p-63L, 0.0L, -1.084202172485504434007452800869941711428e-19L, 0.0L),
    TEST_c_c (casin, 0x1p-63L, -0.0L, 1.084202172485504434007452800869941711428e-19L, -0.0L),
    TEST_c_c (casin, -0x1p-63L, -0.0L, -1.084202172485504434007452800869941711428e-19L, -0.0L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casin, 0x1.fp-16385L, 0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.fp-16385L, -0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, 0x1.fp-16385L, -8.142593549724601460479922838826119584417e-4933L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, -0x1.fp-16385L, -8.142593549724601460479922838826119584417e-4933L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (casin, 0x1p-105L, 0x0.ffffffffffffffffffffffffffcp0L, 1.743152798421049375791483240187377443526e-32L, 8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (casin, 0x1p-105L, -0x0.ffffffffffffffffffffffffffcp0L, 1.743152798421049375791483240187377443526e-32L, -8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (casin, -0x1p-105L, 0x0.ffffffffffffffffffffffffffcp0L, -1.743152798421049375791483240187377443526e-32L, 8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (casin, -0x1p-105L, -0x0.ffffffffffffffffffffffffffcp0L, -1.743152798421049375791483240187377443526e-32L, -8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffcp0L, 0x1p-105L, 1.570796326794896419512475791677915736035L, 1.234330349600789959989661887846005659983e-16L),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffcp0L, 0x1p-105L, -1.570796326794896419512475791677915736035L, 1.234330349600789959989661887846005659983e-16L),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffcp0L, -0x1p-105L, 1.570796326794896419512475791677915736035L, -1.234330349600789959989661887846005659983e-16L),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffcp0L, -0x1p-105L, -1.570796326794896419512475791677915736035L, -1.234330349600789959989661887846005659983e-16L),
    TEST_c_c (casin, 0x1.fp-1025L, 0x0.ffffffffffffffffffffffffffcp0L, 3.810492908885321743133304375216641110205e-309L, 8.813735870195430252326093249797835932642e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1.fp-1025L, -0x0.ffffffffffffffffffffffffffcp0L, 3.810492908885321743133304375216641110205e-309L, -8.813735870195430252326093249797835932642e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, 0x0.ffffffffffffffffffffffffffcp0L, -3.810492908885321743133304375216641110205e-309L, 8.813735870195430252326093249797835932642e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, -0x0.ffffffffffffffffffffffffffcp0L, -3.810492908885321743133304375216641110205e-309L, -8.813735870195430252326093249797835932642e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffcp0L, 0x1.fp-1025L, 1.570796326794896462222075823262245502760L, 3.432186888910770473784103112009399360940e-293L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffcp0L, 0x1.fp-1025L, -1.570796326794896462222075823262245502760L, 3.432186888910770473784103112009399360940e-293L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffcp0L, -0x1.fp-1025L, 1.570796326794896462222075823262245502760L, -3.432186888910770473784103112009399360940e-293L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffcp0L, -0x1.fp-1025L, -1.570796326794896462222075823262245502760L, -3.432186888910770473784103112009399360940e-293L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casin, 0.0L, 0x0.ffffffffffffffffffffffffffcp0L, 0.0L, 8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (casin, 0.0L, -0x0.ffffffffffffffffffffffffffcp0L, 0.0L, -8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (casin, -0.0L, 0x0.ffffffffffffffffffffffffffcp0L, -0.0L, 8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (casin, -0.0L, -0x0.ffffffffffffffffffffffffffcp0L, -0.0L, -8.813735870195430252326093249797835932642e-1L),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffcp0L, 0.0L, 1.570796326794896462222075823262245502760L, 0.0L),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffcp0L, 0.0L, -1.570796326794896462222075823262245502760L, 0.0L),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffcp0L, -0.0L, 1.570796326794896462222075823262245502760L, -0.0L),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffcp0L, -0.0L, -1.570796326794896462222075823262245502760L, -0.0L),
#endif
    TEST_c_c (casin, 0x1p-105L, 0.5L, 2.204933261082751447436996198490110245167e-32L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (casin, 0x1p-105L, -0.5L, 2.204933261082751447436996198490110245167e-32L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (casin, -0x1p-105L, 0.5L, -2.204933261082751447436996198490110245167e-32L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (casin, -0x1p-105L, -0.5L, -2.204933261082751447436996198490110245167e-32L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (casin, 0.5L, 0x1p-105L, 5.235987755982988730771072305465838140329e-1L, 2.846556599890768890940941645338546615370e-32L),
    TEST_c_c (casin, -0.5L, 0x1p-105L, -5.235987755982988730771072305465838140329e-1L, 2.846556599890768890940941645338546615370e-32L),
    TEST_c_c (casin, 0.5L, -0x1p-105L, 5.235987755982988730771072305465838140329e-1L, -2.846556599890768890940941645338546615370e-32L),
    TEST_c_c (casin, -0.5L, -0x1p-105L, -5.235987755982988730771072305465838140329e-1L, -2.846556599890768890940941645338546615370e-32L),
    TEST_c_c (casin, 0x1p-105L, 0x1p-105L, 2.465190328815661891911651766508706967729e-32L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casin, 0x1p-105L, -0x1p-105L, 2.465190328815661891911651766508706967729e-32L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casin, -0x1p-105L, 0x1p-105L, -2.465190328815661891911651766508706967729e-32L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casin, -0x1p-105L, -0x1p-105L, -2.465190328815661891911651766508706967729e-32L, -2.465190328815661891911651766508706967729e-32L),
#ifndef TEST_FLOAT
    /* Bug 15319: underflow exception may be missing.  */
    TEST_c_c (casin, 0x1.fp-1025L, 0x1p-105L, 5.388850751072128349671657362289416093031e-309L, 2.465190328815661891911651766508706967729e-32L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casin, 0x1.fp-1025L, -0x1p-105L, 5.388850751072128349671657362289416093031e-309L, -2.465190328815661891911651766508706967729e-32L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, 0x1p-105L, -5.388850751072128349671657362289416093031e-309L, 2.465190328815661891911651766508706967729e-32L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casin, -0x1.fp-1025L, -0x1p-105L, -5.388850751072128349671657362289416093031e-309L, -2.465190328815661891911651766508706967729e-32L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casin, 0x1p-105L, 0x1.fp-1025L, 2.465190328815661891911651766508706967729e-32L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1p-105L, 0x1.fp-1025L, -2.465190328815661891911651766508706967729e-32L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, 0x1p-105L, -0x1.fp-1025L, 2.465190328815661891911651766508706967729e-32L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casin, -0x1p-105L, -0x1.fp-1025L, -2.465190328815661891911651766508706967729e-32L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
    TEST_c_c (casin, 0.0L, 0x1p-105L, 0.0L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casin, 0.0L, -0x1p-105L, 0.0L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casin, -0.0L, 0x1p-105L, -0.0L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casin, -0.0L, -0x1p-105L, -0.0L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casin, 0x1p-105L, 0.0L, 2.465190328815661891911651766508706967729e-32L, 0.0L),
    TEST_c_c (casin, -0x1p-105L, 0.0L, -2.465190328815661891911651766508706967729e-32L, 0.0L),
    TEST_c_c (casin, 0x1p-105L, -0.0L, 2.465190328815661891911651766508706967729e-32L, -0.0L),
    TEST_c_c (casin, -0x1p-105L, -0.0L, -2.465190328815661891911651766508706967729e-32L, -0.0L),
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (casin, 0x1p-112L, 0x0.ffffffffffffffffffffffffffff8p0L, 1.361838123766444824837096281396380300349e-34L, 8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (casin, 0x1p-112L, -0x0.ffffffffffffffffffffffffffff8p0L, 1.361838123766444824837096281396380300349e-34L, -8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (casin, -0x1p-112L, 0x0.ffffffffffffffffffffffffffff8p0L, -1.361838123766444824837096281396380300349e-34L, 8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (casin, -0x1p-112L, -0x0.ffffffffffffffffffffffffffff8p0L, -1.361838123766444824837096281396380300349e-34L, -8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-112L, 1.570796326794896601578502908312986698624L, 1.091004200533850618345330909857103457781e-17L),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-112L, -1.570796326794896601578502908312986698624L, 1.091004200533850618345330909857103457781e-17L),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-112L, 1.570796326794896601578502908312986698624L, -1.091004200533850618345330909857103457781e-17L),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-112L, -1.570796326794896601578502908312986698624L, -1.091004200533850618345330909857103457781e-17L),
    TEST_c_c (casin, 0x1.fp-16385L, 0x0.ffffffffffffffffffffffffffff8p0L, 5.757683115456107044131264955348449231681e-4933L, 8.813735870195430252326093249797922409363e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.fp-16385L, -0x0.ffffffffffffffffffffffffffff8p0L, 5.757683115456107044131264955348449231681e-4933L, -8.813735870195430252326093249797922409363e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, 0x0.ffffffffffffffffffffffffffff8p0L, -5.757683115456107044131264955348449231681e-4933L, 8.813735870195430252326093249797922409363e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, -0x0.ffffffffffffffffffffffffffff8p0L, -5.757683115456107044131264955348449231681e-4933L, -8.813735870195430252326093249797922409363e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1.fp-16385L, 1.570796326794896605353533883825294686803L, 5.867357004219059108790368268900490184505e-4916L),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1.fp-16385L, -1.570796326794896605353533883825294686803L, 5.867357004219059108790368268900490184505e-4916L),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1.fp-16385L, 1.570796326794896605353533883825294686803L, -5.867357004219059108790368268900490184505e-4916L),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1.fp-16385L, -1.570796326794896605353533883825294686803L, -5.867357004219059108790368268900490184505e-4916L),
    TEST_c_c (casin, 0.0L, 0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, 8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (casin, 0.0L, -0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, -8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (casin, -0.0L, 0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, 8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (casin, -0.0L, -0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, -8.813735870195430252326093249797922409363e-1L),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, 1.570796326794896605353533883825294686803L, 0.0L),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, -1.570796326794896605353533883825294686803L, 0.0L),
    TEST_c_c (casin, 0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, 1.570796326794896605353533883825294686803L, -0.0L),
    TEST_c_c (casin, -0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, -1.570796326794896605353533883825294686803L, -0.0L),
#endif
    TEST_c_c (casin, 0x1p-112L, 0.5L, 1.722604110220899568310153280070398629037e-34L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (casin, 0x1p-112L, -0.5L, 1.722604110220899568310153280070398629037e-34L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (casin, -0x1p-112L, 0.5L, -1.722604110220899568310153280070398629037e-34L, 4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (casin, -0x1p-112L, -0.5L, -1.722604110220899568310153280070398629037e-34L, -4.812118250596034474977589134243684231352e-1L),
    TEST_c_c (casin, 0.5L, 0x1p-112L, 5.235987755982988730771072305465838140329e-1L, 2.223872343664663196047610660420739543258e-34L),
    TEST_c_c (casin, -0.5L, 0x1p-112L, -5.235987755982988730771072305465838140329e-1L, 2.223872343664663196047610660420739543258e-34L),
    TEST_c_c (casin, 0.5L, -0x1p-112L, 5.235987755982988730771072305465838140329e-1L, -2.223872343664663196047610660420739543258e-34L),
    TEST_c_c (casin, -0.5L, -0x1p-112L, -5.235987755982988730771072305465838140329e-1L, -2.223872343664663196047610660420739543258e-34L),
    TEST_c_c (casin, 0x1p-112L, 0x1p-112L, 1.925929944387235853055977942584927318538e-34L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casin, 0x1p-112L, -0x1p-112L, 1.925929944387235853055977942584927318538e-34L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casin, -0x1p-112L, 0x1p-112L, -1.925929944387235853055977942584927318538e-34L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casin, -0x1p-112L, -0x1p-112L, -1.925929944387235853055977942584927318538e-34L, -1.925929944387235853055977942584927318538e-34L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casin, 0x1.fp-16385L, 0x1p-112L, 8.142593549724601460479922838826119584417e-4933L, 1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1.fp-16385L, -0x1p-112L, 8.142593549724601460479922838826119584417e-4933L, -1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, 0x1p-112L, -8.142593549724601460479922838826119584417e-4933L, 1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1.fp-16385L, -0x1p-112L, -8.142593549724601460479922838826119584417e-4933L, -1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1p-112L, 0x1.fp-16385L, 1.925929944387235853055977942584927318538e-34L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1p-112L, 0x1.fp-16385L, -1.925929944387235853055977942584927318538e-34L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, 0x1p-112L, -0x1.fp-16385L, 1.925929944387235853055977942584927318538e-34L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casin, -0x1p-112L, -0x1.fp-16385L, -1.925929944387235853055977942584927318538e-34L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (casin, 0.0L, 0x1p-112L, 0.0L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casin, 0.0L, -0x1p-112L, 0.0L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casin, -0.0L, 0x1p-112L, -0.0L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casin, -0.0L, -0x1p-112L, -0.0L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casin, 0x1p-112L, 0.0L, 1.925929944387235853055977942584927318538e-34L, 0.0L),
    TEST_c_c (casin, -0x1p-112L, 0.0L, -1.925929944387235853055977942584927318538e-34L, 0.0L),
    TEST_c_c (casin, 0x1p-112L, -0.0L, 1.925929944387235853055977942584927318538e-34L, -0.0L),
    TEST_c_c (casin, -0x1p-112L, -0.0L, -1.925929944387235853055977942584927318538e-34L, -0.0L),

    TEST_c_c (casin, 0.75L, 1.25L, 0.453276177638793913448921196101971749L, 1.13239363160530819522266333696834467L),
    TEST_c_c (casin, -2, -3, -0.57065278432109940071028387968566963L, -1.9833870299165354323470769028940395L),
    END_DATA (casin)
  };

static void
casin_test (void)
{
  errno = 0;
  FUNC(casin) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (casin);
  RUN_TEST_LOOP_c_c (casin, casin_test_data, );
  END (casin, complex);
}


static const struct test_c_c_data casinh_test_data[] =
  {
    START_DATA (casinh),
    TEST_c_c (casinh, 0, 0, 0.0, 0.0),
    TEST_c_c (casinh, minus_zero, 0, minus_zero, 0),
    TEST_c_c (casinh, 0, minus_zero, 0.0, minus_zero),
    TEST_c_c (casinh, minus_zero, minus_zero, minus_zero, minus_zero),

    TEST_c_c (casinh, plus_infty, plus_infty, plus_infty, M_PI_4l),
    TEST_c_c (casinh, plus_infty, minus_infty, plus_infty, -M_PI_4l),
    TEST_c_c (casinh, minus_infty, plus_infty, minus_infty, M_PI_4l),
    TEST_c_c (casinh, minus_infty, minus_infty, minus_infty, -M_PI_4l),

    TEST_c_c (casinh, -10.0, plus_infty, minus_infty, M_PI_2l),
    TEST_c_c (casinh, -10.0, minus_infty, minus_infty, -M_PI_2l),
    TEST_c_c (casinh, 0, plus_infty, plus_infty, M_PI_2l),
    TEST_c_c (casinh, 0, minus_infty, plus_infty, -M_PI_2l),
    TEST_c_c (casinh, minus_zero, plus_infty, minus_infty, M_PI_2l),
    TEST_c_c (casinh, minus_zero, minus_infty, minus_infty, -M_PI_2l),
    TEST_c_c (casinh, 0.1L, plus_infty, plus_infty, M_PI_2l),
    TEST_c_c (casinh, 0.1L, minus_infty, plus_infty, -M_PI_2l),

    TEST_c_c (casinh, minus_infty, 0, minus_infty, 0.0),
    TEST_c_c (casinh, minus_infty, minus_zero, minus_infty, minus_zero),
    TEST_c_c (casinh, minus_infty, 100, minus_infty, 0.0),
    TEST_c_c (casinh, minus_infty, -100, minus_infty, minus_zero),

    TEST_c_c (casinh, plus_infty, 0, plus_infty, 0.0),
    TEST_c_c (casinh, plus_infty, minus_zero, plus_infty, minus_zero),
    TEST_c_c (casinh, plus_infty, 0.5, plus_infty, 0.0),
    TEST_c_c (casinh, plus_infty, -0.5, plus_infty, minus_zero),

    TEST_c_c (casinh, plus_infty, qnan_value, plus_infty, qnan_value),
    TEST_c_c (casinh, minus_infty, qnan_value, minus_infty, qnan_value),

    TEST_c_c (casinh, qnan_value, 0, qnan_value, 0.0),
    TEST_c_c (casinh, qnan_value, minus_zero, qnan_value, minus_zero),

    TEST_c_c (casinh, qnan_value, plus_infty, plus_infty, qnan_value, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (casinh, qnan_value, minus_infty, plus_infty, qnan_value, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (casinh, 10.5, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (casinh, -10.5, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (casinh, qnan_value, 0.75, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (casinh, -0.75, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (casinh, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (casinh, plus_zero, -1.5L, 0.9624236501192068949955178268487368462704L, -M_PI_2l),
    TEST_c_c (casinh, minus_zero, -1.5L, -0.9624236501192068949955178268487368462704L, -M_PI_2l),
    TEST_c_c (casinh, plus_zero, -1.0L, plus_zero, -M_PI_2l),
    TEST_c_c (casinh, minus_zero, -1.0L, minus_zero, -M_PI_2l),
    TEST_c_c (casinh, plus_zero, -0.5L, plus_zero, -0.5235987755982988730771072305465838140329L),
    TEST_c_c (casinh, minus_zero, -0.5L, minus_zero, -0.5235987755982988730771072305465838140329L),
    TEST_c_c (casinh, plus_zero, 0.5L, plus_zero, 0.5235987755982988730771072305465838140329L),
    TEST_c_c (casinh, minus_zero, 0.5L, minus_zero, 0.5235987755982988730771072305465838140329L),
    TEST_c_c (casinh, plus_zero, 1.0L, plus_zero, M_PI_2l),
    TEST_c_c (casinh, minus_zero, 1.0L, minus_zero, M_PI_2l),
    TEST_c_c (casinh, plus_zero, 1.5L, 0.9624236501192068949955178268487368462704L, M_PI_2l),
    TEST_c_c (casinh, minus_zero, 1.5L, -0.9624236501192068949955178268487368462704L, M_PI_2l),

    TEST_c_c (casinh, -1.5L, plus_zero, -1.194763217287109304111930828519090523536L, plus_zero),
    TEST_c_c (casinh, -1.5L, minus_zero, -1.194763217287109304111930828519090523536L, minus_zero),
    TEST_c_c (casinh, -1.0L, plus_zero, -0.8813735870195430252326093249797923090282L, plus_zero),
    TEST_c_c (casinh, -1.0L, minus_zero, -0.8813735870195430252326093249797923090282L, minus_zero),
    TEST_c_c (casinh, -0.5L, plus_zero, -0.4812118250596034474977589134243684231352L, plus_zero),
    TEST_c_c (casinh, -0.5L, minus_zero, -0.4812118250596034474977589134243684231352L, minus_zero),
    TEST_c_c (casinh, 0.5L, plus_zero, 0.4812118250596034474977589134243684231352L, plus_zero),
    TEST_c_c (casinh, 0.5L, minus_zero, 0.4812118250596034474977589134243684231352L, minus_zero),
    TEST_c_c (casinh, 1.0L, plus_zero, 0.8813735870195430252326093249797923090282L, plus_zero),
    TEST_c_c (casinh, 1.0L, minus_zero, 0.8813735870195430252326093249797923090282L, minus_zero),
    TEST_c_c (casinh, 1.5L, plus_zero, 1.194763217287109304111930828519090523536L, plus_zero),
    TEST_c_c (casinh, 1.5L, minus_zero, 1.194763217287109304111930828519090523536L, minus_zero),

    TEST_c_c (casinh, 0x1p50L, 1.0L, 3.535050620855721078027883819436759661753e1L, 8.881784197001252323389053344720723756399e-16L),
    TEST_c_c (casinh, 0x1p50L, -1.0L, 3.535050620855721078027883819436759661753e1L, -8.881784197001252323389053344720723756399e-16L),
    TEST_c_c (casinh, -0x1p50L, 1.0L, -3.535050620855721078027883819436759661753e1L, 8.881784197001252323389053344720723756399e-16L),
    TEST_c_c (casinh, -0x1p50L, -1.0L, -3.535050620855721078027883819436759661753e1L, -8.881784197001252323389053344720723756399e-16L),
    TEST_c_c (casinh, 1.0L, 0x1p50L, 3.535050620855721078027883819436720218708e1L, 1.570796326794895731052901991514519103193L),
    TEST_c_c (casinh, -1.0L, 0x1p50L, -3.535050620855721078027883819436720218708e1L, 1.570796326794895731052901991514519103193L),
    TEST_c_c (casinh, 1.0L, -0x1p50L, 3.535050620855721078027883819436720218708e1L, -1.570796326794895731052901991514519103193L),
    TEST_c_c (casinh, -1.0L, -0x1p50L, -3.535050620855721078027883819436720218708e1L, -1.570796326794895731052901991514519103193L),
#ifndef TEST_FLOAT
    TEST_c_c (casinh, 0x1p500L, 1.0L, 3.472667374605326000180332928505464606058e2L, 3.054936363499604682051979393213617699789e-151L),
    TEST_c_c (casinh, 0x1p500L, -1.0L, 3.472667374605326000180332928505464606058e2L, -3.054936363499604682051979393213617699789e-151L),
    TEST_c_c (casinh, -0x1p500L, 1.0L, -3.472667374605326000180332928505464606058e2L, 3.054936363499604682051979393213617699789e-151L),
    TEST_c_c (casinh, -0x1p500L, -1.0L, -3.472667374605326000180332928505464606058e2L, -3.054936363499604682051979393213617699789e-151L),
    TEST_c_c (casinh, 1.0L, 0x1p500L, 3.472667374605326000180332928505464606058e2L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -1.0L, 0x1p500L, -3.472667374605326000180332928505464606058e2L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 1.0L, -0x1p500L, 3.472667374605326000180332928505464606058e2L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -1.0L, -0x1p500L, -3.472667374605326000180332928505464606058e2L, -1.570796326794896619231321691639751442099L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (casinh, 0x1p5000L, 1.0L, 3.466429049980286492395577839412341016946e3L, 7.079811261048172892385615158694057552948e-1506L),
    TEST_c_c (casinh, 0x1p5000L, -1.0L, 3.466429049980286492395577839412341016946e3L, -7.079811261048172892385615158694057552948e-1506L),
    TEST_c_c (casinh, -0x1p5000L, 1.0L, -3.466429049980286492395577839412341016946e3L, 7.079811261048172892385615158694057552948e-1506L),
    TEST_c_c (casinh, -0x1p5000L, -1.0L, -3.466429049980286492395577839412341016946e3L, -7.079811261048172892385615158694057552948e-1506L),
    TEST_c_c (casinh, 1.0L, 0x1p5000L, 3.466429049980286492395577839412341016946e3L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -1.0L, 0x1p5000L, -3.466429049980286492395577839412341016946e3L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 1.0L, -0x1p5000L, 3.466429049980286492395577839412341016946e3L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -1.0L, -0x1p5000L, -3.466429049980286492395577839412341016946e3L, -1.570796326794896619231321691639751442099L),
#endif

    TEST_c_c (casinh, 0x1.fp127L, 0x1.fp127L, 8.973081118419833726837456344608533993585e1L, 7.853981633974483096156608458198757210493e-1L),
#ifndef TEST_FLOAT
    TEST_c_c (casinh, 0x1.fp1023L, 0x1.fp1023L, 7.107906849659093345062145442726115449315e2L, 7.853981633974483096156608458198757210493e-1L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (casinh, 0x1.fp16383L, 0x1.fp16383L, 1.135753137836666928715489992987020363057e4L, 7.853981633974483096156608458198757210493e-1L),
#endif

    TEST_c_c (casinh, 0x1.fp-129L, 1.5L, 9.624236501192068949955178268487368462704e-1L, 1.570796326794896619231321691639751442096L),
    TEST_c_c (casinh, 0x1.fp-129L, -1.5L, 9.624236501192068949955178268487368462704e-1L, -1.570796326794896619231321691639751442096L),
    TEST_c_c (casinh, -0x1.fp-129L, 1.5L, -9.624236501192068949955178268487368462704e-1L, 1.570796326794896619231321691639751442096L),
    TEST_c_c (casinh, -0x1.fp-129L, -1.5L, -9.624236501192068949955178268487368462704e-1L, -1.570796326794896619231321691639751442096L),
    TEST_c_c (casinh, 1.5L, 0x1.fp-129L, 1.194763217287109304111930828519090523536L, 1.579176199917649005841160751101628985741e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -1.5L, 0x1.fp-129L, -1.194763217287109304111930828519090523536L, 1.579176199917649005841160751101628985741e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 1.5L, -0x1.fp-129L, 1.194763217287109304111930828519090523536L, -1.579176199917649005841160751101628985741e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -1.5L, -0x1.fp-129L, -1.194763217287109304111930828519090523536L, -1.579176199917649005841160751101628985741e-39L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (casinh, 0x1.fp-1025L, 1.5L, 9.624236501192068949955178268487368462704e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.fp-1025L, -1.5L, 9.624236501192068949955178268487368462704e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-1025L, 1.5L, -9.624236501192068949955178268487368462704e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-1025L, -1.5L, -9.624236501192068949955178268487368462704e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 1.5L, 0x1.fp-1025L, 1.194763217287109304111930828519090523536L, 2.989196569048182929051881765490354365918e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -1.5L, 0x1.fp-1025L, -1.194763217287109304111930828519090523536L, 2.989196569048182929051881765490354365918e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 1.5L, -0x1.fp-1025L, 1.194763217287109304111930828519090523536L, -2.989196569048182929051881765490354365918e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -1.5L, -0x1.fp-1025L, -1.194763217287109304111930828519090523536L, -2.989196569048182929051881765490354365918e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casinh, 0x1.fp-16385L, 1.5L, 9.624236501192068949955178268487368462704e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.fp-16385L, -1.5L, 9.624236501192068949955178268487368462704e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-16385L, 1.5L, -9.624236501192068949955178268487368462704e-1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-16385L, -1.5L, -9.624236501192068949955178268487368462704e-1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 1.5L, 0x1.fp-16385L, 1.194763217287109304111930828519090523536L, 4.516698239814521372306784062043266700598e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -1.5L, 0x1.fp-16385L, -1.194763217287109304111930828519090523536L, 4.516698239814521372306784062043266700598e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 1.5L, -0x1.fp-16385L, 1.194763217287109304111930828519090523536L, -4.516698239814521372306784062043266700598e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -1.5L, -0x1.fp-16385L, -1.194763217287109304111930828519090523536L, -4.516698239814521372306784062043266700598e-4933L, UNDERFLOW_EXCEPTION),
#endif

    TEST_c_c (casinh, 0.5L, 1.0L, 7.328576759736452608886724437653071523305e-1L, 8.959074812088902390666567243275770102229e-1L),
    TEST_c_c (casinh, 0.5L, -1.0L, 7.328576759736452608886724437653071523305e-1L, -8.959074812088902390666567243275770102229e-1L),
    TEST_c_c (casinh, -0.5L, 1.0L, -7.328576759736452608886724437653071523305e-1L, 8.959074812088902390666567243275770102229e-1L),
    TEST_c_c (casinh, -0.5L, -1.0L, -7.328576759736452608886724437653071523305e-1L, -8.959074812088902390666567243275770102229e-1L),
    TEST_c_c (casinh, 1.0L, 0.5L, 9.261330313501824245501244453057873152694e-1L, 3.494390628572132936274118260753699527325e-1L),
    TEST_c_c (casinh, -1.0L, 0.5L, -9.261330313501824245501244453057873152694e-1L, 3.494390628572132936274118260753699527325e-1L),
    TEST_c_c (casinh, 1.0L, -0.5L, 9.261330313501824245501244453057873152694e-1L, -3.494390628572132936274118260753699527325e-1L),
    TEST_c_c (casinh, -1.0L, -0.5L, -9.261330313501824245501244453057873152694e-1L, -3.494390628572132936274118260753699527325e-1L),
    TEST_c_c (casinh, 0.25L, 1.0L, 5.097911466811016354623559941115413499164e-1L, 1.081751996523816326311037318425097434186L),
    TEST_c_c (casinh, 0.25L, -1.0L, 5.097911466811016354623559941115413499164e-1L, -1.081751996523816326311037318425097434186L),
    TEST_c_c (casinh, -0.25L, 1.0L, -5.097911466811016354623559941115413499164e-1L, 1.081751996523816326311037318425097434186L),
    TEST_c_c (casinh, -0.25L, -1.0L, -5.097911466811016354623559941115413499164e-1L, -1.081751996523816326311037318425097434186L),
    TEST_c_c (casinh, 1.0L, 0.25L, 8.924633639033482359562124741744951972772e-1L, 1.763024327769669304186785666360901026468e-1L),
    TEST_c_c (casinh, -1.0L, 0.25L, -8.924633639033482359562124741744951972772e-1L, 1.763024327769669304186785666360901026468e-1L),
    TEST_c_c (casinh, 1.0L, -0.25L, 8.924633639033482359562124741744951972772e-1L, -1.763024327769669304186785666360901026468e-1L),
    TEST_c_c (casinh, -1.0L, -0.25L, -8.924633639033482359562124741744951972772e-1L, -1.763024327769669304186785666360901026468e-1L),
    TEST_c_c (casinh, 0x1.fp-10L, 1.0L, 4.350501469856803800217957402220976497152e-2L, 1.527305029163877791518741192097931722508L),
    TEST_c_c (casinh, 0x1.fp-10L, -1.0L, 4.350501469856803800217957402220976497152e-2L, -1.527305029163877791518741192097931722508L),
    TEST_c_c (casinh, -0x1.fp-10L, 1.0L, -4.350501469856803800217957402220976497152e-2L, 1.527305029163877791518741192097931722508L),
    TEST_c_c (casinh, -0x1.fp-10L, -1.0L, -4.350501469856803800217957402220976497152e-2L, -1.527305029163877791518741192097931722508L),
    TEST_c_c (casinh, 1.0L, 0x1.fp-10L, 8.813742198809567991336704287826445879025e-1L, 1.337909359557740912558349531052239112857e-3L),
    TEST_c_c (casinh, -1.0L, 0x1.fp-10L, -8.813742198809567991336704287826445879025e-1L, 1.337909359557740912558349531052239112857e-3L),
    TEST_c_c (casinh, 1.0L, -0x1.fp-10L, 8.813742198809567991336704287826445879025e-1L, -1.337909359557740912558349531052239112857e-3L),
    TEST_c_c (casinh, -1.0L, -0x1.fp-10L, -8.813742198809567991336704287826445879025e-1L, -1.337909359557740912558349531052239112857e-3L),
    TEST_c_c (casinh, 0x1.fp-30L, 1.0L, 4.247867098745151888768727039216644758847e-5L, 1.570753848123921942730162693731872690232L),
    TEST_c_c (casinh, 0x1.fp-30L, -1.0L, 4.247867098745151888768727039216644758847e-5L, -1.570753848123921942730162693731872690232L),
    TEST_c_c (casinh, -0x1.fp-30L, 1.0L, -4.247867098745151888768727039216644758847e-5L, 1.570753848123921942730162693731872690232L),
    TEST_c_c (casinh, -0x1.fp-30L, -1.0L, -4.247867098745151888768727039216644758847e-5L, -1.570753848123921942730162693731872690232L),
    TEST_c_c (casinh, 1.0L, 0x1.fp-30L, 8.813735870195430258081932989769495326854e-1L, 1.275929984216518540252717485007112529021e-9L),
    TEST_c_c (casinh, -1.0L, 0x1.fp-30L, -8.813735870195430258081932989769495326854e-1L, 1.275929984216518540252717485007112529021e-9L),
    TEST_c_c (casinh, 1.0L, -0x1.fp-30L, 8.813735870195430258081932989769495326854e-1L, -1.275929984216518540252717485007112529021e-9L),
    TEST_c_c (casinh, -1.0L, -0x1.fp-30L, -8.813735870195430258081932989769495326854e-1L, -1.275929984216518540252717485007112529021e-9L),
    TEST_c_c (casinh, 0x1.fp-100L, 1.0L, 1.236292038260260888664514866457202186027e-15L, 1.570796326794895382939283431378862777584L),
    TEST_c_c (casinh, 0x1.fp-100L, -1.0L, 1.236292038260260888664514866457202186027e-15L, -1.570796326794895382939283431378862777584L),
    TEST_c_c (casinh, -0x1.fp-100L, 1.0L, -1.236292038260260888664514866457202186027e-15L, 1.570796326794895382939283431378862777584L),
    TEST_c_c (casinh, -0x1.fp-100L, -1.0L, -1.236292038260260888664514866457202186027e-15L, -1.570796326794895382939283431378862777584L),
    TEST_c_c (casinh, 1.0L, 0x1.fp-100L, 8.813735870195430252326093249797923090282e-1L, 1.080754735021050612990719608916167354321e-30L),
    TEST_c_c (casinh, -1.0L, 0x1.fp-100L, -8.813735870195430252326093249797923090282e-1L, 1.080754735021050612990719608916167354321e-30L),
    TEST_c_c (casinh, 1.0L, -0x1.fp-100L, 8.813735870195430252326093249797923090282e-1L, -1.080754735021050612990719608916167354321e-30L),
    TEST_c_c (casinh, -1.0L, -0x1.fp-100L, -8.813735870195430252326093249797923090282e-1L, -1.080754735021050612990719608916167354321e-30L),
    TEST_c_c (casinh, 0x1.fp-129L, 1.0L, 5.335635276982233498398987585285818977933e-20L, 1.570796326794896619177965338869929107115L),
    TEST_c_c (casinh, 0x1.fp-129L, -1.0L, 5.335635276982233498398987585285818977933e-20L, -1.570796326794896619177965338869929107115L),
    TEST_c_c (casinh, -0x1.fp-129L, 1.0L, -5.335635276982233498398987585285818977933e-20L, 1.570796326794896619177965338869929107115L),
    TEST_c_c (casinh, -0x1.fp-129L, -1.0L, -5.335635276982233498398987585285818977933e-20L, -1.570796326794896619177965338869929107115L),
    TEST_c_c (casinh, 1.0L, 0x1.fp-129L, 8.813735870195430252326093249797923090282e-1L, 2.013062564695348242280482517399205554874e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -1.0L, 0x1.fp-129L, -8.813735870195430252326093249797923090282e-1L, 2.013062564695348242280482517399205554874e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 1.0L, -0x1.fp-129L, 8.813735870195430252326093249797923090282e-1L, -2.013062564695348242280482517399205554874e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -1.0L, -0x1.fp-129L, -8.813735870195430252326093249797923090282e-1L, -2.013062564695348242280482517399205554874e-39L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (casinh, 0x1.fp-1000L, 1.0L, 4.252291453851660175550490409247739011867e-151L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.fp-1000L, -1.0L, 4.252291453851660175550490409247739011867e-151L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-1000L, 1.0L, -4.252291453851660175550490409247739011867e-151L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-1000L, -1.0L, -4.252291453851660175550490409247739011867e-151L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 1.0L, 0x1.fp-1000L, 8.813735870195430252326093249797923090282e-1L, 1.278589251976747242280879285935084814093e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, -1.0L, 0x1.fp-1000L, -8.813735870195430252326093249797923090282e-1L, 1.278589251976747242280879285935084814093e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, 1.0L, -0x1.fp-1000L, 8.813735870195430252326093249797923090282e-1L, -1.278589251976747242280879285935084814093e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, -1.0L, -0x1.fp-1000L, -8.813735870195430252326093249797923090282e-1L, -1.278589251976747242280879285935084814093e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, 0x1.fp-1025L, 1.0L, 7.340879205566679497036857179189356754017e-155L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.fp-1025L, -1.0L, 7.340879205566679497036857179189356754017e-155L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-1025L, 1.0L, -7.340879205566679497036857179189356754017e-155L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-1025L, -1.0L, -7.340879205566679497036857179189356754017e-155L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 1.0L, 0x1.fp-1025L, 8.813735870195430252326093249797923090282e-1L, 3.810492908885321743133304375216617626230e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -1.0L, 0x1.fp-1025L, -8.813735870195430252326093249797923090282e-1L, 3.810492908885321743133304375216617626230e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 1.0L, -0x1.fp-1025L, 8.813735870195430252326093249797923090282e-1L, -3.810492908885321743133304375216617626230e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -1.0L, -0x1.fp-1025L, -8.813735870195430252326093249797923090282e-1L, -3.810492908885321743133304375216617626230e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casinh, 0x1.fp-10000L, 1.0L, 9.854680208706673586644342922051388714633e-1506L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.fp-10000L, -1.0L, 9.854680208706673586644342922051388714633e-1506L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-10000L, 1.0L, -9.854680208706673586644342922051388714633e-1506L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-10000L, -1.0L, -9.854680208706673586644342922051388714633e-1506L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 1.0L, 0x1.fp-10000L, 8.813735870195430252326093249797923090282e-1L, 6.867047849047171855399183659351043150871e-3011L),
    TEST_c_c (casinh, -1.0L, 0x1.fp-10000L, -8.813735870195430252326093249797923090282e-1L, 6.867047849047171855399183659351043150871e-3011L),
    TEST_c_c (casinh, 1.0L, -0x1.fp-10000L, 8.813735870195430252326093249797923090282e-1L, -6.867047849047171855399183659351043150871e-3011L),
    TEST_c_c (casinh, -1.0L, -0x1.fp-10000L, -8.813735870195430252326093249797923090282e-1L, -6.867047849047171855399183659351043150871e-3011L),
    TEST_c_c (casinh, 0x1.fp-16385L, 1.0L, 9.023632056840860275214893047597614177639e-2467L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.fp-16385L, -1.0L, 9.023632056840860275214893047597614177639e-2467L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-16385L, 1.0L, -9.023632056840860275214893047597614177639e-2467L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-16385L, -1.0L, -9.023632056840860275214893047597614177639e-2467L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 1.0L, 0x1.fp-16385L, 8.813735870195430252326093249797923090282e-1L, 5.757683115456107044131264955348448954458e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -1.0L, 0x1.fp-16385L, -8.813735870195430252326093249797923090282e-1L, 5.757683115456107044131264955348448954458e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 1.0L, -0x1.fp-16385L, 8.813735870195430252326093249797923090282e-1L, -5.757683115456107044131264955348448954458e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -1.0L, -0x1.fp-16385L, -8.813735870195430252326093249797923090282e-1L, -5.757683115456107044131264955348448954458e-4933L, UNDERFLOW_EXCEPTION),
#endif

    TEST_c_c (casinh, 0x1p-23L, 0x1.000002p0L, 5.364668491573609633134147164031476452679e-4L, 1.570574114956455764594481054234834678422L),
    TEST_c_c (casinh, 0x1p-23L, -0x1.000002p0L, 5.364668491573609633134147164031476452679e-4L, -1.570574114956455764594481054234834678422L),
    TEST_c_c (casinh, -0x1p-23L, 0x1.000002p0L, -5.364668491573609633134147164031476452679e-4L, 1.570574114956455764594481054234834678422L),
    TEST_c_c (casinh, -0x1p-23L, -0x1.000002p0L, -5.364668491573609633134147164031476452679e-4L, -1.570574114956455764594481054234834678422L),
    TEST_c_c (casinh, 0x1.000002p0L, 0x1p-23L, 8.813736713132400470205730751186547909968e-1L, 8.429369199749229560964789467980644296420e-8L),
    TEST_c_c (casinh, -0x1.000002p0L, 0x1p-23L, -8.813736713132400470205730751186547909968e-1L, 8.429369199749229560964789467980644296420e-8L),
    TEST_c_c (casinh, 0x1.000002p0L, -0x1p-23L, 8.813736713132400470205730751186547909968e-1L, -8.429369199749229560964789467980644296420e-8L),
    TEST_c_c (casinh, -0x1.000002p0L, -0x1p-23L, -8.813736713132400470205730751186547909968e-1L, -8.429369199749229560964789467980644296420e-8L),
    TEST_c_c (casinh, 0x1.fp-129L, 0x1.000002p0L, 4.882812451493617206486388134172712975070e-4L, 1.570796326794896619231321691639751436268L),
    TEST_c_c (casinh, 0x1.fp-129L, -0x1.000002p0L, 4.882812451493617206486388134172712975070e-4L, -1.570796326794896619231321691639751436268L),
    TEST_c_c (casinh, -0x1.fp-129L, 0x1.000002p0L, -4.882812451493617206486388134172712975070e-4L, 1.570796326794896619231321691639751436268L),
    TEST_c_c (casinh, -0x1.fp-129L, -0x1.000002p0L, -4.882812451493617206486388134172712975070e-4L, -1.570796326794896619231321691639751436268L),
    TEST_c_c (casinh, 0x1.000002p0L, 0x1.fp-129L, 8.813736713132375348727889167749389235161e-1L, 2.013062444707472738895109955455676357057e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0x1.000002p0L, 0x1.fp-129L, -8.813736713132375348727889167749389235161e-1L, 2.013062444707472738895109955455676357057e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 0x1.000002p0L, -0x1.fp-129L, 8.813736713132375348727889167749389235161e-1L, -2.013062444707472738895109955455676357057e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0x1.000002p0L, -0x1.fp-129L, -8.813736713132375348727889167749389235161e-1L, -2.013062444707472738895109955455676357057e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 0.0L, 0x1.000002p0L, 4.882812451493617206486388134172712975070e-4L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0.0L, -0x1.000002p0L, 4.882812451493617206486388134172712975070e-4L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0.0L, 0x1.000002p0L, -4.882812451493617206486388134172712975070e-4L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0.0L, -0x1.000002p0L, -4.882812451493617206486388134172712975070e-4L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.000002p0L, 0.0L, 8.813736713132375348727889167749389235161e-1L, 0.0L),
    TEST_c_c (casinh, -0x1.000002p0L, 0.0L, -8.813736713132375348727889167749389235161e-1L, 0.0L),
    TEST_c_c (casinh, 0x1.000002p0L, -0.0L, 8.813736713132375348727889167749389235161e-1L, -0.0L),
    TEST_c_c (casinh, -0x1.000002p0L, -0.0L, -8.813736713132375348727889167749389235161e-1L, -0.0L),
#ifndef TEST_FLOAT
    TEST_c_c (casinh, 0x1p-52L, 0x1.0000000000001p0L, 2.315303644582684770975188768022139415020e-8L, 1.570796317204594913251280305810847349436L),
    TEST_c_c (casinh, 0x1p-52L, -0x1.0000000000001p0L, 2.315303644582684770975188768022139415020e-8L, -1.570796317204594913251280305810847349436L),
    TEST_c_c (casinh, -0x1p-52L, 0x1.0000000000001p0L, -2.315303644582684770975188768022139415020e-8L, 1.570796317204594913251280305810847349436L),
    TEST_c_c (casinh, -0x1p-52L, -0x1.0000000000001p0L, -2.315303644582684770975188768022139415020e-8L, -1.570796317204594913251280305810847349436L),
    TEST_c_c (casinh, 0x1.0000000000001p0L, 0x1p-52L, 8.813735870195431822418551933572982483664e-1L, 1.570092458683774885078102529858632363236e-16L),
    TEST_c_c (casinh, -0x1.0000000000001p0L, 0x1p-52L, -8.813735870195431822418551933572982483664e-1L, 1.570092458683774885078102529858632363236e-16L),
    TEST_c_c (casinh, 0x1.0000000000001p0L, -0x1p-52L, 8.813735870195431822418551933572982483664e-1L, -1.570092458683774885078102529858632363236e-16L),
    TEST_c_c (casinh, -0x1.0000000000001p0L, -0x1p-52L, -8.813735870195431822418551933572982483664e-1L, -1.570092458683774885078102529858632363236e-16L),
    TEST_c_c (casinh, 0x1.fp-1025L, 0x1.0000000000001p0L, 2.107342425544701550354780375182800088393e-8L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.fp-1025L, -0x1.0000000000001p0L, 2.107342425544701550354780375182800088393e-8L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-1025L, 0x1.0000000000001p0L, -2.107342425544701550354780375182800088393e-8L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-1025L, -0x1.0000000000001p0L, -2.107342425544701550354780375182800088393e-8L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.0000000000001p0L, 0x1.fp-1025L, 8.813735870195431822418551933572895326024e-1L, 3.810492908885321320083608113679347200012e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.0000000000001p0L, 0x1.fp-1025L, -8.813735870195431822418551933572895326024e-1L, 3.810492908885321320083608113679347200012e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0x1.0000000000001p0L, -0x1.fp-1025L, 8.813735870195431822418551933572895326024e-1L, -3.810492908885321320083608113679347200012e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.0000000000001p0L, -0x1.fp-1025L, -8.813735870195431822418551933572895326024e-1L, -3.810492908885321320083608113679347200012e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0.0L, 0x1.0000000000001p0L, 2.107342425544701550354780375182800088393e-8L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0.0L, -0x1.0000000000001p0L, 2.107342425544701550354780375182800088393e-8L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0.0L, 0x1.0000000000001p0L, -2.107342425544701550354780375182800088393e-8L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0.0L, -0x1.0000000000001p0L, -2.107342425544701550354780375182800088393e-8L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.0000000000001p0L, 0.0L, 8.813735870195431822418551933572895326024e-1L, 0.0L),
    TEST_c_c (casinh, -0x1.0000000000001p0L, 0.0L, -8.813735870195431822418551933572895326024e-1L, 0.0L),
    TEST_c_c (casinh, 0x1.0000000000001p0L, -0.0L, 8.813735870195431822418551933572895326024e-1L, -0.0L),
    TEST_c_c (casinh, -0x1.0000000000001p0L, -0.0L, -8.813735870195431822418551933572895326024e-1L, -0.0L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (casinh, 0x1p-63L, 0x1.0000000000000002p0L, 5.116146586219826555037807251857670783420e-10L, 1.570796326582978888921215348380499122131L),
    TEST_c_c (casinh, 0x1p-63L, -0x1.0000000000000002p0L, 5.116146586219826555037807251857670783420e-10L, -1.570796326582978888921215348380499122131L),
    TEST_c_c (casinh, -0x1p-63L, 0x1.0000000000000002p0L, -5.116146586219826555037807251857670783420e-10L, 1.570796326582978888921215348380499122131L),
    TEST_c_c (casinh, -0x1p-63L, -0x1.0000000000000002p0L, -5.116146586219826555037807251857670783420e-10L, -1.570796326582978888921215348380499122131L),
    TEST_c_c (casinh, 0x1.0000000000000002p0L, 0x1p-63L, 8.813735870195430253092739958139610131001e-1L, 7.666467083416870406778649849746878368519e-20L),
    TEST_c_c (casinh, -0x1.0000000000000002p0L, 0x1p-63L, -8.813735870195430253092739958139610131001e-1L, 7.666467083416870406778649849746878368519e-20L),
    TEST_c_c (casinh, 0x1.0000000000000002p0L, -0x1p-63L, 8.813735870195430253092739958139610131001e-1L, -7.666467083416870406778649849746878368519e-20L),
    TEST_c_c (casinh, -0x1.0000000000000002p0L, -0x1p-63L, -8.813735870195430253092739958139610131001e-1L, -7.666467083416870406778649849746878368519e-20L),
# if LDBL_MIN_EXP <= -16381
    TEST_c_c (casinh, 0x1.fp-16385L, 0x1.0000000000000002p0L, 4.656612873077392578082927418388212703712e-10L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.fp-16385L, -0x1.0000000000000002p0L, 4.656612873077392578082927418388212703712e-10L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-16385L, 0x1.0000000000000002p0L, -4.656612873077392578082927418388212703712e-10L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-16385L, -0x1.0000000000000002p0L, -4.656612873077392578082927418388212703712e-10L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.0000000000000002p0L, 0x1.fp-16385L, 8.813735870195430253092739958139610130980e-1L, 5.757683115456107043819140328235418018963e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.0000000000000002p0L, 0x1.fp-16385L, -8.813735870195430253092739958139610130980e-1L, 5.757683115456107043819140328235418018963e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x1.0000000000000002p0L, -0x1.fp-16385L, 8.813735870195430253092739958139610130980e-1L, -5.757683115456107043819140328235418018963e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.0000000000000002p0L, -0x1.fp-16385L, -8.813735870195430253092739958139610130980e-1L, -5.757683115456107043819140328235418018963e-4933L, UNDERFLOW_EXCEPTION),
# endif
    TEST_c_c (casinh, 0.0L, 0x1.0000000000000002p0L, 4.656612873077392578082927418388212703712e-10L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0.0L, -0x1.0000000000000002p0L, 4.656612873077392578082927418388212703712e-10L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0.0L, 0x1.0000000000000002p0L, -4.656612873077392578082927418388212703712e-10L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0.0L, -0x1.0000000000000002p0L, -4.656612873077392578082927418388212703712e-10L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.0000000000000002p0L, 0.0L, 8.813735870195430253092739958139610130980e-1L, 0.0L),
    TEST_c_c (casinh, -0x1.0000000000000002p0L, 0.0L, -8.813735870195430253092739958139610130980e-1L, 0.0L),
    TEST_c_c (casinh, 0x1.0000000000000002p0L, -0.0L, 8.813735870195430253092739958139610130980e-1L, -0.0L),
    TEST_c_c (casinh, -0x1.0000000000000002p0L, -0.0L, -8.813735870195430253092739958139610130980e-1L, -0.0L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (casinh, 0x1p-106L, 0x1.000000000000000000000000008p0L, 2.285028863093221674154232933662774454211e-16L, 1.570796326794896565289107467733682961928L),
    TEST_c_c (casinh, 0x1p-106L, -0x1.000000000000000000000000008p0L, 2.285028863093221674154232933662774454211e-16L, -1.570796326794896565289107467733682961928L),
    TEST_c_c (casinh, -0x1p-106L, 0x1.000000000000000000000000008p0L, -2.285028863093221674154232933662774454211e-16L, 1.570796326794896565289107467733682961928L),
    TEST_c_c (casinh, -0x1p-106L, -0x1.000000000000000000000000008p0L, -2.285028863093221674154232933662774454211e-16L, -1.570796326794896565289107467733682961928L),
    TEST_c_c (casinh, 0x1.000000000000000000000000008p0L, 0x1p-106L, 8.813735870195430252326093249798097405561e-1L, 8.715763992105246878957416200936726072500e-33L),
    TEST_c_c (casinh, -0x1.000000000000000000000000008p0L, 0x1p-106L, -8.813735870195430252326093249798097405561e-1L, 8.715763992105246878957416200936726072500e-33L),
    TEST_c_c (casinh, 0x1.000000000000000000000000008p0L, -0x1p-106L, 8.813735870195430252326093249798097405561e-1L, -8.715763992105246878957416200936726072500e-33L),
    TEST_c_c (casinh, -0x1.000000000000000000000000008p0L, -0x1p-106L, -8.813735870195430252326093249798097405561e-1L, -8.715763992105246878957416200936726072500e-33L),
    TEST_c_c (casinh, 0x1.fp-1025L, 0x1.000000000000000000000000008p0L, 2.220446049250313080847263336181636063482e-16L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.fp-1025L, -0x1.000000000000000000000000008p0L, 2.220446049250313080847263336181636063482e-16L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-1025L, 0x1.000000000000000000000000008p0L, -2.220446049250313080847263336181636063482e-16L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-1025L, -0x1.000000000000000000000000008p0L, -2.220446049250313080847263336181636063482e-16L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.000000000000000000000000008p0L, 0x1.fp-1025L, 8.813735870195430252326093249798097405561e-1L, 3.810492908885321743133304375216570658278e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.000000000000000000000000008p0L, 0x1.fp-1025L, -8.813735870195430252326093249798097405561e-1L, 3.810492908885321743133304375216570658278e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0x1.000000000000000000000000008p0L, -0x1.fp-1025L, 8.813735870195430252326093249798097405561e-1L, -3.810492908885321743133304375216570658278e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.000000000000000000000000008p0L, -0x1.fp-1025L, -8.813735870195430252326093249798097405561e-1L, -3.810492908885321743133304375216570658278e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0.0L, 0x1.000000000000000000000000008p0L, 2.220446049250313080847263336181636063482e-16L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0.0L, -0x1.000000000000000000000000008p0L, 2.220446049250313080847263336181636063482e-16L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0.0L, 0x1.000000000000000000000000008p0L, -2.220446049250313080847263336181636063482e-16L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0.0L, -0x1.000000000000000000000000008p0L, -2.220446049250313080847263336181636063482e-16L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.000000000000000000000000008p0L, 0.0L, 8.813735870195430252326093249798097405561e-1L, 0.0L),
    TEST_c_c (casinh, -0x1.000000000000000000000000008p0L, 0.0L, -8.813735870195430252326093249798097405561e-1L, 0.0L),
    TEST_c_c (casinh, 0x1.000000000000000000000000008p0L, -0.0L, 8.813735870195430252326093249798097405561e-1L, -0.0L),
    TEST_c_c (casinh, -0x1.000000000000000000000000008p0L, -0.0L, -8.813735870195430252326093249798097405561e-1L, -0.0L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (casinh, 0x1p-113L, 0x1.0000000000000000000000000001p0L, 2.019699255375255198156433710951064632386e-17L, 1.570796326794896614463458507897073960405L),
    TEST_c_c (casinh, 0x1p-113L, -0x1.0000000000000000000000000001p0L, 2.019699255375255198156433710951064632386e-17L, -1.570796326794896614463458507897073960405L),
    TEST_c_c (casinh, -0x1p-113L, 0x1.0000000000000000000000000001p0L, -2.019699255375255198156433710951064632386e-17L, 1.570796326794896614463458507897073960405L),
    TEST_c_c (casinh, -0x1p-113L, -0x1.0000000000000000000000000001p0L, -2.019699255375255198156433710951064632386e-17L, -1.570796326794896614463458507897073960405L),
    TEST_c_c (casinh, 0x1.0000000000000000000000000001p0L, 0x1p-113L, 8.813735870195430252326093249797924452120e-1L, 6.809190618832224124185481406981900518193e-35L),
    TEST_c_c (casinh, -0x1.0000000000000000000000000001p0L, 0x1p-113L, -8.813735870195430252326093249797924452120e-1L, 6.809190618832224124185481406981900518193e-35L),
    TEST_c_c (casinh, 0x1.0000000000000000000000000001p0L, -0x1p-113L, 8.813735870195430252326093249797924452120e-1L, -6.809190618832224124185481406981900518193e-35L),
    TEST_c_c (casinh, -0x1.0000000000000000000000000001p0L, -0x1p-113L, -8.813735870195430252326093249797924452120e-1L, -6.809190618832224124185481406981900518193e-35L),
    TEST_c_c (casinh, 0x1.fp-16385L, 0x1.0000000000000000000000000001p0L, 1.962615573354718824241727964954454332780e-17L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.fp-16385L, -0x1.0000000000000000000000000001p0L, 1.962615573354718824241727964954454332780e-17L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-16385L, 0x1.0000000000000000000000000001p0L, -1.962615573354718824241727964954454332780e-17L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0x1.fp-16385L, -0x1.0000000000000000000000000001p0L, -1.962615573354718824241727964954454332780e-17L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.0000000000000000000000000001p0L, 0x1.fp-16385L, 8.813735870195430252326093249797924452120e-1L, 5.757683115456107044131264955348448400014e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.0000000000000000000000000001p0L, 0x1.fp-16385L, -8.813735870195430252326093249797924452120e-1L, 5.757683115456107044131264955348448400014e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x1.0000000000000000000000000001p0L, -0x1.fp-16385L, 8.813735870195430252326093249797924452120e-1L, -5.757683115456107044131264955348448400014e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.0000000000000000000000000001p0L, -0x1.fp-16385L, -8.813735870195430252326093249797924452120e-1L, -5.757683115456107044131264955348448400014e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0.0L, 0x1.0000000000000000000000000001p0L, 1.962615573354718824241727964954454332780e-17L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0.0L, -0x1.0000000000000000000000000001p0L, 1.962615573354718824241727964954454332780e-17L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0.0L, 0x1.0000000000000000000000000001p0L, -1.962615573354718824241727964954454332780e-17L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, -0.0L, -0x1.0000000000000000000000000001p0L, -1.962615573354718824241727964954454332780e-17L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (casinh, 0x1.0000000000000000000000000001p0L, 0.0L, 8.813735870195430252326093249797924452120e-1L, 0.0L),
    TEST_c_c (casinh, -0x1.0000000000000000000000000001p0L, 0.0L, -8.813735870195430252326093249797924452120e-1L, 0.0L),
    TEST_c_c (casinh, 0x1.0000000000000000000000000001p0L, -0.0L, 8.813735870195430252326093249797924452120e-1L, -0.0L),
    TEST_c_c (casinh, -0x1.0000000000000000000000000001p0L, -0.0L, -8.813735870195430252326093249797924452120e-1L, -0.0L),
#endif

    TEST_c_c (casinh, 0x1p-23L, 0x0.ffffffp0L, 2.714321200917194650737217746780928423385e-4L, 1.570357140408705570520377172865353621698L),
    TEST_c_c (casinh, 0x1p-23L, -0x0.ffffffp0L, 2.714321200917194650737217746780928423385e-4L, -1.570357140408705570520377172865353621698L),
    TEST_c_c (casinh, -0x1p-23L, 0x0.ffffffp0L, -2.714321200917194650737217746780928423385e-4L, 1.570357140408705570520377172865353621698L),
    TEST_c_c (casinh, -0x1p-23L, -0x0.ffffffp0L, -2.714321200917194650737217746780928423385e-4L, -1.570357140408705570520377172865353621698L),
    TEST_c_c (casinh, 0x0.ffffffp0L, 0x1p-23L, 8.813735448726963984495965873956465777250e-1L, 8.429369953393598499000760938112968755257e-8L),
    TEST_c_c (casinh, -0x0.ffffffp0L, 0x1p-23L, -8.813735448726963984495965873956465777250e-1L, 8.429369953393598499000760938112968755257e-8L),
    TEST_c_c (casinh, 0x0.ffffffp0L, -0x1p-23L, 8.813735448726963984495965873956465777250e-1L, -8.429369953393598499000760938112968755257e-8L),
    TEST_c_c (casinh, -0x0.ffffffp0L, -0x1p-23L, -8.813735448726963984495965873956465777250e-1L, -8.429369953393598499000760938112968755257e-8L),
    TEST_c_c (casinh, 0x1.fp-129L, 0x0.ffffffp0L, 8.245504387859737323891843603996428534945e-36L, 1.570451059810180415643718442157112705601L),
    TEST_c_c (casinh, 0x1.fp-129L, -0x0.ffffffp0L, 8.245504387859737323891843603996428534945e-36L, -1.570451059810180415643718442157112705601L),
    TEST_c_c (casinh, -0x1.fp-129L, 0x0.ffffffp0L, -8.245504387859737323891843603996428534945e-36L, 1.570451059810180415643718442157112705601L),
    TEST_c_c (casinh, -0x1.fp-129L, -0x0.ffffffp0L, -8.245504387859737323891843603996428534945e-36L, -1.570451059810180415643718442157112705601L),
    TEST_c_c (casinh, 0x0.ffffffp0L, 0x1.fp-129L, 8.813735448726938863015878255140556727969e-1L, 2.013062624689288675911339929224634593425e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0x0.ffffffp0L, 0x1.fp-129L, -8.813735448726938863015878255140556727969e-1L, 2.013062624689288675911339929224634593425e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 0x0.ffffffp0L, -0x1.fp-129L, 8.813735448726938863015878255140556727969e-1L, -2.013062624689288675911339929224634593425e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0x0.ffffffp0L, -0x1.fp-129L, -8.813735448726938863015878255140556727969e-1L, -2.013062624689288675911339929224634593425e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 0.0L, 0x0.ffffffp0L, 0.0L, 1.570451059810180415643718442157112705601L),
    TEST_c_c (casinh, 0.0L, -0x0.ffffffp0L, 0.0L, -1.570451059810180415643718442157112705601L),
    TEST_c_c (casinh, -0.0L, 0x0.ffffffp0L, -0.0L, 1.570451059810180415643718442157112705601L),
    TEST_c_c (casinh, -0.0L, -0x0.ffffffp0L, -0.0L, -1.570451059810180415643718442157112705601L),
    TEST_c_c (casinh, 0x0.ffffffp0L, 0.0L, 8.813735448726938863015878255140556727969e-1L, 0.0L),
    TEST_c_c (casinh, -0x0.ffffffp0L, 0.0L, -8.813735448726938863015878255140556727969e-1L, 0.0L),
    TEST_c_c (casinh, 0x0.ffffffp0L, -0.0L, 8.813735448726938863015878255140556727969e-1L, -0.0L),
    TEST_c_c (casinh, -0x0.ffffffp0L, -0.0L, -8.813735448726938863015878255140556727969e-1L, -0.0L),
    TEST_c_c (casinh, 0x1p-23L, 0.5L, 1.376510308240943236356088341381173571841e-7L, 5.235987755982934033165770729745469098254e-1L),
    TEST_c_c (casinh, 0x1p-23L, -0.5L, 1.376510308240943236356088341381173571841e-7L, -5.235987755982934033165770729745469098254e-1L),
    TEST_c_c (casinh, -0x1p-23L, 0.5L, -1.376510308240943236356088341381173571841e-7L, 5.235987755982934033165770729745469098254e-1L),
    TEST_c_c (casinh, -0x1p-23L, -0.5L, -1.376510308240943236356088341381173571841e-7L, -5.235987755982934033165770729745469098254e-1L),
    TEST_c_c (casinh, 0.5L, 0x1p-23L, 4.812118250596059896127318386463676808798e-1L, 1.066240299940009780479664302917201696886e-7L),
    TEST_c_c (casinh, -0.5L, 0x1p-23L, -4.812118250596059896127318386463676808798e-1L, 1.066240299940009780479664302917201696886e-7L),
    TEST_c_c (casinh, 0.5L, -0x1p-23L, 4.812118250596059896127318386463676808798e-1L, -1.066240299940009780479664302917201696886e-7L),
    TEST_c_c (casinh, -0.5L, -0x1p-23L, -4.812118250596059896127318386463676808798e-1L, -1.066240299940009780479664302917201696886e-7L),
    TEST_c_c (casinh, 0x1.fp-129L, 0.5L, 3.287317402534702257036015056278368274737e-39L, 5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 0x1.fp-129L, -0.5L, 3.287317402534702257036015056278368274737e-39L, -5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0x1.fp-129L, 0.5L, -3.287317402534702257036015056278368274737e-39L, 5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0x1.fp-129L, -0.5L, -3.287317402534702257036015056278368274737e-39L, -5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 0.5L, 0x1.fp-129L, 4.812118250596034474977589134243684231352e-1L, 2.546345110742945032959687790021055102355e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0.5L, 0x1.fp-129L, -4.812118250596034474977589134243684231352e-1L, 2.546345110742945032959687790021055102355e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 0.5L, -0x1.fp-129L, 4.812118250596034474977589134243684231352e-1L, -2.546345110742945032959687790021055102355e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0.5L, -0x1.fp-129L, -4.812118250596034474977589134243684231352e-1L, -2.546345110742945032959687790021055102355e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 0x1p-23L, 0x1p-23L, 1.192092895507818146886315028596704749235e-7L, 1.192092895507806853113684971258850504935e-7L),
    TEST_c_c (casinh, 0x1p-23L, -0x1p-23L, 1.192092895507818146886315028596704749235e-7L, -1.192092895507806853113684971258850504935e-7L),
    TEST_c_c (casinh, -0x1p-23L, 0x1p-23L, -1.192092895507818146886315028596704749235e-7L, 1.192092895507806853113684971258850504935e-7L),
    TEST_c_c (casinh, -0x1p-23L, -0x1p-23L, -1.192092895507818146886315028596704749235e-7L, -1.192092895507806853113684971258850504935e-7L),
    TEST_c_c (casinh, 0x1.fp-129L, 0x1p-23L, 2.846900380897747786805634596726756660388e-39L, 1.192092895507815323443157514352519154304e-7L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 0x1.fp-129L, -0x1p-23L, 2.846900380897747786805634596726756660388e-39L, -1.192092895507815323443157514352519154304e-7L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0x1.fp-129L, 0x1p-23L, -2.846900380897747786805634596726756660388e-39L, 1.192092895507815323443157514352519154304e-7L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0x1.fp-129L, -0x1p-23L, -2.846900380897747786805634596726756660388e-39L, -1.192092895507815323443157514352519154304e-7L, UNDERFLOW_EXCEPTION_FLOAT),
    /* Bug 15319: underflow exception may be missing.  */
    TEST_c_c (casinh, 0x1p-23L, 0x1.fp-129L, 1.192092895507809676556842485683592032154e-7L, 2.846900380897707329917933005874691056564e-39L, UNDERFLOW_EXCEPTION_OK_FLOAT),
    TEST_c_c (casinh, -0x1p-23L, 0x1.fp-129L, -1.192092895507809676556842485683592032154e-7L, 2.846900380897707329917933005874691056564e-39L, UNDERFLOW_EXCEPTION_OK_FLOAT),
    TEST_c_c (casinh, 0x1p-23L, -0x1.fp-129L, 1.192092895507809676556842485683592032154e-7L, -2.846900380897707329917933005874691056564e-39L, UNDERFLOW_EXCEPTION_OK_FLOAT),
    TEST_c_c (casinh, -0x1p-23L, -0x1.fp-129L, -1.192092895507809676556842485683592032154e-7L, -2.846900380897707329917933005874691056564e-39L, UNDERFLOW_EXCEPTION_OK_FLOAT),
    TEST_c_c (casinh, 0.0L, 0x1p-23L, 0.0L, 1.192092895507815323443157514352519154304e-7L),
    TEST_c_c (casinh, 0.0L, -0x1p-23L, 0.0L, -1.192092895507815323443157514352519154304e-7L),
    TEST_c_c (casinh, -0.0L, 0x1p-23L, -0.0L, 1.192092895507815323443157514352519154304e-7L),
    TEST_c_c (casinh, -0.0L, -0x1p-23L, -0.0L, -1.192092895507815323443157514352519154304e-7L),
    TEST_c_c (casinh, 0x1p-23L, 0.0L, 1.192092895507809676556842485683592032154e-7L, 0.0L),
    TEST_c_c (casinh, -0x1p-23L, 0.0L, -1.192092895507809676556842485683592032154e-7L, 0.0L),
    TEST_c_c (casinh, 0x1p-23L, -0.0L, 1.192092895507809676556842485683592032154e-7L, -0.0L),
    TEST_c_c (casinh, -0x1p-23L, -0.0L, -1.192092895507809676556842485683592032154e-7L, -0.0L),
    TEST_c_c (casinh, 0x1.fp-129L, 0x1.fp-129L, 2.846900380897727558361783801085126250967e-39L, 2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, 0x1.fp-129L, -0x1.fp-129L, 2.846900380897727558361783801085126250967e-39L, -2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0x1.fp-129L, 0x1.fp-129L, -2.846900380897727558361783801085126250967e-39L, 2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (casinh, -0x1.fp-129L, -0x1.fp-129L, -2.846900380897727558361783801085126250967e-39L, -2.846900380897727558361783801085126250967e-39L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (casinh, 0x1p-52L, 0x0.fffffffffffff8p0L, 1.171456840272878582596796205397918831268e-8L, 1.570796307840326780080580569360500172043L),
    TEST_c_c (casinh, 0x1p-52L, -0x0.fffffffffffff8p0L, 1.171456840272878582596796205397918831268e-8L, -1.570796307840326780080580569360500172043L),
    TEST_c_c (casinh, -0x1p-52L, 0x0.fffffffffffff8p0L, -1.171456840272878582596796205397918831268e-8L, 1.570796307840326780080580569360500172043L),
    TEST_c_c (casinh, -0x1p-52L, -0x0.fffffffffffff8p0L, -1.171456840272878582596796205397918831268e-8L, -1.570796307840326780080580569360500172043L),
    TEST_c_c (casinh, 0x0.fffffffffffff8p0L, 0x1p-52L, 8.813735870195429467279863907910458761820e-1L, 1.570092458683775146551022293016031474627e-16L),
    TEST_c_c (casinh, -0x0.fffffffffffff8p0L, 0x1p-52L, -8.813735870195429467279863907910458761820e-1L, 1.570092458683775146551022293016031474627e-16L),
    TEST_c_c (casinh, 0x0.fffffffffffff8p0L, -0x1p-52L, 8.813735870195429467279863907910458761820e-1L, -1.570092458683775146551022293016031474627e-16L),
    TEST_c_c (casinh, -0x0.fffffffffffff8p0L, -0x1p-52L, -8.813735870195429467279863907910458761820e-1L, -1.570092458683775146551022293016031474627e-16L),
    TEST_c_c (casinh, 0x1.fp-1025L, 0x0.fffffffffffff8p0L, 3.616396521699973256461764099945789620604e-301L, 1.570796311893735425383665303776316016594L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, 0x1.fp-1025L, -0x0.fffffffffffff8p0L, 3.616396521699973256461764099945789620604e-301L, -1.570796311893735425383665303776316016594L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, -0x1.fp-1025L, 0x0.fffffffffffff8p0L, -3.616396521699973256461764099945789620604e-301L, 1.570796311893735425383665303776316016594L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, -0x1.fp-1025L, -0x0.fffffffffffff8p0L, -3.616396521699973256461764099945789620604e-301L, -1.570796311893735425383665303776316016594L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, 0x0.fffffffffffff8p0L, 0x1.fp-1025L, 8.813735870195429467279863907910371604180e-1L, 3.810492908885321954658152505985270452320e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x0.fffffffffffff8p0L, 0x1.fp-1025L, -8.813735870195429467279863907910371604180e-1L, 3.810492908885321954658152505985270452320e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0x0.fffffffffffff8p0L, -0x1.fp-1025L, 8.813735870195429467279863907910371604180e-1L, -3.810492908885321954658152505985270452320e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x0.fffffffffffff8p0L, -0x1.fp-1025L, -8.813735870195429467279863907910371604180e-1L, -3.810492908885321954658152505985270452320e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0.0L, 0x0.fffffffffffff8p0L, 0.0L, 1.570796311893735425383665303776316016594L),
    TEST_c_c (casinh, 0.0L, -0x0.fffffffffffff8p0L, 0.0L, -1.570796311893735425383665303776316016594L),
    TEST_c_c (casinh, -0.0L, 0x0.fffffffffffff8p0L, -0.0L, 1.570796311893735425383665303776316016594L),
    TEST_c_c (casinh, -0.0L, -0x0.fffffffffffff8p0L, -0.0L, -1.570796311893735425383665303776316016594L),
    TEST_c_c (casinh, 0x0.fffffffffffff8p0L, 0.0L, 8.813735870195429467279863907910371604180e-1L, 0.0L),
    TEST_c_c (casinh, -0x0.fffffffffffff8p0L, 0.0L, -8.813735870195429467279863907910371604180e-1L, 0.0L),
    TEST_c_c (casinh, 0x0.fffffffffffff8p0L, -0.0L, 8.813735870195429467279863907910371604180e-1L, -0.0L),
    TEST_c_c (casinh, -0x0.fffffffffffff8p0L, -0.0L, -8.813735870195429467279863907910371604180e-1L, -0.0L),
#endif
    TEST_c_c (casinh, 0x1p-52L, 0.5L, 2.563950248511418570403591756798643804971e-16L, 5.235987755982988730771072305465648369889e-1L),
    TEST_c_c (casinh, 0x1p-52L, -0.5L, 2.563950248511418570403591756798643804971e-16L, -5.235987755982988730771072305465648369889e-1L),
    TEST_c_c (casinh, -0x1p-52L, 0.5L, -2.563950248511418570403591756798643804971e-16L, 5.235987755982988730771072305465648369889e-1L),
    TEST_c_c (casinh, -0x1p-52L, -0.5L, -2.563950248511418570403591756798643804971e-16L, -5.235987755982988730771072305465648369889e-1L),
    TEST_c_c (casinh, 0.5L, 0x1p-52L, 4.812118250596034474977589134243772428682e-1L, 1.986027322597818397661816253158024383707e-16L),
    TEST_c_c (casinh, -0.5L, 0x1p-52L, -4.812118250596034474977589134243772428682e-1L, 1.986027322597818397661816253158024383707e-16L),
    TEST_c_c (casinh, 0.5L, -0x1p-52L, 4.812118250596034474977589134243772428682e-1L, -1.986027322597818397661816253158024383707e-16L),
    TEST_c_c (casinh, -0.5L, -0x1p-52L, -4.812118250596034474977589134243772428682e-1L, -1.986027322597818397661816253158024383707e-16L),
#ifndef TEST_FLOAT
    TEST_c_c (casinh, 0x1.fp-1025L, 0.5L, 6.222508863508420569166420770843207333493e-309L, 5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0x1.fp-1025L, -0.5L, 6.222508863508420569166420770843207333493e-309L, -5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.fp-1025L, 0.5L, -6.222508863508420569166420770843207333493e-309L, 5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.fp-1025L, -0.5L, -6.222508863508420569166420770843207333493e-309L, -5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0.5L, 0x1.fp-1025L, 4.812118250596034474977589134243684231352e-1L, 4.819934639999230680322935210539402497827e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0.5L, 0x1.fp-1025L, -4.812118250596034474977589134243684231352e-1L, 4.819934639999230680322935210539402497827e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0.5L, -0x1.fp-1025L, 4.812118250596034474977589134243684231352e-1L, -4.819934639999230680322935210539402497827e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0.5L, -0x1.fp-1025L, -4.812118250596034474977589134243684231352e-1L, -4.819934639999230680322935210539402497827e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
    TEST_c_c (casinh, 0x1p-52L, 0x1p-52L, 2.220446049250313080847263336181677117148e-16L, 2.220446049250313080847263336181604132852e-16L),
    TEST_c_c (casinh, 0x1p-52L, -0x1p-52L, 2.220446049250313080847263336181677117148e-16L, -2.220446049250313080847263336181604132852e-16L),
    TEST_c_c (casinh, -0x1p-52L, 0x1p-52L, -2.220446049250313080847263336181677117148e-16L, 2.220446049250313080847263336181604132852e-16L),
    TEST_c_c (casinh, -0x1p-52L, -0x1p-52L, -2.220446049250313080847263336181677117148e-16L, -2.220446049250313080847263336181604132852e-16L),
#ifndef TEST_FLOAT
    TEST_c_c (casinh, 0x1.fp-1025L, 0x1p-52L, 5.388850751072128349671657362289548938458e-309L, 2.220446049250313080847263336181658871074e-16L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0x1.fp-1025L, -0x1p-52L, 5.388850751072128349671657362289548938458e-309L, -2.220446049250313080847263336181658871074e-16L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.fp-1025L, 0x1p-52L, -5.388850751072128349671657362289548938458e-309L, 2.220446049250313080847263336181658871074e-16L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.fp-1025L, -0x1p-52L, -5.388850751072128349671657362289548938458e-309L, -2.220446049250313080847263336181658871074e-16L, UNDERFLOW_EXCEPTION_DOUBLE),
    /* Bug 15319: underflow exception may be missing.  */
    TEST_c_c (casinh, 0x1p-52L, 0x1.fp-1025L, 2.220446049250313080847263336181622378926e-16L, 5.388850751072128349671657362289283247603e-309L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casinh, -0x1p-52L, 0x1.fp-1025L, -2.220446049250313080847263336181622378926e-16L, 5.388850751072128349671657362289283247603e-309L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casinh, 0x1p-52L, -0x1.fp-1025L, 2.220446049250313080847263336181622378926e-16L, -5.388850751072128349671657362289283247603e-309L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casinh, -0x1p-52L, -0x1.fp-1025L, -2.220446049250313080847263336181622378926e-16L, -5.388850751072128349671657362289283247603e-309L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
#endif
    TEST_c_c (casinh, 0.0L, 0x1p-52L, 0.0L, 2.220446049250313080847263336181658871074e-16L),
    TEST_c_c (casinh, 0.0L, -0x1p-52L, 0.0L, -2.220446049250313080847263336181658871074e-16L),
    TEST_c_c (casinh, -0.0L, 0x1p-52L, -0.0L, 2.220446049250313080847263336181658871074e-16L),
    TEST_c_c (casinh, -0.0L, -0x1p-52L, -0.0L, -2.220446049250313080847263336181658871074e-16L),
    TEST_c_c (casinh, 0x1p-52L, 0.0L, 2.220446049250313080847263336181622378926e-16L, 0.0L),
    TEST_c_c (casinh, -0x1p-52L, 0.0L, -2.220446049250313080847263336181622378926e-16L, 0.0L),
    TEST_c_c (casinh, 0x1p-52L, -0.0L, 2.220446049250313080847263336181622378926e-16L, -0.0L),
    TEST_c_c (casinh, -0x1p-52L, -0.0L, -2.220446049250313080847263336181622378926e-16L, -0.0L),
#ifndef TEST_FLOAT
    TEST_c_c (casinh, 0x1.fp-1025L, 0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0x1.fp-1025L, -0x1.fp-1025L, 5.388850751072128349671657362289416093031e-309L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.fp-1025L, 0x1.fp-1025L, -5.388850751072128349671657362289416093031e-309L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.fp-1025L, -0x1.fp-1025L, -5.388850751072128349671657362289416093031e-309L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (casinh, 0x1p-63L, 0x0.ffffffffffffffffp0L, 2.588578361325995866221775673638805081337e-10L, 1.570796326376055842114524927965556247908L),
    TEST_c_c (casinh, 0x1p-63L, -0x0.ffffffffffffffffp0L, 2.588578361325995866221775673638805081337e-10L, -1.570796326376055842114524927965556247908L),
    TEST_c_c (casinh, -0x1p-63L, 0x0.ffffffffffffffffp0L, -2.588578361325995866221775673638805081337e-10L, 1.570796326376055842114524927965556247908L),
    TEST_c_c (casinh, -0x1p-63L, -0x0.ffffffffffffffffp0L, -2.588578361325995866221775673638805081337e-10L, -1.570796326376055842114524927965556247908L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffp0L, 0x1p-63L, 8.813735870195430251942769895627079569937e-1L, 7.666467083416870407402049869781566856442e-20L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffp0L, 0x1p-63L, -8.813735870195430251942769895627079569937e-1L, 7.666467083416870407402049869781566856442e-20L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffp0L, -0x1p-63L, 8.813735870195430251942769895627079569937e-1L, -7.666467083416870407402049869781566856442e-20L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffp0L, -0x1p-63L, -8.813735870195430251942769895627079569937e-1L, -7.666467083416870407402049869781566856442e-20L),
# if LDBL_MIN_EXP <= -16381
    TEST_c_c (casinh, 0x1.fp-16385L, 0x0.ffffffffffffffffp0L, 2.472906068161537187835415298076415423459e-4923L, 1.570796326465624365239962068302568888898L),
    TEST_c_c (casinh, 0x1.fp-16385L, -0x0.ffffffffffffffffp0L, 2.472906068161537187835415298076415423459e-4923L, -1.570796326465624365239962068302568888898L),
    TEST_c_c (casinh, -0x1.fp-16385L, 0x0.ffffffffffffffffp0L, -2.472906068161537187835415298076415423459e-4923L, 1.570796326465624365239962068302568888898L),
    TEST_c_c (casinh, -0x1.fp-16385L, -0x0.ffffffffffffffffp0L, -2.472906068161537187835415298076415423459e-4923L, -1.570796326465624365239962068302568888898L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffp0L, 0x1.fp-16385L, 8.813735870195430251942769895627079569917e-1L, 5.757683115456107044287327268904964422212e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x0.ffffffffffffffffp0L, 0x1.fp-16385L, -8.813735870195430251942769895627079569917e-1L, 5.757683115456107044287327268904964422212e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x0.ffffffffffffffffp0L, -0x1.fp-16385L, 8.813735870195430251942769895627079569917e-1L, -5.757683115456107044287327268904964422212e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x0.ffffffffffffffffp0L, -0x1.fp-16385L, -8.813735870195430251942769895627079569917e-1L, -5.757683115456107044287327268904964422212e-4933L, UNDERFLOW_EXCEPTION),
# endif
    TEST_c_c (casinh, 0.0L, 0x0.ffffffffffffffffp0L, 0.0L, 1.570796326465624365239962068302568888898L),
    TEST_c_c (casinh, 0.0L, -0x0.ffffffffffffffffp0L, 0.0L, -1.570796326465624365239962068302568888898L),
    TEST_c_c (casinh, -0.0L, 0x0.ffffffffffffffffp0L, -0.0L, 1.570796326465624365239962068302568888898L),
    TEST_c_c (casinh, -0.0L, -0x0.ffffffffffffffffp0L, -0.0L, -1.570796326465624365239962068302568888898L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffp0L, 0.0L, 8.813735870195430251942769895627079569917e-1L, 0.0L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffp0L, 0.0L, -8.813735870195430251942769895627079569917e-1L, 0.0L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffp0L, -0.0L, 8.813735870195430251942769895627079569917e-1L, -0.0L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffp0L, -0.0L, -8.813735870195430251942769895627079569917e-1L, -0.0L),
#endif
    TEST_c_c (casinh, 0x1p-63L, 0.5L, 1.251928832280966098829878787499365228659e-19L, 5.235987755982988730771072305465838140283e-1L),
    TEST_c_c (casinh, 0x1p-63L, -0.5L, 1.251928832280966098829878787499365228659e-19L, -5.235987755982988730771072305465838140283e-1L),
    TEST_c_c (casinh, -0x1p-63L, 0.5L, -1.251928832280966098829878787499365228659e-19L, 5.235987755982988730771072305465838140283e-1L),
    TEST_c_c (casinh, -0x1p-63L, -0.5L, -1.251928832280966098829878787499365228659e-19L, -5.235987755982988730771072305465838140283e-1L),
    TEST_c_c (casinh, 0.5L, 0x1p-63L, 4.812118250596034474977589134243684231373e-1L, 9.697399036122160144833087173623140436413e-20L),
    TEST_c_c (casinh, -0.5L, 0x1p-63L, -4.812118250596034474977589134243684231373e-1L, 9.697399036122160144833087173623140436413e-20L),
    TEST_c_c (casinh, 0.5L, -0x1p-63L, 4.812118250596034474977589134243684231373e-1L, -9.697399036122160144833087173623140436413e-20L),
    TEST_c_c (casinh, -0.5L, -0x1p-63L, -4.812118250596034474977589134243684231373e-1L, -9.697399036122160144833087173623140436413e-20L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casinh, 0x1.fp-16385L, 0.5L, 9.402257155670418112159334875199821342890e-4933L, 5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x1.fp-16385L, -0.5L, 9.402257155670418112159334875199821342890e-4933L, -5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.fp-16385L, 0.5L, -9.402257155670418112159334875199821342890e-4933L, 5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.fp-16385L, -0.5L, -9.402257155670418112159334875199821342890e-4933L, -5.235987755982988730771072305465838140329e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0.5L, 0x1.fp-16385L, 4.812118250596034474977589134243684231352e-1L, 7.282957076134209141226696333885150260319e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0.5L, 0x1.fp-16385L, -4.812118250596034474977589134243684231352e-1L, 7.282957076134209141226696333885150260319e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0.5L, -0x1.fp-16385L, 4.812118250596034474977589134243684231352e-1L, -7.282957076134209141226696333885150260319e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0.5L, -0x1.fp-16385L, -4.812118250596034474977589134243684231352e-1L, -7.282957076134209141226696333885150260319e-4933L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (casinh, 0x1p-63L, 0x1p-63L, 1.084202172485504434007452800869941711430e-19L, 1.084202172485504434007452800869941711422e-19L),
    TEST_c_c (casinh, 0x1p-63L, -0x1p-63L, 1.084202172485504434007452800869941711430e-19L, -1.084202172485504434007452800869941711422e-19L),
    TEST_c_c (casinh, -0x1p-63L, 0x1p-63L, -1.084202172485504434007452800869941711430e-19L, 1.084202172485504434007452800869941711422e-19L),
    TEST_c_c (casinh, -0x1p-63L, -0x1p-63L, -1.084202172485504434007452800869941711430e-19L, -1.084202172485504434007452800869941711422e-19L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casinh, 0x1.fp-16385L, 0x1p-63L, 8.142593549724601460479922838826119584465e-4933L, 1.084202172485504434007452800869941711428e-19L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x1.fp-16385L, -0x1p-63L, 8.142593549724601460479922838826119584465e-4933L, -1.084202172485504434007452800869941711428e-19L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.fp-16385L, 0x1p-63L, -8.142593549724601460479922838826119584465e-4933L, 1.084202172485504434007452800869941711428e-19L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.fp-16385L, -0x1p-63L, -8.142593549724601460479922838826119584465e-4933L, -1.084202172485504434007452800869941711428e-19L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x1p-63L, 0x1.fp-16385L, 1.084202172485504434007452800869941711424e-19L, 8.142593549724601460479922838826119584369e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1p-63L, 0x1.fp-16385L, -1.084202172485504434007452800869941711424e-19L, 8.142593549724601460479922838826119584369e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x1p-63L, -0x1.fp-16385L, 1.084202172485504434007452800869941711424e-19L, -8.142593549724601460479922838826119584369e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1p-63L, -0x1.fp-16385L, -1.084202172485504434007452800869941711424e-19L, -8.142593549724601460479922838826119584369e-4933L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (casinh, 0.0L, 0x1p-63L, 0.0L, 1.084202172485504434007452800869941711428e-19L),
    TEST_c_c (casinh, 0.0L, -0x1p-63L, 0.0L, -1.084202172485504434007452800869941711428e-19L),
    TEST_c_c (casinh, -0.0L, 0x1p-63L, -0.0L, 1.084202172485504434007452800869941711428e-19L),
    TEST_c_c (casinh, -0.0L, -0x1p-63L, -0.0L, -1.084202172485504434007452800869941711428e-19L),
    TEST_c_c (casinh, 0x1p-63L, 0.0L, 1.084202172485504434007452800869941711424e-19L, 0.0L),
    TEST_c_c (casinh, -0x1p-63L, 0.0L, -1.084202172485504434007452800869941711424e-19L, 0.0L),
    TEST_c_c (casinh, 0x1p-63L, -0.0L, 1.084202172485504434007452800869941711424e-19L, -0.0L),
    TEST_c_c (casinh, -0x1p-63L, -0.0L, -1.084202172485504434007452800869941711424e-19L, -0.0L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casinh, 0x1.fp-16385L, 0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x1.fp-16385L, -0x1.fp-16385L, 8.142593549724601460479922838826119584417e-4933L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.fp-16385L, 0x1.fp-16385L, -8.142593549724601460479922838826119584417e-4933L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.fp-16385L, -0x1.fp-16385L, -8.142593549724601460479922838826119584417e-4933L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (casinh, 0x1p-105L, 0x0.ffffffffffffffffffffffffffcp0L, 1.234330349600789959989661887846005659983e-16L, 1.570796326794896419512475791677915736035L),
    TEST_c_c (casinh, 0x1p-105L, -0x0.ffffffffffffffffffffffffffcp0L, 1.234330349600789959989661887846005659983e-16L, -1.570796326794896419512475791677915736035L),
    TEST_c_c (casinh, -0x1p-105L, 0x0.ffffffffffffffffffffffffffcp0L, -1.234330349600789959989661887846005659983e-16L, 1.570796326794896419512475791677915736035L),
    TEST_c_c (casinh, -0x1p-105L, -0x0.ffffffffffffffffffffffffffcp0L, -1.234330349600789959989661887846005659983e-16L, -1.570796326794896419512475791677915736035L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffcp0L, 0x1p-105L, 8.813735870195430252326093249797835932642e-1L, 1.743152798421049375791483240187377443526e-32L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffcp0L, 0x1p-105L, -8.813735870195430252326093249797835932642e-1L, 1.743152798421049375791483240187377443526e-32L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffcp0L, -0x1p-105L, 8.813735870195430252326093249797835932642e-1L, -1.743152798421049375791483240187377443526e-32L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffcp0L, -0x1p-105L, -8.813735870195430252326093249797835932642e-1L, -1.743152798421049375791483240187377443526e-32L),
    TEST_c_c (casinh, 0x1.fp-1025L, 0x0.ffffffffffffffffffffffffffcp0L, 3.432186888910770473784103112009399360940e-293L, 1.570796326794896462222075823262245502760L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, 0x1.fp-1025L, -0x0.ffffffffffffffffffffffffffcp0L, 3.432186888910770473784103112009399360940e-293L, -1.570796326794896462222075823262245502760L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, -0x1.fp-1025L, 0x0.ffffffffffffffffffffffffffcp0L, -3.432186888910770473784103112009399360940e-293L, 1.570796326794896462222075823262245502760L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, -0x1.fp-1025L, -0x0.ffffffffffffffffffffffffffcp0L, -3.432186888910770473784103112009399360940e-293L, -1.570796326794896462222075823262245502760L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffcp0L, 0x1.fp-1025L, 8.813735870195430252326093249797835932642e-1L, 3.810492908885321743133304375216641110205e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffcp0L, 0x1.fp-1025L, -8.813735870195430252326093249797835932642e-1L, 3.810492908885321743133304375216641110205e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffcp0L, -0x1.fp-1025L, 8.813735870195430252326093249797835932642e-1L, -3.810492908885321743133304375216641110205e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffcp0L, -0x1.fp-1025L, -8.813735870195430252326093249797835932642e-1L, -3.810492908885321743133304375216641110205e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0.0L, 0x0.ffffffffffffffffffffffffffcp0L, 0.0L, 1.570796326794896462222075823262245502760L),
    TEST_c_c (casinh, 0.0L, -0x0.ffffffffffffffffffffffffffcp0L, 0.0L, -1.570796326794896462222075823262245502760L),
    TEST_c_c (casinh, -0.0L, 0x0.ffffffffffffffffffffffffffcp0L, -0.0L, 1.570796326794896462222075823262245502760L),
    TEST_c_c (casinh, -0.0L, -0x0.ffffffffffffffffffffffffffcp0L, -0.0L, -1.570796326794896462222075823262245502760L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffcp0L, 0.0L, 8.813735870195430252326093249797835932642e-1L, 0.0L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffcp0L, 0.0L, -8.813735870195430252326093249797835932642e-1L, 0.0L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffcp0L, -0.0L, 8.813735870195430252326093249797835932642e-1L, -0.0L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffcp0L, -0.0L, -8.813735870195430252326093249797835932642e-1L, -0.0L),
#endif
    TEST_c_c (casinh, 0x1p-105L, 0.5L, 2.846556599890768890940941645338546615370e-32L, 5.235987755982988730771072305465838140329e-1L),
    TEST_c_c (casinh, 0x1p-105L, -0.5L, 2.846556599890768890940941645338546615370e-32L, -5.235987755982988730771072305465838140329e-1L),
    TEST_c_c (casinh, -0x1p-105L, 0.5L, -2.846556599890768890940941645338546615370e-32L, 5.235987755982988730771072305465838140329e-1L),
    TEST_c_c (casinh, -0x1p-105L, -0.5L, -2.846556599890768890940941645338546615370e-32L, -5.235987755982988730771072305465838140329e-1L),
    TEST_c_c (casinh, 0.5L, 0x1p-105L, 4.812118250596034474977589134243684231352e-1L, 2.204933261082751447436996198490110245167e-32L),
    TEST_c_c (casinh, -0.5L, 0x1p-105L, -4.812118250596034474977589134243684231352e-1L, 2.204933261082751447436996198490110245167e-32L),
    TEST_c_c (casinh, 0.5L, -0x1p-105L, 4.812118250596034474977589134243684231352e-1L, -2.204933261082751447436996198490110245167e-32L),
    TEST_c_c (casinh, -0.5L, -0x1p-105L, -4.812118250596034474977589134243684231352e-1L, -2.204933261082751447436996198490110245167e-32L),
    TEST_c_c (casinh, 0x1p-105L, 0x1p-105L, 2.465190328815661891911651766508706967729e-32L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casinh, 0x1p-105L, -0x1p-105L, 2.465190328815661891911651766508706967729e-32L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casinh, -0x1p-105L, 0x1p-105L, -2.465190328815661891911651766508706967729e-32L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casinh, -0x1p-105L, -0x1p-105L, -2.465190328815661891911651766508706967729e-32L, -2.465190328815661891911651766508706967729e-32L),
#ifndef TEST_FLOAT
    TEST_c_c (casinh, 0x1.fp-1025L, 0x1p-105L, 5.388850751072128349671657362289416093031e-309L, 2.465190328815661891911651766508706967729e-32L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, 0x1.fp-1025L, -0x1p-105L, 5.388850751072128349671657362289416093031e-309L, -2.465190328815661891911651766508706967729e-32L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.fp-1025L, 0x1p-105L, -5.388850751072128349671657362289416093031e-309L, 2.465190328815661891911651766508706967729e-32L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (casinh, -0x1.fp-1025L, -0x1p-105L, -5.388850751072128349671657362289416093031e-309L, -2.465190328815661891911651766508706967729e-32L, UNDERFLOW_EXCEPTION_DOUBLE),
    /* Bug 15319: underflow exception may be missing.  */
    TEST_c_c (casinh, 0x1p-105L, 0x1.fp-1025L, 2.465190328815661891911651766508706967729e-32L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casinh, -0x1p-105L, 0x1.fp-1025L, -2.465190328815661891911651766508706967729e-32L, 5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casinh, 0x1p-105L, -0x1.fp-1025L, 2.465190328815661891911651766508706967729e-32L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
    TEST_c_c (casinh, -0x1p-105L, -0x1.fp-1025L, -2.465190328815661891911651766508706967729e-32L, -5.388850751072128349671657362289416093031e-309L, UNDERFLOW_EXCEPTION_OK_DOUBLE),
#endif
    TEST_c_c (casinh, 0.0L, 0x1p-105L, 0.0L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casinh, 0.0L, -0x1p-105L, 0.0L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casinh, -0.0L, 0x1p-105L, -0.0L, 2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casinh, -0.0L, -0x1p-105L, -0.0L, -2.465190328815661891911651766508706967729e-32L),
    TEST_c_c (casinh, 0x1p-105L, 0.0L, 2.465190328815661891911651766508706967729e-32L, 0.0L),
    TEST_c_c (casinh, -0x1p-105L, 0.0L, -2.465190328815661891911651766508706967729e-32L, 0.0L),
    TEST_c_c (casinh, 0x1p-105L, -0.0L, 2.465190328815661891911651766508706967729e-32L, -0.0L),
    TEST_c_c (casinh, -0x1p-105L, -0.0L, -2.465190328815661891911651766508706967729e-32L, -0.0L),
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (casinh, 0x1p-112L, 0x0.ffffffffffffffffffffffffffff8p0L, 1.091004200533850618345330909857103457781e-17L, 1.570796326794896601578502908312986698624L),
    TEST_c_c (casinh, 0x1p-112L, -0x0.ffffffffffffffffffffffffffff8p0L, 1.091004200533850618345330909857103457781e-17L, -1.570796326794896601578502908312986698624L),
    TEST_c_c (casinh, -0x1p-112L, 0x0.ffffffffffffffffffffffffffff8p0L, -1.091004200533850618345330909857103457781e-17L, 1.570796326794896601578502908312986698624L),
    TEST_c_c (casinh, -0x1p-112L, -0x0.ffffffffffffffffffffffffffff8p0L, -1.091004200533850618345330909857103457781e-17L, -1.570796326794896601578502908312986698624L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-112L, 8.813735870195430252326093249797922409363e-1L, 1.361838123766444824837096281396380300349e-34L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-112L, -8.813735870195430252326093249797922409363e-1L, 1.361838123766444824837096281396380300349e-34L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-112L, 8.813735870195430252326093249797922409363e-1L, -1.361838123766444824837096281396380300349e-34L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-112L, -8.813735870195430252326093249797922409363e-1L, -1.361838123766444824837096281396380300349e-34L),
    TEST_c_c (casinh, 0x1.fp-16385L, 0x0.ffffffffffffffffffffffffffff8p0L, 5.867357004219059108790368268900490184505e-4916L, 1.570796326794896605353533883825294686803L),
    TEST_c_c (casinh, 0x1.fp-16385L, -0x0.ffffffffffffffffffffffffffff8p0L, 5.867357004219059108790368268900490184505e-4916L, -1.570796326794896605353533883825294686803L),
    TEST_c_c (casinh, -0x1.fp-16385L, 0x0.ffffffffffffffffffffffffffff8p0L, -5.867357004219059108790368268900490184505e-4916L, 1.570796326794896605353533883825294686803L),
    TEST_c_c (casinh, -0x1.fp-16385L, -0x0.ffffffffffffffffffffffffffff8p0L, -5.867357004219059108790368268900490184505e-4916L, -1.570796326794896605353533883825294686803L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1.fp-16385L, 8.813735870195430252326093249797922409363e-1L, 5.757683115456107044131264955348449231681e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1.fp-16385L, -8.813735870195430252326093249797922409363e-1L, 5.757683115456107044131264955348449231681e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1.fp-16385L, 8.813735870195430252326093249797922409363e-1L, -5.757683115456107044131264955348449231681e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1.fp-16385L, -8.813735870195430252326093249797922409363e-1L, -5.757683115456107044131264955348449231681e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0.0L, 0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, 1.570796326794896605353533883825294686803L),
    TEST_c_c (casinh, 0.0L, -0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, -1.570796326794896605353533883825294686803L),
    TEST_c_c (casinh, -0.0L, 0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, 1.570796326794896605353533883825294686803L),
    TEST_c_c (casinh, -0.0L, -0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, -1.570796326794896605353533883825294686803L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, 8.813735870195430252326093249797922409363e-1L, 0.0L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffff8p0L, 0.0L, -8.813735870195430252326093249797922409363e-1L, 0.0L),
    TEST_c_c (casinh, 0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, 8.813735870195430252326093249797922409363e-1L, -0.0L),
    TEST_c_c (casinh, -0x0.ffffffffffffffffffffffffffff8p0L, -0.0L, -8.813735870195430252326093249797922409363e-1L, -0.0L),
#endif
    TEST_c_c (casinh, 0x1p-112L, 0.5L, 2.223872343664663196047610660420739543258e-34L, 5.235987755982988730771072305465838140329e-1L),
    TEST_c_c (casinh, 0x1p-112L, -0.5L, 2.223872343664663196047610660420739543258e-34L, -5.235987755982988730771072305465838140329e-1L),
    TEST_c_c (casinh, -0x1p-112L, 0.5L, -2.223872343664663196047610660420739543258e-34L, 5.235987755982988730771072305465838140329e-1L),
    TEST_c_c (casinh, -0x1p-112L, -0.5L, -2.223872343664663196047610660420739543258e-34L, -5.235987755982988730771072305465838140329e-1L),
    TEST_c_c (casinh, 0.5L, 0x1p-112L, 4.812118250596034474977589134243684231352e-1L, 1.722604110220899568310153280070398629037e-34L),
    TEST_c_c (casinh, -0.5L, 0x1p-112L, -4.812118250596034474977589134243684231352e-1L, 1.722604110220899568310153280070398629037e-34L),
    TEST_c_c (casinh, 0.5L, -0x1p-112L, 4.812118250596034474977589134243684231352e-1L, -1.722604110220899568310153280070398629037e-34L),
    TEST_c_c (casinh, -0.5L, -0x1p-112L, -4.812118250596034474977589134243684231352e-1L, -1.722604110220899568310153280070398629037e-34L),
    TEST_c_c (casinh, 0x1p-112L, 0x1p-112L, 1.925929944387235853055977942584927318538e-34L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casinh, 0x1p-112L, -0x1p-112L, 1.925929944387235853055977942584927318538e-34L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casinh, -0x1p-112L, 0x1p-112L, -1.925929944387235853055977942584927318538e-34L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casinh, -0x1p-112L, -0x1p-112L, -1.925929944387235853055977942584927318538e-34L, -1.925929944387235853055977942584927318538e-34L),
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (casinh, 0x1.fp-16385L, 0x1p-112L, 8.142593549724601460479922838826119584417e-4933L, 1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x1.fp-16385L, -0x1p-112L, 8.142593549724601460479922838826119584417e-4933L, -1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.fp-16385L, 0x1p-112L, -8.142593549724601460479922838826119584417e-4933L, 1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1.fp-16385L, -0x1p-112L, -8.142593549724601460479922838826119584417e-4933L, -1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x1p-112L, 0x1.fp-16385L, 1.925929944387235853055977942584927318538e-34L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1p-112L, 0x1.fp-16385L, -1.925929944387235853055977942584927318538e-34L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, 0x1p-112L, -0x1.fp-16385L, 1.925929944387235853055977942584927318538e-34L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (casinh, -0x1p-112L, -0x1.fp-16385L, -1.925929944387235853055977942584927318538e-34L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (casinh, 0.0L, 0x1p-112L, 0.0L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casinh, 0.0L, -0x1p-112L, 0.0L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casinh, -0.0L, 0x1p-112L, -0.0L, 1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casinh, -0.0L, -0x1p-112L, -0.0L, -1.925929944387235853055977942584927318538e-34L),
    TEST_c_c (casinh, 0x1p-112L, 0.0L, 1.925929944387235853055977942584927318538e-34L, 0.0L),
    TEST_c_c (casinh, -0x1p-112L, 0.0L, -1.925929944387235853055977942584927318538e-34L, 0.0L),
    TEST_c_c (casinh, 0x1p-112L, -0.0L, 1.925929944387235853055977942584927318538e-34L, -0.0L),
    TEST_c_c (casinh, -0x1p-112L, -0.0L, -1.925929944387235853055977942584927318538e-34L, -0.0L),

    TEST_c_c (casinh, 0.75L, 1.25L, 1.03171853444778027336364058631006594L, 0.911738290968487636358489564316731207L),
    TEST_c_c (casinh, -2, -3, -1.9686379257930962917886650952454982L, -0.96465850440760279204541105949953237L),
    END_DATA (casinh)
  };

static void
casinh_test (void)
{
  errno = 0;
  FUNC(casinh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (casinh);
  RUN_TEST_LOOP_c_c (casinh, casinh_test_data, );
  END (casinh, complex);
}


static const struct test_c_c_data catan_test_data[] =
  {
    START_DATA (catan),
    TEST_c_c (catan, 0, 0, 0, 0),
    TEST_c_c (catan, minus_zero, 0, minus_zero, 0),
    TEST_c_c (catan, 0, minus_zero, 0, minus_zero),
    TEST_c_c (catan, minus_zero, minus_zero, minus_zero, minus_zero),

    TEST_c_c (catan, 1.0L, plus_zero, M_PI_4l, plus_zero),
    TEST_c_c (catan, 1.0L, minus_zero, M_PI_4l, minus_zero),
    TEST_c_c (catan, -1.0L, plus_zero, -M_PI_4l, plus_zero),
    TEST_c_c (catan, -1.0L, minus_zero, -M_PI_4l, minus_zero),
    TEST_c_c (catan, plus_zero, 1.0L, plus_zero, plus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_c_c (catan, minus_zero, 1.0L, minus_zero, plus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_c_c (catan, plus_zero, -1.0L, plus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_c_c (catan, minus_zero, -1.0L, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),

    TEST_c_c (catan, plus_infty, plus_infty, M_PI_2l, 0),
    TEST_c_c (catan, plus_infty, minus_infty, M_PI_2l, minus_zero),
    TEST_c_c (catan, minus_infty, plus_infty, -M_PI_2l, 0),
    TEST_c_c (catan, minus_infty, minus_infty, -M_PI_2l, minus_zero),


    TEST_c_c (catan, plus_infty, -10.0, M_PI_2l, minus_zero),
    TEST_c_c (catan, minus_infty, -10.0, -M_PI_2l, minus_zero),
    TEST_c_c (catan, plus_infty, minus_zero, M_PI_2l, minus_zero),
    TEST_c_c (catan, minus_infty, minus_zero, -M_PI_2l, minus_zero),
    TEST_c_c (catan, plus_infty, 0.0, M_PI_2l, 0),
    TEST_c_c (catan, minus_infty, 0.0, -M_PI_2l, 0),
    TEST_c_c (catan, plus_infty, 0.1L, M_PI_2l, 0),
    TEST_c_c (catan, minus_infty, 0.1L, -M_PI_2l, 0),

    TEST_c_c (catan, 0.0, minus_infty, M_PI_2l, minus_zero),
    TEST_c_c (catan, minus_zero, minus_infty, -M_PI_2l, minus_zero),
    TEST_c_c (catan, 100.0, minus_infty, M_PI_2l, minus_zero),
    TEST_c_c (catan, -100.0, minus_infty, -M_PI_2l, minus_zero),

    TEST_c_c (catan, 0.0, plus_infty, M_PI_2l, 0),
    TEST_c_c (catan, minus_zero, plus_infty, -M_PI_2l, 0),
    TEST_c_c (catan, 0.5, plus_infty, M_PI_2l, 0),
    TEST_c_c (catan, -0.5, plus_infty, -M_PI_2l, 0),

    TEST_c_c (catan, qnan_value, 0.0, qnan_value, 0),
    TEST_c_c (catan, qnan_value, minus_zero, qnan_value, minus_zero),

    TEST_c_c (catan, qnan_value, plus_infty, qnan_value, 0),
    TEST_c_c (catan, qnan_value, minus_infty, qnan_value, minus_zero),

    TEST_c_c (catan, 0.0, qnan_value, qnan_value, qnan_value),
    TEST_c_c (catan, minus_zero, qnan_value, qnan_value, qnan_value),

    TEST_c_c (catan, plus_infty, qnan_value, M_PI_2l, 0, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (catan, minus_infty, qnan_value, -M_PI_2l, 0, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (catan, qnan_value, 10.5, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (catan, qnan_value, -10.5, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (catan, 0.75, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (catan, -0.75, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (catan, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (catan, 0x1p50L, 0.0L, 1.570796326794895731052901991514519103193L, 0.0L),
    TEST_c_c (catan, 0x1p50L, -0.0L, 1.570796326794895731052901991514519103193L, -0.0L),
    TEST_c_c (catan, -0x1p50L, 0.0L, -1.570796326794895731052901991514519103193L, 0.0L),
    TEST_c_c (catan, -0x1p50L, -0.0L, -1.570796326794895731052901991514519103193L, -0.0L),
    TEST_c_c (catan, 0.0L, 0x1p50L, 1.570796326794896619231321691639751442099L, 8.881784197001252323389053344728897997441e-16L),
    TEST_c_c (catan, -0.0L, 0x1p50L, -1.570796326794896619231321691639751442099L, 8.881784197001252323389053344728897997441e-16L),
    TEST_c_c (catan, 0.0L, -0x1p50L, 1.570796326794896619231321691639751442099L, -8.881784197001252323389053344728897997441e-16L),
    TEST_c_c (catan, -0.0L, -0x1p50L, -1.570796326794896619231321691639751442099L, -8.881784197001252323389053344728897997441e-16L),
#ifndef TEST_FLOAT
    TEST_c_c (catan, 0x1p500L, 0.0L, 1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, 0x1p500L, -0.0L, 1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, -0x1p500L, 0.0L, -1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, -0x1p500L, -0.0L, -1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, 0.0L, 0x1p500L, 1.570796326794896619231321691639751442099L, 3.054936363499604682051979393213617699789e-151L),
    TEST_c_c (catan, -0.0L, 0x1p500L, -1.570796326794896619231321691639751442099L, 3.054936363499604682051979393213617699789e-151L),
    TEST_c_c (catan, 0.0L, -0x1p500L, 1.570796326794896619231321691639751442099L, -3.054936363499604682051979393213617699789e-151L),
    TEST_c_c (catan, -0.0L, -0x1p500L, -1.570796326794896619231321691639751442099L, -3.054936363499604682051979393213617699789e-151L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (catan, 0x1p5000L, 0.0L, 1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, 0x1p5000L, -0.0L, 1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, -0x1p5000L, 0.0L, -1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, -0x1p5000L, -0.0L, -1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, 0.0L, 0x1p5000L, 1.570796326794896619231321691639751442099L, 7.079811261048172892385615158694057552948e-1506L),
    TEST_c_c (catan, -0.0L, 0x1p5000L, -1.570796326794896619231321691639751442099L, 7.079811261048172892385615158694057552948e-1506L),
    TEST_c_c (catan, 0.0L, -0x1p5000L, 1.570796326794896619231321691639751442099L, -7.079811261048172892385615158694057552948e-1506L),
    TEST_c_c (catan, -0.0L, -0x1p5000L, -1.570796326794896619231321691639751442099L, -7.079811261048172892385615158694057552948e-1506L),
#endif
    TEST_c_c (catan, 0x1p63L, 0.5L, 1.570796326794896619122901474391200998698L, 5.877471754111437539843682686111228389007e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, 0x1p63L, -0.5L, 1.570796326794896619122901474391200998698L, -5.877471754111437539843682686111228389007e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, -0x1p63L, 0.5L, -1.570796326794896619122901474391200998698L, 5.877471754111437539843682686111228389007e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, -0x1p63L, -0.5L, -1.570796326794896619122901474391200998698L, -5.877471754111437539843682686111228389007e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, 0.5L, 0x1p63L, 1.570796326794896619231321691639751442093L, 1.084202172485504434007452800869941711427e-19L),
    TEST_c_c (catan, -0.5L, 0x1p63L, -1.570796326794896619231321691639751442093L, 1.084202172485504434007452800869941711427e-19L),
    TEST_c_c (catan, 0.5L, -0x1p63L, 1.570796326794896619231321691639751442093L, -1.084202172485504434007452800869941711427e-19L),
    TEST_c_c (catan, -0.5L, -0x1p63L, -1.570796326794896619231321691639751442093L, -1.084202172485504434007452800869941711427e-19L),
#ifndef TEST_FLOAT
    TEST_c_c (catan, 0x1p511L, 0.5L, 1.570796326794896619231321691639751442099L, 1.112536929253600691545116358666202032110e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1p511L, -0.5L, 1.570796326794896619231321691639751442099L, -1.112536929253600691545116358666202032110e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x1p511L, 0.5L, -1.570796326794896619231321691639751442099L, 1.112536929253600691545116358666202032110e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x1p511L, -0.5L, -1.570796326794896619231321691639751442099L, -1.112536929253600691545116358666202032110e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0.5L, 0x1p511L, 1.570796326794896619231321691639751442099L, 1.491668146240041348658193063092586767475e-154L),
    TEST_c_c (catan, -0.5L, 0x1p511L, -1.570796326794896619231321691639751442099L, 1.491668146240041348658193063092586767475e-154L),
    TEST_c_c (catan, 0.5L, -0x1p511L, 1.570796326794896619231321691639751442099L, -1.491668146240041348658193063092586767475e-154L),
    TEST_c_c (catan, -0.5L, -0x1p511L, -1.570796326794896619231321691639751442099L, -1.491668146240041348658193063092586767475e-154L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (catan, 0x1p8191L, 0.5L, 1.570796326794896619231321691639751442099L, 1.681051571556046753131338908660876301299e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0x1p8191L, -0.5L, 1.570796326794896619231321691639751442099L, -1.681051571556046753131338908660876301299e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x1p8191L, 0.5L, -1.570796326794896619231321691639751442099L, 1.681051571556046753131338908660876301299e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x1p8191L, -0.5L, -1.570796326794896619231321691639751442099L, -1.681051571556046753131338908660876301299e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0.5L, 0x1p8191L, 1.570796326794896619231321691639751442099L, 1.833603867554847165621412392048483165956e-2466L),
    TEST_c_c (catan, -0.5L, 0x1p8191L, -1.570796326794896619231321691639751442099L, 1.833603867554847165621412392048483165956e-2466L),
    TEST_c_c (catan, 0.5L, -0x1p8191L, 1.570796326794896619231321691639751442099L, -1.833603867554847165621412392048483165956e-2466L),
    TEST_c_c (catan, -0.5L, -0x1p8191L, -1.570796326794896619231321691639751442099L, -1.833603867554847165621412392048483165956e-2466L),
#endif
    TEST_c_c (catan, 0x1p100L, 0.0L, 1.570796326794896619231321691638962581193L, 0.0L),
    TEST_c_c (catan, 0x1p100L, -0.0L, 1.570796326794896619231321691638962581193L, -0.0L),
    TEST_c_c (catan, -0x1p100L, 0.0L, -1.570796326794896619231321691638962581193L, 0.0L),
    TEST_c_c (catan, -0x1p100L, -0.0L, -1.570796326794896619231321691638962581193L, -0.0L),
    TEST_c_c (catan, 0.0L, 0x1p100L, 1.570796326794896619231321691639751442099L, 7.888609052210118054117285652827862296732e-31L),
    TEST_c_c (catan, -0.0L, 0x1p100L, -1.570796326794896619231321691639751442099L, 7.888609052210118054117285652827862296732e-31L),
    TEST_c_c (catan, 0.0L, -0x1p100L, 1.570796326794896619231321691639751442099L, -7.888609052210118054117285652827862296732e-31L),
    TEST_c_c (catan, -0.0L, -0x1p100L, -1.570796326794896619231321691639751442099L, -7.888609052210118054117285652827862296732e-31L),
    TEST_c_c (catan, 0x1.fp127L, 0.0L, 1.570796326794896619231321691639751442096L, 0.0L),
    TEST_c_c (catan, 0x1.fp127L, -0.0L, 1.570796326794896619231321691639751442096L, -0.0L),
    TEST_c_c (catan, -0x1.fp127L, 0.0L, -1.570796326794896619231321691639751442096L, 0.0L),
    TEST_c_c (catan, -0x1.fp127L, -0.0L, -1.570796326794896619231321691639751442096L, -0.0L),
    TEST_c_c (catan, 0.0L, 0x1.fp127L, 1.570796326794896619231321691639751442099L, 3.033533808573645181854803967025150136306e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, -0.0L, 0x1.fp127L, -1.570796326794896619231321691639751442099L, 3.033533808573645181854803967025150136306e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, 0.0L, -0x1.fp127L, 1.570796326794896619231321691639751442099L, -3.033533808573645181854803967025150136306e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, -0.0L, -0x1.fp127L, -1.570796326794896619231321691639751442099L, -3.033533808573645181854803967025150136306e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, 0x1.fp127L, 0x1.fp127L, 1.570796326794896619231321691639751442097L, 1.516766904286822590927401983512575068153e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, 0x1.fp127L, -0x1.fp127L, 1.570796326794896619231321691639751442097L, -1.516766904286822590927401983512575068153e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, -0x1.fp127L, 0x1.fp127L, -1.570796326794896619231321691639751442097L, 1.516766904286822590927401983512575068153e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, -0x1.fp127L, -0x1.fp127L, -1.570796326794896619231321691639751442097L, -1.516766904286822590927401983512575068153e-39L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (catan, 0x1p900L, 0.0L, 1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, 0x1p900L, -0.0L, 1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, -0x1p900L, 0.0L, -1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, -0x1p900L, -0.0L, -1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, 0.0L, 0x1p900L, 1.570796326794896619231321691639751442099L, 1.183052186166774710972751597518026531652e-271L),
    TEST_c_c (catan, -0.0L, 0x1p900L, -1.570796326794896619231321691639751442099L, 1.183052186166774710972751597518026531652e-271L),
    TEST_c_c (catan, 0.0L, -0x1p900L, 1.570796326794896619231321691639751442099L, -1.183052186166774710972751597518026531652e-271L),
    TEST_c_c (catan, -0.0L, -0x1p900L, -1.570796326794896619231321691639751442099L, -1.183052186166774710972751597518026531652e-271L),
    TEST_c_c (catan, 0x1.fp1023L, 0.0L, 1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, 0x1.fp1023L, -0.0L, 1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, -0x1.fp1023L, 0.0L, -1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, -0x1.fp1023L, -0.0L, -1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, 0.0L, 0x1.fp1023L, 1.570796326794896619231321691639751442099L, 5.742126086470197117652213464083623391533e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0.0L, 0x1.fp1023L, -1.570796326794896619231321691639751442099L, 5.742126086470197117652213464083623391533e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0.0L, -0x1.fp1023L, 1.570796326794896619231321691639751442099L, -5.742126086470197117652213464083623391533e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0.0L, -0x1.fp1023L, -1.570796326794896619231321691639751442099L, -5.742126086470197117652213464083623391533e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1.fp1023L, 0x1.fp1023L, 1.570796326794896619231321691639751442099L, 2.871063043235098558826106732041811695767e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1.fp1023L, -0x1.fp1023L, 1.570796326794896619231321691639751442099L, -2.871063043235098558826106732041811695767e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x1.fp1023L, 0x1.fp1023L, -1.570796326794896619231321691639751442099L, 2.871063043235098558826106732041811695767e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x1.fp1023L, -0x1.fp1023L, -1.570796326794896619231321691639751442099L, -2.871063043235098558826106732041811695767e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (catan, 0x1p10000L, 0.0L, 1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, 0x1p10000L, -0.0L, 1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, -0x1p10000L, 0.0L, -1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, -0x1p10000L, -0.0L, -1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, 0.0L, 0x1p10000L, 1.570796326794896619231321691639751442099L, 5.012372749206452009297555933742977749322e-3011L),
    TEST_c_c (catan, -0.0L, 0x1p10000L, -1.570796326794896619231321691639751442099L, 5.012372749206452009297555933742977749322e-3011L),
    TEST_c_c (catan, 0.0L, -0x1p10000L, 1.570796326794896619231321691639751442099L, -5.012372749206452009297555933742977749322e-3011L),
    TEST_c_c (catan, -0.0L, -0x1p10000L, -1.570796326794896619231321691639751442099L, -5.012372749206452009297555933742977749322e-3011L),
    TEST_c_c (catan, 0x1.fp16383L, 0.0L, 1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, 0x1.fp16383L, -0.0L, 1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, -0x1.fp16383L, 0.0L, -1.570796326794896619231321691639751442099L, 0.0L),
    TEST_c_c (catan, -0x1.fp16383L, -0.0L, -1.570796326794896619231321691639751442099L, -0.0L),
    TEST_c_c (catan, 0.0L, 0x1.fp16383L, 1.570796326794896619231321691639751442099L, 8.676395208031209048419813722120651877672e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0.0L, 0x1.fp16383L, -1.570796326794896619231321691639751442099L, 8.676395208031209048419813722120651877672e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0.0L, -0x1.fp16383L, 1.570796326794896619231321691639751442099L, -8.676395208031209048419813722120651877672e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0.0L, -0x1.fp16383L, -1.570796326794896619231321691639751442099L, -8.676395208031209048419813722120651877672e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0x1.fp16383L, 0x1.fp16383L, 1.570796326794896619231321691639751442099L, 4.338197604015604524209906861060325938836e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0x1.fp16383L, -0x1.fp16383L, 1.570796326794896619231321691639751442099L, -4.338197604015604524209906861060325938836e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x1.fp16383L, 0x1.fp16383L, -1.570796326794896619231321691639751442099L, 4.338197604015604524209906861060325938836e-4933L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x1.fp16383L, -0x1.fp16383L, -1.570796326794896619231321691639751442099L, -4.338197604015604524209906861060325938836e-4933L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (catan, 0x1p-13L, 1.0L, 7.854286809755354140031716771044626356262e-1L, 4.852030264850939738801379894163661227127L),
    TEST_c_c (catan, 0x1p-13L, -1.0L, 7.854286809755354140031716771044626356262e-1L, -4.852030264850939738801379894163661227127L),
    TEST_c_c (catan, -0x1p-13L, 1.0L, -7.854286809755354140031716771044626356262e-1L, 4.852030264850939738801379894163661227127L),
    TEST_c_c (catan, -0x1p-13L, -1.0L, -7.854286809755354140031716771044626356262e-1L, -4.852030264850939738801379894163661227127L),
    TEST_c_c (catan, 1.0L, 0x1p-13L, 7.853981671227386080775748393881580082970e-1L, 6.103515609841754902688560615027452023669e-5L),
    TEST_c_c (catan, -1.0L, 0x1p-13L, -7.853981671227386080775748393881580082970e-1L, 6.103515609841754902688560615027452023669e-5L),
    TEST_c_c (catan, 1.0L, -0x1p-13L, 7.853981671227386080775748393881580082970e-1L, -6.103515609841754902688560615027452023669e-5L),
    TEST_c_c (catan, -1.0L, -0x1p-13L, -7.853981671227386080775748393881580082970e-1L, -6.103515609841754902688560615027452023669e-5L),
    TEST_c_c (catan, 0x1p-27L, 1.0L, 7.853981652600934588466178684534110069553e-1L, 9.704060527839234335310696652368086117807L),
    TEST_c_c (catan, 0x1p-27L, -1.0L, 7.853981652600934588466178684534110069553e-1L, -9.704060527839234335310696652368086117807L),
    TEST_c_c (catan, -0x1p-27L, 1.0L, -7.853981652600934588466178684534110069553e-1L, 9.704060527839234335310696652368086117807L),
    TEST_c_c (catan, -0x1p-27L, -1.0L, -7.853981652600934588466178684534110069553e-1L, -9.704060527839234335310696652368086117807L),
    TEST_c_c (catan, 1.0L, 0x1p-27L, 7.853981633974483234934486536343324763447e-1L, 3.725290298461914028034141143623846306386e-9L),
    TEST_c_c (catan, -1.0L, 0x1p-27L, -7.853981633974483234934486536343324763447e-1L, 3.725290298461914028034141143623846306386e-9L),
    TEST_c_c (catan, 1.0L, -0x1p-27L, 7.853981633974483234934486536343324763447e-1L, -3.725290298461914028034141143623846306386e-9L),
    TEST_c_c (catan, -1.0L, -0x1p-27L, -7.853981633974483234934486536343324763447e-1L, -3.725290298461914028034141143623846306386e-9L),
    TEST_c_c (catan, 0x1p-33L, 1.0L, 7.853981634265521400723945494331241018449e-1L, 1.178350206951907026009379309773625595762e1L),
    TEST_c_c (catan, 0x1p-33L, -1.0L, 7.853981634265521400723945494331241018449e-1L, -1.178350206951907026009379309773625595762e1L),
    TEST_c_c (catan, -0x1p-33L, 1.0L, -7.853981634265521400723945494331241018449e-1L, 1.178350206951907026009379309773625595762e1L),
    TEST_c_c (catan, -0x1p-33L, -1.0L, -7.853981634265521400723945494331241018449e-1L, -1.178350206951907026009379309773625595762e1L),
    TEST_c_c (catan, 1.0L, 0x1p-33L, 7.853981633974483096190489776088929224056e-1L, 5.820766091346740722643102318246316469910e-11L),
    TEST_c_c (catan, -1.0L, 0x1p-33L, -7.853981633974483096190489776088929224056e-1L, 5.820766091346740722643102318246316469910e-11L),
    TEST_c_c (catan, 1.0L, -0x1p-33L, 7.853981633974483096190489776088929224056e-1L, -5.820766091346740722643102318246316469910e-11L),
    TEST_c_c (catan, -1.0L, -0x1p-33L, -7.853981633974483096190489776088929224056e-1L, -5.820766091346740722643102318246316469910e-11L),
    TEST_c_c (catan, 0x1p-54L, 1.0L, 7.853981633974483234934486536343324763447e-1L, 1.906154746539849600897388334009985581467e1L),
    TEST_c_c (catan, 0x1p-54L, -1.0L, 7.853981633974483234934486536343324763447e-1L, -1.906154746539849600897388334009985581467e1L),
    TEST_c_c (catan, -0x1p-54L, 1.0L, -7.853981633974483234934486536343324763447e-1L, 1.906154746539849600897388334009985581467e1L),
    TEST_c_c (catan, -0x1p-54L, -1.0L, -7.853981633974483234934486536343324763447e-1L, -1.906154746539849600897388334009985581467e1L),
    TEST_c_c (catan, 1.0L, 0x1p-54L, 7.853981633974483096156608458198764914213e-1L, 2.775557561562891351059079170227049355775e-17L),
    TEST_c_c (catan, -1.0L, 0x1p-54L, -7.853981633974483096156608458198764914213e-1L, 2.775557561562891351059079170227049355775e-17L),
    TEST_c_c (catan, 1.0L, -0x1p-54L, 7.853981633974483096156608458198764914213e-1L, -2.775557561562891351059079170227049355775e-17L),
    TEST_c_c (catan, -1.0L, -0x1p-54L, -7.853981633974483096156608458198764914213e-1L, -2.775557561562891351059079170227049355775e-17L),
    TEST_c_c (catan, 0x1p-57L, 1.0L, 7.853981633974483113503843217966828154612e-1L, 2.010126823623841397309973152228712047720e1L),
    TEST_c_c (catan, 0x1p-57L, -1.0L, 7.853981633974483113503843217966828154612e-1L, -2.010126823623841397309973152228712047720e1L),
    TEST_c_c (catan, -0x1p-57L, 1.0L, -7.853981633974483113503843217966828154612e-1L, 2.010126823623841397309973152228712047720e1L),
    TEST_c_c (catan, -0x1p-57L, -1.0L, -7.853981633974483113503843217966828154612e-1L, -2.010126823623841397309973152228712047720e1L),
    TEST_c_c (catan, 1.0L, 0x1p-57L, 7.853981633974483096156608458198757330864e-1L, 3.469446951953614188823848962783813448721e-18L),
    TEST_c_c (catan, -1.0L, 0x1p-57L, -7.853981633974483096156608458198757330864e-1L, 3.469446951953614188823848962783813448721e-18L),
    TEST_c_c (catan, 1.0L, -0x1p-57L, 7.853981633974483096156608458198757330864e-1L, -3.469446951953614188823848962783813448721e-18L),
    TEST_c_c (catan, -1.0L, -0x1p-57L, -7.853981633974483096156608458198757330864e-1L, -3.469446951953614188823848962783813448721e-18L),
    TEST_c_c (catan, 0x1p-13L, 0x1.000002p0L, 7.859169620684960844300240092596908675974e-1L, 4.852030056234795712498957387213592193975L),
    TEST_c_c (catan, 0x1p-13L, -0x1.000002p0L, 7.859169620684960844300240092596908675974e-1L, -4.852030056234795712498957387213592193975L),
    TEST_c_c (catan, -0x1p-13L, 0x1.000002p0L, -7.859169620684960844300240092596908675974e-1L, 4.852030056234795712498957387213592193975L),
    TEST_c_c (catan, -0x1p-13L, -0x1.000002p0L, -7.859169620684960844300240092596908675974e-1L, -4.852030056234795712498957387213592193975L),
    TEST_c_c (catan, 0x1.000002p0L, 0x1p-13L, 7.853982267273793866654490522673596014524e-1L, 6.103514882246036852433556327261700380577e-5L),
    TEST_c_c (catan, -0x1.000002p0L, 0x1p-13L, -7.853982267273793866654490522673596014524e-1L, 6.103514882246036852433556327261700380577e-5L),
    TEST_c_c (catan, 0x1.000002p0L, -0x1p-13L, 7.853982267273793866654490522673596014524e-1L, -6.103514882246036852433556327261700380577e-5L),
    TEST_c_c (catan, -0x1.000002p0L, -0x1p-13L, -7.853982267273793866654490522673596014524e-1L, -6.103514882246036852433556327261700380577e-5L),
    TEST_c_c (catan, 0x1p-13L, 0x0.ffffffp0L, 7.851845403708474595909269086711426246675e-1L, 4.852030190345140708455871037447717761868L),
    TEST_c_c (catan, 0x1p-13L, -0x0.ffffffp0L, 7.851845403708474595909269086711426246675e-1L, -4.852030190345140708455871037447717761868L),
    TEST_c_c (catan, -0x1p-13L, 0x0.ffffffp0L, -7.851845403708474595909269086711426246675e-1L, 4.852030190345140708455871037447717761868L),
    TEST_c_c (catan, -0x1p-13L, -0x0.ffffffp0L, -7.851845403708474595909269086711426246675e-1L, -4.852030190345140708455871037447717761868L),
    TEST_c_c (catan, 0x0.ffffffp0L, 0x1p-13L, 7.853981373204155542484315721351697277336e-1L, 6.103515973639646453881721999956617260502e-5L),
    TEST_c_c (catan, -0x0.ffffffp0L, 0x1p-13L, -7.853981373204155542484315721351697277336e-1L, 6.103515973639646453881721999956617260502e-5L),
    TEST_c_c (catan, 0x0.ffffffp0L, -0x1p-13L, 7.853981373204155542484315721351697277336e-1L, -6.103515973639646453881721999956617260502e-5L),
    TEST_c_c (catan, -0x0.ffffffp0L, -0x1p-13L, -7.853981373204155542484315721351697277336e-1L, -6.103515973639646453881721999956617260502e-5L),
#ifndef TEST_FLOAT
    TEST_c_c (catan, 0x1p-27L, 0x1.0000000000001p0L, 7.853981801612546526942695000283242525531e-1L, 9.704060527839234168777242958594699810015L),
    TEST_c_c (catan, 0x1p-27L, -0x1.0000000000001p0L, 7.853981801612546526942695000283242525531e-1L, -9.704060527839234168777242958594699810015L),
    TEST_c_c (catan, -0x1p-27L, 0x1.0000000000001p0L, -7.853981801612546526942695000283242525531e-1L, 9.704060527839234168777242958594699810015L),
    TEST_c_c (catan, -0x1p-27L, -0x1.0000000000001p0L, -7.853981801612546526942695000283242525531e-1L, -9.704060527839234168777242958594699810015L),
    TEST_c_c (catan, 0x1.0000000000001p0L, 0x1p-27L, 7.853981633974484345157511161499711112683e-1L, 3.725290298461913200853528590596263270474e-9L),
    TEST_c_c (catan, -0x1.0000000000001p0L, 0x1p-27L, -7.853981633974484345157511161499711112683e-1L, 3.725290298461913200853528590596263270474e-9L),
    TEST_c_c (catan, 0x1.0000000000001p0L, -0x1p-27L, 7.853981633974484345157511161499711112683e-1L, -3.725290298461913200853528590596263270474e-9L),
    TEST_c_c (catan, -0x1.0000000000001p0L, -0x1p-27L, -7.853981633974484345157511161499711112683e-1L, -3.725290298461913200853528590596263270474e-9L),
    TEST_c_c (catan, 0x1p-27L, 0x0.fffffffffffff8p0L, 7.853981578095128619227903983047292781021e-1L, 9.704060527839234252043969805481351363824L),
    TEST_c_c (catan, 0x1p-27L, -0x0.fffffffffffff8p0L, 7.853981578095128619227903983047292781021e-1L, -9.704060527839234252043969805481351363824L),
    TEST_c_c (catan, -0x1p-27L, 0x0.fffffffffffff8p0L, -7.853981578095128619227903983047292781021e-1L, 9.704060527839234252043969805481351363824L),
    TEST_c_c (catan, -0x1p-27L, -0x0.fffffffffffff8p0L, -7.853981578095128619227903983047292781021e-1L, -9.704060527839234252043969805481351363824L),
    TEST_c_c (catan, 0x0.fffffffffffff8p0L, 0x1p-27L, 7.853981633974482679822974223765039144191e-1L, 3.725290298461914441624447420137706700965e-9L),
    TEST_c_c (catan, -0x0.fffffffffffff8p0L, 0x1p-27L, -7.853981633974482679822974223765039144191e-1L, 3.725290298461914441624447420137706700965e-9L),
    TEST_c_c (catan, 0x0.fffffffffffff8p0L, -0x1p-27L, 7.853981633974482679822974223765039144191e-1L, -3.725290298461914441624447420137706700965e-9L),
    TEST_c_c (catan, -0x0.fffffffffffff8p0L, -0x1p-27L, -7.853981633974482679822974223765039144191e-1L, -3.725290298461914441624447420137706700965e-9L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (catan, 0x1p-33L, 0x1.0000000000000002p0L, 7.853981638922134273801338071094141188767e-1L, 1.178350206951907025990405771755129268176e1L),
    TEST_c_c (catan, 0x1p-33L, -0x1.0000000000000002p0L, 7.853981638922134273801338071094141188767e-1L, -1.178350206951907025990405771755129268176e1L),
    TEST_c_c (catan, -0x1p-33L, 0x1.0000000000000002p0L, -7.853981638922134273801338071094141188767e-1L, 1.178350206951907025990405771755129268176e1L),
    TEST_c_c (catan, -0x1p-33L, -0x1.0000000000000002p0L, -7.853981638922134273801338071094141188767e-1L, -1.178350206951907025990405771755129268176e1L),
    TEST_c_c (catan, 0x1.0000000000000002p0L, 0x1p-33L, 7.853981633974483096732590862331681441026e-1L, 5.820766091346740722012013594069507025615e-11L),
    TEST_c_c (catan, -0x1.0000000000000002p0L, 0x1p-33L, -7.853981633974483096732590862331681441026e-1L, 5.820766091346740722012013594069507025615e-11L),
    TEST_c_c (catan, 0x1.0000000000000002p0L, -0x1p-33L, 7.853981633974483096732590862331681441026e-1L, -5.820766091346740722012013594069507025615e-11L),
    TEST_c_c (catan, -0x1.0000000000000002p0L, -0x1p-33L, -7.853981633974483096732590862331681441026e-1L, -5.820766091346740722012013594069507025615e-11L),
    TEST_c_c (catan, 0x1p-33L, 0x0.ffffffffffffffffp0L, 7.853981631937214964185249205444919953948e-1L, 1.178350206951907026002603046195591193050e1L),
    TEST_c_c (catan, 0x1p-33L, -0x0.ffffffffffffffffp0L, 7.853981631937214964185249205444919953948e-1L, -1.178350206951907026002603046195591193050e1L),
    TEST_c_c (catan, -0x1p-33L, 0x0.ffffffffffffffffp0L, -7.853981631937214964185249205444919953948e-1L, 1.178350206951907026002603046195591193050e1L),
    TEST_c_c (catan, -0x1p-33L, -0x0.ffffffffffffffffp0L, -7.853981631937214964185249205444919953948e-1L, -1.178350206951907026002603046195591193050e1L),
    TEST_c_c (catan, 0x0.ffffffffffffffffp0L, 0x1p-33L, 7.853981633974483095919439232967553115548e-1L, 5.820766091346740722958646680334721192083e-11L),
    TEST_c_c (catan, -0x0.ffffffffffffffffp0L, 0x1p-33L, -7.853981633974483095919439232967553115548e-1L, 5.820766091346740722958646680334721192083e-11L),
    TEST_c_c (catan, 0x0.ffffffffffffffffp0L, -0x1p-33L, 7.853981633974483095919439232967553115548e-1L, -5.820766091346740722958646680334721192083e-11L),
    TEST_c_c (catan, -0x0.ffffffffffffffffp0L, -0x1p-33L, -7.853981633974483095919439232967553115548e-1L, -5.820766091346740722958646680334721192083e-11L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (catan, 0x1p-54L, 0x1.000000000000000000000000008p0L, 7.853981633974485455380535786656405610710e-1L, 1.906154746539849600897388334009981267384e1L),
    TEST_c_c (catan, 0x1p-54L, -0x1.000000000000000000000000008p0L, 7.853981633974485455380535786656405610710e-1L, -1.906154746539849600897388334009981267384e1L),
    TEST_c_c (catan, -0x1p-54L, 0x1.000000000000000000000000008p0L, -7.853981633974485455380535786656405610710e-1L, 1.906154746539849600897388334009981267384e1L),
    TEST_c_c (catan, -0x1p-54L, -0x1.000000000000000000000000008p0L, -7.853981633974485455380535786656405610710e-1L, -1.906154746539849600897388334009981267384e1L),
    TEST_c_c (catan, 0x1.000000000000000000000000008p0L, 0x1p-54L, 7.853981633974483096156608458198888173729e-1L, 2.775557561562891351059079170226980932999e-17L),
    TEST_c_c (catan, -0x1.000000000000000000000000008p0L, 0x1p-54L, -7.853981633974483096156608458198888173729e-1L, 2.775557561562891351059079170226980932999e-17L),
    TEST_c_c (catan, 0x1.000000000000000000000000008p0L, -0x1p-54L, 7.853981633974483096156608458198888173729e-1L, -2.775557561562891351059079170226980932999e-17L),
    TEST_c_c (catan, -0x1.000000000000000000000000008p0L, -0x1p-54L, -7.853981633974483096156608458198888173729e-1L, -2.775557561562891351059079170226980932999e-17L),
    TEST_c_c (catan, 0x1p-54L, 0x0.ffffffffffffffffffffffffffcp0L, 7.853981633974482124711461911186784339815e-1L, 1.906154746539849600897388334009984040723e1L),
    TEST_c_c (catan, 0x1p-54L, -0x0.ffffffffffffffffffffffffffcp0L, 7.853981633974482124711461911186784339815e-1L, -1.906154746539849600897388334009984040723e1L),
    TEST_c_c (catan, -0x1p-54L, 0x0.ffffffffffffffffffffffffffcp0L, -7.853981633974482124711461911186784339815e-1L, 1.906154746539849600897388334009984040723e1L),
    TEST_c_c (catan, -0x1p-54L, -0x0.ffffffffffffffffffffffffffcp0L, -7.853981633974482124711461911186784339815e-1L, -1.906154746539849600897388334009984040723e1L),
    TEST_c_c (catan, 0x0.ffffffffffffffffffffffffffcp0L, 0x1p-54L, 7.853981633974483096156608458198703284454e-1L, 2.775557561562891351059079170227083567164e-17L),
    TEST_c_c (catan, -0x0.ffffffffffffffffffffffffffcp0L, 0x1p-54L, -7.853981633974483096156608458198703284454e-1L, 2.775557561562891351059079170227083567164e-17L),
    TEST_c_c (catan, 0x0.ffffffffffffffffffffffffffcp0L, -0x1p-54L, 7.853981633974483096156608458198703284454e-1L, -2.775557561562891351059079170227083567164e-17L),
    TEST_c_c (catan, -0x0.ffffffffffffffffffffffffffcp0L, -0x1p-54L, -7.853981633974483096156608458198703284454e-1L, -2.775557561562891351059079170227083567164e-17L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (catan, 0x1p-57L, 0x1.0000000000000000000000000001p0L, 7.853981633974483252281721296111395707566e-1L, 2.010126823623841397309973152228712033275e1L),
    TEST_c_c (catan, 0x1p-57L, -0x1.0000000000000000000000000001p0L, 7.853981633974483252281721296111395707566e-1L, -2.010126823623841397309973152228712033275e1L),
    TEST_c_c (catan, -0x1p-57L, 0x1.0000000000000000000000000001p0L, -7.853981633974483252281721296111395707566e-1L, 2.010126823623841397309973152228712033275e1L),
    TEST_c_c (catan, -0x1p-57L, -0x1.0000000000000000000000000001p0L, -7.853981633974483252281721296111395707566e-1L, -2.010126823623841397309973152228712033275e1L),
    TEST_c_c (catan, 0x1.0000000000000000000000000001p0L, 0x1p-57L, 7.853981633974483096156608458198758293829e-1L, 3.469446951953614188823848962783812780530e-18L),
    TEST_c_c (catan, -0x1.0000000000000000000000000001p0L, 0x1p-57L, -7.853981633974483096156608458198758293829e-1L, 3.469446951953614188823848962783812780530e-18L),
    TEST_c_c (catan, 0x1.0000000000000000000000000001p0L, -0x1p-57L, 7.853981633974483096156608458198758293829e-1L, -3.469446951953614188823848962783812780530e-18L),
    TEST_c_c (catan, -0x1.0000000000000000000000000001p0L, -0x1p-57L, -7.853981633974483096156608458198758293829e-1L, -3.469446951953614188823848962783812780530e-18L),
    TEST_c_c (catan, 0x1p-57L, 0x0.ffffffffffffffffffffffffffff8p0L, 7.853981633974483044114904178894544378135e-1L, 2.010126823623841397309973152228712040498e1L),
    TEST_c_c (catan, 0x1p-57L, -0x0.ffffffffffffffffffffffffffff8p0L, 7.853981633974483044114904178894544378135e-1L, -2.010126823623841397309973152228712040498e1L),
    TEST_c_c (catan, -0x1p-57L, 0x0.ffffffffffffffffffffffffffff8p0L, -7.853981633974483044114904178894544378135e-1L, 2.010126823623841397309973152228712040498e1L),
    TEST_c_c (catan, -0x1p-57L, -0x0.ffffffffffffffffffffffffffff8p0L, -7.853981633974483044114904178894544378135e-1L, -2.010126823623841397309973152228712040498e1L),
    TEST_c_c (catan, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-57L, 7.853981633974483096156608458198756849381e-1L, 3.469446951953614188823848962783813782817e-18L),
    TEST_c_c (catan, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-57L, -7.853981633974483096156608458198756849381e-1L, 3.469446951953614188823848962783813782817e-18L),
    TEST_c_c (catan, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-57L, 7.853981633974483096156608458198756849381e-1L, -3.469446951953614188823848962783813782817e-18L),
    TEST_c_c (catan, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-57L, -7.853981633974483096156608458198756849381e-1L, -3.469446951953614188823848962783813782817e-18L),
#endif
    TEST_c_c (catan, 1.0L, 0x1p-64L, 7.853981633974483096156608458198757210500e-1L, 2.710505431213761085018632002174854278563e-20L),
    TEST_c_c (catan, 1.0L, -0x1p-64L, 7.853981633974483096156608458198757210500e-1L, -2.710505431213761085018632002174854278563e-20L),
    TEST_c_c (catan, -1.0L, 0x1p-64L, -7.853981633974483096156608458198757210500e-1L, 2.710505431213761085018632002174854278563e-20L),
    TEST_c_c (catan, -1.0L, -0x1p-64L, -7.853981633974483096156608458198757210500e-1L, -2.710505431213761085018632002174854278563e-20L),
    TEST_c_c (catan, 0x1p-64L, 1.0L, 7.853981633974483096292133729759445264744e-1L, 2.252728336819822255606004394739073846245e1L),
    TEST_c_c (catan, -0x1p-64L, 1.0L, -7.853981633974483096292133729759445264744e-1L, 2.252728336819822255606004394739073846245e1L),
    TEST_c_c (catan, 0x1p-64L, -1.0L, 7.853981633974483096292133729759445264744e-1L, -2.252728336819822255606004394739073846245e1L),
    TEST_c_c (catan, -0x1p-64L, -1.0L, -7.853981633974483096292133729759445264744e-1L, -2.252728336819822255606004394739073846245e1L),
    TEST_c_c (catan, 1.0L, 0x1.3p-73L, 7.853981633974483096156608458198757210493e-1L, 6.286572655403010329022706059731717687100e-23L),
    TEST_c_c (catan, 1.0L, -0x1.3p-73L, 7.853981633974483096156608458198757210493e-1L, -6.286572655403010329022706059731717687100e-23L),
    TEST_c_c (catan, -1.0L, 0x1.3p-73L, -7.853981633974483096156608458198757210493e-1L, 6.286572655403010329022706059731717687100e-23L),
    TEST_c_c (catan, -1.0L, -0x1.3p-73L, -7.853981633974483096156608458198757210493e-1L, -6.286572655403010329022706059731717687100e-23L),
    TEST_c_c (catan, 0x1.3p-73L, 1.0L, 7.853981633974483096156922786831527361009e-1L, 2.556052055225464683726753902092495938633e1L),
    TEST_c_c (catan, -0x1.3p-73L, 1.0L, -7.853981633974483096156922786831527361009e-1L, 2.556052055225464683726753902092495938633e1L),
    TEST_c_c (catan, 0x1.3p-73L, -1.0L, 7.853981633974483096156922786831527361009e-1L, -2.556052055225464683726753902092495938633e1L),
    TEST_c_c (catan, -0x1.3p-73L, -1.0L, -7.853981633974483096156922786831527361009e-1L, -2.556052055225464683726753902092495938633e1L),
    TEST_c_c (catan, 1.0L, 0x1p-124L, 7.853981633974483096156608458198757210493e-1L, 2.350988701644575015937473074444491355637e-38L),
    TEST_c_c (catan, 1.0L, -0x1p-124L, 7.853981633974483096156608458198757210493e-1L, -2.350988701644575015937473074444491355637e-38L),
    TEST_c_c (catan, -1.0L, 0x1p-124L, -7.853981633974483096156608458198757210493e-1L, 2.350988701644575015937473074444491355637e-38L),
    TEST_c_c (catan, -1.0L, -0x1p-124L, -7.853981633974483096156608458198757210493e-1L, -2.350988701644575015937473074444491355637e-38L),
    TEST_c_c (catan, 0x1p-124L, 1.0L, 7.853981633974483096156608458198757210610e-1L, 4.332169878499658183857700759113603550472e1L),
    TEST_c_c (catan, -0x1p-124L, 1.0L, -7.853981633974483096156608458198757210610e-1L, 4.332169878499658183857700759113603550472e1L),
    TEST_c_c (catan, 0x1p-124L, -1.0L, 7.853981633974483096156608458198757210610e-1L, -4.332169878499658183857700759113603550472e1L),
    TEST_c_c (catan, -0x1p-124L, -1.0L, -7.853981633974483096156608458198757210610e-1L, -4.332169878499658183857700759113603550472e1L),
    TEST_c_c (catan, 0x1.000002p0L, 0x1p-126L, 7.853982230020895322927482174768220815493e-1L, 5.877471053462247139383742414506671233336e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, 0x1.000002p0L, -0x1p-126L, 7.853982230020895322927482174768220815493e-1L, -5.877471053462247139383742414506671233336e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, -0x1.000002p0L, 0x1p-126L, -7.853982230020895322927482174768220815493e-1L, 5.877471053462247139383742414506671233336e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, -0x1.000002p0L, -0x1p-126L, -7.853982230020895322927482174768220815493e-1L, -5.877471053462247139383742414506671233336e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, 0x1p-126L, 0x1.000002p0L, 1.570796326794896619231321691639702138295L, 8.317766196521665212523713550410777685446L),
    TEST_c_c (catan, -0x1p-126L, 0x1.000002p0L, -1.570796326794896619231321691639702138295L, 8.317766196521665212523713550410777685446L),
    TEST_c_c (catan, 0x1p-126L, -0x1.000002p0L, 1.570796326794896619231321691639702138295L, -8.317766196521665212523713550410777685446L),
    TEST_c_c (catan, -0x1p-126L, -0x1.000002p0L, -1.570796326794896619231321691639702138295L, -8.317766196521665212523713550410777685446L),
    TEST_c_c (catan, 0x0.ffffffp0L, 0x1p-126L, 7.853981335951250337419109991749089175724e-1L, 5.877472104436064061535099214664320916184e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, 0x0.ffffffp0L, -0x1p-126L, 7.853981335951250337419109991749089175724e-1L, -5.877472104436064061535099214664320916184e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, -0x0.ffffffp0L, 0x1p-126L, -7.853981335951250337419109991749089175724e-1L, 5.877472104436064061535099214664320916184e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, -0x0.ffffffp0L, -0x1p-126L, -7.853981335951250337419109991749089175724e-1L, -5.877472104436064061535099214664320916184e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catan, 0x1p-126L, 0x0.ffffffp0L, 9.860761609136244031334147100512154160097e-32L, 8.664339742098154951823135931565866792454L),
    TEST_c_c (catan, -0x1p-126L, 0x0.ffffffp0L, -9.860761609136244031334147100512154160097e-32L, 8.664339742098154951823135931565866792454L),
    TEST_c_c (catan, 0x1p-126L, -0x0.ffffffp0L, 9.860761609136244031334147100512154160097e-32L, -8.664339742098154951823135931565866792454L),
    TEST_c_c (catan, -0x1p-126L, -0x0.ffffffp0L, -9.860761609136244031334147100512154160097e-32L, -8.664339742098154951823135931565866792454L),
#ifndef TEST_FLOAT
    TEST_c_c (catan, 1.0L, 0x1p-512L, 7.853981633974483096156608458198757210493e-1L, 3.729170365600103371645482657731466918688e-155L),
    TEST_c_c (catan, 1.0L, -0x1p-512L, 7.853981633974483096156608458198757210493e-1L, -3.729170365600103371645482657731466918688e-155L),
    TEST_c_c (catan, -1.0L, 0x1p-512L, -7.853981633974483096156608458198757210493e-1L, 3.729170365600103371645482657731466918688e-155L),
    TEST_c_c (catan, -1.0L, -0x1p-512L, -7.853981633974483096156608458198757210493e-1L, -3.729170365600103371645482657731466918688e-155L),
    TEST_c_c (catan, 0x1p-512L, 1.0L, 7.853981633974483096156608458198757210493e-1L, 1.777922518136259718655200391540222897114e2L),
    TEST_c_c (catan, -0x1p-512L, 1.0L, -7.853981633974483096156608458198757210493e-1L, 1.777922518136259718655200391540222897114e2L),
    TEST_c_c (catan, 0x1p-512L, -1.0L, 7.853981633974483096156608458198757210493e-1L, -1.777922518136259718655200391540222897114e2L),
    TEST_c_c (catan, -0x1p-512L, -1.0L, -7.853981633974483096156608458198757210493e-1L, -1.777922518136259718655200391540222897114e2L),
    TEST_c_c (catan, 1.0L, 0x1.3p-536L, 7.853981633974483096156608458198757210493e-1L, 2.639526015013529511588222179446290115084e-162L),
    TEST_c_c (catan, 1.0L, -0x1.3p-536L, 7.853981633974483096156608458198757210493e-1L, -2.639526015013529511588222179446290115084e-162L),
    TEST_c_c (catan, -1.0L, 0x1.3p-536L, -7.853981633974483096156608458198757210493e-1L, 2.639526015013529511588222179446290115084e-162L),
    TEST_c_c (catan, -1.0L, -0x1.3p-536L, -7.853981633974483096156608458198757210493e-1L, -2.639526015013529511588222179446290115084e-162L),
    TEST_c_c (catan, 0x1.3p-536L, 1.0L, 7.853981633974483096156608458198757210493e-1L, 1.860240928518819859673567751384928348958e2L),
    TEST_c_c (catan, -0x1.3p-536L, 1.0L, -7.853981633974483096156608458198757210493e-1L, 1.860240928518819859673567751384928348958e2L),
    TEST_c_c (catan, 0x1.3p-536L, -1.0L, 7.853981633974483096156608458198757210493e-1L, -1.860240928518819859673567751384928348958e2L),
    TEST_c_c (catan, -0x1.3p-536L, -1.0L, -7.853981633974483096156608458198757210493e-1L, -1.860240928518819859673567751384928348958e2L),
    TEST_c_c (catan, 1.0L, 0x1p-1020L, 7.853981633974483096156608458198757210493e-1L, 4.450147717014402766180465434664808128438e-308L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catan, 1.0L, -0x1p-1020L, 7.853981633974483096156608458198757210493e-1L, -4.450147717014402766180465434664808128438e-308L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catan, -1.0L, 0x1p-1020L, -7.853981633974483096156608458198757210493e-1L, 4.450147717014402766180465434664808128438e-308L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catan, -1.0L, -0x1p-1020L, -7.853981633974483096156608458198757210493e-1L, -4.450147717014402766180465434664808128438e-308L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catan, 0x1p-1020L, 1.0L, 7.853981633974483096156608458198757210493e-1L, 3.538516356758520804574969980043991380025e2L),
    TEST_c_c (catan, -0x1p-1020L, 1.0L, -7.853981633974483096156608458198757210493e-1L, 3.538516356758520804574969980043991380025e2L),
    TEST_c_c (catan, 0x1p-1020L, -1.0L, 7.853981633974483096156608458198757210493e-1L, -3.538516356758520804574969980043991380025e2L),
    TEST_c_c (catan, -0x1p-1020L, -1.0L, -7.853981633974483096156608458198757210493e-1L, -3.538516356758520804574969980043991380025e2L),
    TEST_c_c (catan, 0x1.0000000000001p0L, 0x1p-1022L, 7.853981633974484206379633083355174374608e-1L, 1.112536929253600444512293438042957369978e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1.0000000000001p0L, -0x1p-1022L, 7.853981633974484206379633083355174374608e-1L, -1.112536929253600444512293438042957369978e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x1.0000000000001p0L, 0x1p-1022L, -7.853981633974484206379633083355174374608e-1L, 1.112536929253600444512293438042957369978e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x1.0000000000001p0L, -0x1p-1022L, -7.853981633974484206379633083355174374608e-1L, -1.112536929253600444512293438042957369978e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1p-1022L, 0x1.0000000000001p0L, 1.570796326794896619231321691639751442099L, 1.836840028483855075506780244989950299369e1L),
    TEST_c_c (catan, -0x1p-1022L, 0x1.0000000000001p0L, -1.570796326794896619231321691639751442099L, 1.836840028483855075506780244989950299369e1L),
    TEST_c_c (catan, 0x1p-1022L, -0x1.0000000000001p0L, 1.570796326794896619231321691639751442099L, -1.836840028483855075506780244989950299369e1L),
    TEST_c_c (catan, -0x1p-1022L, -0x1.0000000000001p0L, -1.570796326794896619231321691639751442099L, -1.836840028483855075506780244989950299369e1L),
    TEST_c_c (catan, 0x0.fffffffffffff8p0L, 0x1p-1022L, 7.853981633974482541045096145620456183798e-1L, 1.112536929253600815061527818977844932790e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x0.fffffffffffff8p0L, -0x1p-1022L, 7.853981633974482541045096145620456183798e-1L, -1.112536929253600815061527818977844932790e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x0.fffffffffffff8p0L, 0x1p-1022L, -7.853981633974482541045096145620456183798e-1L, 1.112536929253600815061527818977844932790e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x0.fffffffffffff8p0L, -0x1p-1022L, -7.853981633974482541045096145620456183798e-1L, -1.112536929253600815061527818977844932790e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1p-1022L, 0x0.fffffffffffff8p0L, 1.002084180004486444624900488355118689113e-292L, 1.871497387511852332650969166374185305708e1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catan, -0x1p-1022L, 0x0.fffffffffffff8p0L, -1.002084180004486444624900488355118689113e-292L, 1.871497387511852332650969166374185305708e1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catan, 0x1p-1022L, -0x0.fffffffffffff8p0L, 1.002084180004486444624900488355118689113e-292L, -1.871497387511852332650969166374185305708e1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catan, -0x1p-1022L, -0x0.fffffffffffff8p0L, -1.002084180004486444624900488355118689113e-292L, -1.871497387511852332650969166374185305708e1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (catan, 0x1.0000000000000002p0L, 0x1p-1022L, 7.853981633974483096698709544441509427467e-1L, 1.112536929253600691424494863099491450042e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1.0000000000000002p0L, -0x1p-1022L, 7.853981633974483096698709544441509427467e-1L, -1.112536929253600691424494863099491450042e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x1.0000000000000002p0L, 0x1p-1022L, -7.853981633974483096698709544441509427467e-1L, 1.112536929253600691424494863099491450042e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x1.0000000000000002p0L, -0x1p-1022L, -7.853981633974483096698709544441509427467e-1L, -1.112536929253600691424494863099491450042e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1p-1022L, 0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, 2.218070977791824990137853294097378778927e1L),
    TEST_c_c (catan, -0x1p-1022L, 0x1.0000000000000002p0L, -1.570796326794896619231321691639751442099L, 2.218070977791824990137853294097378778927e1L),
    TEST_c_c (catan, 0x1p-1022L, -0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, -2.218070977791824990137853294097378778927e1L),
    TEST_c_c (catan, -0x1p-1022L, -0x1.0000000000000002p0L, -1.570796326794896619231321691639751442099L, -2.218070977791824990137853294097378778927e1L),
    TEST_c_c (catan, 0x0.ffffffffffffffffp0L, 0x1p-1022L, 7.853981633974483095885557915077381101984e-1L, 1.112536929253600691605427106449557323148e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x0.ffffffffffffffffp0L, -0x1p-1022L, 7.853981633974483095885557915077381101984e-1L, -1.112536929253600691605427106449557323148e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x0.ffffffffffffffffp0L, 0x1p-1022L, -7.853981633974483095885557915077381101984e-1L, 1.112536929253600691605427106449557323148e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x0.ffffffffffffffffp0L, -0x1p-1022L, -7.853981633974483095885557915077381101984e-1L, -1.112536929253600691605427106449557323148e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1p-1022L, 0x0.ffffffffffffffffp0L, 2.052268400649188124723641491045245971623e-289L, 2.252728336819822255604649142023466965703e1L),
    TEST_c_c (catan, -0x1p-1022L, 0x0.ffffffffffffffffp0L, -2.052268400649188124723641491045245971623e-289L, 2.252728336819822255604649142023466965703e1L),
    TEST_c_c (catan, 0x1p-1022L, -0x0.ffffffffffffffffp0L, 2.052268400649188124723641491045245971623e-289L, -2.252728336819822255604649142023466965703e1L),
    TEST_c_c (catan, -0x1p-1022L, -0x0.ffffffffffffffffp0L, -2.052268400649188124723641491045245971623e-289L, -2.252728336819822255604649142023466965703e1L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (catan, 0x1.000000000000000000000000008p0L, 0x1p-1022L, 7.853981633974483096156608458198880470009e-1L, 1.112536929253600691545116358666174605957e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1.000000000000000000000000008p0L, -0x1p-1022L, 7.853981633974483096156608458198880470009e-1L, -1.112536929253600691545116358666174605957e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x1.000000000000000000000000008p0L, 0x1p-1022L, -7.853981633974483096156608458198880470009e-1L, 1.112536929253600691545116358666174605957e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x1.000000000000000000000000008p0L, -0x1p-1022L, -7.853981633974483096156608458198880470009e-1L, -1.112536929253600691545116358666174605957e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1p-1022L, 0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639751442099L, 3.673680056967710139911330243728336427098e1L),
    TEST_c_c (catan, -0x1p-1022L, 0x1.000000000000000000000000008p0L, -1.570796326794896619231321691639751442099L, 3.673680056967710139911330243728336427098e1L),
    TEST_c_c (catan, 0x1p-1022L, -0x1.000000000000000000000000008p0L, 1.570796326794896619231321691639751442099L, -3.673680056967710139911330243728336427098e1L),
    TEST_c_c (catan, -0x1p-1022L, -0x1.000000000000000000000000008p0L, -1.570796326794896619231321691639751442099L, -3.673680056967710139911330243728336427098e1L),
    TEST_c_c (catan, 0x0.ffffffffffffffffffffffffffcp0L, 0x1p-1022L, 7.853981633974483096156608458198695580735e-1L, 1.112536929253600691545116358666215745186e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x0.ffffffffffffffffffffffffffcp0L, -0x1p-1022L, 7.853981633974483096156608458198695580735e-1L, -1.112536929253600691545116358666215745186e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x0.ffffffffffffffffffffffffffcp0L, 0x1p-1022L, -7.853981633974483096156608458198695580735e-1L, 1.112536929253600691545116358666215745186e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, -0x0.ffffffffffffffffffffffffffcp0L, -0x1p-1022L, -7.853981633974483096156608458198695580735e-1L, -1.112536929253600691545116358666215745186e-308L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catan, 0x1p-1022L, 0x0.ffffffffffffffffffffffffffcp0L, 9.025971879324147880346310405868788320726e-277L, 3.708337415995707405382191849801244331055e1L),
    TEST_c_c (catan, -0x1p-1022L, 0x0.ffffffffffffffffffffffffffcp0L, -9.025971879324147880346310405868788320726e-277L, 3.708337415995707405382191849801244331055e1L),
    TEST_c_c (catan, 0x1p-1022L, -0x0.ffffffffffffffffffffffffffcp0L, 9.025971879324147880346310405868788320726e-277L, -3.708337415995707405382191849801244331055e1L),
    TEST_c_c (catan, -0x1p-1022L, -0x0.ffffffffffffffffffffffffffcp0L, -9.025971879324147880346310405868788320726e-277L, -3.708337415995707405382191849801244331055e1L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (catan, 1.0L, 0x1p-8192L, 7.853981633974483096156608458198757210493e-1L, 4.584009668887117914053530980121207914891e-2467L),
    TEST_c_c (catan, 1.0L, -0x1p-8192L, 7.853981633974483096156608458198757210493e-1L, -4.584009668887117914053530980121207914891e-2467L),
    TEST_c_c (catan, -1.0L, 0x1p-8192L, -7.853981633974483096156608458198757210493e-1L, 4.584009668887117914053530980121207914891e-2467L),
    TEST_c_c (catan, -1.0L, -0x1p-8192L, -7.853981633974483096156608458198757210493e-1L, -4.584009668887117914053530980121207914891e-2467L),
    TEST_c_c (catan, 0x1p-8192L, 1.0L, 7.853981633974483096156608458198757210493e-1L, 2.839477425163815960027691385553420311121e3L),
    TEST_c_c (catan, -0x1p-8192L, 1.0L, -7.853981633974483096156608458198757210493e-1L, 2.839477425163815960027691385553420311121e3L),
    TEST_c_c (catan, 0x1p-8192L, -1.0L, 7.853981633974483096156608458198757210493e-1L, -2.839477425163815960027691385553420311121e3L),
    TEST_c_c (catan, -0x1p-8192L, -1.0L, -7.853981633974483096156608458198757210493e-1L, -2.839477425163815960027691385553420311121e3L),
    TEST_c_c (catan, 1.0L, 0x1.3p-8221L, 7.853981633974483096156608458198757210493e-1L, 1.013933025636421986471594877335044443409e-2475L),
    TEST_c_c (catan, 1.0L, -0x1.3p-8221L, 7.853981633974483096156608458198757210493e-1L, -1.013933025636421986471594877335044443409e-2475L),
    TEST_c_c (catan, -1.0L, 0x1.3p-8221L, -7.853981633974483096156608458198757210493e-1L, 1.013933025636421986471594877335044443409e-2475L),
    TEST_c_c (catan, -1.0L, -0x1.3p-8221L, -7.853981633974483096156608458198757210493e-1L, -1.013933025636421986471594877335044443409e-2475L),
    TEST_c_c (catan, 0x1.3p-8221L, 1.0L, 7.853981633974483096156608458198757210493e-1L, 2.849442134153471837403071201841536297726e3L),
    TEST_c_c (catan, -0x1.3p-8221L, 1.0L, -7.853981633974483096156608458198757210493e-1L, 2.849442134153471837403071201841536297726e3L),
    TEST_c_c (catan, 0x1.3p-8221L, -1.0L, 7.853981633974483096156608458198757210493e-1L, -2.849442134153471837403071201841536297726e3L),
    TEST_c_c (catan, -0x1.3p-8221L, -1.0L, -7.853981633974483096156608458198757210493e-1L, -2.849442134153471837403071201841536297726e3L),
    TEST_c_c (catan, 1.0L, 0x1.3p-8246L, 7.853981633974483096156608458198757210493e-1L, 3.021755890954798419688924781486524472858e-2483L),
    TEST_c_c (catan, 1.0L, -0x1.3p-8246L, 7.853981633974483096156608458198757210493e-1L, -3.021755890954798419688924781486524472858e-2483L),
    TEST_c_c (catan, -1.0L, 0x1.3p-8246L, -7.853981633974483096156608458198757210493e-1L, 3.021755890954798419688924781486524472858e-2483L),
    TEST_c_c (catan, -1.0L, -0x1.3p-8246L, -7.853981633974483096156608458198757210493e-1L, -3.021755890954798419688924781486524472858e-2483L),
    TEST_c_c (catan, 0x1.3p-8246L, 1.0L, 7.853981633974483096156608458198757210493e-1L, 2.858106473910471153770786603359763504827e3L),
    TEST_c_c (catan, -0x1.3p-8246L, 1.0L, -7.853981633974483096156608458198757210493e-1L, 2.858106473910471153770786603359763504827e3L),
    TEST_c_c (catan, 0x1.3p-8246L, -1.0L, 7.853981633974483096156608458198757210493e-1L, -2.858106473910471153770786603359763504827e3L),
    TEST_c_c (catan, -0x1.3p-8246L, -1.0L, -7.853981633974483096156608458198757210493e-1L, -2.858106473910471153770786603359763504827e3L),
    TEST_c_c (catan, 1.0L, 0x1p-16380L, 7.853981633974483096156608458198757210493e-1L, 6.724206286224187012525355634643505205196e-4932L),
    TEST_c_c (catan, 1.0L, -0x1p-16380L, 7.853981633974483096156608458198757210493e-1L, -6.724206286224187012525355634643505205196e-4932L),
    TEST_c_c (catan, -1.0L, 0x1p-16380L, -7.853981633974483096156608458198757210493e-1L, 6.724206286224187012525355634643505205196e-4932L),
    TEST_c_c (catan, -1.0L, -0x1p-16380L, -7.853981633974483096156608458198757210493e-1L, -6.724206286224187012525355634643505205196e-4932L),
    TEST_c_c (catan, 0x1p-16380L, 1.0L, 7.853981633974483096156608458198757210493e-1L, 5.677221982376232056781839690803195180822e3L),
    TEST_c_c (catan, -0x1p-16380L, 1.0L, -7.853981633974483096156608458198757210493e-1L, 5.677221982376232056781839690803195180822e3L),
    TEST_c_c (catan, 0x1p-16380L, -1.0L, 7.853981633974483096156608458198757210493e-1L, -5.677221982376232056781839690803195180822e3L),
    TEST_c_c (catan, -0x1p-16380L, -1.0L, -7.853981633974483096156608458198757210493e-1L, -5.677221982376232056781839690803195180822e3L),
    TEST_c_c (catan, 0x1.0000000000000002p0L, 0x1p-16382L, 7.853981633974483096698709544441509427467e-1L, 1.681051571556046752949078932066752571182e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0x1.0000000000000002p0L, -0x1p-16382L, 7.853981633974483096698709544441509427467e-1L, -1.681051571556046752949078932066752571182e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x1.0000000000000002p0L, 0x1p-16382L, -7.853981633974483096698709544441509427467e-1L, 1.681051571556046752949078932066752571182e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x1.0000000000000002p0L, -0x1p-16382L, -7.853981633974483096698709544441509427467e-1L, -1.681051571556046752949078932066752571182e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0x1p-16382L, 0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, 2.218070977791824990137853294097378778927e1L),
    TEST_c_c (catan, -0x1p-16382L, 0x1.0000000000000002p0L, -1.570796326794896619231321691639751442099L, 2.218070977791824990137853294097378778927e1L),
    TEST_c_c (catan, 0x1p-16382L, -0x1.0000000000000002p0L, 1.570796326794896619231321691639751442099L, -2.218070977791824990137853294097378778927e1L),
    TEST_c_c (catan, -0x1p-16382L, -0x1.0000000000000002p0L, -1.570796326794896619231321691639751442099L, -2.218070977791824990137853294097378778927e1L),
    TEST_c_c (catan, 0x0.ffffffffffffffffp0L, 0x1p-16382L, 7.853981633974483095885557915077381101984e-1L, 1.681051571556046753222468896957938166365e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0x0.ffffffffffffffffp0L, -0x1p-16382L, 7.853981633974483095885557915077381101984e-1L, -1.681051571556046753222468896957938166365e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x0.ffffffffffffffffp0L, 0x1p-16382L, -7.853981633974483095885557915077381101984e-1L, 1.681051571556046753222468896957938166365e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x0.ffffffffffffffffp0L, -0x1p-16382L, -7.853981633974483095885557915077381101984e-1L, -1.681051571556046753222468896957938166365e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0x1p-16382L, 0x0.ffffffffffffffffp0L, 3.100992811520163369065387859792822623745e-4913L, 2.252728336819822255604649142023466965703e1L),
    TEST_c_c (catan, -0x1p-16382L, 0x0.ffffffffffffffffp0L, -3.100992811520163369065387859792822623745e-4913L, 2.252728336819822255604649142023466965703e1L),
    TEST_c_c (catan, 0x1p-16382L, -0x0.ffffffffffffffffp0L, 3.100992811520163369065387859792822623745e-4913L, -2.252728336819822255604649142023466965703e1L),
    TEST_c_c (catan, -0x1p-16382L, -0x0.ffffffffffffffffp0L, -3.100992811520163369065387859792822623745e-4913L, -2.252728336819822255604649142023466965703e1L),
# if LDBL_MANT_DIG >= 113
    TEST_c_c (catan, 0x1.0000000000000000000000000001p0L, 0x1p-16382L, 7.853981633974483096156608458198758173458e-1L, 1.681051571556046753131338908660875977540e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0x1.0000000000000000000000000001p0L, -0x1p-16382L, 7.853981633974483096156608458198758173458e-1L, -1.681051571556046753131338908660875977540e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x1.0000000000000000000000000001p0L, 0x1p-16382L, -7.853981633974483096156608458198758173458e-1L, 1.681051571556046753131338908660875977540e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x1.0000000000000000000000000001p0L, -0x1p-16382L, -7.853981633974483096156608458198758173458e-1L, -1.681051571556046753131338908660875977540e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0x1p-16382L, 0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751442099L, 3.916281570163690998207361486238697614441e1L),
    TEST_c_c (catan, -0x1p-16382L, 0x1.0000000000000000000000000001p0L, -1.570796326794896619231321691639751442099L, 3.916281570163690998207361486238697614441e1L),
    TEST_c_c (catan, 0x1p-16382L, -0x1.0000000000000000000000000001p0L, 1.570796326794896619231321691639751442099L, -3.916281570163690998207361486238697614441e1L),
    TEST_c_c (catan, -0x1p-16382L, -0x1.0000000000000000000000000001p0L, -1.570796326794896619231321691639751442099L, -3.916281570163690998207361486238697614441e1L),
    TEST_c_c (catan, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-16382L, 7.853981633974483096156608458198756729010e-1L, 1.681051571556046753131338908660876463178e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-16382L, 7.853981633974483096156608458198756729010e-1L, -1.681051571556046753131338908660876463178e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-16382L, -7.853981633974483096156608458198756729010e-1L, 1.681051571556046753131338908660876463178e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-16382L, -7.853981633974483096156608458198756729010e-1L, -1.681051571556046753131338908660876463178e-4932L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catan, 0x1p-16382L, 0x0.ffffffffffffffffffffffffffff8p0L, 1.745703758805099310527547423749501866998e-4898L, 3.950938929191688263678223092311606435623e1L),
    TEST_c_c (catan, -0x1p-16382L, 0x0.ffffffffffffffffffffffffffff8p0L, -1.745703758805099310527547423749501866998e-4898L, 3.950938929191688263678223092311606435623e1L),
    TEST_c_c (catan, 0x1p-16382L, -0x0.ffffffffffffffffffffffffffff8p0L, 1.745703758805099310527547423749501866998e-4898L, -3.950938929191688263678223092311606435623e1L),
    TEST_c_c (catan, -0x1p-16382L, -0x0.ffffffffffffffffffffffffffff8p0L, -1.745703758805099310527547423749501866998e-4898L, -3.950938929191688263678223092311606435623e1L),
# endif
#endif

    TEST_c_c (catan, 0.75L, 1.25L, 1.10714871779409050301706546017853704L, 0.549306144334054845697622618461262852L),
    TEST_c_c (catan, -2, -3, -1.4099210495965755225306193844604208L, -0.22907268296853876629588180294200276L),
    END_DATA (catan)
  };

static void
catan_test (void)
{
  errno = 0;
  FUNC(catan) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (catan);
  RUN_TEST_LOOP_c_c (catan, catan_test_data, );
  END (catan, complex);
}

static const struct test_c_c_data catanh_test_data[] =
  {
    START_DATA (catanh),
    TEST_c_c (catanh, 0, 0, 0.0, 0.0),
    TEST_c_c (catanh, minus_zero, 0, minus_zero, 0.0),
    TEST_c_c (catanh, 0, minus_zero, 0.0, minus_zero),
    TEST_c_c (catanh, minus_zero, minus_zero, minus_zero, minus_zero),

    TEST_c_c (catanh, 1.0L, plus_zero, plus_infty, plus_zero, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_c_c (catanh, 1.0L, minus_zero, plus_infty, minus_zero, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_c_c (catanh, -1.0L, plus_zero, minus_infty, plus_zero, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_c_c (catanh, -1.0L, minus_zero, minus_infty, minus_zero, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_c_c (catanh, plus_zero, 1.0L, plus_zero, M_PI_4l),
    TEST_c_c (catanh, minus_zero, 1.0L, minus_zero, M_PI_4l),
    TEST_c_c (catanh, plus_zero, -1.0L, plus_zero, -M_PI_4l),
    TEST_c_c (catanh, minus_zero, -1.0L, minus_zero, -M_PI_4l),

    TEST_c_c (catanh, plus_infty, plus_infty, 0.0, M_PI_2l),
    TEST_c_c (catanh, plus_infty, minus_infty, 0.0, -M_PI_2l),
    TEST_c_c (catanh, minus_infty, plus_infty, minus_zero, M_PI_2l),
    TEST_c_c (catanh, minus_infty, minus_infty, minus_zero, -M_PI_2l),

    TEST_c_c (catanh, -10.0, plus_infty, minus_zero, M_PI_2l),
    TEST_c_c (catanh, -10.0, minus_infty, minus_zero, -M_PI_2l),
    TEST_c_c (catanh, minus_zero, plus_infty, minus_zero, M_PI_2l),
    TEST_c_c (catanh, minus_zero, minus_infty, minus_zero, -M_PI_2l),
    TEST_c_c (catanh, 0, plus_infty, 0.0, M_PI_2l),
    TEST_c_c (catanh, 0, minus_infty, 0.0, -M_PI_2l),
    TEST_c_c (catanh, 0.1L, plus_infty, 0.0, M_PI_2l),
    TEST_c_c (catanh, 0.1L, minus_infty, 0.0, -M_PI_2l),

    TEST_c_c (catanh, minus_infty, 0, minus_zero, M_PI_2l),
    TEST_c_c (catanh, minus_infty, minus_zero, minus_zero, -M_PI_2l),
    TEST_c_c (catanh, minus_infty, 100, minus_zero, M_PI_2l),
    TEST_c_c (catanh, minus_infty, -100, minus_zero, -M_PI_2l),

    TEST_c_c (catanh, plus_infty, 0, 0.0, M_PI_2l),
    TEST_c_c (catanh, plus_infty, minus_zero, 0.0, -M_PI_2l),
    TEST_c_c (catanh, plus_infty, 0.5, 0.0, M_PI_2l),
    TEST_c_c (catanh, plus_infty, -0.5, 0.0, -M_PI_2l),

    TEST_c_c (catanh, 0, qnan_value, 0.0, qnan_value),
    TEST_c_c (catanh, minus_zero, qnan_value, minus_zero, qnan_value),

    TEST_c_c (catanh, plus_infty, qnan_value, 0.0, qnan_value),
    TEST_c_c (catanh, minus_infty, qnan_value, minus_zero, qnan_value),

    TEST_c_c (catanh, qnan_value, 0, qnan_value, qnan_value),
    TEST_c_c (catanh, qnan_value, minus_zero, qnan_value, qnan_value),

    TEST_c_c (catanh, qnan_value, plus_infty, 0.0, M_PI_2l, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (catanh, qnan_value, minus_infty, 0.0, -M_PI_2l, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (catanh, 10.5, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (catanh, -10.5, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (catanh, qnan_value, 0.75, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (catanh, qnan_value, -0.75, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (catanh, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (catanh, 0x1p50L, 0.0L, 8.881784197001252323389053344728897997441e-16L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p50L, -0.0L, 8.881784197001252323389053344728897997441e-16L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p50L, 0.0L, -8.881784197001252323389053344728897997441e-16L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p50L, -0.0L, -8.881784197001252323389053344728897997441e-16L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, 0x1p50L, 0.0L, 1.570796326794895731052901991514519103193L),
    TEST_c_c (catanh, -0.0L, 0x1p50L, -0.0L, 1.570796326794895731052901991514519103193L),
    TEST_c_c (catanh, 0.0L, -0x1p50L, 0.0L, -1.570796326794895731052901991514519103193L),
    TEST_c_c (catanh, -0.0L, -0x1p50L, -0.0L, -1.570796326794895731052901991514519103193L),
#ifndef TEST_FLOAT
    TEST_c_c (catanh, 0x1p500L, 0.0L, 3.054936363499604682051979393213617699789e-151L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p500L, -0.0L, 3.054936363499604682051979393213617699789e-151L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p500L, 0.0L, -3.054936363499604682051979393213617699789e-151L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p500L, -0.0L, -3.054936363499604682051979393213617699789e-151L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, 0x1p500L, 0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, 0x1p500L, -0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, -0x1p500L, 0.0L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, -0x1p500L, -0.0L, -1.570796326794896619231321691639751442099L),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (catanh, 0x1p5000L, 0.0L, 7.079811261048172892385615158694057552948e-1506L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p5000L, -0.0L, 7.079811261048172892385615158694057552948e-1506L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p5000L, 0.0L, -7.079811261048172892385615158694057552948e-1506L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p5000L, -0.0L, -7.079811261048172892385615158694057552948e-1506L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, 0x1p5000L, 0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, 0x1p5000L, -0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, -0x1p5000L, 0.0L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, -0x1p5000L, -0.0L, -1.570796326794896619231321691639751442099L),
#endif
    TEST_c_c (catanh, 0x1p63L, 0.5L, 1.084202172485504434007452800869941711427e-19L, 1.570796326794896619231321691639751442093L),
    TEST_c_c (catanh, 0x1p63L, -0.5L, 1.084202172485504434007452800869941711427e-19L, -1.570796326794896619231321691639751442093L),
    TEST_c_c (catanh, -0x1p63L, 0.5L, -1.084202172485504434007452800869941711427e-19L, 1.570796326794896619231321691639751442093L),
    TEST_c_c (catanh, -0x1p63L, -0.5L, -1.084202172485504434007452800869941711427e-19L, -1.570796326794896619231321691639751442093L),
    TEST_c_c (catanh, 0.5L, 0x1p63L, 5.877471754111437539843682686111228389007e-39L, 1.570796326794896619122901474391200998698L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, -0.5L, 0x1p63L, -5.877471754111437539843682686111228389007e-39L, 1.570796326794896619122901474391200998698L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, 0.5L, -0x1p63L, 5.877471754111437539843682686111228389007e-39L, -1.570796326794896619122901474391200998698L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, -0.5L, -0x1p63L, -5.877471754111437539843682686111228389007e-39L, -1.570796326794896619122901474391200998698L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (catanh, 0x1p511L, 0.5L, 1.491668146240041348658193063092586767475e-154L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p511L, -0.5L, 1.491668146240041348658193063092586767475e-154L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p511L, 0.5L, -1.491668146240041348658193063092586767475e-154L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p511L, -0.5L, -1.491668146240041348658193063092586767475e-154L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.5L, 0x1p511L, 1.112536929253600691545116358666202032110e-308L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0.5L, 0x1p511L, -1.112536929253600691545116358666202032110e-308L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0.5L, -0x1p511L, 1.112536929253600691545116358666202032110e-308L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0.5L, -0x1p511L, -1.112536929253600691545116358666202032110e-308L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (catanh, 0x1p8191L, 0.5L, 1.833603867554847165621412392048483165956e-2466L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p8191L, -0.5L, 1.833603867554847165621412392048483165956e-2466L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p8191L, 0.5L, -1.833603867554847165621412392048483165956e-2466L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p8191L, -0.5L, -1.833603867554847165621412392048483165956e-2466L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.5L, 0x1p8191L, 1.681051571556046753131338908660876301299e-4932L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0.5L, 0x1p8191L, -1.681051571556046753131338908660876301299e-4932L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, 0.5L, -0x1p8191L, 1.681051571556046753131338908660876301299e-4932L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0.5L, -0x1p8191L, -1.681051571556046753131338908660876301299e-4932L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (catanh, 0x1p100L, 0.0L, 7.888609052210118054117285652827862296732e-31L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p100L, -0.0L, 7.888609052210118054117285652827862296732e-31L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p100L, 0.0L, -7.888609052210118054117285652827862296732e-31L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p100L, -0.0L, -7.888609052210118054117285652827862296732e-31L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, 0x1p100L, 0.0L, 1.570796326794896619231321691638962581193L),
    TEST_c_c (catanh, -0.0L, 0x1p100L, -0.0L, 1.570796326794896619231321691638962581193L),
    TEST_c_c (catanh, 0.0L, -0x1p100L, 0.0L, -1.570796326794896619231321691638962581193L),
    TEST_c_c (catanh, -0.0L, -0x1p100L, -0.0L, -1.570796326794896619231321691638962581193L),
    TEST_c_c (catanh, 0x1.fp127L, 0.0L, 3.033533808573645181854803967025150136306e-39L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, 0x1.fp127L, -0.0L, 3.033533808573645181854803967025150136306e-39L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, -0x1.fp127L, 0.0L, -3.033533808573645181854803967025150136306e-39L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, -0x1.fp127L, -0.0L, -3.033533808573645181854803967025150136306e-39L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, 0.0L, 0x1.fp127L, 0.0L, 1.570796326794896619231321691639751442096L),
    TEST_c_c (catanh, -0.0L, 0x1.fp127L, -0.0L, 1.570796326794896619231321691639751442096L),
    TEST_c_c (catanh, 0.0L, -0x1.fp127L, 0.0L, -1.570796326794896619231321691639751442096L),
    TEST_c_c (catanh, -0.0L, -0x1.fp127L, -0.0L, -1.570796326794896619231321691639751442096L),
    TEST_c_c (catanh, 0x1.fp127L, 0x1.fp127L, 1.516766904286822590927401983512575068153e-39L, 1.570796326794896619231321691639751442097L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, 0x1.fp127L, -0x1.fp127L, 1.516766904286822590927401983512575068153e-39L, -1.570796326794896619231321691639751442097L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, -0x1.fp127L, 0x1.fp127L, -1.516766904286822590927401983512575068153e-39L, 1.570796326794896619231321691639751442097L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, -0x1.fp127L, -0x1.fp127L, -1.516766904286822590927401983512575068153e-39L, -1.570796326794896619231321691639751442097L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (catanh, 0x1p900L, 0.0L, 1.183052186166774710972751597518026531652e-271L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p900L, -0.0L, 1.183052186166774710972751597518026531652e-271L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p900L, 0.0L, -1.183052186166774710972751597518026531652e-271L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p900L, -0.0L, -1.183052186166774710972751597518026531652e-271L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, 0x1p900L, 0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, 0x1p900L, -0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, -0x1p900L, 0.0L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, -0x1p900L, -0.0L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1.fp1023L, 0.0L, 5.742126086470197117652213464083623391533e-309L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0x1.fp1023L, -0.0L, 5.742126086470197117652213464083623391533e-309L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1.fp1023L, 0.0L, -5.742126086470197117652213464083623391533e-309L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1.fp1023L, -0.0L, -5.742126086470197117652213464083623391533e-309L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0.0L, 0x1.fp1023L, 0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, 0x1.fp1023L, -0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, -0x1.fp1023L, 0.0L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, -0x1.fp1023L, -0.0L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1.fp1023L, 0x1.fp1023L, 2.871063043235098558826106732041811695767e-309L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0x1.fp1023L, -0x1.fp1023L, 2.871063043235098558826106732041811695767e-309L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1.fp1023L, 0x1.fp1023L, -2.871063043235098558826106732041811695767e-309L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1.fp1023L, -0x1.fp1023L, -2.871063043235098558826106732041811695767e-309L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (catanh, 0x1p10000L, 0.0L, 5.012372749206452009297555933742977749322e-3011L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p10000L, -0.0L, 5.012372749206452009297555933742977749322e-3011L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p10000L, 0.0L, -5.012372749206452009297555933742977749322e-3011L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1p10000L, -0.0L, -5.012372749206452009297555933742977749322e-3011L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, 0x1p10000L, 0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, 0x1p10000L, -0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, -0x1p10000L, 0.0L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, -0x1p10000L, -0.0L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1.fp16383L, 0.0L, 8.676395208031209048419813722120651877672e-4933L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, 0x1.fp16383L, -0.0L, 8.676395208031209048419813722120651877672e-4933L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1.fp16383L, 0.0L, -8.676395208031209048419813722120651877672e-4933L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1.fp16383L, -0.0L, -8.676395208031209048419813722120651877672e-4933L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, 0.0L, 0x1.fp16383L, 0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, 0x1.fp16383L, -0.0L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0.0L, -0x1.fp16383L, 0.0L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0.0L, -0x1.fp16383L, -0.0L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1.fp16383L, 0x1.fp16383L, 4.338197604015604524209906861060325938836e-4933L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, 0x1.fp16383L, -0x1.fp16383L, 4.338197604015604524209906861060325938836e-4933L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1.fp16383L, 0x1.fp16383L, -4.338197604015604524209906861060325938836e-4933L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1.fp16383L, -0x1.fp16383L, -4.338197604015604524209906861060325938836e-4933L, -1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (catanh, 0x1p-13L, 1.0L, 6.103515609841754902688560615027452023669e-5L, 7.853981671227386080775748393881580082970e-1L),
    TEST_c_c (catanh, 0x1p-13L, -1.0L, 6.103515609841754902688560615027452023669e-5L, -7.853981671227386080775748393881580082970e-1L),
    TEST_c_c (catanh, -0x1p-13L, 1.0L, -6.103515609841754902688560615027452023669e-5L, 7.853981671227386080775748393881580082970e-1L),
    TEST_c_c (catanh, -0x1p-13L, -1.0L, -6.103515609841754902688560615027452023669e-5L, -7.853981671227386080775748393881580082970e-1L),
    TEST_c_c (catanh, 1.0L, 0x1p-13L, 4.852030264850939738801379894163661227127L, 7.854286809755354140031716771044626356262e-1L),
    TEST_c_c (catanh, -1.0L, 0x1p-13L, -4.852030264850939738801379894163661227127L, 7.854286809755354140031716771044626356262e-1L),
    TEST_c_c (catanh, 1.0L, -0x1p-13L, 4.852030264850939738801379894163661227127L, -7.854286809755354140031716771044626356262e-1L),
    TEST_c_c (catanh, -1.0L, -0x1p-13L, -4.852030264850939738801379894163661227127L, -7.854286809755354140031716771044626356262e-1L),
    TEST_c_c (catanh, 0x1p-27L, 1.0L, 3.725290298461914028034141143623846306386e-9L, 7.853981633974483234934486536343324763447e-1L),
    TEST_c_c (catanh, 0x1p-27L, -1.0L, 3.725290298461914028034141143623846306386e-9L, -7.853981633974483234934486536343324763447e-1L),
    TEST_c_c (catanh, -0x1p-27L, 1.0L, -3.725290298461914028034141143623846306386e-9L, 7.853981633974483234934486536343324763447e-1L),
    TEST_c_c (catanh, -0x1p-27L, -1.0L, -3.725290298461914028034141143623846306386e-9L, -7.853981633974483234934486536343324763447e-1L),
    TEST_c_c (catanh, 1.0L, 0x1p-27L, 9.704060527839234335310696652368086117807L, 7.853981652600934588466178684534110069553e-1L),
    TEST_c_c (catanh, -1.0L, 0x1p-27L, -9.704060527839234335310696652368086117807L, 7.853981652600934588466178684534110069553e-1L),
    TEST_c_c (catanh, 1.0L, -0x1p-27L, 9.704060527839234335310696652368086117807L, -7.853981652600934588466178684534110069553e-1L),
    TEST_c_c (catanh, -1.0L, -0x1p-27L, -9.704060527839234335310696652368086117807L, -7.853981652600934588466178684534110069553e-1L),
    TEST_c_c (catanh, 0x1p-33L, 1.0L, 5.820766091346740722643102318246316469910e-11L, 7.853981633974483096190489776088929224056e-1L),
    TEST_c_c (catanh, 0x1p-33L, -1.0L, 5.820766091346740722643102318246316469910e-11L, -7.853981633974483096190489776088929224056e-1L),
    TEST_c_c (catanh, -0x1p-33L, 1.0L, -5.820766091346740722643102318246316469910e-11L, 7.853981633974483096190489776088929224056e-1L),
    TEST_c_c (catanh, -0x1p-33L, -1.0L, -5.820766091346740722643102318246316469910e-11L, -7.853981633974483096190489776088929224056e-1L),
    TEST_c_c (catanh, 1.0L, 0x1p-33L, 1.178350206951907026009379309773625595762e1L, 7.853981634265521400723945494331241018449e-1L),
    TEST_c_c (catanh, -1.0L, 0x1p-33L, -1.178350206951907026009379309773625595762e1L, 7.853981634265521400723945494331241018449e-1L),
    TEST_c_c (catanh, 1.0L, -0x1p-33L, 1.178350206951907026009379309773625595762e1L, -7.853981634265521400723945494331241018449e-1L),
    TEST_c_c (catanh, -1.0L, -0x1p-33L, -1.178350206951907026009379309773625595762e1L, -7.853981634265521400723945494331241018449e-1L),
    TEST_c_c (catanh, 0x1p-54L, 1.0L, 2.775557561562891351059079170227049355775e-17L, 7.853981633974483096156608458198764914213e-1L),
    TEST_c_c (catanh, 0x1p-54L, -1.0L, 2.775557561562891351059079170227049355775e-17L, -7.853981633974483096156608458198764914213e-1L),
    TEST_c_c (catanh, -0x1p-54L, 1.0L, -2.775557561562891351059079170227049355775e-17L, 7.853981633974483096156608458198764914213e-1L),
    TEST_c_c (catanh, -0x1p-54L, -1.0L, -2.775557561562891351059079170227049355775e-17L, -7.853981633974483096156608458198764914213e-1L),
    TEST_c_c (catanh, 1.0L, 0x1p-54L, 1.906154746539849600897388334009985581467e1L, 7.853981633974483234934486536343324763447e-1L),
    TEST_c_c (catanh, -1.0L, 0x1p-54L, -1.906154746539849600897388334009985581467e1L, 7.853981633974483234934486536343324763447e-1L),
    TEST_c_c (catanh, 1.0L, -0x1p-54L, 1.906154746539849600897388334009985581467e1L, -7.853981633974483234934486536343324763447e-1L),
    TEST_c_c (catanh, -1.0L, -0x1p-54L, -1.906154746539849600897388334009985581467e1L, -7.853981633974483234934486536343324763447e-1L),
    TEST_c_c (catanh, 0x1p-57L, 1.0L, 3.469446951953614188823848962783813448721e-18L, 7.853981633974483096156608458198757330864e-1L),
    TEST_c_c (catanh, 0x1p-57L, -1.0L, 3.469446951953614188823848962783813448721e-18L, -7.853981633974483096156608458198757330864e-1L),
    TEST_c_c (catanh, -0x1p-57L, 1.0L, -3.469446951953614188823848962783813448721e-18L, 7.853981633974483096156608458198757330864e-1L),
    TEST_c_c (catanh, -0x1p-57L, -1.0L, -3.469446951953614188823848962783813448721e-18L, -7.853981633974483096156608458198757330864e-1L),
    TEST_c_c (catanh, 1.0L, 0x1p-57L, 2.010126823623841397309973152228712047720e1L, 7.853981633974483113503843217966828154612e-1L),
    TEST_c_c (catanh, -1.0L, 0x1p-57L, -2.010126823623841397309973152228712047720e1L, 7.853981633974483113503843217966828154612e-1L),
    TEST_c_c (catanh, 1.0L, -0x1p-57L, 2.010126823623841397309973152228712047720e1L, -7.853981633974483113503843217966828154612e-1L),
    TEST_c_c (catanh, -1.0L, -0x1p-57L, -2.010126823623841397309973152228712047720e1L, -7.853981633974483113503843217966828154612e-1L),
    TEST_c_c (catanh, 0x1p-13L, 0x1.000002p0L, 6.103514882246036852433556327261700380577e-5L, 7.853982267273793866654490522673596014524e-1L),
    TEST_c_c (catanh, 0x1p-13L, -0x1.000002p0L, 6.103514882246036852433556327261700380577e-5L, -7.853982267273793866654490522673596014524e-1L),
    TEST_c_c (catanh, -0x1p-13L, 0x1.000002p0L, -6.103514882246036852433556327261700380577e-5L, 7.853982267273793866654490522673596014524e-1L),
    TEST_c_c (catanh, -0x1p-13L, -0x1.000002p0L, -6.103514882246036852433556327261700380577e-5L, -7.853982267273793866654490522673596014524e-1L),
    TEST_c_c (catanh, 0x1.000002p0L, 0x1p-13L, 4.852030056234795712498957387213592193975L, 7.859169620684960844300240092596908675974e-1L),
    TEST_c_c (catanh, -0x1.000002p0L, 0x1p-13L, -4.852030056234795712498957387213592193975L, 7.859169620684960844300240092596908675974e-1L),
    TEST_c_c (catanh, 0x1.000002p0L, -0x1p-13L, 4.852030056234795712498957387213592193975L, -7.859169620684960844300240092596908675974e-1L),
    TEST_c_c (catanh, -0x1.000002p0L, -0x1p-13L, -4.852030056234795712498957387213592193975L, -7.859169620684960844300240092596908675974e-1L),
    TEST_c_c (catanh, 0x1p-13L, 0x0.ffffffp0L, 6.103515973639646453881721999956617260502e-5L, 7.853981373204155542484315721351697277336e-1L),
    TEST_c_c (catanh, 0x1p-13L, -0x0.ffffffp0L, 6.103515973639646453881721999956617260502e-5L, -7.853981373204155542484315721351697277336e-1L),
    TEST_c_c (catanh, -0x1p-13L, 0x0.ffffffp0L, -6.103515973639646453881721999956617260502e-5L, 7.853981373204155542484315721351697277336e-1L),
    TEST_c_c (catanh, -0x1p-13L, -0x0.ffffffp0L, -6.103515973639646453881721999956617260502e-5L, -7.853981373204155542484315721351697277336e-1L),
    TEST_c_c (catanh, 0x0.ffffffp0L, 0x1p-13L, 4.852030190345140708455871037447717761868L, 7.851845403708474595909269086711426246675e-1L),
    TEST_c_c (catanh, -0x0.ffffffp0L, 0x1p-13L, -4.852030190345140708455871037447717761868L, 7.851845403708474595909269086711426246675e-1L),
    TEST_c_c (catanh, 0x0.ffffffp0L, -0x1p-13L, 4.852030190345140708455871037447717761868L, -7.851845403708474595909269086711426246675e-1L),
    TEST_c_c (catanh, -0x0.ffffffp0L, -0x1p-13L, -4.852030190345140708455871037447717761868L, -7.851845403708474595909269086711426246675e-1L),
#ifndef TEST_FLOAT
    TEST_c_c (catanh, 0x1p-27L, 0x1.0000000000001p0L, 3.725290298461913200853528590596263270474e-9L, 7.853981633974484345157511161499711112683e-1L),
    TEST_c_c (catanh, 0x1p-27L, -0x1.0000000000001p0L, 3.725290298461913200853528590596263270474e-9L, -7.853981633974484345157511161499711112683e-1L),
    TEST_c_c (catanh, -0x1p-27L, 0x1.0000000000001p0L, -3.725290298461913200853528590596263270474e-9L, 7.853981633974484345157511161499711112683e-1L),
    TEST_c_c (catanh, -0x1p-27L, -0x1.0000000000001p0L, -3.725290298461913200853528590596263270474e-9L, -7.853981633974484345157511161499711112683e-1L),
    TEST_c_c (catanh, 0x1.0000000000001p0L, 0x1p-27L, 9.704060527839234168777242958594699810015L, 7.853981801612546526942695000283242525531e-1L),
    TEST_c_c (catanh, -0x1.0000000000001p0L, 0x1p-27L, -9.704060527839234168777242958594699810015L, 7.853981801612546526942695000283242525531e-1L),
    TEST_c_c (catanh, 0x1.0000000000001p0L, -0x1p-27L, 9.704060527839234168777242958594699810015L, -7.853981801612546526942695000283242525531e-1L),
    TEST_c_c (catanh, -0x1.0000000000001p0L, -0x1p-27L, -9.704060527839234168777242958594699810015L, -7.853981801612546526942695000283242525531e-1L),
    TEST_c_c (catanh, 0x1p-27L, 0x0.fffffffffffff8p0L, 3.725290298461914441624447420137706700965e-9L, 7.853981633974482679822974223765039144191e-1L),
    TEST_c_c (catanh, 0x1p-27L, -0x0.fffffffffffff8p0L, 3.725290298461914441624447420137706700965e-9L, -7.853981633974482679822974223765039144191e-1L),
    TEST_c_c (catanh, -0x1p-27L, 0x0.fffffffffffff8p0L, -3.725290298461914441624447420137706700965e-9L, 7.853981633974482679822974223765039144191e-1L),
    TEST_c_c (catanh, -0x1p-27L, -0x0.fffffffffffff8p0L, -3.725290298461914441624447420137706700965e-9L, -7.853981633974482679822974223765039144191e-1L),
    TEST_c_c (catanh, 0x0.fffffffffffff8p0L, 0x1p-27L, 9.704060527839234252043969805481351363824L, 7.853981578095128619227903983047292781021e-1L),
    TEST_c_c (catanh, -0x0.fffffffffffff8p0L, 0x1p-27L, -9.704060527839234252043969805481351363824L, 7.853981578095128619227903983047292781021e-1L),
    TEST_c_c (catanh, 0x0.fffffffffffff8p0L, -0x1p-27L, 9.704060527839234252043969805481351363824L, -7.853981578095128619227903983047292781021e-1L),
    TEST_c_c (catanh, -0x0.fffffffffffff8p0L, -0x1p-27L, -9.704060527839234252043969805481351363824L, -7.853981578095128619227903983047292781021e-1L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (catanh, 0x1p-33L, 0x1.0000000000000002p0L, 5.820766091346740722012013594069507025615e-11L, 7.853981633974483096732590862331681441026e-1L),
    TEST_c_c (catanh, 0x1p-33L, -0x1.0000000000000002p0L, 5.820766091346740722012013594069507025615e-11L, -7.853981633974483096732590862331681441026e-1L),
    TEST_c_c (catanh, -0x1p-33L, 0x1.0000000000000002p0L, -5.820766091346740722012013594069507025615e-11L, 7.853981633974483096732590862331681441026e-1L),
    TEST_c_c (catanh, -0x1p-33L, -0x1.0000000000000002p0L, -5.820766091346740722012013594069507025615e-11L, -7.853981633974483096732590862331681441026e-1L),
    TEST_c_c (catanh, 0x1.0000000000000002p0L, 0x1p-33L, 1.178350206951907025990405771755129268176e1L, 7.853981638922134273801338071094141188767e-1L),
    TEST_c_c (catanh, -0x1.0000000000000002p0L, 0x1p-33L, -1.178350206951907025990405771755129268176e1L, 7.853981638922134273801338071094141188767e-1L),
    TEST_c_c (catanh, 0x1.0000000000000002p0L, -0x1p-33L, 1.178350206951907025990405771755129268176e1L, -7.853981638922134273801338071094141188767e-1L),
    TEST_c_c (catanh, -0x1.0000000000000002p0L, -0x1p-33L, -1.178350206951907025990405771755129268176e1L, -7.853981638922134273801338071094141188767e-1L),
    TEST_c_c (catanh, 0x1p-33L, 0x0.ffffffffffffffffp0L, 5.820766091346740722958646680334721192083e-11L, 7.853981633974483095919439232967553115548e-1L),
    TEST_c_c (catanh, 0x1p-33L, -0x0.ffffffffffffffffp0L, 5.820766091346740722958646680334721192083e-11L, -7.853981633974483095919439232967553115548e-1L),
    TEST_c_c (catanh, -0x1p-33L, 0x0.ffffffffffffffffp0L, -5.820766091346740722958646680334721192083e-11L, 7.853981633974483095919439232967553115548e-1L),
    TEST_c_c (catanh, -0x1p-33L, -0x0.ffffffffffffffffp0L, -5.820766091346740722958646680334721192083e-11L, -7.853981633974483095919439232967553115548e-1L),
    TEST_c_c (catanh, 0x0.ffffffffffffffffp0L, 0x1p-33L, 1.178350206951907026002603046195591193050e1L, 7.853981631937214964185249205444919953948e-1L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffp0L, 0x1p-33L, -1.178350206951907026002603046195591193050e1L, 7.853981631937214964185249205444919953948e-1L),
    TEST_c_c (catanh, 0x0.ffffffffffffffffp0L, -0x1p-33L, 1.178350206951907026002603046195591193050e1L, -7.853981631937214964185249205444919953948e-1L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffp0L, -0x1p-33L, -1.178350206951907026002603046195591193050e1L, -7.853981631937214964185249205444919953948e-1L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (catanh, 0x1p-54L, 0x1.000000000000000000000000008p0L, 2.775557561562891351059079170226980932999e-17L, 7.853981633974483096156608458198888173729e-1L),
    TEST_c_c (catanh, 0x1p-54L, -0x1.000000000000000000000000008p0L, 2.775557561562891351059079170226980932999e-17L, -7.853981633974483096156608458198888173729e-1L),
    TEST_c_c (catanh, -0x1p-54L, 0x1.000000000000000000000000008p0L, -2.775557561562891351059079170226980932999e-17L, 7.853981633974483096156608458198888173729e-1L),
    TEST_c_c (catanh, -0x1p-54L, -0x1.000000000000000000000000008p0L, -2.775557561562891351059079170226980932999e-17L, -7.853981633974483096156608458198888173729e-1L),
    TEST_c_c (catanh, 0x1.000000000000000000000000008p0L, 0x1p-54L, 1.906154746539849600897388334009981267384e1L, 7.853981633974485455380535786656405610710e-1L),
    TEST_c_c (catanh, -0x1.000000000000000000000000008p0L, 0x1p-54L, -1.906154746539849600897388334009981267384e1L, 7.853981633974485455380535786656405610710e-1L),
    TEST_c_c (catanh, 0x1.000000000000000000000000008p0L, -0x1p-54L, 1.906154746539849600897388334009981267384e1L, -7.853981633974485455380535786656405610710e-1L),
    TEST_c_c (catanh, -0x1.000000000000000000000000008p0L, -0x1p-54L, -1.906154746539849600897388334009981267384e1L, -7.853981633974485455380535786656405610710e-1L),
    TEST_c_c (catanh, 0x1p-54L, 0x0.ffffffffffffffffffffffffffcp0L, 2.775557561562891351059079170227083567164e-17L, 7.853981633974483096156608458198703284454e-1L),
    TEST_c_c (catanh, 0x1p-54L, -0x0.ffffffffffffffffffffffffffcp0L, 2.775557561562891351059079170227083567164e-17L, -7.853981633974483096156608458198703284454e-1L),
    TEST_c_c (catanh, -0x1p-54L, 0x0.ffffffffffffffffffffffffffcp0L, -2.775557561562891351059079170227083567164e-17L, 7.853981633974483096156608458198703284454e-1L),
    TEST_c_c (catanh, -0x1p-54L, -0x0.ffffffffffffffffffffffffffcp0L, -2.775557561562891351059079170227083567164e-17L, -7.853981633974483096156608458198703284454e-1L),
    TEST_c_c (catanh, 0x0.ffffffffffffffffffffffffffcp0L, 0x1p-54L, 1.906154746539849600897388334009984040723e1L, 7.853981633974482124711461911186784339815e-1L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffffffffffffcp0L, 0x1p-54L, -1.906154746539849600897388334009984040723e1L, 7.853981633974482124711461911186784339815e-1L),
    TEST_c_c (catanh, 0x0.ffffffffffffffffffffffffffcp0L, -0x1p-54L, 1.906154746539849600897388334009984040723e1L, -7.853981633974482124711461911186784339815e-1L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffffffffffffcp0L, -0x1p-54L, -1.906154746539849600897388334009984040723e1L, -7.853981633974482124711461911186784339815e-1L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (catanh, 0x1p-57L, 0x1.0000000000000000000000000001p0L, 3.469446951953614188823848962783812780530e-18L, 7.853981633974483096156608458198758293829e-1L),
    TEST_c_c (catanh, 0x1p-57L, -0x1.0000000000000000000000000001p0L, 3.469446951953614188823848962783812780530e-18L, -7.853981633974483096156608458198758293829e-1L),
    TEST_c_c (catanh, -0x1p-57L, 0x1.0000000000000000000000000001p0L, -3.469446951953614188823848962783812780530e-18L, 7.853981633974483096156608458198758293829e-1L),
    TEST_c_c (catanh, -0x1p-57L, -0x1.0000000000000000000000000001p0L, -3.469446951953614188823848962783812780530e-18L, -7.853981633974483096156608458198758293829e-1L),
    TEST_c_c (catanh, 0x1.0000000000000000000000000001p0L, 0x1p-57L, 2.010126823623841397309973152228712033275e1L, 7.853981633974483252281721296111395707566e-1L),
    TEST_c_c (catanh, -0x1.0000000000000000000000000001p0L, 0x1p-57L, -2.010126823623841397309973152228712033275e1L, 7.853981633974483252281721296111395707566e-1L),
    TEST_c_c (catanh, 0x1.0000000000000000000000000001p0L, -0x1p-57L, 2.010126823623841397309973152228712033275e1L, -7.853981633974483252281721296111395707566e-1L),
    TEST_c_c (catanh, -0x1.0000000000000000000000000001p0L, -0x1p-57L, -2.010126823623841397309973152228712033275e1L, -7.853981633974483252281721296111395707566e-1L),
    TEST_c_c (catanh, 0x1p-57L, 0x0.ffffffffffffffffffffffffffff8p0L, 3.469446951953614188823848962783813782817e-18L, 7.853981633974483096156608458198756849381e-1L),
    TEST_c_c (catanh, 0x1p-57L, -0x0.ffffffffffffffffffffffffffff8p0L, 3.469446951953614188823848962783813782817e-18L, -7.853981633974483096156608458198756849381e-1L),
    TEST_c_c (catanh, -0x1p-57L, 0x0.ffffffffffffffffffffffffffff8p0L, -3.469446951953614188823848962783813782817e-18L, 7.853981633974483096156608458198756849381e-1L),
    TEST_c_c (catanh, -0x1p-57L, -0x0.ffffffffffffffffffffffffffff8p0L, -3.469446951953614188823848962783813782817e-18L, -7.853981633974483096156608458198756849381e-1L),
    TEST_c_c (catanh, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-57L, 2.010126823623841397309973152228712040498e1L, 7.853981633974483044114904178894544378135e-1L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-57L, -2.010126823623841397309973152228712040498e1L, 7.853981633974483044114904178894544378135e-1L),
    TEST_c_c (catanh, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-57L, 2.010126823623841397309973152228712040498e1L, -7.853981633974483044114904178894544378135e-1L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-57L, -2.010126823623841397309973152228712040498e1L, -7.853981633974483044114904178894544378135e-1L),
#endif
    TEST_c_c (catanh, 1.0L, 0x1p-64L, 2.252728336819822255606004394739073846245e1L, 7.853981633974483096292133729759445264744e-1L),
    TEST_c_c (catanh, 1.0L, -0x1p-64L, 2.252728336819822255606004394739073846245e1L, -7.853981633974483096292133729759445264744e-1L),
    TEST_c_c (catanh, -1.0L, 0x1p-64L, -2.252728336819822255606004394739073846245e1L, 7.853981633974483096292133729759445264744e-1L),
    TEST_c_c (catanh, -1.0L, -0x1p-64L, -2.252728336819822255606004394739073846245e1L, -7.853981633974483096292133729759445264744e-1L),
    TEST_c_c (catanh, 0x1p-64L, 1.0L, 2.710505431213761085018632002174854278563e-20L, 7.853981633974483096156608458198757210500e-1L),
    TEST_c_c (catanh, -0x1p-64L, 1.0L, -2.710505431213761085018632002174854278563e-20L, 7.853981633974483096156608458198757210500e-1L),
    TEST_c_c (catanh, 0x1p-64L, -1.0L, 2.710505431213761085018632002174854278563e-20L, -7.853981633974483096156608458198757210500e-1L),
    TEST_c_c (catanh, -0x1p-64L, -1.0L, -2.710505431213761085018632002174854278563e-20L, -7.853981633974483096156608458198757210500e-1L),
    TEST_c_c (catanh, 1.0L, 0x1.3p-73L, 2.556052055225464683726753902092495938633e1L, 7.853981633974483096156922786831527361009e-1L),
    TEST_c_c (catanh, 1.0L, -0x1.3p-73L, 2.556052055225464683726753902092495938633e1L, -7.853981633974483096156922786831527361009e-1L),
    TEST_c_c (catanh, -1.0L, 0x1.3p-73L, -2.556052055225464683726753902092495938633e1L, 7.853981633974483096156922786831527361009e-1L),
    TEST_c_c (catanh, -1.0L, -0x1.3p-73L, -2.556052055225464683726753902092495938633e1L, -7.853981633974483096156922786831527361009e-1L),
    TEST_c_c (catanh, 0x1.3p-73L, 1.0L, 6.286572655403010329022706059731717687100e-23L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1.3p-73L, 1.0L, -6.286572655403010329022706059731717687100e-23L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1.3p-73L, -1.0L, 6.286572655403010329022706059731717687100e-23L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1.3p-73L, -1.0L, -6.286572655403010329022706059731717687100e-23L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, 0x1p-124L, 4.332169878499658183857700759113603550472e1L, 7.853981633974483096156608458198757210610e-1L),
    TEST_c_c (catanh, 1.0L, -0x1p-124L, 4.332169878499658183857700759113603550472e1L, -7.853981633974483096156608458198757210610e-1L),
    TEST_c_c (catanh, -1.0L, 0x1p-124L, -4.332169878499658183857700759113603550472e1L, 7.853981633974483096156608458198757210610e-1L),
    TEST_c_c (catanh, -1.0L, -0x1p-124L, -4.332169878499658183857700759113603550472e1L, -7.853981633974483096156608458198757210610e-1L),
    TEST_c_c (catanh, 0x1p-124L, 1.0L, 2.350988701644575015937473074444491355637e-38L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1p-124L, 1.0L, -2.350988701644575015937473074444491355637e-38L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1p-124L, -1.0L, 2.350988701644575015937473074444491355637e-38L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1p-124L, -1.0L, -2.350988701644575015937473074444491355637e-38L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1.000002p0L, 0x1p-126L, 8.317766196521665212523713550410777685446L, 1.570796326794896619231321691639702138295L),
    TEST_c_c (catanh, 0x1.000002p0L, -0x1p-126L, 8.317766196521665212523713550410777685446L, -1.570796326794896619231321691639702138295L),
    TEST_c_c (catanh, -0x1.000002p0L, 0x1p-126L, -8.317766196521665212523713550410777685446L, 1.570796326794896619231321691639702138295L),
    TEST_c_c (catanh, -0x1.000002p0L, -0x1p-126L, -8.317766196521665212523713550410777685446L, -1.570796326794896619231321691639702138295L),
    TEST_c_c (catanh, 0x1p-126L, 0x1.000002p0L, 5.877471053462247139383742414506671233336e-39L, 7.853982230020895322927482174768220815493e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, -0x1p-126L, 0x1.000002p0L, -5.877471053462247139383742414506671233336e-39L, 7.853982230020895322927482174768220815493e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, 0x1p-126L, -0x1.000002p0L, 5.877471053462247139383742414506671233336e-39L, -7.853982230020895322927482174768220815493e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, -0x1p-126L, -0x1.000002p0L, -5.877471053462247139383742414506671233336e-39L, -7.853982230020895322927482174768220815493e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, 0x0.ffffffp0L, 0x1p-126L, 8.664339742098154951823135931565866792454L, 9.860761609136244031334147100512154160097e-32L),
    TEST_c_c (catanh, 0x0.ffffffp0L, -0x1p-126L, 8.664339742098154951823135931565866792454L, -9.860761609136244031334147100512154160097e-32L),
    TEST_c_c (catanh, -0x0.ffffffp0L, 0x1p-126L, -8.664339742098154951823135931565866792454L, 9.860761609136244031334147100512154160097e-32L),
    TEST_c_c (catanh, -0x0.ffffffp0L, -0x1p-126L, -8.664339742098154951823135931565866792454L, -9.860761609136244031334147100512154160097e-32L),
    TEST_c_c (catanh, 0x1p-126L, 0x0.ffffffp0L, 5.877472104436064061535099214664320916184e-39L, 7.853981335951250337419109991749089175724e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, -0x1p-126L, 0x0.ffffffp0L, -5.877472104436064061535099214664320916184e-39L, 7.853981335951250337419109991749089175724e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, 0x1p-126L, -0x0.ffffffp0L, 5.877472104436064061535099214664320916184e-39L, -7.853981335951250337419109991749089175724e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (catanh, -0x1p-126L, -0x0.ffffffp0L, -5.877472104436064061535099214664320916184e-39L, -7.853981335951250337419109991749089175724e-1L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (catanh, 1.0L, 0x1p-512L, 1.777922518136259718655200391540222897114e2L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, -0x1p-512L, 1.777922518136259718655200391540222897114e2L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, 0x1p-512L, -1.777922518136259718655200391540222897114e2L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, -0x1p-512L, -1.777922518136259718655200391540222897114e2L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1p-512L, 1.0L, 3.729170365600103371645482657731466918688e-155L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1p-512L, 1.0L, -3.729170365600103371645482657731466918688e-155L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1p-512L, -1.0L, 3.729170365600103371645482657731466918688e-155L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1p-512L, -1.0L, -3.729170365600103371645482657731466918688e-155L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, 0x1.3p-536L, 1.860240928518819859673567751384928348958e2L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, -0x1.3p-536L, 1.860240928518819859673567751384928348958e2L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, 0x1.3p-536L, -1.860240928518819859673567751384928348958e2L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, -0x1.3p-536L, -1.860240928518819859673567751384928348958e2L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1.3p-536L, 1.0L, 2.639526015013529511588222179446290115084e-162L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1.3p-536L, 1.0L, -2.639526015013529511588222179446290115084e-162L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1.3p-536L, -1.0L, 2.639526015013529511588222179446290115084e-162L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1.3p-536L, -1.0L, -2.639526015013529511588222179446290115084e-162L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, 0x1p-1020L, 3.538516356758520804574969980043991380025e2L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, -0x1p-1020L, 3.538516356758520804574969980043991380025e2L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, 0x1p-1020L, -3.538516356758520804574969980043991380025e2L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, -0x1p-1020L, -3.538516356758520804574969980043991380025e2L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1p-1020L, 1.0L, 4.450147717014402766180465434664808128438e-308L, 7.853981633974483096156608458198757210493e-1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catanh, -0x1p-1020L, 1.0L, -4.450147717014402766180465434664808128438e-308L, 7.853981633974483096156608458198757210493e-1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catanh, 0x1p-1020L, -1.0L, 4.450147717014402766180465434664808128438e-308L, -7.853981633974483096156608458198757210493e-1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catanh, -0x1p-1020L, -1.0L, -4.450147717014402766180465434664808128438e-308L, -7.853981633974483096156608458198757210493e-1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catanh, 0x1.0000000000001p0L, 0x1p-1022L, 1.836840028483855075506780244989950299369e1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1.0000000000001p0L, -0x1p-1022L, 1.836840028483855075506780244989950299369e1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1.0000000000001p0L, 0x1p-1022L, -1.836840028483855075506780244989950299369e1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1.0000000000001p0L, -0x1p-1022L, -1.836840028483855075506780244989950299369e1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p-1022L, 0x1.0000000000001p0L, 1.112536929253600444512293438042957369978e-308L, 7.853981633974484206379633083355174374608e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, 0x1.0000000000001p0L, -1.112536929253600444512293438042957369978e-308L, 7.853981633974484206379633083355174374608e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0x1p-1022L, -0x1.0000000000001p0L, 1.112536929253600444512293438042957369978e-308L, -7.853981633974484206379633083355174374608e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, -0x1.0000000000001p0L, -1.112536929253600444512293438042957369978e-308L, -7.853981633974484206379633083355174374608e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0x0.fffffffffffff8p0L, 0x1p-1022L, 1.871497387511852332650969166374185305708e1L, 1.002084180004486444624900488355118689113e-292L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catanh, 0x0.fffffffffffff8p0L, -0x1p-1022L, 1.871497387511852332650969166374185305708e1L, -1.002084180004486444624900488355118689113e-292L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catanh, -0x0.fffffffffffff8p0L, 0x1p-1022L, -1.871497387511852332650969166374185305708e1L, 1.002084180004486444624900488355118689113e-292L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catanh, -0x0.fffffffffffff8p0L, -0x1p-1022L, -1.871497387511852332650969166374185305708e1L, -1.002084180004486444624900488355118689113e-292L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (catanh, 0x1p-1022L, 0x0.fffffffffffff8p0L, 1.112536929253600815061527818977844932790e-308L, 7.853981633974482541045096145620456183798e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, 0x0.fffffffffffff8p0L, -1.112536929253600815061527818977844932790e-308L, 7.853981633974482541045096145620456183798e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0x1p-1022L, -0x0.fffffffffffff8p0L, 1.112536929253600815061527818977844932790e-308L, -7.853981633974482541045096145620456183798e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, -0x0.fffffffffffff8p0L, -1.112536929253600815061527818977844932790e-308L, -7.853981633974482541045096145620456183798e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (catanh, 0x1.0000000000000002p0L, 0x1p-1022L, 2.218070977791824990137853294097378778927e1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1.0000000000000002p0L, -0x1p-1022L, 2.218070977791824990137853294097378778927e1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1.0000000000000002p0L, 0x1p-1022L, -2.218070977791824990137853294097378778927e1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1.0000000000000002p0L, -0x1p-1022L, -2.218070977791824990137853294097378778927e1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p-1022L, 0x1.0000000000000002p0L, 1.112536929253600691424494863099491450042e-308L, 7.853981633974483096698709544441509427467e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, 0x1.0000000000000002p0L, -1.112536929253600691424494863099491450042e-308L, 7.853981633974483096698709544441509427467e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0x1p-1022L, -0x1.0000000000000002p0L, 1.112536929253600691424494863099491450042e-308L, -7.853981633974483096698709544441509427467e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, -0x1.0000000000000002p0L, -1.112536929253600691424494863099491450042e-308L, -7.853981633974483096698709544441509427467e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0x0.ffffffffffffffffp0L, 0x1p-1022L, 2.252728336819822255604649142023466965703e1L, 2.052268400649188124723641491045245971623e-289L),
    TEST_c_c (catanh, 0x0.ffffffffffffffffp0L, -0x1p-1022L, 2.252728336819822255604649142023466965703e1L, -2.052268400649188124723641491045245971623e-289L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffp0L, 0x1p-1022L, -2.252728336819822255604649142023466965703e1L, 2.052268400649188124723641491045245971623e-289L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffp0L, -0x1p-1022L, -2.252728336819822255604649142023466965703e1L, -2.052268400649188124723641491045245971623e-289L),
    TEST_c_c (catanh, 0x1p-1022L, 0x0.ffffffffffffffffp0L, 1.112536929253600691605427106449557323148e-308L, 7.853981633974483095885557915077381101984e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, 0x0.ffffffffffffffffp0L, -1.112536929253600691605427106449557323148e-308L, 7.853981633974483095885557915077381101984e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0x1p-1022L, -0x0.ffffffffffffffffp0L, 1.112536929253600691605427106449557323148e-308L, -7.853981633974483095885557915077381101984e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, -0x0.ffffffffffffffffp0L, -1.112536929253600691605427106449557323148e-308L, -7.853981633974483095885557915077381101984e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (catanh, 0x1.000000000000000000000000008p0L, 0x1p-1022L, 3.673680056967710139911330243728336427098e1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1.000000000000000000000000008p0L, -0x1p-1022L, 3.673680056967710139911330243728336427098e1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1.000000000000000000000000008p0L, 0x1p-1022L, -3.673680056967710139911330243728336427098e1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1.000000000000000000000000008p0L, -0x1p-1022L, -3.673680056967710139911330243728336427098e1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p-1022L, 0x1.000000000000000000000000008p0L, 1.112536929253600691545116358666174605957e-308L, 7.853981633974483096156608458198880470009e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, 0x1.000000000000000000000000008p0L, -1.112536929253600691545116358666174605957e-308L, 7.853981633974483096156608458198880470009e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0x1p-1022L, -0x1.000000000000000000000000008p0L, 1.112536929253600691545116358666174605957e-308L, -7.853981633974483096156608458198880470009e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, -0x1.000000000000000000000000008p0L, -1.112536929253600691545116358666174605957e-308L, -7.853981633974483096156608458198880470009e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0x0.ffffffffffffffffffffffffffcp0L, 0x1p-1022L, 3.708337415995707405382191849801244331055e1L, 9.025971879324147880346310405868788320726e-277L),
    TEST_c_c (catanh, 0x0.ffffffffffffffffffffffffffcp0L, -0x1p-1022L, 3.708337415995707405382191849801244331055e1L, -9.025971879324147880346310405868788320726e-277L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffffffffffffcp0L, 0x1p-1022L, -3.708337415995707405382191849801244331055e1L, 9.025971879324147880346310405868788320726e-277L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffffffffffffcp0L, -0x1p-1022L, -3.708337415995707405382191849801244331055e1L, -9.025971879324147880346310405868788320726e-277L),
    TEST_c_c (catanh, 0x1p-1022L, 0x0.ffffffffffffffffffffffffffcp0L, 1.112536929253600691545116358666215745186e-308L, 7.853981633974483096156608458198695580735e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, 0x0.ffffffffffffffffffffffffffcp0L, -1.112536929253600691545116358666215745186e-308L, 7.853981633974483096156608458198695580735e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, 0x1p-1022L, -0x0.ffffffffffffffffffffffffffcp0L, 1.112536929253600691545116358666215745186e-308L, -7.853981633974483096156608458198695580735e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (catanh, -0x1p-1022L, -0x0.ffffffffffffffffffffffffffcp0L, -1.112536929253600691545116358666215745186e-308L, -7.853981633974483096156608458198695580735e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (catanh, 1.0L, 0x1p-8192L, 2.839477425163815960027691385553420311121e3L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, -0x1p-8192L, 2.839477425163815960027691385553420311121e3L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, 0x1p-8192L, -2.839477425163815960027691385553420311121e3L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, -0x1p-8192L, -2.839477425163815960027691385553420311121e3L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1p-8192L, 1.0L, 4.584009668887117914053530980121207914891e-2467L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1p-8192L, 1.0L, -4.584009668887117914053530980121207914891e-2467L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1p-8192L, -1.0L, 4.584009668887117914053530980121207914891e-2467L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1p-8192L, -1.0L, -4.584009668887117914053530980121207914891e-2467L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, 0x1.3p-8221L, 2.849442134153471837403071201841536297726e3L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, -0x1.3p-8221L, 2.849442134153471837403071201841536297726e3L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, 0x1.3p-8221L, -2.849442134153471837403071201841536297726e3L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, -0x1.3p-8221L, -2.849442134153471837403071201841536297726e3L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1.3p-8221L, 1.0L, 1.013933025636421986471594877335044443409e-2475L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1.3p-8221L, 1.0L, -1.013933025636421986471594877335044443409e-2475L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1.3p-8221L, -1.0L, 1.013933025636421986471594877335044443409e-2475L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1.3p-8221L, -1.0L, -1.013933025636421986471594877335044443409e-2475L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, 0x1.3p-8246L, 2.858106473910471153770786603359763504827e3L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, -0x1.3p-8246L, 2.858106473910471153770786603359763504827e3L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, 0x1.3p-8246L, -2.858106473910471153770786603359763504827e3L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, -0x1.3p-8246L, -2.858106473910471153770786603359763504827e3L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1.3p-8246L, 1.0L, 3.021755890954798419688924781486524472858e-2483L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1.3p-8246L, 1.0L, -3.021755890954798419688924781486524472858e-2483L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1.3p-8246L, -1.0L, 3.021755890954798419688924781486524472858e-2483L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1.3p-8246L, -1.0L, -3.021755890954798419688924781486524472858e-2483L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, 0x1p-16380L, 5.677221982376232056781839690803195180822e3L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 1.0L, -0x1p-16380L, 5.677221982376232056781839690803195180822e3L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, 0x1p-16380L, -5.677221982376232056781839690803195180822e3L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -1.0L, -0x1p-16380L, -5.677221982376232056781839690803195180822e3L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1p-16380L, 1.0L, 6.724206286224187012525355634643505205196e-4932L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1p-16380L, 1.0L, -6.724206286224187012525355634643505205196e-4932L, 7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1p-16380L, -1.0L, 6.724206286224187012525355634643505205196e-4932L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, -0x1p-16380L, -1.0L, -6.724206286224187012525355634643505205196e-4932L, -7.853981633974483096156608458198757210493e-1L),
    TEST_c_c (catanh, 0x1.0000000000000002p0L, 0x1p-16382L, 2.218070977791824990137853294097378778927e1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1.0000000000000002p0L, -0x1p-16382L, 2.218070977791824990137853294097378778927e1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1.0000000000000002p0L, 0x1p-16382L, -2.218070977791824990137853294097378778927e1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1.0000000000000002p0L, -0x1p-16382L, -2.218070977791824990137853294097378778927e1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p-16382L, 0x1.0000000000000002p0L, 1.681051571556046752949078932066752571182e-4932L, 7.853981633974483096698709544441509427467e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1p-16382L, 0x1.0000000000000002p0L, -1.681051571556046752949078932066752571182e-4932L, 7.853981633974483096698709544441509427467e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, 0x1p-16382L, -0x1.0000000000000002p0L, 1.681051571556046752949078932066752571182e-4932L, -7.853981633974483096698709544441509427467e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1p-16382L, -0x1.0000000000000002p0L, -1.681051571556046752949078932066752571182e-4932L, -7.853981633974483096698709544441509427467e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, 0x0.ffffffffffffffffp0L, 0x1p-16382L, 2.252728336819822255604649142023466965703e1L, 3.100992811520163369065387859792822623745e-4913L),
    TEST_c_c (catanh, 0x0.ffffffffffffffffp0L, -0x1p-16382L, 2.252728336819822255604649142023466965703e1L, -3.100992811520163369065387859792822623745e-4913L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffp0L, 0x1p-16382L, -2.252728336819822255604649142023466965703e1L, 3.100992811520163369065387859792822623745e-4913L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffp0L, -0x1p-16382L, -2.252728336819822255604649142023466965703e1L, -3.100992811520163369065387859792822623745e-4913L),
    TEST_c_c (catanh, 0x1p-16382L, 0x0.ffffffffffffffffp0L, 1.681051571556046753222468896957938166365e-4932L, 7.853981633974483095885557915077381101984e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1p-16382L, 0x0.ffffffffffffffffp0L, -1.681051571556046753222468896957938166365e-4932L, 7.853981633974483095885557915077381101984e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, 0x1p-16382L, -0x0.ffffffffffffffffp0L, 1.681051571556046753222468896957938166365e-4932L, -7.853981633974483095885557915077381101984e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1p-16382L, -0x0.ffffffffffffffffp0L, -1.681051571556046753222468896957938166365e-4932L, -7.853981633974483095885557915077381101984e-1L, UNDERFLOW_EXCEPTION),
# if LDBL_MANT_DIG >= 113
    TEST_c_c (catanh, 0x1.0000000000000000000000000001p0L, 0x1p-16382L, 3.916281570163690998207361486238697614441e1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1.0000000000000000000000000001p0L, -0x1p-16382L, 3.916281570163690998207361486238697614441e1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1.0000000000000000000000000001p0L, 0x1p-16382L, -3.916281570163690998207361486238697614441e1L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, -0x1.0000000000000000000000000001p0L, -0x1p-16382L, -3.916281570163690998207361486238697614441e1L, -1.570796326794896619231321691639751442099L),
    TEST_c_c (catanh, 0x1p-16382L, 0x1.0000000000000000000000000001p0L, 1.681051571556046753131338908660875977540e-4932L, 7.853981633974483096156608458198758173458e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1p-16382L, 0x1.0000000000000000000000000001p0L, -1.681051571556046753131338908660875977540e-4932L, 7.853981633974483096156608458198758173458e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, 0x1p-16382L, -0x1.0000000000000000000000000001p0L, 1.681051571556046753131338908660875977540e-4932L, -7.853981633974483096156608458198758173458e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1p-16382L, -0x1.0000000000000000000000000001p0L, -1.681051571556046753131338908660875977540e-4932L, -7.853981633974483096156608458198758173458e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-16382L, 3.950938929191688263678223092311606435623e1L, 1.745703758805099310527547423749501866998e-4898L),
    TEST_c_c (catanh, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-16382L, 3.950938929191688263678223092311606435623e1L, -1.745703758805099310527547423749501866998e-4898L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffffffffffffff8p0L, 0x1p-16382L, -3.950938929191688263678223092311606435623e1L, 1.745703758805099310527547423749501866998e-4898L),
    TEST_c_c (catanh, -0x0.ffffffffffffffffffffffffffff8p0L, -0x1p-16382L, -3.950938929191688263678223092311606435623e1L, -1.745703758805099310527547423749501866998e-4898L),
    TEST_c_c (catanh, 0x1p-16382L, 0x0.ffffffffffffffffffffffffffff8p0L, 1.681051571556046753131338908660876463178e-4932L, 7.853981633974483096156608458198756729010e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1p-16382L, 0x0.ffffffffffffffffffffffffffff8p0L, -1.681051571556046753131338908660876463178e-4932L, 7.853981633974483096156608458198756729010e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, 0x1p-16382L, -0x0.ffffffffffffffffffffffffffff8p0L, 1.681051571556046753131338908660876463178e-4932L, -7.853981633974483096156608458198756729010e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (catanh, -0x1p-16382L, -0x0.ffffffffffffffffffffffffffff8p0L, -1.681051571556046753131338908660876463178e-4932L, -7.853981633974483096156608458198756729010e-1L, UNDERFLOW_EXCEPTION),
# endif
#endif

    TEST_c_c (catanh, 0.75L, 1.25L, 0.261492138795671927078652057366532140L, 0.996825126463918666098902241310446708L),
    TEST_c_c (catanh, -2, -3, -0.14694666622552975204743278515471595L, -1.3389725222944935611241935759091443L),
    END_DATA (catanh)
  };

static void
catanh_test (void)
{
  errno = 0;
  FUNC(catanh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (catanh);
  RUN_TEST_LOOP_c_c (catanh, catanh_test_data, );
  END (catanh, complex);
}

static const struct test_f_f_data cbrt_test_data[] =
  {
    START_DATA (cbrt),
    TEST_f_f (cbrt, 0.0, 0.0),
    TEST_f_f (cbrt, minus_zero, minus_zero),

    TEST_f_f (cbrt, plus_infty, plus_infty),
    TEST_f_f (cbrt, minus_infty, minus_infty),
    TEST_f_f (cbrt, qnan_value, qnan_value),

    TEST_f_f (cbrt, -0.001L, -0.1L),
    TEST_f_f (cbrt, 8, 2),
    TEST_f_f (cbrt, -27.0, -3.0),
    TEST_f_f (cbrt, 0.9921875L, 0.997389022060725270579075195353955217L),
    TEST_f_f (cbrt, 0.75L, 0.908560296416069829445605878163630251L),
    END_DATA (cbrt)
  };

static void
cbrt_test (void)
{
  errno = 0;
  FUNC(cbrt) (8);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cbrt);
  RUN_TEST_LOOP_f_f (cbrt, cbrt_test_data, );
  END (cbrt);
}


static const struct test_c_c_data ccos_test_data[] =
  {
    START_DATA (ccos),
    TEST_c_c (ccos, 0.0, 0.0, 1.0, minus_zero),
    TEST_c_c (ccos, minus_zero, 0.0, 1.0, 0.0),
    TEST_c_c (ccos, 0.0, minus_zero, 1.0, 0.0),
    TEST_c_c (ccos, minus_zero, minus_zero, 1.0, minus_zero),

    TEST_c_c (ccos, plus_infty, 0.0, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ccos, plus_infty, minus_zero, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ccos, minus_infty, 0.0, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ccos, minus_infty, minus_zero, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),

    TEST_c_c (ccos, 0.0, plus_infty, plus_infty, minus_zero),
    TEST_c_c (ccos, 0.0, minus_infty, plus_infty, 0.0),
    TEST_c_c (ccos, minus_zero, plus_infty, plus_infty, 0.0),
    TEST_c_c (ccos, minus_zero, minus_infty, plus_infty, minus_zero),

    TEST_c_c (ccos, plus_infty, plus_infty, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccos, minus_infty, plus_infty, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccos, plus_infty, minus_infty, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccos, minus_infty, minus_infty, plus_infty, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (ccos, 4.625, plus_infty, minus_infty, plus_infty),
    TEST_c_c (ccos, 4.625, minus_infty, minus_infty, minus_infty),
    TEST_c_c (ccos, -4.625, plus_infty, minus_infty, minus_infty),
    TEST_c_c (ccos, -4.625, minus_infty, minus_infty, plus_infty),

    TEST_c_c (ccos, plus_infty, 6.75, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccos, plus_infty, -6.75, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccos, minus_infty, 6.75, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccos, minus_infty, -6.75, qnan_value, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (ccos, qnan_value, 0.0, qnan_value, 0.0, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ccos, qnan_value, minus_zero, qnan_value, 0.0, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (ccos, qnan_value, plus_infty, plus_infty, qnan_value),
    TEST_c_c (ccos, qnan_value, minus_infty, plus_infty, qnan_value),

    TEST_c_c (ccos, qnan_value, 9.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ccos, qnan_value, -9.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (ccos, 0.0, qnan_value, qnan_value, 0.0, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ccos, minus_zero, qnan_value, qnan_value, 0.0, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (ccos, 10.0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ccos, -10.0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (ccos, plus_infty, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ccos, minus_infty, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (ccos, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (ccos, 0.75L, 1.25L, 1.38173873063425888530729933139078645L, -1.09193013555397466170919531722024128L),
    TEST_c_c (ccos, -2, -3, -4.18962569096880723013255501961597373L, -9.10922789375533659797919726277886212L),

    TEST_c_c (ccos, 0.75, 89.5, 2.708024460708609732016532185663087200560e38L, -2.522786001038096774676288412995370563339e38L),
    TEST_c_c (ccos, 0.75, -89.5, 2.708024460708609732016532185663087200560e38L, 2.522786001038096774676288412995370563339e38L),
    TEST_c_c (ccos, -0.75, 89.5, 2.708024460708609732016532185663087200560e38L, 2.522786001038096774676288412995370563339e38L),
    TEST_c_c (ccos, -0.75, -89.5, 2.708024460708609732016532185663087200560e38L, -2.522786001038096774676288412995370563339e38L),

#ifndef TEST_FLOAT
    TEST_c_c (ccos, 0.75, 710.5, 1.347490911916428129246890157395342279438e308L, -1.255317763348154410745082950806112487736e308L),
    TEST_c_c (ccos, 0.75, -710.5, 1.347490911916428129246890157395342279438e308L, 1.255317763348154410745082950806112487736e308L),
    TEST_c_c (ccos, -0.75, 710.5, 1.347490911916428129246890157395342279438e308L, 1.255317763348154410745082950806112487736e308L),
    TEST_c_c (ccos, -0.75, -710.5, 1.347490911916428129246890157395342279438e308L, -1.255317763348154410745082950806112487736e308L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (ccos, 0.75, 11357.25, 9.001213196851067077465606717616495588201e4931L, -8.385498349388321535962327491346664141020e4931L),
    TEST_c_c (ccos, 0.75, -11357.25, 9.001213196851067077465606717616495588201e4931L, 8.385498349388321535962327491346664141020e4931L),
    TEST_c_c (ccos, -0.75, 11357.25, 9.001213196851067077465606717616495588201e4931L, 8.385498349388321535962327491346664141020e4931L),
    TEST_c_c (ccos, -0.75, -11357.25, 9.001213196851067077465606717616495588201e4931L, -8.385498349388321535962327491346664141020e4931L),
#endif

#ifdef TEST_FLOAT
    TEST_c_c (ccos, 0x1p-149, 180, plus_infty, -1.043535896672617552965983803453927655332e33L, OVERFLOW_EXCEPTION),
#endif

#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MAX_EXP == 1024)
    TEST_c_c (ccos, 0x1p-1074, 1440, plus_infty, -5.981479269486130556466515778180916082415e301L, OVERFLOW_EXCEPTION),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (ccos, 0x1p-16434L, 22730, plus_infty, -1.217853148905605987081057582351152052687e4924L, OVERFLOW_EXCEPTION),
#endif

    TEST_c_c (ccos, min_subnorm_value * 0x1p120, 0x1p-120, 1.0, -min_subnorm_value, UNDERFLOW_EXCEPTION),
    TEST_c_c (ccos, 0x1p-120, min_subnorm_value * 0x1p120, 1.0, -min_subnorm_value, UNDERFLOW_EXCEPTION),
    END_DATA (ccos)
  };

static void
ccos_test (void)
{
  errno = 0;
  FUNC(ccos) (BUILD_COMPLEX (0, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ccos);
  RUN_TEST_LOOP_c_c (ccos, ccos_test_data, );
  END (ccos, complex);
}


static const struct test_c_c_data ccosh_test_data[] =
  {
    START_DATA (ccosh),
    TEST_c_c (ccosh, 0.0, 0.0, 1.0, 0.0),
    TEST_c_c (ccosh, minus_zero, 0.0, 1.0, minus_zero),
    TEST_c_c (ccosh, 0.0, minus_zero, 1.0, minus_zero),
    TEST_c_c (ccosh, minus_zero, minus_zero, 1.0, 0.0),

    TEST_c_c (ccosh, 0.0, plus_infty, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ccosh, minus_zero, plus_infty, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ccosh, 0.0, minus_infty, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ccosh, minus_zero, minus_infty, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),

    TEST_c_c (ccosh, plus_infty, 0.0, plus_infty, 0.0),
    TEST_c_c (ccosh, minus_infty, 0.0, plus_infty, minus_zero),
    TEST_c_c (ccosh, plus_infty, minus_zero, plus_infty, minus_zero),
    TEST_c_c (ccosh, minus_infty, minus_zero, plus_infty, 0.0),

    TEST_c_c (ccosh, plus_infty, plus_infty, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccosh, minus_infty, plus_infty, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccosh, plus_infty, minus_infty, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccosh, minus_infty, minus_infty, plus_infty, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (ccosh, plus_infty, 4.625, minus_infty, minus_infty),
    TEST_c_c (ccosh, minus_infty, 4.625, minus_infty, plus_infty),
    TEST_c_c (ccosh, plus_infty, -4.625, minus_infty, plus_infty),
    TEST_c_c (ccosh, minus_infty, -4.625, minus_infty, minus_infty),

    TEST_c_c (ccosh, 6.75, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccosh, -6.75, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccosh, 6.75, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ccosh, -6.75, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (ccosh, 0.0, qnan_value, qnan_value, 0.0, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ccosh, minus_zero, qnan_value, qnan_value, 0.0, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (ccosh, plus_infty, qnan_value, plus_infty, qnan_value),
    TEST_c_c (ccosh, minus_infty, qnan_value, plus_infty, qnan_value),

    TEST_c_c (ccosh, 9.0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ccosh, -9.0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (ccosh, qnan_value, 0.0, qnan_value, 0.0, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ccosh, qnan_value, minus_zero, qnan_value, 0.0, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (ccosh, qnan_value, 10.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ccosh, qnan_value, -10.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (ccosh, qnan_value, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ccosh, qnan_value, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (ccosh, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (ccosh, 0.75L, 1.25L, 0.408242591877968807788852146397499084L, 0.780365930845853240391326216300863152L),

    TEST_c_c (ccosh, -2, -3, -3.72454550491532256547397070325597253L, 0.511822569987384608834463849801875634L),

    TEST_c_c (ccosh, 89.5, 0.75, 2.708024460708609732016532185663087200560e38L, 2.522786001038096774676288412995370563339e38L),
    TEST_c_c (ccosh, -89.5, 0.75, 2.708024460708609732016532185663087200560e38L, -2.522786001038096774676288412995370563339e38L),
    TEST_c_c (ccosh, 89.5, -0.75, 2.708024460708609732016532185663087200560e38L, -2.522786001038096774676288412995370563339e38L),
    TEST_c_c (ccosh, -89.5, -0.75, 2.708024460708609732016532185663087200560e38L, 2.522786001038096774676288412995370563339e38L),

#ifndef TEST_FLOAT
    TEST_c_c (ccosh, 710.5, 0.75, 1.347490911916428129246890157395342279438e308L, 1.255317763348154410745082950806112487736e308L),
    TEST_c_c (ccosh, -710.5, 0.75, 1.347490911916428129246890157395342279438e308L, -1.255317763348154410745082950806112487736e308L),
    TEST_c_c (ccosh, 710.5, -0.75, 1.347490911916428129246890157395342279438e308L, -1.255317763348154410745082950806112487736e308L),
    TEST_c_c (ccosh, -710.5, -0.75, 1.347490911916428129246890157395342279438e308L, 1.255317763348154410745082950806112487736e308L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (ccosh, 11357.25, 0.75, 9.001213196851067077465606717616495588201e4931L, 8.385498349388321535962327491346664141020e4931L),
    TEST_c_c (ccosh, -11357.25, 0.75, 9.001213196851067077465606717616495588201e4931L, -8.385498349388321535962327491346664141020e4931L),
    TEST_c_c (ccosh, 11357.25, -0.75, 9.001213196851067077465606717616495588201e4931L, -8.385498349388321535962327491346664141020e4931L),
    TEST_c_c (ccosh, -11357.25, -0.75, 9.001213196851067077465606717616495588201e4931L, 8.385498349388321535962327491346664141020e4931L),
#endif

#ifdef TEST_FLOAT
    TEST_c_c (ccosh, 180, 0x1p-149, plus_infty, 1.043535896672617552965983803453927655332e33L, OVERFLOW_EXCEPTION),
#endif

#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MAX_EXP == 1024)
    TEST_c_c (ccosh, 1440, 0x1p-1074, plus_infty, 5.981479269486130556466515778180916082415e301L, OVERFLOW_EXCEPTION),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (ccosh, 22730, 0x1p-16434L, plus_infty, 1.217853148905605987081057582351152052687e4924L, OVERFLOW_EXCEPTION),
#endif

    TEST_c_c (ccosh, min_subnorm_value * 0x1p120, 0x1p-120, 1.0, min_subnorm_value, UNDERFLOW_EXCEPTION),
    TEST_c_c (ccosh, 0x1p-120, min_subnorm_value * 0x1p120, 1.0, min_subnorm_value, UNDERFLOW_EXCEPTION),
    END_DATA (ccosh)
  };

static void
ccosh_test (void)
{
  errno = 0;
  FUNC(ccosh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ccosh);
  RUN_TEST_LOOP_c_c (ccosh, ccosh_test_data, );
  END (ccosh, complex);
}


static const struct test_f_f_data ceil_test_data[] =
  {
    START_DATA (ceil),
    TEST_f_f (ceil, 0.0, 0.0),
    TEST_f_f (ceil, minus_zero, minus_zero),
    TEST_f_f (ceil, plus_infty, plus_infty),
    TEST_f_f (ceil, minus_infty, minus_infty),
    TEST_f_f (ceil, qnan_value, qnan_value),

    TEST_f_f (ceil, M_PIl, 4.0),
    TEST_f_f (ceil, -M_PIl, -3.0),
    TEST_f_f (ceil, 0.1, 1.0),
    TEST_f_f (ceil, 0.25, 1.0),
    TEST_f_f (ceil, 0.625, 1.0),
    TEST_f_f (ceil, -0.1, minus_zero),
    TEST_f_f (ceil, -0.25, minus_zero),
    TEST_f_f (ceil, -0.625, minus_zero),

#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (ceil, 4503599627370495.5L, 4503599627370496.0L),
    TEST_f_f (ceil, 4503599627370496.25L, 4503599627370497.0L),
    TEST_f_f (ceil, 4503599627370496.5L, 4503599627370497.0L),
    TEST_f_f (ceil, 4503599627370496.75L, 4503599627370497.0L),
    TEST_f_f (ceil, 4503599627370497.5L, 4503599627370498.0L),

    TEST_f_f (ceil, -4503599627370495.5L, -4503599627370495.0L),
    TEST_f_f (ceil, -4503599627370496.25L, -4503599627370496.0L),
    TEST_f_f (ceil, -4503599627370496.5L, -4503599627370496.0L),
    TEST_f_f (ceil, -4503599627370496.75L, -4503599627370496.0L),
    TEST_f_f (ceil, -4503599627370497.5L, -4503599627370497.0L),

# if LDBL_MANT_DIG > 100
    TEST_f_f (ceil, 4503599627370494.5000000000001L, 4503599627370495.0L),
    TEST_f_f (ceil, 4503599627370495.5000000000001L, 4503599627370496.0L),
    TEST_f_f (ceil, 4503599627370496.5000000000001L, 4503599627370497.0L),
    TEST_f_f (ceil, -4503599627370494.5000000000001L, -4503599627370494.0L),
    TEST_f_f (ceil, -4503599627370495.5000000000001L, -4503599627370495.0L),
    TEST_f_f (ceil, -4503599627370496.5000000000001L, -4503599627370496.0L),
# endif

    TEST_f_f (ceil, 9007199254740991.5L, 9007199254740992.0L),
    TEST_f_f (ceil, 9007199254740992.25L, 9007199254740993.0L),
    TEST_f_f (ceil, 9007199254740992.5L, 9007199254740993.0L),
    TEST_f_f (ceil, 9007199254740992.75L, 9007199254740993.0L),
    TEST_f_f (ceil, 9007199254740993.5L, 9007199254740994.0L),

    TEST_f_f (ceil, -9007199254740991.5L, -9007199254740991.0L),
    TEST_f_f (ceil, -9007199254740992.25L, -9007199254740992.0L),
    TEST_f_f (ceil, -9007199254740992.5L, -9007199254740992.0L),
    TEST_f_f (ceil, -9007199254740992.75L, -9007199254740992.0L),
    TEST_f_f (ceil, -9007199254740993.5L, -9007199254740993.0L),

# if LDBL_MANT_DIG > 100
    TEST_f_f (ceil, 9007199254740991.0000000000001L, 9007199254740992.0L),
    TEST_f_f (ceil, 9007199254740992.0000000000001L, 9007199254740993.0L),
    TEST_f_f (ceil, 9007199254740993.0000000000001L, 9007199254740994.0L),
    TEST_f_f (ceil, 9007199254740991.5000000000001L, 9007199254740992.0L),
    TEST_f_f (ceil, 9007199254740992.5000000000001L, 9007199254740993.0L),
    TEST_f_f (ceil, 9007199254740993.5000000000001L, 9007199254740994.0L),

    TEST_f_f (ceil, -9007199254740991.0000000000001L, -9007199254740991.0L),
    TEST_f_f (ceil, -9007199254740992.0000000000001L, -9007199254740992.0L),
    TEST_f_f (ceil, -9007199254740993.0000000000001L, -9007199254740993.0L),
    TEST_f_f (ceil, -9007199254740991.5000000000001L, -9007199254740991.0L),
    TEST_f_f (ceil, -9007199254740992.5000000000001L, -9007199254740992.0L),
    TEST_f_f (ceil, -9007199254740993.5000000000001L, -9007199254740993.0L),
# endif

    TEST_f_f (ceil, 72057594037927935.5L, 72057594037927936.0L),
    TEST_f_f (ceil, 72057594037927936.25L, 72057594037927937.0L),
    TEST_f_f (ceil, 72057594037927936.5L, 72057594037927937.0L),
    TEST_f_f (ceil, 72057594037927936.75L, 72057594037927937.0L),
    TEST_f_f (ceil, 72057594037927937.5L, 72057594037927938.0L),

    TEST_f_f (ceil, -72057594037927935.5L, -72057594037927935.0L),
    TEST_f_f (ceil, -72057594037927936.25L, -72057594037927936.0L),
    TEST_f_f (ceil, -72057594037927936.5L, -72057594037927936.0L),
    TEST_f_f (ceil, -72057594037927936.75L, -72057594037927936.0L),
    TEST_f_f (ceil, -72057594037927937.5L, -72057594037927937.0L),

    TEST_f_f (ceil, 10141204801825835211973625643007.5L, 10141204801825835211973625643008.0L),
    TEST_f_f (ceil, 10141204801825835211973625643008.25L, 10141204801825835211973625643009.0L),
    TEST_f_f (ceil, 10141204801825835211973625643008.5L, 10141204801825835211973625643009.0L),
    TEST_f_f (ceil, 10141204801825835211973625643008.75L, 10141204801825835211973625643009.0L),
    TEST_f_f (ceil, 10141204801825835211973625643009.5L, 10141204801825835211973625643010.0L),
#endif
    END_DATA (ceil)
  };

static void
ceil_test (void)
{
  START (ceil);
  RUN_TEST_LOOP_f_f (ceil, ceil_test_data, );
  END (ceil);
}


static const struct test_c_c_data cexp_test_data[] =
  {
    START_DATA (cexp),
    TEST_c_c (cexp, plus_zero, plus_zero, 1, 0.0),
    TEST_c_c (cexp, minus_zero, plus_zero, 1, 0.0),
    TEST_c_c (cexp, plus_zero, minus_zero, 1, minus_zero),
    TEST_c_c (cexp, minus_zero, minus_zero, 1, minus_zero),

    TEST_c_c (cexp, plus_infty, plus_zero, plus_infty, 0.0),
    TEST_c_c (cexp, plus_infty, minus_zero, plus_infty, minus_zero),

    TEST_c_c (cexp, minus_infty, plus_zero, 0.0, 0.0),
    TEST_c_c (cexp, minus_infty, minus_zero, 0.0, minus_zero),

    TEST_c_c (cexp, 0.0, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (cexp, minus_zero, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (cexp, 0.0, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (cexp, minus_zero, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (cexp, 100.0, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (cexp, -100.0, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (cexp, 100.0, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (cexp, -100.0, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (cexp, minus_infty, 2.0, minus_zero, 0.0),
    TEST_c_c (cexp, minus_infty, 4.0, minus_zero, minus_zero),
    TEST_c_c (cexp, plus_infty, 2.0, minus_infty, plus_infty),
    TEST_c_c (cexp, plus_infty, 4.0, minus_infty, minus_infty),

    TEST_c_c (cexp, plus_infty, plus_infty, plus_infty, qnan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (cexp, plus_infty, minus_infty, plus_infty, qnan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),

    TEST_c_c (cexp, minus_infty, plus_infty, 0.0, 0.0, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (cexp, minus_infty, minus_infty, 0.0, minus_zero, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (cexp, minus_infty, qnan_value, 0, 0, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (cexp, plus_infty, qnan_value, plus_infty, qnan_value),

    TEST_c_c (cexp, qnan_value, 0.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (cexp, qnan_value, 1.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (cexp, qnan_value, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (cexp, 0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (cexp, 1, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (cexp, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (cexp, 0.75L, 1.25L, 0.667537446429131586942201977015932112L, 2.00900045494094876258347228145863909L),
    TEST_c_c (cexp, -2.0, -3.0, -0.13398091492954261346140525546115575L, -0.019098516261135196432576240858800925L),

    TEST_c_c (cexp, 0, 0x1p65, 0.99888622066058013610642172179340364209972L, -0.047183876212354673805106149805700013943218L),
    TEST_c_c (cexp, 0, -0x1p65, 0.99888622066058013610642172179340364209972L, 0.047183876212354673805106149805700013943218L),
    TEST_c_c (cexp, 50, 0x1p127, 4.053997150228616856622417636046265337193e21L, 3.232070315463388524466674772633810238819e21L),

#ifndef TEST_FLOAT
    TEST_c_c (cexp, 0, 1e22, 0.5232147853951389454975944733847094921409L, -0.8522008497671888017727058937530293682618L),
    TEST_c_c (cexp, 0, 0x1p1023, -0.826369834614147994500785680811743734805L, 0.5631277798508840134529434079444683477104L),
    TEST_c_c (cexp, 500, 0x1p1023, -1.159886268932754433233243794561351783426e217L, 7.904017694554466595359379965081774849708e216L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (cexp, 0, 0x1p16383L, 0.9210843909921906206874509522505756251609L, 0.3893629985894208126948115852610595405563L),
    TEST_c_c (cexp, -10000, 0x1p16383L, 1.045876464564882298442774542991176546722e-4343L, 4.421154026488516836023811173959413420548e-4344L),
#endif

    TEST_c_c (cexp, 88.75, 0.75, 2.558360358486542817001900410314204322891e38L, 2.383359453227311447654736314679677655100e38L),
    TEST_c_c (cexp, -95, 0.75, 4.039714446238306526889476684000081624047e-42L, 3.763383677300535390271646960780570275931e-42L, UNDERFLOW_EXCEPTION_FLOAT),

#ifndef TEST_FLOAT
    TEST_c_c (cexp, 709.8125, 0.75, 1.355121963080879535248452862759108365762e308L, 1.262426823598609432507811340856186873507e308L),
    TEST_c_c (cexp, -720, 0.75, 1.486960657116368433685753325516638551722e-313L, 1.385247284245720590980701226843815229385e-313L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (cexp, 11356.5625, 0.75, 9.052188470850960144814815984311663764287e4931L, 8.432986734191301036267148978260970230200e4931L),
    TEST_c_c (cexp, -11370, 0.75, 8.631121063182211587489310508568170739592e-4939L, 8.040721827809267291427062346918413482824e-4939L, UNDERFLOW_EXCEPTION),
#endif

#ifdef TEST_FLOAT
    TEST_c_c (cexp, 180, 0x1p-149, plus_infty, 2.087071793345235105931967606907855310664e33L, OVERFLOW_EXCEPTION),
#endif

#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MAX_EXP == 1024)
    TEST_c_c (cexp, 1440, 0x1p-1074, plus_infty, 1.196295853897226111293303155636183216483e302L, OVERFLOW_EXCEPTION),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (cexp, 22730, 0x1p-16434L, plus_infty, 2.435706297811211974162115164702304105374e4924L, OVERFLOW_EXCEPTION),
#endif

    TEST_c_c (cexp, 1e6, 0, plus_infty, 0, OVERFLOW_EXCEPTION),
    TEST_c_c (cexp, 1e6, min_value, plus_infty, plus_infty, OVERFLOW_EXCEPTION),
    TEST_c_c (cexp, 1e6, -min_value, plus_infty, minus_infty, OVERFLOW_EXCEPTION),

    TEST_c_c (cexp, min_value, min_subnorm_value, 1.0, min_subnorm_value, UNDERFLOW_EXCEPTION),
    TEST_c_c (cexp, min_value, -min_subnorm_value, 1.0, -min_subnorm_value, UNDERFLOW_EXCEPTION),
    END_DATA (cexp)
  };

static void
cexp_test (void)
{
  errno = 0;
  FUNC(cexp) (BUILD_COMPLEX (0, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cexp);
  RUN_TEST_LOOP_c_c (cexp, cexp_test_data, );
  END (cexp, complex);
}


static const struct test_c_f_data cimag_test_data[] =
  {
    START_DATA (cimag);
    TEST_c_f (cimag, 1.0, 0.0, 0.0),
    TEST_c_f (cimag, 1.0, minus_zero, minus_zero),
    TEST_c_f (cimag, 1.0, qnan_value, qnan_value),
    TEST_c_f (cimag, qnan_value, qnan_value, qnan_value),
    TEST_c_f (cimag, 1.0, plus_infty, plus_infty),
    TEST_c_f (cimag, 1.0, minus_infty, minus_infty),
    TEST_c_f (cimag, 2.0, 3.0, 3.0),
    END_DATA (cimag);
  };

static void
cimag_test (void)
{
  START (cimag);
  RUN_TEST_LOOP_c_f (cimag, cimag_test_data, );
  END (cimag);
}

static const struct test_c_c_data clog_test_data[] =
  {
    START_DATA (clog),
    TEST_c_c (clog, minus_zero, 0, minus_infty, M_PIl, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_c_c (clog, minus_zero, minus_zero, minus_infty, -M_PIl, DIVIDE_BY_ZERO_EXCEPTION),

    TEST_c_c (clog, 0, 0, minus_infty, 0.0, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_c_c (clog, 0, minus_zero, minus_infty, minus_zero, DIVIDE_BY_ZERO_EXCEPTION),

    TEST_c_c (clog, minus_infty, plus_infty, plus_infty, M_PI_34l),
    TEST_c_c (clog, minus_infty, minus_infty, plus_infty, -M_PI_34l),

    TEST_c_c (clog, plus_infty, plus_infty, plus_infty, M_PI_4l),
    TEST_c_c (clog, plus_infty, minus_infty, plus_infty, -M_PI_4l),

    TEST_c_c (clog, 0, plus_infty, plus_infty, M_PI_2l),
    TEST_c_c (clog, 3, plus_infty, plus_infty, M_PI_2l),
    TEST_c_c (clog, minus_zero, plus_infty, plus_infty, M_PI_2l),
    TEST_c_c (clog, -3, plus_infty, plus_infty, M_PI_2l),
    TEST_c_c (clog, 0, minus_infty, plus_infty, -M_PI_2l),
    TEST_c_c (clog, 3, minus_infty, plus_infty, -M_PI_2l),
    TEST_c_c (clog, minus_zero, minus_infty, plus_infty, -M_PI_2l),
    TEST_c_c (clog, -3, minus_infty, plus_infty, -M_PI_2l),

    TEST_c_c (clog, minus_infty, 0, plus_infty, M_PIl),
    TEST_c_c (clog, minus_infty, 1, plus_infty, M_PIl),
    TEST_c_c (clog, minus_infty, minus_zero, plus_infty, -M_PIl),
    TEST_c_c (clog, minus_infty, -1, plus_infty, -M_PIl),

    TEST_c_c (clog, plus_infty, 0, plus_infty, 0.0),
    TEST_c_c (clog, plus_infty, 1, plus_infty, 0.0),
    TEST_c_c (clog, plus_infty, minus_zero, plus_infty, minus_zero),
    TEST_c_c (clog, plus_infty, -1, plus_infty, minus_zero),

    TEST_c_c (clog, plus_infty, qnan_value, plus_infty, qnan_value),
    TEST_c_c (clog, minus_infty, qnan_value, plus_infty, qnan_value),

    TEST_c_c (clog, qnan_value, plus_infty, plus_infty, qnan_value),
    TEST_c_c (clog, qnan_value, minus_infty, plus_infty, qnan_value),

    TEST_c_c (clog, 0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog, 3, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog, minus_zero, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog, -3, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (clog, qnan_value, 0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog, qnan_value, 5, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog, qnan_value, minus_zero, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog, qnan_value, -5, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (clog, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (clog, 0.75L, 1.25L, 0.376885901188190075998919126749298416L, 1.03037682652431246378774332703115153L),
    TEST_c_c (clog, -2, -3, 1.2824746787307683680267437207826593L, -2.1587989303424641704769327722648368L),

    TEST_c_c (clog, 0x1.fffffep+127L, 0x1.fffffep+127L, 89.06941264234832570836679262104313101776L, M_PI_4l),
    TEST_c_c (clog, 0x1.fffffep+127L, 1.0L, 88.72283905206835305365817656031404273372L, 2.938736052218037251011746307725933020145e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (clog, 0x1p-149L, 0x1p-149L, -102.9323563131518784484589700365392203592L, M_PI_4l),
    TEST_c_c (clog, 0x1p-147L, 0x1p-147L, -101.5460619520319878296245057936228672231L, M_PI_4l),

#ifndef TEST_FLOAT
    TEST_c_c (clog, 0x1.fffffffffffffp+1023L, 0x1.fffffffffffffp+1023L, 710.1292864836639693869320059713862337880L, M_PI_4l),
    TEST_c_c (clog, 0x1.fffffffffffffp+1023L, 0x1p+1023L, 709.8942846690411016323109979483151967689L, 0.4636476090008061606231772164674799632783L),
    TEST_c_c (clog, 0x1p-1074L, 0x1p-1074L, -744.0934983311012896593986823853525458290L, M_PI_4l),
    TEST_c_c (clog, 0x1p-1073L, 0x1p-1073L, -743.4003511505413443499814502638943692610L, M_PI_4l),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (clog, 0x1.fp+16383L, 0x1.fp+16383L, 11356.83823118610934184548269774874545400L, M_PI_4l),
    TEST_c_c (clog, 0x1.fp+16383L, 0x1p+16383L, 11356.60974243783798653123798337822335902L, 0.4764674194737066993385333770295162295856L),
    TEST_c_c (clog, 0x1p-16440L, 0x1p-16441L, -11395.22807662984378194141292922726786191L, 0.4636476090008061162142562314612144020285L),
#endif

    TEST_c_c (clog, 0x1p-149L, 0x1.fp+127L, 88.69109041335841930424871526389807508374L, M_PI_2l),
    TEST_c_c (clog, -0x1p-149L, 0x1.fp+127L, 88.69109041335841930424871526389807508374L, M_PI_2l),
    TEST_c_c (clog, 0x1p-149L, -0x1.fp+127L, 88.69109041335841930424871526389807508374L, -M_PI_2l),
    TEST_c_c (clog, -0x1p-149L, -0x1.fp+127L, 88.69109041335841930424871526389807508374L, -M_PI_2l),
    TEST_c_c (clog, -0x1.fp+127L, 0x1p-149L, 88.69109041335841930424871526389807508374L, M_PIl),
    TEST_c_c (clog, -0x1.fp+127L, -0x1p-149L, 88.69109041335841930424871526389807508374L, -M_PIl),
#ifdef TEST_FLOAT
    TEST_c_c (clog, 0x1.fp+127L, 0x1p-149L, 88.69109041335841930424871526389807508374L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (clog, 0x1.fp+127L, -0x1p-149L, 88.69109041335841930424871526389807508374L, minus_zero, UNDERFLOW_EXCEPTION),
#endif

#ifndef TEST_FLOAT
    TEST_c_c (clog, 0x1p-1074L, 0x1.fp+1023L, 709.7509641950694165420886960904242800794L, M_PI_2l),
    TEST_c_c (clog, -0x1p-1074L, 0x1.fp+1023L, 709.7509641950694165420886960904242800794L, M_PI_2l),
    TEST_c_c (clog, 0x1p-1074L, -0x1.fp+1023L, 709.7509641950694165420886960904242800794L, -M_PI_2l),
    TEST_c_c (clog, -0x1p-1074L, -0x1.fp+1023L, 709.7509641950694165420886960904242800794L, -M_PI_2l),
    TEST_c_c (clog, -0x1.fp+1023L, 0x1p-1074L, 709.7509641950694165420886960904242800794L, M_PIl),
    TEST_c_c (clog, -0x1.fp+1023L, -0x1p-1074L, 709.7509641950694165420886960904242800794L, -M_PIl),
#endif
#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MAX_EXP == 1024)
    TEST_c_c (clog, 0x1.fp+1023L, 0x1p-1074L, 709.7509641950694165420886960904242800794L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (clog, 0x1.fp+1023L, -0x1p-1074L, 709.7509641950694165420886960904242800794L, minus_zero, UNDERFLOW_EXCEPTION),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (clog, 0x1p-16445L, 0x1.fp+16383L, 11356.49165759582936919077408168801636572L, M_PI_2l),
    TEST_c_c (clog, -0x1p-16445L, 0x1.fp+16383L, 11356.49165759582936919077408168801636572L, M_PI_2l),
    TEST_c_c (clog, 0x1p-16445L, -0x1.fp+16383L, 11356.49165759582936919077408168801636572L, -M_PI_2l),
    TEST_c_c (clog, -0x1p-16445L, -0x1.fp+16383L, 11356.49165759582936919077408168801636572L, -M_PI_2l),
    TEST_c_c (clog, -0x1.fp+16383L, 0x1p-16445L, 11356.49165759582936919077408168801636572L, M_PIl),
    TEST_c_c (clog, -0x1.fp+16383L, -0x1p-16445L, 11356.49165759582936919077408168801636572L, -M_PIl),
    TEST_c_c (clog, 0x1.fp+16383L, 0x1p-16445L, 11356.49165759582936919077408168801636572L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (clog, 0x1.fp+16383L, -0x1p-16445L, 11356.49165759582936919077408168801636572L, minus_zero, UNDERFLOW_EXCEPTION),
# if LDBL_MANT_DIG >= 113
    TEST_c_c (clog, 0x1p-16494L, 0x1.fp+16383L, 11356.49165759582936919077408168801636572L, M_PI_2l),
    TEST_c_c (clog, -0x1p-16494L, 0x1.fp+16383L, 11356.49165759582936919077408168801636572L, M_PI_2l),
    TEST_c_c (clog, 0x1p-16494L, -0x1.fp+16383L, 11356.49165759582936919077408168801636572L, -M_PI_2l),
    TEST_c_c (clog, -0x1p-16494L, -0x1.fp+16383L, 11356.49165759582936919077408168801636572L, -M_PI_2l),
    TEST_c_c (clog, -0x1.fp+16383L, 0x1p-16494L, 11356.49165759582936919077408168801636572L, M_PIl),
    TEST_c_c (clog, -0x1.fp+16383L, -0x1p-16494L, 11356.49165759582936919077408168801636572L, -M_PIl),
    TEST_c_c (clog, 0x1.fp+16383L, 0x1p-16494L, 11356.49165759582936919077408168801636572L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (clog, 0x1.fp+16383L, -0x1p-16494L, 11356.49165759582936919077408168801636572L, minus_zero, UNDERFLOW_EXCEPTION),
# endif
#endif

    TEST_c_c (clog, 1.0L, 0x1.234566p-10L, 6.172834701221959432440126967147726538097e-7L, 1.111110564353742042376451655136933182201e-3L),
    TEST_c_c (clog, -1.0L, 0x1.234566p-20L, 5.886877547844618300918562490463748605537e-13L, 3.141591568520436206990380699322226378452L),
    TEST_c_c (clog, 0x1.234566p-30L, 1.0L, 5.614163921211322622623353961365728040115e-19L, 1.570796325735258575254858696548386439740L),
    TEST_c_c (clog, -0x1.234566p-40L, -1.0L, 5.354083939753840089583620652120903838944e-25L, -1.570796326795931422008642456283782656359L),
    TEST_c_c (clog, 0x1.234566p-50L, 1.0L, 5.106052341226425256332038420428899201070e-31L, 1.570796326794895608681734464330528755366L),
    TEST_c_c (clog, 0x1.234566p-60L, 1.0L, 4.869510976053643471080816669875627875933e-37L, 1.570796326794896618244456860363082279319L),
    TEST_c_c (clog, 0x1p-62L, 1.0L, 2.350988701644575015937473074444491355582e-38L, 1.570796326794896619014481257142650555297L),
    TEST_c_c (clog, 0x1p-63L, 1.0L, 5.877471754111437539843682686111228389059e-39L, 1.570796326794896619122901474391200998698L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (clog, 0x1p-64L, 1.0L, 1.469367938527859384960920671527807097271e-39L, 1.570796326794896619177111583015476220398L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (clog, 0x1p-510L, 1.0L, 4.450147717014402766180465434664808128438e-308L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (clog, 0x1p-511L, 1.0L, 1.112536929253600691545116358666202032110e-308L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (clog, 0x1p-512L, 1.0L, 2.781342323134001728862790896665505080274e-309L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (clog, 0x1p-8190L, 1.0L, 6.724206286224187012525355634643505205196e-4932L, 1.570796326794896619231321691639751442099L),
    TEST_c_c (clog, 0x1p-8191L, 1.0L, 1.681051571556046753131338908660876301299e-4932L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
    TEST_c_c (clog, 0x1p-8192L, 1.0L, 4.202628928890116882828347271652190753248e-4933L, 1.570796326794896619231321691639751442099L, UNDERFLOW_EXCEPTION),
#endif

    TEST_c_c (clog, 0x1.000566p0L, 0x1.234p-10L, 8.298731898331237038231468223024422855654e-5L, 1.110938609507128729312743251313024793990e-3L),
    TEST_c_c (clog, 0x1.000566p0L, 0x1.234p-100L, 8.237022655933121125560939513260027133767e-5L, 8.974094312218060110948251664314290484113e-31L),
#ifndef TEST_FLOAT
    TEST_c_c (clog, -0x1.0000000123456p0L, 0x1.2345678p-30L, 2.649094282537168795982991778475646793277e-10L, 3.141592652530155111500161671113150737892L),
    TEST_c_c (clog, -0x1.0000000123456p0L, 0x1.2345678p-1000L, 2.649094276923003995420209214900915462737e-10L, 3.141592653589793238462643383279502884197L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (clog, 0x1.00000000000000123456789abcp0L, 0x1.23456789p-60L, 9.868649107778739757272772275265050767867e-19L, 9.868649106423871142816660980898339912137e-19L),
    TEST_c_c (clog, 0x1.00000000000000123456789abcp0L, 0x1.23456789p-1000L, 9.868649107778739752403260515979017248596e-19L, 1.061846605795612822522063052130030717368e-301L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
#endif

    TEST_c_c (clog, 0x0.ffffffp0L, 0x0.ffffffp-100L, -5.960464655174753498633255797994360530379e-8L, 7.888609052210118054117285652827862296732e-31L),
#ifndef TEST_FLOAT
    TEST_c_c (clog, 0x0.fffffffffffff8p0L, 0x0.fffffffffffff8p-1000L, -1.110223024625156602053389888482372171810e-16L, 9.332636185032188789900895447238171696171e-302L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (clog, 0x0.ffffffffffffffffp0L, 0x0.ffffffffffffffffp-15000L, -5.421010862427522170184200798202494495630e-20L, 3.548665303440282824232502561095699343814e-4516L),
#endif

    TEST_c_c (clog, 0x1a6p-10L, 0x3a5p-10L, -1.4305135209763571252847059962654228661815e-06L, 1.1460277178115757370775644871674016684074L),
    TEST_c_c (clog, 0xf2p-10L, 0x3e3p-10L, 6.1988446308070710970664736815277450078106e-06L, 1.3322126499153926210226335249558203898460L),
    TEST_c_c (clog, 0x4d4ep-15L, 0x6605p-15L, -1.6298145321400412054744424587143483169412e-08L, 0.9223574537155056772124552172295398141249L),
    TEST_c_c (clog, 0x2818p-15L, 0x798fp-15L, 1.5366822245016167178749091974664853785194e-08L, 1.2522014929038946066987318471922169174157L),
    TEST_c_c (clog, 0x9b57bp-20L, 0xcb7b4p-20L, -3.9563019528687610863490232935890272740908e-11L, 0.9187593477446338910857133065497364950682L),
    TEST_c_c (clog, 0x2731p-20L, 0xfffd0p-20L, 4.4110493034041283943115971658295280288115e-11L, 1.5612279663766352262688735061954290528838L),
    TEST_c_c (clog, 0x2ede88p-23L, 0x771c3fp-23L, -4.4764192352906350039050902870893173560494e-13L, 1.1959106857549200806818600493552847793381L),
    TEST_c_c (clog, 0x11682p-23L, 0x7ffed1p-23L, 1.1723955140027907954461000991619077811832e-12L, 1.5622968405332756349813737986164832897108L),
    TEST_c_c (clog, 0xa1f2c1p-24L, 0xc643aep-24L, -1.0480505352462576151523512837107080269981e-13L, 0.8858771987699967480545613322309315260313L),
    TEST_c_c (clog, 0x659feap-24L, 0xeaf6f9p-24L, 3.7303493627403868207597214252239749960738e-14L, 1.1625816408046866464773042283673653469061L),
#ifndef TEST_FLOAT
    TEST_c_c (clog, 0x4447d7175p-35L, 0x6c445e00ap-35L, -1.4823076576950255933915367361099865652625e-20L, 1.0081311552703893116404606212158840190615L),
    TEST_c_c (clog, 0x2dd46725bp-35L, 0x7783a1284p-35L, 4.4469229730850767799109418892826021157328e-20L, 1.2046235979300843056806465045930070146351L),
    TEST_c_c (clog, 0x164c74eea876p-45L, 0x16f393482f77p-45L, -3.0292258760486853327810377824479932031744e-26L, 0.7998237934177411746093524982030330293980L),
    TEST_c_c (clog, 0xfe961079616p-45L, 0x1bc37e09e6d1p-45L, 5.3718272201930019901317065495843842735179e-26L, 1.0503831592447830576186444373011142397404L),
    TEST_c_c (clog, 0xa4722f19346cp-51L, 0x7f9631c5e7f07p-51L, -6.2122796286154679676173624516405339768606e-30L, 1.4904138780720095276446375492434049214172L),
    TEST_c_c (clog, 0x10673dd0f2481p-51L, 0x7ef1d17cefbd2p-51L, 3.2047474274603604594851472963586149973093e-29L, 1.4422922682185099608731642353544207976604L),
    TEST_c_c (clog, 0x8ecbf810c4ae6p-52L, 0xd479468b09a37p-52L, -9.7375017988218644730510244778042114638107e-30L, 0.9790637929494922564724108399524154766631L),
    TEST_c_c (clog, 0x5b06b680ea2ccp-52L, 0xef452b965da9fp-52L, 8.3076914081087805757422664530653247447136e-30L, 1.2072712126771536614482822173033535043206L),
    TEST_c_c (clog, 0x659b70ab7971bp-53L, 0x1f5d111e08abecp-53L, -2.5083311595699359750201056724289010648701e-30L, 1.3710185432462268491534742969536240564640L),
    TEST_c_c (clog, 0x15cfbd1990d1ffp-53L, 0x176a3973e09a9ap-53L, 1.0168910106364605304135563536838075568606e-30L, 0.8208373755522359859870890246475340086663L),
    TEST_c_c (clog, 0x1367a310575591p-54L, 0x3cfcc0a0541f60p-54L, 5.0844550531823026520677817684239496041087e-32L, 1.2627468605458094918919206628466016525397L),
    TEST_c_c (clog, 0x55cb6d0c83af5p-55L, 0x7fe33c0c7c4e90p-55L, -5.2000108498455368032511404449795741611813e-32L, 1.5288921536982513453421343495466824420259L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (clog, 0x298c62cb546588a7p-63L, 0x7911b1dfcc4ecdaep-63L, -1.1931267660846218205882675852805793644095e-36L, 1.2402109774337032400594953899784058127412L),
    TEST_c_c (clog, 0x4d9c37e2b5cb4533p-63L, 0x65c98be2385a042ep-63L, 6.4064442119814669184296141278612389400075e-37L, 0.9193591364645830864185131402313014890145L),
    TEST_c_c (clog, 0x602fd5037c4792efp-64L, 0xed3e2086dcca80b8p-64L, -2.3362950222592964220878638677292132852104e-37L, 1.1856121127236268105413184264288408265852L),
    TEST_c_c (clog, 0x6b10b4f3520217b6p-64L, 0xe8893cbb449253a1p-64L, 2.4244570985709679851855191080208817099132e-37L, 1.1393074519572050614551047548718495655972L),
    TEST_c_c (clog, 0x81b7efa81fc35ad1p-65L, 0x1ef4b835f1c79d812p-65L, -9.9182335850630508484862145328126979066934e-39L, 1.3146479888794807046338799047003947008804L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (clog, 0x3f96469050f650869c2p-75L, 0x6f16b2c9c8b05988335p-75L, -1.0509738482436128031927971874674370984602e-45L, 1.0509191467640012308402149909370784281448L),
    TEST_c_c (clog, 0x3157fc1d73233e580c8p-75L, 0x761b52ccd435d7c7f5fp-75L, 1.3487497719126364307640897239165442763573e-43L, 1.1750493008528425228929764149024375035382L),
    TEST_c_c (clog, 0x155f8afc4c48685bf63610p-85L, 0x17d0cf2652cdbeb1294e19p-85L, -4.7775669192897997174762089350332738583822e-50L, 0.8393953487996880419413728440067635213372L),
    TEST_c_c (clog, 0x13836d58a13448d750b4b9p-85L, 0x195ca7bc3ab4f9161edbe6p-85L, 2.8398125044729578740243199963484494962411e-50L, 0.9149964976334130461795060758257083099706L),
    TEST_c_c (clog, 0x1df515eb171a808b9e400266p-95L, 0x7c71eb0cd4688dfe98581c77p-95L, -3.5048022044913950094635368750889659723004e-57L, 1.3345633256521815205858155673950177421079L),
    TEST_c_c (clog, 0xe33f66c9542ca25cc43c867p-95L, 0x7f35a68ebd3704a43c465864p-95L, 4.1101771307217268747345114262406964584250e-56L, 1.4596065864518742494094402406719567059585L),
    TEST_c_c (clog, 0x6771f22c64ed551b857c128b4cp-105L, 0x1f570e7a13cc3cf2f44fd793ea1p-105L, -1.4281333889622737316199756373421183559948e-62L, 1.3673546561165378090903506783353927980633L),
    TEST_c_c (clog, 0x15d8ab6ed05ca514086ac3a1e84p-105L, 0x1761e480aa094c0b10b34b09ce9p-105L, 1.0027319539522347477331743836657426754857e-62L, 0.8193464073721167323313606647411269414759L),
    TEST_c_c (clog, 0x187190c1a334497bdbde5a95f48p-106L, 0x3b25f08062d0a095c4cfbbc338dp-106L, -1.7471844652198029695350765775994001163767e-63L, 1.1789110097072986038243729592318526094314L),
    TEST_c_c (clog, 0x6241ef0da53f539f02fad67dabp-106L, 0x3fb46641182f7efd9caa769dac0p-106L, 4.3299788920664682288477984749202524623248e-63L, 1.4746938237585656250866370987773473745867L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (clog, 0x3e1d0a105ac4ebeacd9c6952d34cp-112L, 0xf859b3d1b06d005dcbb5516d5479p-112L, -1.1683999374665377365054966073875064467108e-66L, 1.3257197596350832748781065387304444940172L),
    TEST_c_c (clog, 0x47017a2e36807acb1e5214b209dep-112L, 0xf5f4a550c9d75e3bb1839d865f0dp-112L, 1.5077923002544367932999503838191154621839e-65L, 1.2897445708311412721399861948957141824914L),
    TEST_c_c (clog, 0x148f818cb7a9258fca942ade2a0cap-113L, 0x18854a34780b8333ec53310ad7001p-113L, -7.1865869169568789348552370692485515571497e-67L, 0.8730167479365994646287897223471819363668L),
    TEST_c_c (clog, 0xfd95243681c055c2632286921092p-113L, 0x1bccabcd29ca2152860ec29e34ef7p-113L, 6.6255694866654064502633121109394710807528e-66L, 1.0526409614996288387567810726095850312049L),
    TEST_c_c (clog, 0xdb85c467ee2aadd5f425fe0f4b8dp-114L, 0x3e83162a0f95f1dcbf97dddf410eap-114L, 4.6017338806965821566734340588575402712716e-67L, 1.3547418904611758959096647942223384691728L),
    TEST_c_c (clog, 0x1415bcaf2105940d49a636e98ae59p-115L, 0x7e6a150adfcd1b0921d44b31f40f4p-115L, 2.5993421227864195179698176012564317527271e-67L, 1.4132318089683022770487383611430906982461L),
#endif
    END_DATA (clog)
  };

static void
clog_test (void)
{
  errno = 0;
  FUNC(clog) (BUILD_COMPLEX (-2, -3));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (clog);
  RUN_TEST_LOOP_c_c (clog, clog_test_data, );
  END (clog, complex);
}


static const struct test_c_c_data clog10_test_data[] =
  {
    START_DATA (clog10),
    TEST_c_c (clog10, minus_zero, 0, minus_infty, M_PIl, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_c_c (clog10, minus_zero, minus_zero, minus_infty, -M_PIl, DIVIDE_BY_ZERO_EXCEPTION),

    TEST_c_c (clog10, 0, 0, minus_infty, 0.0, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_c_c (clog10, 0, minus_zero, minus_infty, minus_zero, DIVIDE_BY_ZERO_EXCEPTION),

    TEST_c_c (clog10, minus_infty, plus_infty, plus_infty, M_PI_34_LOG10El),

    TEST_c_c (clog10, plus_infty, plus_infty, plus_infty, M_PI4_LOG10El),
    TEST_c_c (clog10, plus_infty, minus_infty, plus_infty, -M_PI4_LOG10El),

    TEST_c_c (clog10, 0, plus_infty, plus_infty, M_PI2_LOG10El),
    TEST_c_c (clog10, 3, plus_infty, plus_infty, M_PI2_LOG10El),
    TEST_c_c (clog10, minus_zero, plus_infty, plus_infty, M_PI2_LOG10El),
    TEST_c_c (clog10, -3, plus_infty, plus_infty, M_PI2_LOG10El),
    TEST_c_c (clog10, 0, minus_infty, plus_infty, -M_PI2_LOG10El),
    TEST_c_c (clog10, 3, minus_infty, plus_infty, -M_PI2_LOG10El),
    TEST_c_c (clog10, minus_zero, minus_infty, plus_infty, -M_PI2_LOG10El),
    TEST_c_c (clog10, -3, minus_infty, plus_infty, -M_PI2_LOG10El),

    TEST_c_c (clog10, minus_infty, 0, plus_infty, M_PI_LOG10El),
    TEST_c_c (clog10, minus_infty, 1, plus_infty, M_PI_LOG10El),
    TEST_c_c (clog10, minus_infty, minus_zero, plus_infty, -M_PI_LOG10El),
    TEST_c_c (clog10, minus_infty, -1, plus_infty, -M_PI_LOG10El),

    TEST_c_c (clog10, plus_infty, 0, plus_infty, 0.0),
    TEST_c_c (clog10, plus_infty, 1, plus_infty, 0.0),
    TEST_c_c (clog10, plus_infty, minus_zero, plus_infty, minus_zero),
    TEST_c_c (clog10, plus_infty, -1, plus_infty, minus_zero),

    TEST_c_c (clog10, plus_infty, qnan_value, plus_infty, qnan_value),
    TEST_c_c (clog10, minus_infty, qnan_value, plus_infty, qnan_value),

    TEST_c_c (clog10, qnan_value, plus_infty, plus_infty, qnan_value),
    TEST_c_c (clog10, qnan_value, minus_infty, plus_infty, qnan_value),

    TEST_c_c (clog10, 0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog10, 3, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog10, minus_zero, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog10, -3, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (clog10, qnan_value, 0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog10, qnan_value, 5, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog10, qnan_value, minus_zero, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (clog10, qnan_value, -5, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (clog10, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (clog10, 0.75L, 1.25L, 0.163679467193165171449476605077428975L, 0.447486970040493067069984724340855636L),
    TEST_c_c (clog10, -2, -3, 0.556971676153418384603252578971164214L, -0.937554462986374708541507952140189646L),

    TEST_c_c (clog10, 0x1.fffffep+127L, 0x1.fffffep+127L, 38.68235441693561449174780668781319348761L, M_PI4_LOG10El),
    TEST_c_c (clog10, 0x1.fffffep+127L, 1.0L, 38.53183941910362389414093724045094697423L, 1.276276851248440096917018665609900318458e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (clog10, 0x1p-149L, 0x1p-149L, -44.70295435610120748924022586658721447508L, M_PI4_LOG10El),
    TEST_c_c (clog10, 0x1p-147L, 0x1p-147L, -44.10089436477324509881274807713822842154L, M_PI4_LOG10El),

#ifndef TEST_FLOAT
    TEST_c_c (clog10, 0x1.fffffffffffffp+1023L, 0x1.fffffffffffffp+1023L, 308.4052305577487344482591243175787477115L, M_PI4_LOG10El),
    TEST_c_c (clog10, 0x1.fffffffffffffp+1023L, 0x1p+1023L, 308.3031705664207720674749211936626341569L, 0.2013595981366865903254995612594728746470L),
    TEST_c_c (clog10, 0x1p-1074L, 0x1p-1074L, -323.1557003452838130619487034867432642357L, M_PI4_LOG10El),
    TEST_c_c (clog10, 0x1p-1073L, 0x1p-1073L, -322.8546703496198318667349645920187712089L, M_PI4_LOG10El),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (clog10, 0x1.fp+16383L, 0x1.fp+16383L, 4932.212175672014259683102930239951947672L, M_PI4_LOG10El),
    TEST_c_c (clog10, 0x1.fp+16383L, 0x1p+16383L, 4932.112944269463028900262609694408579449L, 0.2069271710841128115912940666587802677383L),
    TEST_c_c (clog10, 0x1p-16440L, 0x1p-16441L, -4948.884673709346821106688037612752099609L, 0.2013595981366865710389502301937289472543L),
#endif

    TEST_c_c (clog10, 0x1p-149L, 0x1.fp+127L, 38.51805116050395969095658815123105801479L, 0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1p-149L, 0x1.fp+127L, 38.51805116050395969095658815123105801479L, 0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, 0x1p-149L, -0x1.fp+127L, 38.51805116050395969095658815123105801479L, -0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1p-149L, -0x1.fp+127L, 38.51805116050395969095658815123105801479L, -0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1.fp+127L, 0x1p-149L, 38.51805116050395969095658815123105801479L, 1.364376353841841347485783625431355770210L),
    TEST_c_c (clog10, -0x1.fp+127L, -0x1p-149L, 38.51805116050395969095658815123105801479L, -1.364376353841841347485783625431355770210L),
#ifdef TEST_FLOAT
    TEST_c_c (clog10, 0x1.fp+127L, 0x1p-149L, 38.51805116050395969095658815123105801479L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (clog10, 0x1.fp+127L, -0x1p-149L, 38.51805116050395969095658815123105801479L, minus_zero, UNDERFLOW_EXCEPTION),
#endif

#ifndef TEST_FLOAT
    TEST_c_c (clog10, 0x1p-1074L, 0x1.fp+1023L, 308.2409272754311106024666378243768099991L, 0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1p-1074L, 0x1.fp+1023L, 308.2409272754311106024666378243768099991L, 0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, 0x1p-1074L, -0x1.fp+1023L, 308.2409272754311106024666378243768099991L, -0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1p-1074L, -0x1.fp+1023L, 308.2409272754311106024666378243768099991L, -0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1.fp+1023L, 0x1p-1074L, 308.2409272754311106024666378243768099991L, 1.364376353841841347485783625431355770210L),
    TEST_c_c (clog10, -0x1.fp+1023L, -0x1p-1074L, 308.2409272754311106024666378243768099991L, -1.364376353841841347485783625431355770210L),
#endif
#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MAX_EXP == 1024)
    TEST_c_c (clog10, 0x1.fp+1023L, 0x1p-1074L, 308.2409272754311106024666378243768099991L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (clog10, 0x1.fp+1023L, -0x1p-1074L, 308.2409272754311106024666378243768099991L, minus_zero, UNDERFLOW_EXCEPTION),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (clog10, 0x1p-16445L, 0x1.fp+16383L, 4932.061660674182269085496060792589701158L, 0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1p-16445L, 0x1.fp+16383L, 4932.061660674182269085496060792589701158L, 0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, 0x1p-16445L, -0x1.fp+16383L, 4932.061660674182269085496060792589701158L, -0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1p-16445L, -0x1.fp+16383L, 4932.061660674182269085496060792589701158L, -0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1.fp+16383L, 0x1p-16445L, 4932.061660674182269085496060792589701158L, 1.364376353841841347485783625431355770210L),
    TEST_c_c (clog10, -0x1.fp+16383L, -0x1p-16445L, 4932.061660674182269085496060792589701158L, -1.364376353841841347485783625431355770210L),
    TEST_c_c (clog10, 0x1.fp+16383L, 0x1p-16445L, 4932.061660674182269085496060792589701158L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (clog10, 0x1.fp+16383L, -0x1p-16445L, 4932.061660674182269085496060792589701158L, minus_zero, UNDERFLOW_EXCEPTION),
# if LDBL_MANT_DIG >= 113
    TEST_c_c (clog10, 0x1p-16494L, 0x1.fp+16383L, 4932.061660674182269085496060792589701158L, 0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1p-16494L, 0x1.fp+16383L, 4932.061660674182269085496060792589701158L, 0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, 0x1p-16494L, -0x1.fp+16383L, 4932.061660674182269085496060792589701158L, -0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1p-16494L, -0x1.fp+16383L, 4932.061660674182269085496060792589701158L, -0.6821881769209206737428918127156778851051L),
    TEST_c_c (clog10, -0x1.fp+16383L, 0x1p-16494L, 4932.061660674182269085496060792589701158L, 1.364376353841841347485783625431355770210L),
    TEST_c_c (clog10, -0x1.fp+16383L, -0x1p-16494L, 4932.061660674182269085496060792589701158L, -1.364376353841841347485783625431355770210L),
    TEST_c_c (clog10, 0x1.fp+16383L, 0x1p-16494L, 4932.061660674182269085496060792589701158L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (clog10, 0x1.fp+16383L, -0x1p-16494L, 4932.061660674182269085496060792589701158L, minus_zero, UNDERFLOW_EXCEPTION),
# endif
#endif

    TEST_c_c (clog10, 1.0L, 0x1.234566p-10L, 2.680828048441605163181684680300513080769e-7L, 4.825491868832381486767558728169977751564e-4L),
    TEST_c_c (clog10, -1.0L, 0x1.234566p-20L, 2.556638434669064077889576526006849923281e-13L, 1.364375882602207106407956770293808181427L),
    TEST_c_c (clog10, 0x1.234566p-30L, 1.0L, 2.438200411482400072282924063740535840474e-19L, 6.821881764607257184291586401763604544928e-1L),
    TEST_c_c (clog10, -0x1.234566p-40L, -1.0L, 2.325249110681915353442924915876654139373e-25L, -6.821881769213700828789403802671540158935e-1L),
    TEST_c_c (clog10, 0x1.234566p-50L, 1.0L, 2.217530356103816369479108963807448194409e-31L, 6.821881769209202348667823902864283966959e-1L),
    TEST_c_c (clog10, 0x1.234566p-60L, 1.0L, 2.114801746467415208319767917450504756866e-37L, 6.821881769209206733143018621078368211515e-1L),
    TEST_c_c (clog10, 0x1p-61L, 1.0L, 4.084085680564517578238994467153626207224e-38L, 6.821881769209206735545466044044889962925e-1L),
    TEST_c_c (clog10, 0x1p-62L, 1.0L, 1.021021420141129394559748616788406551878e-38L, 6.821881769209206736487192085600834406988e-1L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (clog10, 0x1p-63L, 1.0L, 2.552553550352823486399371541971016379740e-39L, 6.821881769209206736958055106378806629019e-1L, UNDERFLOW_EXCEPTION_FLOAT),
#ifndef TEST_FLOAT
    TEST_c_c (clog10, 0x1p-509L, 1.0L, 7.730698388614835910296270976605350994446e-308L, 6.821881769209206737428918127156778851051e-1L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_c_c (clog10, 0x1p-510L, 1.0L, 1.932674597153708977574067744151337748612e-308L, 6.821881769209206737428918127156778851051e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (clog10, 0x1p-511L, 1.0L, 4.831686492884272443935169360378344371529e-309L, 6.821881769209206737428918127156778851051e-1L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (clog10, 0x1p-8189L, 1.0L, 1.168114274114528946314738738025008370069e-4931L, 6.821881769209206737428918127156778851051e-1L),
    TEST_c_c (clog10, 0x1p-8190L, 1.0L, 2.920285685286322365786846845062520925172e-4932L, 6.821881769209206737428918127156778851051e-1L, UNDERFLOW_EXCEPTION),
    TEST_c_c (clog10, 0x1p-8191L, 1.0L, 7.300714213215805914467117112656302312931e-4933L, 6.821881769209206737428918127156778851051e-1L, UNDERFLOW_EXCEPTION),
#endif

    TEST_c_c (clog10, 0x1.000566p0L, 0x1.234p-10L, 3.604093470239754109961125085078190708674e-5L, 4.824745078422174667425851670822596859720e-4L),
    TEST_c_c (clog10, 0x1.000566p0L, 0x1.234p-100L, 3.577293486783822178310971763308187385546e-5L, 3.897399639875661463735636919790792140598e-31L),
#ifndef TEST_FLOAT
    TEST_c_c (clog10, -0x1.0000000123456p0L, 0x1.2345678p-30L, 1.150487028947346337782682105935961875822e-10L, 1.364376353381646356131680448946397884147L),
    TEST_c_c (clog10, -0x1.0000000123456p0L, 0x1.2345678p-1000L, 1.150487026509145544402795327729455391948e-10L, 1.364376353841841347485783625431355770210L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (clog10, 0x1.00000000000000123456789abcp0L, 0x1.23456789p-60L, 4.285899851347756188767674032946882584784e-19L, 4.285899850759344225805480528847018395861e-19L),
    TEST_c_c (clog10, 0x1.00000000000000123456789abcp0L, 0x1.23456789p-1000L, 4.285899851347756186652871946325962330640e-19L, 4.611541215247321502041995872887317363241e-302L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
#endif

    TEST_c_c (clog10, 0x0.ffffffp0L, 0x0.ffffffp-100L, -2.588596909321764128428416045209904492216e-8L, 3.425979381266895667295625489912064603415e-31L),
#ifndef TEST_FLOAT
    TEST_c_c (clog10, 0x0.fffffffffffff8p0L, 0x0.fffffffffffff8p-1000L, -4.821637332766435821255375046554377090472e-17L, 4.053112396770095089737411317782466262176e-302L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (clog10, 0x0.ffffffffffffffffp0L, 0x0.ffffffffffffffffp-15000L, -2.354315103889861110220423157644627849164e-20L, 1.541165759405643564697852372112893034397e-4516L),
#endif

    TEST_c_c (clog10, 0x1a6p-10L, 0x3a5p-10L, -6.2126412844802358329771948751248003038444e-07L, 0.4977135139537443711784513409096950995985L),
    TEST_c_c (clog10, 0xf2p-10L, 0x3e3p-10L, 2.6921240173351112953324592659528481616879e-06L, 0.5785726025799636431142862788413361783862L),
    TEST_c_c (clog10, 0x4d4ep-15L, 0x6605p-15L, -7.0781945783414996953799915941870192015212e-09L, 0.4005747524909781155537088181659175147564L),
    TEST_c_c (clog10, 0x2818p-15L, 0x798fp-15L, 6.6737261053986614395049481326819059203910e-09L, 0.5438241985991753781478398141908629586460L),
    TEST_c_c (clog10, 0x9b57bp-20L, 0xcb7b4p-20L, -1.7182001068739620267773842120965071561416e-11L, 0.3990121149225253562859800593935899629087L),
    TEST_c_c (clog10, 0x2731p-20L, 0xfffd0p-20L, 1.9156943718715958194239364991329064049438e-11L, 0.6780326907904082601285090019969008967595L),
    TEST_c_c (clog10, 0x2ede88p-23L, 0x771c3fp-23L, -1.9440841725722970687903291200493082253766e-13L, 0.5193774116724956222518530053006822210323L),
    TEST_c_c (clog10, 0x11682p-23L, 0x7ffed1p-23L, 5.0916490233953865181284669870035717560498e-13L, 0.6784968969384861816694467029319146542069L),
    TEST_c_c (clog10, 0xa1f2c1p-24L, 0xc643aep-24L, -4.5516256421319921959681423447271490869664e-14L, 0.3847315790697197749315054516562206543710L),
    TEST_c_c (clog10, 0x659feap-24L, 0xeaf6f9p-24L, 1.6200701438094619117335617123525612051457e-14L, 0.5049027913635038013499728086604870749732L),
#ifndef TEST_FLOAT
    TEST_c_c (clog10, 0x4447d7175p-35L, 0x6c445e00ap-35L, -6.4375803621988389731799033530075237868110e-21L, 0.4378257977686804492768642780897650927167L),
    TEST_c_c (clog10, 0x2dd46725bp-35L, 0x7783a1284p-35L, 1.9312741086596516918394613098872836703188e-20L, 0.5231613813514771042838490538484014771862L),
    TEST_c_c (clog10, 0x164c74eea876p-45L, 0x16f393482f77p-45L, -1.3155760824064879362415202279780039150764e-26L, 0.3473590599762514228227328130640352044313L),
    TEST_c_c (clog10, 0xfe961079616p-45L, 0x1bc37e09e6d1p-45L, 2.3329549194675052736016290082882121135546e-26L, 0.4561756099441139182878993697611751382976L),
    TEST_c_c (clog10, 0xa4722f19346cp-51L, 0x7f9631c5e7f07p-51L, -2.6979587627476803379953050733225113494503e-30L, 0.6472785229986997177606324374555347813105L),
    TEST_c_c (clog10, 0x10673dd0f2481p-51L, 0x7ef1d17cefbd2p-51L, 1.3918041236396763648388478552321724382899e-29L, 0.6263795733790237053262025311642907438291L),
    TEST_c_c (clog10, 0x8ecbf810c4ae6p-52L, 0xd479468b09a37p-52L, -4.2289432987513243393180377141513840878196e-30L, 0.4252020027092323591068799049905597805296L),
    TEST_c_c (clog10, 0x5b06b680ea2ccp-52L, 0xef452b965da9fp-52L, 3.6079845358966994996207055940336690133424e-30L, 0.5243112258263349992771652393178033846555L),
    TEST_c_c (clog10, 0x659b70ab7971bp-53L, 0x1f5d111e08abecp-53L, -1.0893543813872082317104059174982092534059e-30L, 0.5954257879188711495921161433751775633232L),
    TEST_c_c (clog10, 0x15cfbd1990d1ffp-53L, 0x176a3973e09a9ap-53L, 4.4163015461643576961232672330852798804976e-31L, 0.3564851427422832755956993418877523303529L),
    TEST_c_c (clog10, 0x1367a310575591p-54L, 0x3cfcc0a0541f60p-54L, 2.2081507730821788480616336165447731164865e-32L, 0.5484039935757001196548030312819898864760L),
    TEST_c_c (clog10, 0x55cb6d0c83af5p-55L, 0x7fe33c0c7c4e90p-55L, -2.2583360179249556400630343805573865814771e-32L, 0.6639894257763289307423302343317622430835L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64
    TEST_c_c (clog10, 0x298c62cb546588a7p-63L, 0x7911b1dfcc4ecdaep-63L, -5.1816837072162316773907242302011632570857e-37L, 0.5386167838952956925896424154370364458140L),
    TEST_c_c (clog10, 0x4d9c37e2b5cb4533p-63L, 0x65c98be2385a042ep-63L, 2.7822833698845776001753149807484078521508e-37L, 0.3992725998539071066769046272515417679815L),
    TEST_c_c (clog10, 0x602fd5037c4792efp-64L, 0xed3e2086dcca80b8p-64L, -1.0146400362652473358437501879334790111898e-37L, 0.5149047982335273098246594109614460842099L),
    TEST_c_c (clog10, 0x6b10b4f3520217b6p-64L, 0xe8893cbb449253a1p-64L, 1.0529283395205396881397407610630442563938e-37L, 0.4947949395762683446121140513971996916447L),
    TEST_c_c (clog10, 0x81b7efa81fc35ad1p-65L, 0x1ef4b835f1c79d812p-65L, -4.3074341162203896332989394770760901408798e-39L, 0.5709443672155660428417571212549720987784L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_c_c (clog10, 0x3f96469050f650869c2p-75L, 0x6f16b2c9c8b05988335p-75L, -4.5643214291682663316715446865040356750881e-46L, 0.4564083863660793840592614609053162690362L),
    TEST_c_c (clog10, 0x3157fc1d73233e580c8p-75L, 0x761b52ccd435d7c7f5fp-75L, 5.8575458340992751256451490143468457830297e-44L, 0.5103174273246635294300470585396890237265L),
    TEST_c_c (clog10, 0x155f8afc4c48685bf63610p-85L, 0x17d0cf2652cdbeb1294e19p-85L, -2.0748709499710785084693619097712106753591e-50L, 0.3645447681189598740620098186365764884771L),
    TEST_c_c (clog10, 0x13836d58a13448d750b4b9p-85L, 0x195ca7bc3ab4f9161edbe6p-85L, 1.2333149003324592532859843519619084433953e-50L, 0.3973779298829931059309198145608711073016L),
    TEST_c_c (clog10, 0x1df515eb171a808b9e400266p-95L, 0x7c71eb0cd4688dfe98581c77p-95L, -1.5221162575729652613635150540947625639689e-57L, 0.5795934880811949230121092882659698986043L),
    TEST_c_c (clog10, 0xe33f66c9542ca25cc43c867p-95L, 0x7f35a68ebd3704a43c465864p-95L, 1.7850272475173865337808494725293124613817e-56L, 0.6338990862456906754888183278564382516852L),
    TEST_c_c (clog10, 0x6771f22c64ed551b857c128b4cp-105L, 0x1f570e7a13cc3cf2f44fd793ea1p-105L, -6.2023045024810589256360494043570293518879e-63L, 0.5938345819561308555003145899438513900776L),
    TEST_c_c (clog10, 0x15d8ab6ed05ca514086ac3a1e84p-105L, 0x1761e480aa094c0b10b34b09ce9p-105L, 4.3548095442952115860848857519953610343042e-63L, 0.3558376234889641500775150477035448866763L),
    TEST_c_c (clog10, 0x187190c1a334497bdbde5a95f48p-106L, 0x3b25f08062d0a095c4cfbbc338dp-106L, -7.5879257211204444302994221436282805900756e-64L, 0.5119945461708707332160859198685423099187L),
    TEST_c_c (clog10, 0x6241ef0da53f539f02fad67dabp-106L, 0x3fb46641182f7efd9caa769dac0p-106L, 1.8804859395820231849002915747252695375405e-63L, 0.6404513901551516189871978418046651877394L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_c_c (clog10, 0x3e1d0a105ac4ebeacd9c6952d34cp-112L, 0xf859b3d1b06d005dcbb5516d5479p-112L, -5.0742964549782184008668435276046798273476e-67L, 0.5757527761596220360985719127090110408283L),
    TEST_c_c (clog10, 0x47017a2e36807acb1e5214b209dep-112L, 0xf5f4a550c9d75e3bb1839d865f0dp-112L, 6.5482587585671294601662599808612773010057e-66L, 0.5601289501766423782280643144987875760229L),
    TEST_c_c (clog10, 0x148f818cb7a9258fca942ade2a0cap-113L, 0x18854a34780b8333ec53310ad7001p-113L, -3.1210950417524756037077807411854181477733e-67L, 0.3791463562379872585396164879981280044658L),
    TEST_c_c (clog10, 0xfd95243681c055c2632286921092p-113L, 0x1bccabcd29ca2152860ec29e34ef7p-113L, 2.8774482675253468630312378575186855052697e-66L, 0.4571561610046221605554903008571429975493L),
    TEST_c_c (clog10, 0xdb85c467ee2aadd5f425fe0f4b8dp-114L, 0x3e83162a0f95f1dcbf97dddf410eap-114L, 1.9985076315737626043096596036300177494613e-67L, 0.5883569274304683249184005177865521205198L),
    TEST_c_c (clog10, 0x1415bcaf2105940d49a636e98ae59p-115L, 0x7e6a150adfcd1b0921d44b31f40f4p-115L, 1.1288799405048268615023706955013387413519e-67L, 0.6137587762850841972073301550420510507903L),
#endif
    END_DATA (clog10)
  };

static void
clog10_test (void)
{
  errno = 0;
  FUNC(clog10) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (clog10);
  RUN_TEST_LOOP_c_c (clog10, clog10_test_data, );
  END (clog10, complex);
}


static const struct test_c_c_data conj_test_data[] =
  {
    START_DATA (conj),
    TEST_c_c (conj, 0.0, 0.0, 0.0, minus_zero),
    TEST_c_c (conj, 0.0, minus_zero, 0.0, 0.0),
    TEST_c_c (conj, qnan_value, qnan_value, qnan_value, qnan_value),
    TEST_c_c (conj, plus_infty, minus_infty, plus_infty, plus_infty),
    TEST_c_c (conj, plus_infty, plus_infty, plus_infty, minus_infty),
    TEST_c_c (conj, 1.0, 2.0, 1.0, -2.0),
    TEST_c_c (conj, 3.0, -4.0, 3.0, 4.0),
    END_DATA (conj)
  };

static void
conj_test (void)
{
  START (conj);
  RUN_TEST_LOOP_c_c (conj, conj_test_data, );
  END (conj, complex);
}


static const struct test_ff_f_data copysign_test_data[] =
  {
    START_DATA (copysign),
    TEST_ff_f (copysign, 0, 4, 0),
    TEST_ff_f (copysign, 0, -4, minus_zero),
    TEST_ff_f (copysign, minus_zero, 4, 0),
    TEST_ff_f (copysign, minus_zero, -4, minus_zero),

    TEST_ff_f (copysign, plus_infty, 0, plus_infty),
    TEST_ff_f (copysign, plus_infty, minus_zero, minus_infty),
    TEST_ff_f (copysign, minus_infty, 0, plus_infty),
    TEST_ff_f (copysign, minus_infty, minus_zero, minus_infty),

    TEST_ff_f (copysign, 0, plus_infty, 0),
    TEST_ff_f (copysign, 0, minus_zero, minus_zero),
    TEST_ff_f (copysign, minus_zero, plus_infty, 0),
    TEST_ff_f (copysign, minus_zero, minus_zero, minus_zero),

    /* XXX More correctly we would have to check the sign of the NaN.  */
    TEST_ff_f (copysign, qnan_value, 0, qnan_value),
    TEST_ff_f (copysign, qnan_value, minus_zero, qnan_value),
    TEST_ff_f (copysign, -qnan_value, 0, qnan_value),
    TEST_ff_f (copysign, -qnan_value, minus_zero, qnan_value),
    END_DATA (copysign)
  };

static void
copysign_test (void)
{
  START (copysign);
  RUN_TEST_LOOP_ff_f (copysign, copysign_test_data, );
  END (copysign);
}


static const struct test_f_f_data cos_test_data[] =
  {
    START_DATA (cos),
    TEST_f_f (cos, 0, 1),
    TEST_f_f (cos, minus_zero, 1),
    TEST_f_f (cos, plus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (cos, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (cos, qnan_value, qnan_value, ERRNO_UNCHANGED),

    TEST_f_f (cos, M_PI_6l * 2.0, 0.5),
    TEST_f_f (cos, M_PI_6l * 4.0, -0.5),

    /* The value of M_PI_2l is never exactly PI/2, and therefore the
       answer is never exactly zero. The answer is equal to the error
       in rounding PI/2 for the type used.  Thus the answer is unique
       to each type.  */
#ifdef TEST_FLOAT
    /* 32-bit float.  */
    TEST_f_f (cos, M_PI_2l, -0x1.777a5cp-25L),
#endif
#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MANT_DIG == 53)
    /* 64-bit double or 64-bit long double.  */
    TEST_f_f (cos, M_PI_2l, 0x1.1a62633145c07p-54L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG == 64
    /* 96-bit long double.  */
    TEST_f_f (cos, M_PI_2l, -0xe.ce675d1fc8f8cbbp-69L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG == 106
    /* 128-bit IBM long double.  */
    TEST_f_f (cos, M_PI_2l, 0x1.c1cd129024e088a67cc74020bcp-107L),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG == 113
    /* 128-bit long double.  */
    TEST_f_f (cos, M_PI_2l, 0x1.cd129024e088a67cc74020bbea64p-115L),
#endif

    TEST_f_f (cos, 0.75L, 0.731688868873820886311838753000084544L),

    TEST_f_f (cos, 0x1p65, 0.99888622066058013610642172179340364209972L),
    TEST_f_f (cos, -0x1p65, 0.99888622066058013610642172179340364209972L),

#ifdef TEST_DOUBLE
    TEST_f_f (cos, 0.80190127184058835, 0.69534156199418473),
#endif

    TEST_f_f (cos, 0x1.442f74p+15, 2.4407839902314016628485779006274989801517e-06L),

#ifndef TEST_FLOAT
    TEST_f_f (cos, 1e22, 0.5232147853951389454975944733847094921409L),
    TEST_f_f (cos, 0x1p1023, -0.826369834614147994500785680811743734805L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_f_f (cos, 0x1p16383L, 0.9210843909921906206874509522505756251609L),
#endif

    TEST_f_f (cos, 0x1p+120, -9.25879022854837867303861764107414946730833e-01L),
    TEST_f_f (cos, 0x1p+127, 7.81914638714960072263910298466369236613162e-01L),
    TEST_f_f (cos, 0x1.fffff8p+127, 9.98819362551949040703862043664101081064641e-01L),
    TEST_f_f (cos, 0x1.fffffep+127, 8.53021039830304158051791467692161107353094e-01L),
    TEST_f_f (cos, 0x1p+50, 8.68095904660550604334592502063501320395739e-01L),
    TEST_f_f (cos, 0x1p+28, -1.65568979490578758865468278195361551113358e-01L),
    END_DATA (cos)
  };

static void
cos_test (void)
{
  errno = 0;
  FUNC(cos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cos);
  RUN_TEST_LOOP_f_f (cos, cos_test_data, );
  END (cos);
}


static const struct test_f_f_data cos_tonearest_test_data[] =
  {
    START_DATA (cos_tonearest),
    TEST_f_f (cos, 1, 0.5403023058681397174009366074429766037323L),
    TEST_f_f (cos, 2, -0.4161468365471423869975682295007621897660L),
    TEST_f_f (cos, 3, -0.9899924966004454572715727947312613023937L),
    TEST_f_f (cos, 4, -0.6536436208636119146391681830977503814241L),
    TEST_f_f (cos, 5, 0.2836621854632262644666391715135573083344L),
    TEST_f_f (cos, 6, 0.9601702866503660205456522979229244054519L),
    TEST_f_f (cos, 7, 0.7539022543433046381411975217191820122183L),
    TEST_f_f (cos, 8, -0.1455000338086135258688413818311946826093L),
    TEST_f_f (cos, 9, -0.9111302618846769883682947111811653112463L),
    TEST_f_f (cos, 10, -0.8390715290764524522588639478240648345199L),
    END_DATA (cos_tonearest)
  };

static void
cos_test_tonearest (void)
{
  errno = 0;
  FUNC(cos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cos_tonearest);
  RUN_TEST_LOOP_f_f (cos, cos_tonearest_test_data, FE_TONEAREST);
  END (cos_tonearest);
}


static const struct test_f_f_data cos_towardzero_test_data[] =
  {
    START_DATA (cos_towardzero),
    TEST_f_f (cos, 1, 0.5403023058681397174009366074429766037323L),
    TEST_f_f (cos, 2, -0.4161468365471423869975682295007621897660L),
    TEST_f_f (cos, 3, -0.9899924966004454572715727947312613023937L),
    TEST_f_f (cos, 4, -0.6536436208636119146391681830977503814241L),
    TEST_f_f (cos, 5, 0.2836621854632262644666391715135573083344L),
    TEST_f_f (cos, 6, 0.9601702866503660205456522979229244054519L),
    TEST_f_f (cos, 7, 0.7539022543433046381411975217191820122183L),
    TEST_f_f (cos, 8, -0.1455000338086135258688413818311946826093L),
    TEST_f_f (cos, 9, -0.9111302618846769883682947111811653112463L),
    TEST_f_f (cos, 10, -0.8390715290764524522588639478240648345199L),
    END_DATA (cos_towardzero)
  };

static void
cos_test_towardzero (void)
{
  errno = 0;
  FUNC(cos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cos_towardzero);
  RUN_TEST_LOOP_f_f (cos, cos_towardzero_test_data, FE_TOWARDZERO);
  END (cos_towardzero);
}


static const struct test_f_f_data cos_downward_test_data[] =
  {
    START_DATA (cos_downward),
    TEST_f_f (cos, 1, 0.5403023058681397174009366074429766037323L),
    TEST_f_f (cos, 2, -0.4161468365471423869975682295007621897660L),
    TEST_f_f (cos, 3, -0.9899924966004454572715727947312613023937L),
    TEST_f_f (cos, 4, -0.6536436208636119146391681830977503814241L),
    TEST_f_f (cos, 5, 0.2836621854632262644666391715135573083344L),
    TEST_f_f (cos, 6, 0.9601702866503660205456522979229244054519L),
    TEST_f_f (cos, 7, 0.7539022543433046381411975217191820122183L),
    TEST_f_f (cos, 8, -0.1455000338086135258688413818311946826093L),
    TEST_f_f (cos, 9, -0.9111302618846769883682947111811653112463L),
    TEST_f_f (cos, 10, -0.8390715290764524522588639478240648345199L),
    END_DATA (cos_downward)
  };

static void
cos_test_downward (void)
{
  errno = 0;
  FUNC(cos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cos_downward);
  RUN_TEST_LOOP_f_f (cos, cos_downward_test_data, FE_DOWNWARD);
  END (cos_downward);
}


static const struct test_f_f_data cos_upward_test_data[] =
  {
    START_DATA (cos_upward),
    TEST_f_f (cos, 1, 0.5403023058681397174009366074429766037323L),
    TEST_f_f (cos, 2, -0.4161468365471423869975682295007621897660L),
    TEST_f_f (cos, 3, -0.9899924966004454572715727947312613023937L),
    TEST_f_f (cos, 4, -0.6536436208636119146391681830977503814241L),
    TEST_f_f (cos, 5, 0.2836621854632262644666391715135573083344L),
    TEST_f_f (cos, 6, 0.9601702866503660205456522979229244054519L),
    TEST_f_f (cos, 7, 0.7539022543433046381411975217191820122183L),
    TEST_f_f (cos, 8, -0.1455000338086135258688413818311946826093L),
    TEST_f_f (cos, 9, -0.9111302618846769883682947111811653112463L),
    TEST_f_f (cos, 10, -0.8390715290764524522588639478240648345199L),
    END_DATA (cos_upward)
  };

static void
cos_test_upward (void)
{
  errno = 0;
  FUNC(cos) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cos_upward);
  RUN_TEST_LOOP_f_f (cos, cos_upward_test_data, FE_UPWARD);
  END (cos_upward);
}


static const struct test_f_f_data cosh_test_data[] =
  {
    START_DATA (cosh),
    TEST_f_f (cosh, 0, 1),
    TEST_f_f (cosh, minus_zero, 1),

#ifndef TEST_INLINE
    TEST_f_f (cosh, plus_infty, plus_infty),
    TEST_f_f (cosh, minus_infty, plus_infty),
#endif
    TEST_f_f (cosh, qnan_value, qnan_value),

    TEST_f_f (cosh, 0.75L, 1.29468328467684468784170818539018176L),

#ifndef TEST_FLOAT
    TEST_f_f (cosh, 709.8893558127259666434838436543941497802734375L, 9.9999998999995070652573675944761818416035e+307L),
    TEST_f_f (cosh, -709.8893558127259666434838436543941497802734375L, 9.9999998999995070652573675944761818416035e+307L),
#endif
    END_DATA (cosh)
  };

static void
cosh_test (void)
{
  errno = 0;
  FUNC(cosh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cosh);
  RUN_TEST_LOOP_f_f (cosh, cosh_test_data, );
  END (cosh);
}


static const struct test_f_f_data cosh_tonearest_test_data[] =
  {
    START_DATA (cosh_tonearest),
    TEST_f_f (cosh, 22, 1792456423.065795780980053377632656584997L),
    TEST_f_f (cosh, 23, 4872401723.124451300068625740569997090344L),
    TEST_f_f (cosh, 24, 13244561064.92173614708845674912733665919L),
    END_DATA (cosh_tonearest)
  };

static void
cosh_test_tonearest (void)
{
  errno = 0;
  FUNC(cosh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cosh_tonearest);
  RUN_TEST_LOOP_f_f (cosh, cosh_tonearest_test_data, FE_TONEAREST);
  END (cosh_tonearest);
}


static const struct test_f_f_data cosh_towardzero_test_data[] =
  {
    START_DATA (cosh_towardzero),
    TEST_f_f (cosh, 22, 1792456423.065795780980053377632656584997L),
    TEST_f_f (cosh, 23, 4872401723.124451300068625740569997090344L),
    TEST_f_f (cosh, 24, 13244561064.92173614708845674912733665919L),
    END_DATA (cosh_towardzero)
  };

static void
cosh_test_towardzero (void)
{
  errno = 0;
  FUNC(cosh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cosh_towardzero);
  RUN_TEST_LOOP_f_f (cosh, cosh_towardzero_test_data, FE_TOWARDZERO);
  END (cosh_towardzero);
}


static const struct test_f_f_data cosh_downward_test_data[] =
  {
    START_DATA (cosh_downward),
    TEST_f_f (cosh, 22, 1792456423.065795780980053377632656584997L),
    TEST_f_f (cosh, 23, 4872401723.124451300068625740569997090344L),
    TEST_f_f (cosh, 24, 13244561064.92173614708845674912733665919L),
    END_DATA (cosh_downward)
  };

static void
cosh_test_downward (void)
{
  errno = 0;
  FUNC(cosh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cosh_downward);
  RUN_TEST_LOOP_f_f (cosh, cosh_downward_test_data, FE_DOWNWARD);
  END (cosh_downward);
}


static const struct test_f_f_data cosh_upward_test_data[] =
  {
    START_DATA (cosh_upward),
    TEST_f_f (cosh, 22, 1792456423.065795780980053377632656584997L),
    TEST_f_f (cosh, 23, 4872401723.124451300068625740569997090344L),
    TEST_f_f (cosh, 24, 13244561064.92173614708845674912733665919L),
    END_DATA (cosh_upward)
  };

static void
cosh_test_upward (void)
{
  errno = 0;
  FUNC(cosh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (cosh_upward);
  RUN_TEST_LOOP_f_f (cosh, cosh_upward_test_data, FE_UPWARD);
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

  TEST_cc_c (cpow, qnan_value, qnan_value, qnan_value, qnan_value, qnan_value, qnan_value);

  TEST_cc_c (cpow, 0.75L, 1.25L, 0.75L, 1.25L, 0.117506293914473555420279832210420483L, 0.346552747708338676483025352060418001L);
  TEST_cc_c (cpow, 0.75L, 1.25L, 1.0L, 1.0L, 0.0846958290317209430433805274189191353L, 0.513285749182902449043287190519090481L);
  TEST_cc_c (cpow, 0.75L, 1.25L, 1.0L, 0.0L, 0.75L, 1.25L);
  TEST_cc_c (cpow, 0.75L, 1.25L, 0.0L, 1.0L, 0.331825439177608832276067945276730566L, 0.131338600281188544930936345230903032L);

  END (cpow, complex);
}


static const struct test_c_c_data cproj_test_data[] =
  {
    START_DATA (cproj),
    TEST_c_c (cproj, 0.0, 0.0, 0.0, 0.0),
    TEST_c_c (cproj, minus_zero, minus_zero, minus_zero, minus_zero),
    TEST_c_c (cproj, 0.0, minus_zero, 0.0, minus_zero),
    TEST_c_c (cproj, minus_zero, 0.0, minus_zero, 0.0),

    TEST_c_c (cproj, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (cproj, plus_infty, plus_infty, plus_infty, 0.0),
    TEST_c_c (cproj, plus_infty, minus_infty, plus_infty, minus_zero),
    TEST_c_c (cproj, minus_infty, plus_infty, plus_infty, 0.0),
    TEST_c_c (cproj, minus_infty, minus_infty, plus_infty, minus_zero),

    TEST_c_c (cproj, 1.0, 0.0, 1.0, 0.0),
    TEST_c_c (cproj, 2.0, 3.0, 2.0, 3.0),
    END_DATA (cproj)
  };

static void
cproj_test (void)
{
  START (cproj);
  RUN_TEST_LOOP_c_c (cproj, cproj_test_data, );
  END (cproj, complex);
}


static const struct test_c_f_data creal_test_data[] =
  {
    START_DATA (creal);
    TEST_c_f (creal, 0.0, 1.0, 0.0),
    TEST_c_f (creal, minus_zero, 1.0, minus_zero),
    TEST_c_f (creal, qnan_value, 1.0, qnan_value),
    TEST_c_f (creal, qnan_value, qnan_value, qnan_value),
    TEST_c_f (creal, plus_infty, 1.0, plus_infty),
    TEST_c_f (creal, minus_infty, 1.0, minus_infty),
    TEST_c_f (creal, 2.0, 3.0, 2.0),
    END_DATA (creal);
  };

static void
creal_test (void)
{
  START (creal);
  RUN_TEST_LOOP_c_f (creal, creal_test_data, );
  END (creal);
}

static const struct test_c_c_data csin_test_data[] =
  {
    START_DATA (csin),
    TEST_c_c (csin, 0.0, 0.0, 0.0, 0.0),
    TEST_c_c (csin, minus_zero, 0.0, minus_zero, 0.0),
    TEST_c_c (csin, 0.0, minus_zero, 0, minus_zero),
    TEST_c_c (csin, minus_zero, minus_zero, minus_zero, minus_zero),

    TEST_c_c (csin, 0.0, plus_infty, 0.0, plus_infty),
    TEST_c_c (csin, minus_zero, plus_infty, minus_zero, plus_infty),
    TEST_c_c (csin, 0.0, minus_infty, 0.0, minus_infty),
    TEST_c_c (csin, minus_zero, minus_infty, minus_zero, minus_infty),

    TEST_c_c (csin, plus_infty, 0.0, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csin, minus_infty, 0.0, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csin, plus_infty, minus_zero, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csin, minus_infty, minus_zero, qnan_value, 0.0, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),

    TEST_c_c (csin, plus_infty, plus_infty, qnan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csin, minus_infty, plus_infty, qnan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csin, plus_infty, minus_infty, qnan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csin, minus_infty, minus_infty, qnan_value, plus_infty, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),

    TEST_c_c (csin, plus_infty, 6.75, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (csin, plus_infty, -6.75, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (csin, minus_infty, 6.75, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (csin, minus_infty, -6.75,  qnan_value, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (csin, 4.625, plus_infty, minus_infty, minus_infty),
    TEST_c_c (csin, 4.625, minus_infty, minus_infty, plus_infty),
    TEST_c_c (csin, -4.625, plus_infty, plus_infty, minus_infty),
    TEST_c_c (csin, -4.625, minus_infty, plus_infty, plus_infty),

    TEST_c_c (csin, qnan_value, 0.0, qnan_value, 0.0, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csin, qnan_value, minus_zero, qnan_value, 0.0, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (csin, qnan_value, plus_infty, qnan_value, plus_infty, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csin, qnan_value, minus_infty, qnan_value, plus_infty, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (csin, qnan_value, 9.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csin, qnan_value, -9.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (csin, 0.0, qnan_value, 0.0, qnan_value),
    TEST_c_c (csin, minus_zero, qnan_value, minus_zero, qnan_value),

    TEST_c_c (csin, 10.0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csin, qnan_value, -10.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (csin, plus_infty, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csin, minus_infty, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (csin, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (csin, 0.75L, 1.25L, 1.28722291002649188575873510790565441L, 1.17210635989270256101081285116138863L),
    TEST_c_c (csin, -2, -3, -9.15449914691142957346729954460983256L, 4.16890695996656435075481305885375484L),

    TEST_c_c (csin, 0.75, 89.5, 2.522786001038096774676288412995370563339e38L, 2.708024460708609732016532185663087200560e38L),
    TEST_c_c (csin, 0.75, -89.5, 2.522786001038096774676288412995370563339e38L, -2.708024460708609732016532185663087200560e38L),
    TEST_c_c (csin, -0.75, 89.5, -2.522786001038096774676288412995370563339e38L, 2.708024460708609732016532185663087200560e38L),
    TEST_c_c (csin, -0.75, -89.5, -2.522786001038096774676288412995370563339e38L, -2.708024460708609732016532185663087200560e38L),

#ifndef TEST_FLOAT
    TEST_c_c (csin, 0.75, 710.5, 1.255317763348154410745082950806112487736e308L, 1.347490911916428129246890157395342279438e308L),
    TEST_c_c (csin, 0.75, -710.5, 1.255317763348154410745082950806112487736e308L, -1.347490911916428129246890157395342279438e308L),
    TEST_c_c (csin, -0.75, 710.5, -1.255317763348154410745082950806112487736e308L, 1.347490911916428129246890157395342279438e308L),
    TEST_c_c (csin, -0.75, -710.5, -1.255317763348154410745082950806112487736e308L, -1.347490911916428129246890157395342279438e308L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (csin, 0.75, 11357.25, 8.385498349388321535962327491346664141020e4931L, 9.001213196851067077465606717616495588201e4931L),
    TEST_c_c (csin, 0.75, -11357.25, 8.385498349388321535962327491346664141020e4931L, -9.001213196851067077465606717616495588201e4931L),
    TEST_c_c (csin, -0.75, 11357.25, -8.385498349388321535962327491346664141020e4931L, 9.001213196851067077465606717616495588201e4931L),
    TEST_c_c (csin, -0.75, -11357.25, -8.385498349388321535962327491346664141020e4931L, -9.001213196851067077465606717616495588201e4931L),
#endif

#ifdef TEST_FLOAT
    TEST_c_c (csin, 0x1p-149, 180, 1.043535896672617552965983803453927655332e33L, plus_infty, OVERFLOW_EXCEPTION),
#endif

#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MAX_EXP == 1024)
    TEST_c_c (csin, 0x1p-1074, 1440, 5.981479269486130556466515778180916082415e301L, plus_infty, OVERFLOW_EXCEPTION),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (csin, 0x1p-16434L, 22730, 1.217853148905605987081057582351152052687e4924L, plus_infty, OVERFLOW_EXCEPTION),
#endif

    TEST_c_c (csin, min_subnorm_value, min_value, min_subnorm_value, min_value, UNDERFLOW_EXCEPTION),
    TEST_c_c (csin, min_value, min_subnorm_value, min_value, min_subnorm_value, UNDERFLOW_EXCEPTION),
    END_DATA (csin)
  };

static void
csin_test (void)
{
  errno = 0;
  FUNC(csin) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (csin);
  RUN_TEST_LOOP_c_c (csin, csin_test_data, );
  END (csin, complex);
}


static const struct test_c_c_data csinh_test_data[] =
  {
    START_DATA (csinh),
    TEST_c_c (csinh, 0.0, 0.0, 0.0, 0.0),
    TEST_c_c (csinh, minus_zero, 0.0, minus_zero, 0.0),
    TEST_c_c (csinh, 0.0, minus_zero, 0.0, minus_zero),
    TEST_c_c (csinh, minus_zero, minus_zero, minus_zero, minus_zero),

    TEST_c_c (csinh, 0.0, plus_infty, 0.0, qnan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csinh, minus_zero, plus_infty, 0.0, qnan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csinh, 0.0, minus_infty, 0.0, qnan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csinh, minus_zero, minus_infty, 0.0, qnan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),

    TEST_c_c (csinh, plus_infty, 0.0, plus_infty, 0.0),
    TEST_c_c (csinh, minus_infty, 0.0, minus_infty, 0.0),
    TEST_c_c (csinh, plus_infty, minus_zero, plus_infty, minus_zero),
    TEST_c_c (csinh, minus_infty, minus_zero, minus_infty, minus_zero),

    TEST_c_c (csinh, plus_infty, plus_infty, plus_infty, qnan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csinh, minus_infty, plus_infty, plus_infty, qnan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csinh, plus_infty, minus_infty, plus_infty, qnan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csinh, minus_infty, minus_infty, plus_infty, qnan_value, INVALID_EXCEPTION|IGNORE_ZERO_INF_SIGN),

    TEST_c_c (csinh, plus_infty, 4.625, minus_infty, minus_infty),
    TEST_c_c (csinh, minus_infty, 4.625, plus_infty, minus_infty),
    TEST_c_c (csinh, plus_infty, -4.625, minus_infty, plus_infty),
    TEST_c_c (csinh, minus_infty, -4.625, plus_infty, plus_infty),

    TEST_c_c (csinh, 6.75, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (csinh, -6.75, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (csinh, 6.75, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (csinh, -6.75, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (csinh, 0.0, qnan_value, 0.0, qnan_value, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csinh, minus_zero, qnan_value, 0.0, qnan_value, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (csinh, plus_infty, qnan_value, plus_infty, qnan_value, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (csinh, minus_infty, qnan_value, plus_infty, qnan_value, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (csinh, 9.0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csinh, -9.0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (csinh, qnan_value, 0.0, qnan_value, 0.0),
    TEST_c_c (csinh, qnan_value, minus_zero, qnan_value, minus_zero),

    TEST_c_c (csinh, qnan_value, 10.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csinh, qnan_value, -10.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (csinh, qnan_value, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csinh, qnan_value, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (csinh, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (csinh, 0.75L, 1.25L, 0.259294854551162779153349830618433028L, 1.22863452409509552219214606515777594L),
    TEST_c_c (csinh, -2, -3, 3.59056458998577995201256544779481679L, -0.530921086248519805267040090660676560L),

    TEST_c_c (csinh, 89.5, 0.75, 2.708024460708609732016532185663087200560e38L, 2.522786001038096774676288412995370563339e38L),
    TEST_c_c (csinh, -89.5, 0.75, -2.708024460708609732016532185663087200560e38L, 2.522786001038096774676288412995370563339e38L),
    TEST_c_c (csinh, 89.5, -0.75, 2.708024460708609732016532185663087200560e38L, -2.522786001038096774676288412995370563339e38L),
    TEST_c_c (csinh, -89.5, -0.75, -2.708024460708609732016532185663087200560e38L, -2.522786001038096774676288412995370563339e38L),

#ifndef TEST_FLOAT
    TEST_c_c (csinh, 710.5, 0.75, 1.347490911916428129246890157395342279438e308L, 1.255317763348154410745082950806112487736e308L),
    TEST_c_c (csinh, -710.5, 0.75, -1.347490911916428129246890157395342279438e308L, 1.255317763348154410745082950806112487736e308L),
    TEST_c_c (csinh, 710.5, -0.75, 1.347490911916428129246890157395342279438e308L, -1.255317763348154410745082950806112487736e308L),
    TEST_c_c (csinh, -710.5, -0.75, -1.347490911916428129246890157395342279438e308L, -1.255317763348154410745082950806112487736e308L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (csinh, 11357.25, 0.75, 9.001213196851067077465606717616495588201e4931L, 8.385498349388321535962327491346664141020e4931L),
    TEST_c_c (csinh, -11357.25, 0.75, -9.001213196851067077465606717616495588201e4931L, 8.385498349388321535962327491346664141020e4931L),
    TEST_c_c (csinh, 11357.25, -0.75, 9.001213196851067077465606717616495588201e4931L, -8.385498349388321535962327491346664141020e4931L),
    TEST_c_c (csinh, -11357.25, -0.75, -9.001213196851067077465606717616495588201e4931L, -8.385498349388321535962327491346664141020e4931L),
#endif

#ifdef TEST_FLOAT
    TEST_c_c (csinh, 180, 0x1p-149, plus_infty, 1.043535896672617552965983803453927655332e33L, OVERFLOW_EXCEPTION),
#endif

#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MAX_EXP == 1024)
    TEST_c_c (csinh, 1440, 0x1p-1074, plus_infty, 5.981479269486130556466515778180916082415e301L, OVERFLOW_EXCEPTION),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (csinh, 22730, 0x1p-16434L, plus_infty, 1.217853148905605987081057582351152052687e4924L, OVERFLOW_EXCEPTION),
#endif

    TEST_c_c (csinh, min_subnorm_value, min_value, min_subnorm_value, min_value, UNDERFLOW_EXCEPTION),
    TEST_c_c (csinh, min_value, min_subnorm_value, min_value, min_subnorm_value, UNDERFLOW_EXCEPTION),
    END_DATA (csinh)
  };

static void
csinh_test (void)
{
  errno = 0;
  FUNC(csinh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (csinh);
  RUN_TEST_LOOP_c_c (csinh, csinh_test_data, );
  END (csinh, complex);
}


static const struct test_c_c_data csqrt_test_data[] =
  {
    START_DATA (csqrt),
    TEST_c_c (csqrt, 0, 0, 0.0, 0.0),
    TEST_c_c (csqrt, 0, minus_zero, 0, minus_zero),
    TEST_c_c (csqrt, minus_zero, 0, 0.0, 0.0),
    TEST_c_c (csqrt, minus_zero, minus_zero, 0.0, minus_zero),

    TEST_c_c (csqrt, minus_infty, 0, 0.0, plus_infty),
    TEST_c_c (csqrt, minus_infty, 6, 0.0, plus_infty),
    TEST_c_c (csqrt, minus_infty, minus_zero, 0.0, minus_infty),
    TEST_c_c (csqrt, minus_infty, -6, 0.0, minus_infty),

    TEST_c_c (csqrt, plus_infty, 0, plus_infty, 0.0),
    TEST_c_c (csqrt, plus_infty, 6, plus_infty, 0.0),
    TEST_c_c (csqrt, plus_infty, minus_zero, plus_infty, minus_zero),
    TEST_c_c (csqrt, plus_infty, -6, plus_infty, minus_zero),

    TEST_c_c (csqrt, 0, plus_infty, plus_infty, plus_infty),
    TEST_c_c (csqrt, 4, plus_infty, plus_infty, plus_infty),
    TEST_c_c (csqrt, plus_infty, plus_infty, plus_infty, plus_infty),
    TEST_c_c (csqrt, minus_zero, plus_infty, plus_infty, plus_infty),
    TEST_c_c (csqrt, -4, plus_infty, plus_infty, plus_infty),
    TEST_c_c (csqrt, minus_infty, plus_infty, plus_infty, plus_infty),
    TEST_c_c (csqrt, 0, minus_infty, plus_infty, minus_infty),
    TEST_c_c (csqrt, 4, minus_infty, plus_infty, minus_infty),
    TEST_c_c (csqrt, plus_infty, minus_infty, plus_infty, minus_infty),
    TEST_c_c (csqrt, minus_zero, minus_infty, plus_infty, minus_infty),
    TEST_c_c (csqrt, -4, minus_infty, plus_infty, minus_infty),
    TEST_c_c (csqrt, minus_infty, minus_infty, plus_infty, minus_infty),

    TEST_c_c (csqrt, minus_infty, qnan_value, qnan_value, plus_infty, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (csqrt, plus_infty, qnan_value, plus_infty, qnan_value),

    TEST_c_c (csqrt, 0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csqrt, 1, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csqrt, minus_zero, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csqrt, -1, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (csqrt, qnan_value, 0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csqrt, qnan_value, 8, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csqrt, qnan_value, minus_zero, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (csqrt, qnan_value, -8, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (csqrt, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (csqrt, 16.0, -30.0, 5.0, -3.0),
    TEST_c_c (csqrt, -1, 0, 0.0, 1.0),
    TEST_c_c (csqrt, 0, 2, 1.0, 1.0),
    TEST_c_c (csqrt, 119, 120, 12.0, 5.0),
    TEST_c_c (csqrt, 0.75L, 1.25L, 1.05065169626078392338656675760808326L, 0.594868882070379067881984030639932657L),
    TEST_c_c (csqrt, -2, -3, 0.89597747612983812471573375529004348L, -1.6741492280355400404480393008490519L),
    TEST_c_c (csqrt, -2, 3, 0.89597747612983812471573375529004348L, 1.6741492280355400404480393008490519L),
    /* Principal square root should be returned (i.e., non-negative real
       part).  */
    TEST_c_c (csqrt, 0, -1, M_SQRT1_2l, -M_SQRT1_2l),

    TEST_c_c (csqrt, 0x1.fffffep+127L, 0x1.fffffep+127L, 2.026714405498316804978751017492482558075e+19L, 8.394925938143272988211878516208015586281e+18L),
    TEST_c_c (csqrt, 0x1.fffffep+127L, 1.0L, 1.844674352395372953599975585936590505260e+19L, 2.710505511993121390769065968615872097053e-20L),
    TEST_c_c (csqrt, 0x1p-149L, 0x1p-149L, 4.112805464342778798097003462770175200803e-23L, 1.703579802732953750368659735601389709551e-23L),
    TEST_c_c (csqrt, 0x1p-147L, 0x1p-147L, 8.225610928685557596194006925540350401606e-23L, 3.407159605465907500737319471202779419102e-23L),

    TEST_c_c (csqrt, plus_zero, 0x1p-149L, 2.646977960169688559588507814623881131411e-23L, 2.646977960169688559588507814623881131411e-23L),
    TEST_c_c (csqrt, 0x1p-50L, 0x1p-149L, 2.980232238769531250000000000000000000000e-8L, 2.350988701644575015937473074444491355637e-38L),
#ifdef TEST_FLOAT
    TEST_c_c (csqrt, 0x1p+127L, 0x1p-149L, 1.304381782533278221234957180625250836888e19L, plus_zero, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (csqrt, 0x1p-149L, 0x1p+127L, 9.223372036854775808000000000000000000000e18L, 9.223372036854775808000000000000000000000e18L),
    TEST_c_c (csqrt, 0x1.000002p-126L, 0x1.000002p-126L, 1.191195773697904627170323731331667740087e-19L, 4.934094449071842328766868579214125217132e-20L),
    TEST_c_c (csqrt, -0x1.000002p-126L, -0x1.000002p-126L, 4.934094449071842328766868579214125217132e-20L, -1.191195773697904627170323731331667740087e-19L),

#ifndef TEST_FLOAT
    TEST_c_c (csqrt, 0x1.fffffffffffffp+1023L, 0x1.fffffffffffffp+1023L, 1.473094556905565378990473658199034571917e+154L, 6.101757441282702188537080005372547713595e+153L),
    TEST_c_c (csqrt, 0x1.fffffffffffffp+1023L, 0x1p+1023L, 1.379778091031440685006200821918878702861e+154L, 3.257214233483129514781233066898042490248e+153L),
    TEST_c_c (csqrt, 0x1p-1074L, 0x1p-1074L, 2.442109726130830256743814843868934877597e-162L, 1.011554969366634726113090867589031782487e-162L),
    TEST_c_c (csqrt, 0x1p-1073L, 0x1p-1073L, 3.453664695497464982856905711457966660085e-162L, 1.430554756764195530630723976279903095110e-162L),

    TEST_c_c (csqrt, plus_zero, 0x1p-1074L, 1.571727784702628688909515672805082228285e-162L, 1.571727784702628688909515672805082228285e-162L),
    TEST_c_c (csqrt, 0x1p-500L, 0x1p-1074L, 5.527147875260444560247265192192255725514e-76L, 4.469444793151709302716387622440056066334e-249L),
#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MAX_EXP == 1024)
    TEST_c_c (csqrt, 0x1p+1023L, 0x1p-1074L, 9.480751908109176726832526455652159260085e153L, plus_zero, UNDERFLOW_EXCEPTION),
#endif
    TEST_c_c (csqrt, 0x1p-1074L, 0x1p+1023L, 6.703903964971298549787012499102923063740e153L, 6.703903964971298549787012499102923063740e153L),
    TEST_c_c (csqrt, 0x1.0000000000001p-1022L, 0x1.0000000000001p-1022L, 1.638872094839911521020410942677082920935e-154L, 6.788430486774966350907249113759995429568e-155L),
    TEST_c_c (csqrt, -0x1.0000000000001p-1022L, -0x1.0000000000001p-1022L, 6.788430486774966350907249113759995429568e-155L, -1.638872094839911521020410942677082920935e-154L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (csqrt, 0x1.fp+16383L, 0x1.fp+16383L, 1.179514222452201722651836720466795901016e+2466L, 4.885707879516577666702435054303191575148e+2465L),
    TEST_c_c (csqrt, 0x1.fp+16383L, 0x1p+16383L, 1.106698967236475180613254276996359485630e+2466L, 2.687568007603946993388538156299100955642e+2465L),
    TEST_c_c (csqrt, 0x1p-16440L, 0x1p-16441L, 3.514690655930285351254618340783294558136e-2475L,  8.297059146828716918029689466551384219370e-2476L),

    TEST_c_c (csqrt, plus_zero, 0x1p-16445L, 4.269191686890197837775136325621239761720e-2476L, 4.269191686890197837775136325621239761720e-2476L),
    TEST_c_c (csqrt, 0x1p-5000L, 0x1p-16445L, 2.660791472672778409283210520357607795518e-753L, 6.849840675828785164910701384823702064234e-4199L),
    TEST_c_c (csqrt, 0x1p+16383L, 0x1p-16445L, 7.712754032630730034273323365543179095045e2465L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (csqrt, 0x1p-16445L, 0x1p+16383L, 5.453740678097079647314921223668914312241e2465L, 5.453740678097079647314921223668914312241e2465L),
    TEST_c_c (csqrt, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-16382L, 2.014551439675644900131815801350165472778e-2466L, 8.344545284118961664300307045791497724440e-2467L),
    TEST_c_c (csqrt, -0x1.0000000000000002p-16382L, -0x1.0000000000000002p-16382L, 8.344545284118961664300307045791497724440e-2467L, -2.014551439675644900131815801350165472778e-2466L),

# if LDBL_MANT_DIG >= 113
    TEST_c_c (csqrt, plus_zero, 0x1p-16494L, 1.799329752913293143453817328207572571442e-2483L, 1.799329752913293143453817328207572571442e-2483L),
    TEST_c_c (csqrt, 0x1p-5000L, 0x1p-16494L, 2.660791472672778409283210520357607795518e-753L, 1.216776133331049643422030716668249905907e-4213L),
    TEST_c_c (csqrt, 0x1p+16383L, 0x1p-16494L, 7.712754032630730034273323365543179095045e2465L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (csqrt, 0x1p-16494L, 0x1p+16383L, 5.453740678097079647314921223668914312241e2465L, 5.453740678097079647314921223668914312241e2465L),
    TEST_c_c (csqrt, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-16382L, 2.014551439675644900022606748976158925145e-2466L, 8.344545284118961663847948339519226074126e-2467L),
    TEST_c_c (csqrt, -0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-16382L, 8.344545284118961663847948339519226074126e-2467L, -2.014551439675644900022606748976158925145e-2466L),
# endif
#endif
    END_DATA (csqrt)
  };

static void
csqrt_test (void)
{
  errno = 0;
  FUNC(csqrt) (BUILD_COMPLEX (-1, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (csqrt);
  RUN_TEST_LOOP_c_c (csqrt, csqrt_test_data, );
  END (csqrt, complex);
}

static const struct test_c_c_data ctan_test_data[] =
  {
    START_DATA (ctan),
    TEST_c_c (ctan, 0, 0, 0.0, 0.0),
    TEST_c_c (ctan, 0, minus_zero, 0.0, minus_zero),
    TEST_c_c (ctan, minus_zero, 0, minus_zero, 0.0),
    TEST_c_c (ctan, minus_zero, minus_zero, minus_zero, minus_zero),

    TEST_c_c (ctan, 0, plus_infty, 0.0, 1.0),
    TEST_c_c (ctan, 1, plus_infty, 0.0, 1.0),
    TEST_c_c (ctan, minus_zero, plus_infty, minus_zero, 1.0),
    TEST_c_c (ctan, -1, plus_infty, minus_zero, 1.0),

    TEST_c_c (ctan, 0, minus_infty, 0.0, -1.0),
    TEST_c_c (ctan, 1, minus_infty, 0.0, -1.0),
    TEST_c_c (ctan, minus_zero, minus_infty, minus_zero, -1.0),
    TEST_c_c (ctan, -1, minus_infty, minus_zero, -1.0),

    TEST_c_c (ctan, plus_infty, 0, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctan, plus_infty, 2, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctan, minus_infty, 0, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctan, minus_infty, 2, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctan, plus_infty, minus_zero, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctan, plus_infty, -2, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctan, minus_infty, minus_zero, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctan, minus_infty, -2, qnan_value, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (ctan, qnan_value, plus_infty, 0.0, 1.0, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ctan, qnan_value, minus_infty, 0.0, -1.0, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (ctan, 0, qnan_value, 0.0, qnan_value),
    TEST_c_c (ctan, minus_zero, qnan_value, minus_zero, qnan_value),

    TEST_c_c (ctan, 0.5, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ctan, -4.5, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (ctan, qnan_value, 0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ctan, qnan_value, 5, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ctan, qnan_value, minus_zero, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ctan, qnan_value, -0.25, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (ctan, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (ctan, 0.75L, 1.25L, 0.160807785916206426725166058173438663L, 0.975363285031235646193581759755216379L),
    TEST_c_c (ctan, -2, -3, 0.376402564150424829275122113032269084e-2L, -1.00323862735360980144635859782192726L),

    TEST_c_c (ctan, 1, 45, 1.490158918874345552942703234806348520895e-39L, 1.000000000000000000000000000000000000001L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (ctan, 1, 47, 2.729321264492904590777293425576722354636e-41L, 1.0, UNDERFLOW_EXCEPTION_FLOAT),

#ifndef TEST_FLOAT
    TEST_c_c (ctan, 1, 355, 8.140551093483276762350406321792653551513e-309L, 1.0, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (ctan, 1, 365, 1.677892637497921890115075995898773550884e-317L, 1.0, UNDERFLOW_EXCEPTION_DOUBLE),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (ctan, 1, 5680, 4.725214596136812019616700920476949798307e-4934L, 1.0, UNDERFLOW_EXCEPTION),
    TEST_c_c (ctan, 1, 5690, 9.739393181626937151720816611272607059057e-4943L, 1.0, UNDERFLOW_EXCEPTION),
#endif

    TEST_c_c (ctan, 0x3.243f6cp-1, 0, -2.287733242885645987394874673945769518150e7L, 0.0),

    TEST_c_c (ctan, 0x1p127, 1, 0.2446359391192790896381501310437708987204L, 0.9101334047676183761532873794426475906201L),

#ifndef TEST_FLOAT
    TEST_c_c (ctan, 0x1p1023, 1, -0.2254627924997545057926782581695274244229L, 0.8786063118883068695462540226219865087189L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (ctan, 0x1p16383L, 1, 0.1608598776370396607204448234354670036772L, 0.8133818522051542536316746743877629761488L),
#endif

    TEST_c_c (ctan, 50000, 50000, plus_zero, 1.0, UNDERFLOW_EXCEPTION),
    TEST_c_c (ctan, 50000, -50000, plus_zero, -1.0, UNDERFLOW_EXCEPTION),
    TEST_c_c (ctan, -50000, 50000, minus_zero, 1.0, UNDERFLOW_EXCEPTION),
    TEST_c_c (ctan, -50000, -50000, minus_zero, -1.0, UNDERFLOW_EXCEPTION),
    END_DATA (ctan)
  };

static void
ctan_test (void)
{
  errno = 0;
  FUNC(ctan) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctan);
  RUN_TEST_LOOP_c_c (ctan, ctan_test_data, );
  END (ctan, complex);
}


static const struct test_c_c_data ctan_tonearest_test_data[] =
  {
    START_DATA (ctan_tonearest),
    TEST_c_c (ctan, 0x1.921fb6p+0, 0x1p-149, -2.287733242885645987394874673945769518150e7L, 7.334008549954377778731880988481078535821e-31L),

#ifndef TEST_FLOAT
    TEST_c_c (ctan, 0x1.921fb54442d18p+0, 0x1p-1074, 1.633123935319536975596773704152891653086e16L, 1.317719414943508315995636961402669067843e-291L),
#endif

#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (ctan, 0x1.921fb54442d1846ap+0L, 0x1p-16445L, -3.986797629811710706723242948653362815645e19L, 5.793882568875674066286163141055208625180e-4912L),
#endif
    END_DATA (ctan_tonearest)
  };

static void
ctan_test_tonearest (void)
{
  errno = 0;
  FUNC(ctan) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctan_tonearest);
  RUN_TEST_LOOP_c_c (ctan, ctan_tonearest_test_data, FE_TONEAREST);
  END (ctan_tonearest, complex);
}


static const struct test_c_c_data ctan_towardzero_test_data[] =
  {
    START_DATA (ctan_towardzero),
    TEST_c_c (ctan, 0x1.921fb6p+0, 0x1p-149, -2.287733242885645987394874673945769518150e7L, 7.334008549954377778731880988481078535821e-31L),

#ifndef TEST_FLOAT
    TEST_c_c (ctan, 0x1.921fb54442d18p+0, 0x1p-1074, 1.633123935319536975596773704152891653086e16L, 1.317719414943508315995636961402669067843e-291L),
#endif

#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (ctan, 0x1.921fb54442d1846ap+0L, 0x1p-16445L, -3.986797629811710706723242948653362815645e19L, 5.793882568875674066286163141055208625180e-4912L),
#endif
    END_DATA (ctan_towardzero)
  };

static void
ctan_test_towardzero (void)
{
  errno = 0;
  FUNC(ctan) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctan_towardzero);
  RUN_TEST_LOOP_c_c (ctan, ctan_towardzero_test_data, FE_TOWARDZERO);
  END (ctan_towardzero, complex);
}


static const struct test_c_c_data ctan_downward_test_data[] =
  {
    START_DATA (ctan_downward),
    TEST_c_c (ctan, 0x1.921fb6p+0, 0x1p-149, -2.287733242885645987394874673945769518150e7L, 7.334008549954377778731880988481078535821e-31L),

#ifndef TEST_FLOAT
    TEST_c_c (ctan, 0x1.921fb54442d18p+0, 0x1p-1074, 1.633123935319536975596773704152891653086e16L, 1.317719414943508315995636961402669067843e-291L),
#endif

#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (ctan, 0x1.921fb54442d1846ap+0L, 0x1p-16445L, -3.986797629811710706723242948653362815645e19L, 5.793882568875674066286163141055208625180e-4912L),
#endif
    END_DATA (ctan_downward)
  };

static void
ctan_test_downward (void)
{
  errno = 0;
  FUNC(ctan) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctan_downward);
  RUN_TEST_LOOP_c_c (ctan, ctan_downward_test_data, FE_DOWNWARD);
  END (ctan_downward, complex);
}


static const struct test_c_c_data ctan_upward_test_data[] =
  {
    START_DATA (ctan_upward),
    TEST_c_c (ctan, 0x1.921fb6p+0, 0x1p-149, -2.287733242885645987394874673945769518150e7L, 7.334008549954377778731880988481078535821e-31L),

#ifndef TEST_FLOAT
    TEST_c_c (ctan, 0x1.921fb54442d18p+0, 0x1p-1074, 1.633123935319536975596773704152891653086e16L, 1.317719414943508315995636961402669067843e-291L),
#endif

#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (ctan, 0x1.921fb54442d1846ap+0L, 0x1p-16445L, -3.986797629811710706723242948653362815645e19L, 5.793882568875674066286163141055208625180e-4912L),
#endif
    END_DATA (ctan_upward)
  };

static void
ctan_test_upward (void)
{
  errno = 0;
  FUNC(ctan) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctan_upward);
  RUN_TEST_LOOP_c_c (ctan, ctan_upward_test_data, FE_UPWARD);
  END (ctan_upward, complex);
}


static const struct test_c_c_data ctanh_test_data[] =
  {
    START_DATA (ctanh),
    TEST_c_c (ctanh, 0, 0, 0.0, 0.0),
    TEST_c_c (ctanh, 0, minus_zero, 0.0, minus_zero),
    TEST_c_c (ctanh, minus_zero, 0, minus_zero, 0.0),
    TEST_c_c (ctanh, minus_zero, minus_zero, minus_zero, minus_zero),

    TEST_c_c (ctanh, plus_infty, 0, 1.0, 0.0),
    TEST_c_c (ctanh, plus_infty, 1, 1.0, 0.0),
    TEST_c_c (ctanh, plus_infty, minus_zero, 1.0, minus_zero),
    TEST_c_c (ctanh, plus_infty, -1, 1.0, minus_zero),
    TEST_c_c (ctanh, minus_infty, 0, -1.0, 0.0),
    TEST_c_c (ctanh, minus_infty, 1, -1.0, 0.0),
    TEST_c_c (ctanh, minus_infty, minus_zero, -1.0, minus_zero),
    TEST_c_c (ctanh, minus_infty, -1, -1.0, minus_zero),

    TEST_c_c (ctanh, 0, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctanh, 2, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctanh, 0, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctanh, 2, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctanh, minus_zero, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctanh, -2, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctanh, minus_zero, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_c_c (ctanh, -2, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),

    TEST_c_c (ctanh, plus_infty, qnan_value, 1.0, 0.0, IGNORE_ZERO_INF_SIGN),
    TEST_c_c (ctanh, minus_infty, qnan_value, -1.0, 0.0, IGNORE_ZERO_INF_SIGN),

    TEST_c_c (ctanh, qnan_value, 0, qnan_value, 0.0),
    TEST_c_c (ctanh, qnan_value, minus_zero, qnan_value, minus_zero),

    TEST_c_c (ctanh, qnan_value, 0.5, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ctanh, qnan_value, -4.5, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (ctanh, 0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ctanh, 5, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ctanh, minus_zero, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (ctanh, -0.25, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (ctanh, qnan_value, qnan_value, qnan_value, qnan_value),

    TEST_c_c (ctanh, 0, M_PI_4l, 0.0, 1.0),

    TEST_c_c (ctanh, 0.75L, 1.25L, 1.37260757053378320258048606571226857L, 0.385795952609750664177596760720790220L),
    TEST_c_c (ctanh, -2, -3, -0.965385879022133124278480269394560686L, 0.988437503832249372031403430350121098e-2L),

    TEST_c_c (ctanh, 45, 1, 1.000000000000000000000000000000000000001L, 1.490158918874345552942703234806348520895e-39L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_c_c (ctanh, 47, 1, 1.0, 2.729321264492904590777293425576722354636e-41L, UNDERFLOW_EXCEPTION_FLOAT),

#ifndef TEST_FLOAT
    TEST_c_c (ctanh, 355, 1, 1.0, 8.140551093483276762350406321792653551513e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_c_c (ctanh, 365, 1, 1.0, 1.677892637497921890115075995898773550884e-317L, UNDERFLOW_EXCEPTION_DOUBLE),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (ctanh, 5680, 1, 1.0, 4.725214596136812019616700920476949798307e-4934L, UNDERFLOW_EXCEPTION),
    TEST_c_c (ctanh, 5690, 1, 1.0, 9.739393181626937151720816611272607059057e-4943L, UNDERFLOW_EXCEPTION),
#endif

    TEST_c_c (ctanh, 0, 0x3.243f6cp-1, 0.0, -2.287733242885645987394874673945769518150e7L),

    TEST_c_c (ctanh, 1, 0x1p127, 0.9101334047676183761532873794426475906201L, 0.2446359391192790896381501310437708987204L),

#ifndef TEST_FLOAT
    TEST_c_c (ctanh, 1, 0x1p1023, 0.8786063118883068695462540226219865087189L, -0.2254627924997545057926782581695274244229L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_c_c (ctanh, 1, 0x1p16383L, 0.8133818522051542536316746743877629761488L, 0.1608598776370396607204448234354670036772L),
#endif

    TEST_c_c (ctanh, 50000, 50000, 1.0, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (ctanh, 50000, -50000, 1.0, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (ctanh, -50000, 50000, -1.0, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_c_c (ctanh, -50000, -50000, -1.0, minus_zero, UNDERFLOW_EXCEPTION),
    END_DATA (ctanh)
  };

static void
ctanh_test (void)
{
  errno = 0;
  FUNC(ctanh) (BUILD_COMPLEX (0, 0));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctanh);
  RUN_TEST_LOOP_c_c (ctanh, ctanh_test_data, );
  END (ctanh, complex);
}


static const struct test_c_c_data ctanh_tonearest_test_data[] =
  {
    START_DATA (ctanh_tonearest),
    TEST_c_c (ctanh, 0x1p-149, 0x1.921fb6p+0, 7.334008549954377778731880988481078535821e-31L, -2.287733242885645987394874673945769518150e7L),

#ifndef TEST_FLOAT
    TEST_c_c (ctanh, 0x1p-1074, 0x1.921fb54442d18p+0, 1.317719414943508315995636961402669067843e-291L, 1.633123935319536975596773704152891653086e16L),
#endif

#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (ctanh, 0x1p-16445L, 0x1.921fb54442d1846ap+0L, 5.793882568875674066286163141055208625180e-4912L, -3.986797629811710706723242948653362815645e19L),
#endif
    END_DATA (ctanh_tonearest)
  };

static void
ctanh_test_tonearest (void)
{
  errno = 0;
  FUNC(ctanh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctanh_tonearest);
  RUN_TEST_LOOP_c_c (ctanh, ctanh_tonearest_test_data, FE_TONEAREST);
  END (ctanh_tonearest, complex);
}


static const struct test_c_c_data ctanh_towardzero_test_data[] =
  {
    START_DATA (ctanh_towardzero),
    TEST_c_c (ctanh, 0x1p-149, 0x1.921fb6p+0, 7.334008549954377778731880988481078535821e-31L, -2.287733242885645987394874673945769518150e7L),

#ifndef TEST_FLOAT
    TEST_c_c (ctanh, 0x1p-1074, 0x1.921fb54442d18p+0, 1.317719414943508315995636961402669067843e-291L, 1.633123935319536975596773704152891653086e16L),
#endif

#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (ctanh, 0x1p-16445L, 0x1.921fb54442d1846ap+0L, 5.793882568875674066286163141055208625180e-4912L, -3.986797629811710706723242948653362815645e19L),
#endif
    END_DATA (ctanh_towardzero)
  };

static void
ctanh_test_towardzero (void)
{
  errno = 0;
  FUNC(ctanh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctanh_towardzero);
  RUN_TEST_LOOP_c_c (ctanh, ctanh_towardzero_test_data, FE_TOWARDZERO);
  END (ctanh_towardzero, complex);
}


static const struct test_c_c_data ctanh_downward_test_data[] =
  {
    START_DATA (ctanh_downward),
    TEST_c_c (ctanh, 0x1p-149, 0x1.921fb6p+0, 7.334008549954377778731880988481078535821e-31L, -2.287733242885645987394874673945769518150e7L),

#ifndef TEST_FLOAT
    TEST_c_c (ctanh, 0x1p-1074, 0x1.921fb54442d18p+0, 1.317719414943508315995636961402669067843e-291L, 1.633123935319536975596773704152891653086e16L),
#endif

#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (ctanh, 0x1p-16445L, 0x1.921fb54442d1846ap+0L, 5.793882568875674066286163141055208625180e-4912L, -3.986797629811710706723242948653362815645e19L),
#endif
    END_DATA (ctanh_downward)
  };

static void
ctanh_test_downward (void)
{
  errno = 0;
  FUNC(ctanh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctanh_downward);
  RUN_TEST_LOOP_c_c (ctanh, ctanh_downward_test_data, FE_DOWNWARD);
  END (ctanh_downward, complex);
}


static const struct test_c_c_data ctanh_upward_test_data[] =
  {
    START_DATA (ctanh_upward),
    TEST_c_c (ctanh, 0x1p-149, 0x1.921fb6p+0, 7.334008549954377778731880988481078535821e-31L, -2.287733242885645987394874673945769518150e7L),

#ifndef TEST_FLOAT
    TEST_c_c (ctanh, 0x1p-1074, 0x1.921fb54442d18p+0, 1.317719414943508315995636961402669067843e-291L, 1.633123935319536975596773704152891653086e16L),
#endif

#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_c_c (ctanh, 0x1p-16445L, 0x1.921fb54442d1846ap+0L, 5.793882568875674066286163141055208625180e-4912L, -3.986797629811710706723242948653362815645e19L),
#endif
    END_DATA (ctanh_upward)
  };

static void
ctanh_test_upward (void)
{
  errno = 0;
  FUNC(ctanh) (BUILD_COMPLEX (0.7L, 1.2L));
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (ctanh_upward);
  RUN_TEST_LOOP_c_c (ctanh, ctanh_upward_test_data, FE_UPWARD);
  END (ctanh_upward, complex);
}


static const struct test_f_f_data erf_test_data[] =
  {
    START_DATA (erf),
    TEST_f_f (erf, 0, 0),
    TEST_f_f (erf, minus_zero, minus_zero),
    TEST_f_f (erf, plus_infty, 1),
    TEST_f_f (erf, minus_infty, -1),
    TEST_f_f (erf, qnan_value, qnan_value),

    TEST_f_f (erf, 0.125L, 0.140316204801333817393029446521623398L),
    TEST_f_f (erf, 0.75L, 0.711155633653515131598937834591410777L),
    TEST_f_f (erf, 1.25L, 0.922900128256458230136523481197281140L),
    TEST_f_f (erf, 2.0L, 0.995322265018952734162069256367252929L),
    TEST_f_f (erf, 4.125L, 0.999999994576599200434933994687765914L),
    TEST_f_f (erf, 27.0L, 1.0L),
    END_DATA (erf)
  };

static void
erf_test (void)
{
  errno = 0;
  FUNC(erf) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (erf);
  RUN_TEST_LOOP_f_f (erf, erf_test_data, );
  END (erf);
}


static const struct test_f_f_data erfc_test_data[] =
  {
    START_DATA (erfc),
    TEST_f_f (erfc, plus_infty, 0.0),
    TEST_f_f (erfc, minus_infty, 2.0),
    TEST_f_f (erfc, 0.0, 1.0),
    TEST_f_f (erfc, minus_zero, 1.0),
    TEST_f_f (erfc, qnan_value, qnan_value),

    TEST_f_f (erfc, 0.125L, 0.859683795198666182606970553478376602L),
    TEST_f_f (erfc, 0.75L, 0.288844366346484868401062165408589223L),
    TEST_f_f (erfc, 1.25L, 0.0770998717435417698634765188027188596L),
    TEST_f_f (erfc, 2.0L, 0.00467773498104726583793074363274707139L),
    TEST_f_f (erfc, 0x1.f7303cp+1L, 2.705500297238986897105236321218861842255e-8L),
    TEST_f_f (erfc, 4.125L, 0.542340079956506600531223408575531062e-8L),
    TEST_f_f (erfc, 0x1.ffa002p+2L, 1.233585992097580296336099501489175967033e-29L),
    TEST_f_f (erfc, 0x1.ffffc8p+2L, 1.122671365033056305522366683719541099329e-29L),
#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
# if LDBL_MIN_10_EXP < -319
    TEST_f_f (erfc, 27.0L, 0.523704892378925568501606768284954709e-318L),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_f_f (erfc, 0x1.ffff56789abcdef0123456789a8p+2L, 1.123161416304655390092138725253789378459e-29L),
# endif
#endif
    END_DATA (erfc)
  };

static void
erfc_test (void)
{
  errno = 0;
  FUNC(erfc) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (erfc);
  RUN_TEST_LOOP_f_f (erfc, erfc_test_data, );
  END (erfc);
}


static const struct test_f_f_data exp_test_data[] =
  {
    START_DATA (exp),
    TEST_f_f (exp, 0, 1),
    TEST_f_f (exp, minus_zero, 1),

#ifndef TEST_INLINE
    TEST_f_f (exp, plus_infty, plus_infty),
    TEST_f_f (exp, minus_infty, 0),
#endif
    TEST_f_f (exp, qnan_value, qnan_value),
    TEST_f_f (exp, 1, M_El),

    TEST_f_f (exp, 2, M_E2l),
    TEST_f_f (exp, 3, M_E3l),
    TEST_f_f (exp, 0.75L, 2.11700001661267466854536981983709561L),
    TEST_f_f (exp, 50.0L, 5184705528587072464087.45332293348538L),
    TEST_f_f (exp, 88.72269439697265625L, 3.40233126623160774937554134772290447915e38L),
#ifndef TEST_FLOAT
    TEST_f_f (exp, 709.75L, 1.739836873264160557698252711673830393864768e+308L),
#endif
#if defined TEST_LDOUBLE && __LDBL_MAX_EXP__ > 1024
    /* The result can only be represented in sane long double.  */
    TEST_f_f (exp, 1000.0L, 0.197007111401704699388887935224332313e435L),
#endif

#if !(defined TEST_LDOUBLE && LDBL_MAX_EXP > 1024)
    TEST_f_f (exp, 710, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (exp, -1234, plus_zero, UNDERFLOW_EXCEPTION),
#endif
    TEST_f_f (exp, 1e5, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (exp, max_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (exp, -max_value, 0, UNDERFLOW_EXCEPTION),
    END_DATA (exp)
  };

static void
exp_test (void)
{
  errno = 0;
  FUNC(exp) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp);
  RUN_TEST_LOOP_f_f (exp, exp_test_data, );
  END (exp);
}


static const struct test_f_f_data exp_tonearest_test_data[] =
  {
    START_DATA (exp_tonearest),
    TEST_f_f (exp, 1, M_El),
    TEST_f_f (exp, 2, M_E2l),
    TEST_f_f (exp, 3, M_E3l),
    END_DATA (exp_tonearest)
  };

static void
exp_test_tonearest (void)
{
  errno = 0;
  FUNC(exp) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp_tonearest);
  RUN_TEST_LOOP_f_f (exp, exp_tonearest_test_data, FE_TONEAREST);
  END (exp_tonearest);
}


static const struct test_f_f_data exp_towardzero_test_data[] =
  {
    START_DATA (exp_towardzero),
    TEST_f_f (exp, 1, M_El),
    TEST_f_f (exp, 2, M_E2l),
    TEST_f_f (exp, 3, M_E3l),
    END_DATA (exp_towardzero)
  };

static void
exp_test_towardzero (void)
{
  errno = 0;
  FUNC(exp) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp_towardzero);
  RUN_TEST_LOOP_f_f (exp, exp_towardzero_test_data, FE_TOWARDZERO);
  END (exp_towardzero);
}


static const struct test_f_f_data exp_downward_test_data[] =
  {
    START_DATA (exp_downward),
    TEST_f_f (exp, 1, M_El),
    TEST_f_f (exp, 2, M_E2l),
    TEST_f_f (exp, 3, M_E3l),
    END_DATA (exp_downward)
  };

static void
exp_test_downward (void)
{
  errno = 0;
  FUNC(exp) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp_downward);
  RUN_TEST_LOOP_f_f (exp, exp_downward_test_data, FE_DOWNWARD);
  END (exp_downward);
}


static const struct test_f_f_data exp_upward_test_data[] =
  {
    START_DATA (exp_upward),
    TEST_f_f (exp, 1, M_El),
    TEST_f_f (exp, 2, M_E2l),
    TEST_f_f (exp, 3, M_E3l),
    END_DATA (exp_upward)
  };

static void
exp_test_upward (void)
{
  errno = 0;
  FUNC(exp) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp_upward);
  RUN_TEST_LOOP_f_f (exp, exp_upward_test_data, FE_UPWARD);
  END (exp_upward);
}


static const struct test_f_f_data exp10_test_data[] =
  {
    START_DATA (exp10),
    TEST_f_f (exp10, 0, 1),
    TEST_f_f (exp10, minus_zero, 1),

    TEST_f_f (exp10, plus_infty, plus_infty),
    TEST_f_f (exp10, minus_infty, 0),
    TEST_f_f (exp10, qnan_value, qnan_value),
    TEST_f_f (exp10, 3, 1000),
    TEST_f_f (exp10, -1, 0.1L),
    TEST_f_f (exp10, 36, 1.0e36L),
    TEST_f_f (exp10, -36, 1.0e-36L),
#ifndef TEST_FLOAT
    TEST_f_f (exp10, 305, 1.0e305L),
    TEST_f_f (exp10, -305, 1.0e-305L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_10_EXP >= 4932
    TEST_f_f (exp10, 4932, 1.0e4932L),
    TEST_f_f (exp10, -4932, 1.0e-4932L, UNDERFLOW_EXCEPTION),
#endif
    TEST_f_f (exp10, 1e6, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (exp10, -1e6, 0, UNDERFLOW_EXCEPTION),
    TEST_f_f (exp10, max_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (exp10, -max_value, 0, UNDERFLOW_EXCEPTION),
    TEST_f_f (exp10, 0.75L, 5.62341325190349080394951039776481231L),
    END_DATA (exp10)
  };

static void
exp10_test (void)
{
  errno = 0;
  FUNC(exp10) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp10);
  RUN_TEST_LOOP_f_f (exp10, exp10_test_data, );
  END (exp10);
}


static const struct test_f_f_data exp2_test_data[] =
  {
    START_DATA (exp2),
    TEST_f_f (exp2, 0, 1),
    TEST_f_f (exp2, minus_zero, 1),
    TEST_f_f (exp2, plus_infty, plus_infty),
    TEST_f_f (exp2, minus_infty, 0),
    TEST_f_f (exp2, qnan_value, qnan_value),

    TEST_f_f (exp2, 10, 1024),
    TEST_f_f (exp2, -1, 0.5),
    TEST_f_f (exp2, 1e6, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (exp2, -1e6, 0, UNDERFLOW_EXCEPTION),
    TEST_f_f (exp2, max_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (exp2, -max_value, 0, UNDERFLOW_EXCEPTION),
    TEST_f_f (exp2, 0.75L, 1.68179283050742908606225095246642979L),

    TEST_f_f (exp2, 100.5, 1.792728671193156477399422023278661496394e+30L),
    TEST_f_f (exp2, 127, 0x1p127),
    TEST_f_f (exp2, -149, 0x1p-149),

#ifndef TEST_FLOAT
    TEST_f_f (exp2, 1000.25, 1.274245659452564874772384918171765416737e+301L),
    TEST_f_f (exp2, 1023, 0x1p1023),
    TEST_f_f (exp2, -1074, 0x1p-1074),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_f_f (exp2, 16383, 0x1p16383L),
    TEST_f_f (exp2, -16400, 0x1p-16400L),
#endif
    END_DATA (exp2)
  };

static void
exp2_test (void)
{
  errno = 0;
  FUNC(exp2) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (exp2);
  RUN_TEST_LOOP_f_f (exp2, exp2_test_data, );
  END (exp2);
}


static const struct test_f_f_data expm1_test_data[] =
  {
    START_DATA (expm1),
    TEST_f_f (expm1, 0, 0),
    TEST_f_f (expm1, minus_zero, minus_zero),

#ifndef TEST_INLINE
    TEST_f_f (expm1, plus_infty, plus_infty),
    TEST_f_f (expm1, minus_infty, -1),
#endif
    TEST_f_f (expm1, qnan_value, qnan_value),

    TEST_f_f (expm1, 1, M_El - 1.0),
    TEST_f_f (expm1, 0.75L, 1.11700001661267466854536981983709561L),

    TEST_f_f (expm1, 50.0L, 5.1847055285870724640864533229334853848275e+21L),

#ifndef TEST_FLOAT
    TEST_f_f (expm1, 127.0L, 1.4302079958348104463583671072905261080748e+55L),
    TEST_f_f (expm1, 500.0L, 1.4035922178528374107397703328409120821806e+217L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_f_f (expm1, 11356.25L, 9.05128237311923300051376115753226014206e+4931L),
#endif

    TEST_f_f (expm1, -10.0, -0.9999546000702375151484644084844394493898L),
    TEST_f_f (expm1, -16.0, -0.9999998874648252807408854862248209398728L),
    TEST_f_f (expm1, -17.0, -0.9999999586006228121483334034897228104472L),
    TEST_f_f (expm1, -18.0, -0.9999999847700202552873715638633707664826L),
    TEST_f_f (expm1, -36.0, -0.9999999999999997680477169756430611687736L),
    TEST_f_f (expm1, -37.0, -0.9999999999999999146695237425593420572195L),
    TEST_f_f (expm1, -38.0, -0.9999999999999999686086720795197037129104L),
    TEST_f_f (expm1, -44.0, -0.9999999999999999999221886775886620348429L),
    TEST_f_f (expm1, -45.0, -0.9999999999999999999713748141945060635553L),
    TEST_f_f (expm1, -46.0, -0.9999999999999999999894693826424461876212L),
    TEST_f_f (expm1, -73.0, -0.9999999999999999999999999999999802074012L),
    TEST_f_f (expm1, -74.0, -0.9999999999999999999999999999999927187098L),
    TEST_f_f (expm1, -75.0, -0.9999999999999999999999999999999973213630L),
    TEST_f_f (expm1, -78.0, -0.9999999999999999999999999999999998666385L),
    TEST_f_f (expm1, -79.0, -0.9999999999999999999999999999999999509391L),
    TEST_f_f (expm1, -80.0, -0.9999999999999999999999999999999999819515L),
    TEST_f_f (expm1, -100.0, -1.0),
    TEST_f_f (expm1, -1000.0, -1.0),
    TEST_f_f (expm1, -10000.0, -1.0),
    TEST_f_f (expm1, -100000.0, -1.0),

    TEST_f_f (expm1, 100000.0, plus_infty, OVERFLOW_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (expm1, max_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (expm1, -max_value, -1),
    END_DATA (expm1)
  };

static void
expm1_test (void)
{
  errno = 0;
  FUNC(expm1) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (expm1);
  RUN_TEST_LOOP_f_f (expm1, expm1_test_data, );
  END (expm1);
}


static const struct test_f_f_data fabs_test_data[] =
  {
    START_DATA (fabs),
    TEST_f_f (fabs, 0, 0),
    TEST_f_f (fabs, minus_zero, 0),

    TEST_f_f (fabs, plus_infty, plus_infty),
    TEST_f_f (fabs, minus_infty, plus_infty),
    TEST_f_f (fabs, qnan_value, qnan_value),

    TEST_f_f (fabs, 38.0, 38.0),
    TEST_f_f (fabs, -M_El, M_El),
    END_DATA (fabs)
  };

static void
fabs_test (void)
{
  START (fabs);
  RUN_TEST_LOOP_f_f (fabs, fabs_test_data, );
  END (fabs);
}


static const struct test_ff_f_data fdim_test_data[] =
  {
    START_DATA (fdim),
    TEST_ff_f (fdim, 0, 0, 0),
    TEST_ff_f (fdim, 9, 0, 9),
    TEST_ff_f (fdim, 0, 9, 0),
    TEST_ff_f (fdim, -9, 0, 0),
    TEST_ff_f (fdim, 0, -9, 9),

    TEST_ff_f (fdim, plus_infty, 9, plus_infty),
    TEST_ff_f (fdim, plus_infty, -9, plus_infty),
    TEST_ff_f (fdim, minus_infty, 9, 0),
    TEST_ff_f (fdim, minus_infty, -9, 0),
    TEST_ff_f (fdim, 9, minus_infty, plus_infty),
    TEST_ff_f (fdim, -9, minus_infty, plus_infty),
    TEST_ff_f (fdim, 9, plus_infty, 0),
    TEST_ff_f (fdim, -9, plus_infty, 0),

    TEST_ff_f (fdim, 0, qnan_value, qnan_value),
    TEST_ff_f (fdim, 9, qnan_value, qnan_value),
    TEST_ff_f (fdim, -9, qnan_value, qnan_value),
    TEST_ff_f (fdim, qnan_value, 9, qnan_value),
    TEST_ff_f (fdim, qnan_value, -9, qnan_value),
    TEST_ff_f (fdim, plus_infty, qnan_value, qnan_value),
    TEST_ff_f (fdim, minus_infty, qnan_value, qnan_value),
    TEST_ff_f (fdim, qnan_value, plus_infty, qnan_value),
    TEST_ff_f (fdim, qnan_value, minus_infty, qnan_value),
    TEST_ff_f (fdim, qnan_value, qnan_value, qnan_value),

    TEST_ff_f (fdim, plus_infty, plus_infty, 0),
    END_DATA (fdim)
  };

static void
fdim_test (void)
{
  START (fdim);
  RUN_TEST_LOOP_ff_f (fdim, fdim_test_data, );
  END (fdim);
}


static void
finite_test (void)
{
  START (finite);

  TEST_f_b (finite, 0, 1);
  TEST_f_b (finite, minus_zero, 1);
  TEST_f_b (finite, 10, 1);
  TEST_f_b (finite, min_subnorm_value, 1);
  TEST_f_b (finite, plus_infty, 0);
  TEST_f_b (finite, minus_infty, 0);
  TEST_f_b (finite, qnan_value, 0);

  END (finite);
}


static const struct test_f_f_data floor_test_data[] =
  {
    START_DATA (floor),
    TEST_f_f (floor, 0.0, 0.0),
    TEST_f_f (floor, minus_zero, minus_zero),
    TEST_f_f (floor, plus_infty, plus_infty),
    TEST_f_f (floor, minus_infty, minus_infty),
    TEST_f_f (floor, qnan_value, qnan_value),

    TEST_f_f (floor, M_PIl, 3.0),
    TEST_f_f (floor, -M_PIl, -4.0),

    TEST_f_f (floor, 0.1, 0.0),
    TEST_f_f (floor, 0.25, 0.0),
    TEST_f_f (floor, 0.625, 0.0),
    TEST_f_f (floor, -0.1, -1.0),
    TEST_f_f (floor, -0.25, -1.0),
    TEST_f_f (floor, -0.625, -1.0),

#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (floor, 4503599627370495.5L, 4503599627370495.0L),
    TEST_f_f (floor, 4503599627370496.25L, 4503599627370496.0L),
    TEST_f_f (floor, 4503599627370496.5L, 4503599627370496.0L),
    TEST_f_f (floor, 4503599627370496.75L, 4503599627370496.0L),
    TEST_f_f (floor, 4503599627370497.5L, 4503599627370497.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (floor, 4503599627370494.5000000000001L, 4503599627370494.0L),
    TEST_f_f (floor, 4503599627370495.5000000000001L, 4503599627370495.0L),
    TEST_f_f (floor, 4503599627370496.5000000000001L, 4503599627370496.0L),
# endif

    TEST_f_f (floor, -4503599627370495.5L, -4503599627370496.0L),
    TEST_f_f (floor, -4503599627370496.25L, -4503599627370497.0L),
    TEST_f_f (floor, -4503599627370496.5L, -4503599627370497.0L),
    TEST_f_f (floor, -4503599627370496.75L, -4503599627370497.0L),
    TEST_f_f (floor, -4503599627370497.5L, -4503599627370498.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (floor, -4503599627370494.5000000000001L, -4503599627370495.0L),
    TEST_f_f (floor, -4503599627370495.5000000000001L, -4503599627370496.0L),
    TEST_f_f (floor, -4503599627370496.5000000000001L, -4503599627370497.0L),
# endif

    TEST_f_f (floor, 9007199254740991.5L, 9007199254740991.0L),
    TEST_f_f (floor, 9007199254740992.25L, 9007199254740992.0L),
    TEST_f_f (floor, 9007199254740992.5L, 9007199254740992.0L),
    TEST_f_f (floor, 9007199254740992.75L, 9007199254740992.0L),
    TEST_f_f (floor, 9007199254740993.5L, 9007199254740993.0L),

# if LDBL_MANT_DIG > 100
    TEST_f_f (floor, 9007199254740991.0000000000001L, 9007199254740991.0L),
    TEST_f_f (floor, 9007199254740992.0000000000001L, 9007199254740992.0L),
    TEST_f_f (floor, 9007199254740993.0000000000001L, 9007199254740993.0L),
    TEST_f_f (floor, 9007199254740991.5000000000001L, 9007199254740991.0L),
    TEST_f_f (floor, 9007199254740992.5000000000001L, 9007199254740992.0L),
    TEST_f_f (floor, 9007199254740993.5000000000001L, 9007199254740993.0L),
# endif

    TEST_f_f (floor, -9007199254740991.5L, -9007199254740992.0L),
    TEST_f_f (floor, -9007199254740992.25L, -9007199254740993.0L),
    TEST_f_f (floor, -9007199254740992.5L, -9007199254740993.0L),
    TEST_f_f (floor, -9007199254740992.75L, -9007199254740993.0L),
    TEST_f_f (floor, -9007199254740993.5L, -9007199254740994.0L),

# if LDBL_MANT_DIG > 100
    TEST_f_f (floor, -9007199254740991.0000000000001L, -9007199254740992.0L),
    TEST_f_f (floor, -9007199254740992.0000000000001L, -9007199254740993.0L),
    TEST_f_f (floor, -9007199254740993.0000000000001L, -9007199254740994.0L),
    TEST_f_f (floor, -9007199254740991.5000000000001L, -9007199254740992.0L),
    TEST_f_f (floor, -9007199254740992.5000000000001L, -9007199254740993.0L),
    TEST_f_f (floor, -9007199254740993.5000000000001L, -9007199254740994.0L),
# endif

    TEST_f_f (floor, 72057594037927935.5L, 72057594037927935.0L),
    TEST_f_f (floor, 72057594037927936.25L, 72057594037927936.0L),
    TEST_f_f (floor, 72057594037927936.5L, 72057594037927936.0L),
    TEST_f_f (floor, 72057594037927936.75L, 72057594037927936.0L),
    TEST_f_f (floor, 72057594037927937.5L, 72057594037927937.0L),

    TEST_f_f (floor, -72057594037927935.5L, -72057594037927936.0L),
    TEST_f_f (floor, -72057594037927936.25L, -72057594037927937.0L),
    TEST_f_f (floor, -72057594037927936.5L, -72057594037927937.0L),
    TEST_f_f (floor, -72057594037927936.75L, -72057594037927937.0L),
    TEST_f_f (floor, -72057594037927937.5L, -72057594037927938.0L),

    TEST_f_f (floor, 10141204801825835211973625643007.5L, 10141204801825835211973625643007.0L),
    TEST_f_f (floor, 10141204801825835211973625643008.25L, 10141204801825835211973625643008.0L),
    TEST_f_f (floor, 10141204801825835211973625643008.5L, 10141204801825835211973625643008.0L),
    TEST_f_f (floor, 10141204801825835211973625643008.75L, 10141204801825835211973625643008.0L),
    TEST_f_f (floor, 10141204801825835211973625643009.5L, 10141204801825835211973625643009.0L),

    TEST_f_f (floor, 0xf.ffffffffffffff8p+47L, 0xf.fffffffffffep+47L),
    TEST_f_f (floor, -0x8.000000000000004p+48L, -0x8.000000000001p+48L),
#endif
    END_DATA (floor)
  };

static void
floor_test (void)
{
  START (floor);
  RUN_TEST_LOOP_f_f (floor, floor_test_data, );
  END (floor);
}


static const struct test_fff_f_data fma_test_data[] =
  {
    START_DATA (fma),
    TEST_fff_f (fma, 1.0, 2.0, 3.0, 5.0),
    TEST_fff_f (fma, qnan_value, 2.0, 3.0, qnan_value),
    TEST_fff_f (fma, 1.0, qnan_value, 3.0, qnan_value),
    TEST_fff_f (fma, 1.0, 2.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_fff_f (fma, plus_infty, 0.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_fff_f (fma, minus_infty, 0.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_fff_f (fma, 0.0, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_fff_f (fma, 0.0, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_fff_f (fma, plus_infty, 0.0, 1.0, qnan_value, INVALID_EXCEPTION),
    TEST_fff_f (fma, minus_infty, 0.0, 1.0, qnan_value, INVALID_EXCEPTION),
    TEST_fff_f (fma, 0.0, plus_infty, 1.0, qnan_value, INVALID_EXCEPTION),
    TEST_fff_f (fma, 0.0, minus_infty, 1.0, qnan_value, INVALID_EXCEPTION),

    TEST_fff_f (fma, plus_infty, plus_infty, minus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_fff_f (fma, minus_infty, plus_infty, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_fff_f (fma, plus_infty, minus_infty, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_fff_f (fma, minus_infty, minus_infty, minus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_fff_f (fma, plus_infty, 3.5L, minus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_fff_f (fma, minus_infty, -7.5L, minus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_fff_f (fma, -13.5L, plus_infty, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_fff_f (fma, minus_infty, 7.5L, plus_infty, qnan_value, INVALID_EXCEPTION),

    TEST_fff_f (fma, 1.25L, 0.75L, 0.0625L, 1.0L),

    TEST_fff_f (fma, -max_value, -max_value, minus_infty, minus_infty),
    TEST_fff_f (fma, max_value / 2, max_value / 2, minus_infty, minus_infty),
    TEST_fff_f (fma, -max_value, max_value, plus_infty, plus_infty),
    TEST_fff_f (fma, max_value / 2, -max_value / 4, plus_infty, plus_infty),
    TEST_fff_f (fma, plus_infty, 4, plus_infty, plus_infty),
    TEST_fff_f (fma, 2, minus_infty, minus_infty, minus_infty),
    TEST_fff_f (fma, minus_infty, minus_infty, plus_infty, plus_infty),
    TEST_fff_f (fma, plus_infty, minus_infty, minus_infty, minus_infty),

    TEST_fff_f (fma, plus_zero, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, plus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, minus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, plus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, minus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, plus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, minus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, -1.0, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, -1.0, plus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, -1.0, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, -1.0, minus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, 1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, 1.0, minus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, -1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, -1.0, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, 1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, 1.0, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, -1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, -1.0, minus_zero, plus_zero),

    TEST_fff_f (fma, 1.0, 1.0, -1.0, plus_zero),
    TEST_fff_f (fma, 1.0, -1.0, 1.0, plus_zero),
    TEST_fff_f (fma, -1.0, 1.0, 1.0, plus_zero),
    TEST_fff_f (fma, -1.0, -1.0, -1.0, plus_zero),

    TEST_fff_f (fma, min_value, min_value, plus_zero, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, min_value, minus_zero, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, -min_value, plus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, -min_value, minus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, min_value, plus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, min_value, minus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, -min_value, plus_zero, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, -min_value, minus_zero, plus_zero, UNDERFLOW_EXCEPTION),

    TEST_fff_f (fma, max_value, max_value, min_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, max_value, -min_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, -max_value, min_value, minus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, -max_value, -min_value, minus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, max_value, min_value, minus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, max_value, -min_value, minus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, -max_value, min_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, -max_value, -min_value, plus_infty, OVERFLOW_EXCEPTION),

#if defined (TEST_FLOAT) && FLT_MANT_DIG == 24
    TEST_fff_f (fma, 0x1.7ff8p+13, 0x1.000002p+0, 0x1.ffffp-24, 0x1.7ff802p+13),
    TEST_fff_f (fma, 0x1.fffp+0, 0x1.00001p+0, -0x1.fffp+0, 0x1.fffp-20),
    TEST_fff_f (fma, 0x1.9abcdep+127, 0x0.9abcdep-126, -0x1.f08948p+0, 0x1.bb421p-25),
    TEST_fff_f (fma, 0x1.9abcdep+100, 0x0.9abcdep-126, -0x1.f08948p-27, 0x1.bb421p-52),
    TEST_fff_f (fma, 0x1.fffffep+127, 0x1.001p+0, -0x1.fffffep+127, 0x1.fffffep+115),
    TEST_fff_f (fma, -0x1.fffffep+127, 0x1.fffffep+0, 0x1.fffffep+127, -0x1.fffffap+127),
    TEST_fff_f (fma, 0x1.fffffep+127, 2.0, -0x1.fffffep+127, 0x1.fffffep+127),
    TEST_fff_f (fma, 0x1.4p-126, 0x1.000004p-1, 0x1p-128, 0x1.c00004p-127, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-126, 0x1.000004p-1, -0x1p-128, -0x1.c00004p-127, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffff8p-126, 0x1.000002p-1, 0x1p-149, 0x1p-126, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1.fffff8p-126, 0x1.000002p-1, -0x1p-149, -0x1p-126, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-149, 0x1p-1, 0x0.fffffep-126, 0x1p-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-149, 0x1p-1, -0x0.fffffep-126, -0x1p-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1.1p-1, 0x0.fffffep-126, 0x1p-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-149, 0x1.1p-1, -0x0.fffffep-126, -0x1p-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p127, 0x1p127),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p127, 0x1p127),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p127, -0x1p127),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p127, -0x1p127),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p-126, 0x1p-126),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p-126, 0x1p-126, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p-126, -0x1p-126, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p-126, -0x1p-126),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x0.fffffep-126, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x0.fffffep-126, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x0.fffffep-126, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x0.fffffep-126, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p-149, 0x1p-149, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p-149, 0x1p-149, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p-149, -0x1p-149, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p-149, -0x1p-149, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.fffp0, 0x0.fffp0, -0x0.ffep0, 0x1p-24),
    TEST_fff_f (fma, 0x0.fffp0, -0x0.fffp0, 0x0.ffep0, -0x1p-24),
    TEST_fff_f (fma, -0x0.fffp0, 0x0.fffp0, 0x0.ffep0, -0x1p-24),
    TEST_fff_f (fma, -0x0.fffp0, -0x0.fffp0, -0x0.ffep0, 0x1p-24),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, 0x1p127, 0x1p127),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, 0x1p127, 0x1p127),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, -0x1p127, -0x1p127),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, -0x1p127, -0x1p127),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, 0x1p103, 0x1p103),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, 0x1p103, 0x1p103),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, -0x1p103, -0x1p103),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, -0x1p103, -0x1p103),
#endif
#if defined (TEST_DOUBLE) && DBL_MANT_DIG == 53
    TEST_fff_f (fma, 0x1.7fp+13, 0x1.0000000000001p+0, 0x1.ffep-48, 0x1.7f00000000001p+13),
    TEST_fff_f (fma, 0x1.fffp+0, 0x1.0000000000001p+0, -0x1.fffp+0, 0x1.fffp-52),
    TEST_fff_f (fma, 0x1.0000002p+0, 0x1.ffffffcp-1, 0x1p-300, 1.0),
    TEST_fff_f (fma, 0x1.0000002p+0, 0x1.ffffffcp-1, -0x1p-300, 0x1.fffffffffffffp-1),
    TEST_fff_f (fma, 0x1.deadbeef2feedp+1023, 0x0.deadbeef2feedp-1022, -0x1.a05f8c01a4bfbp+1, 0x1.0989687bc9da4p-53),
    TEST_fff_f (fma, 0x1.deadbeef2feedp+900, 0x0.deadbeef2feedp-1022, -0x1.a05f8c01a4bfbp-122, 0x1.0989687bc9da4p-176),
    TEST_fff_f (fma, 0x1.fffffffffffffp+1023, 0x1.001p+0, -0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1011),
    TEST_fff_f (fma, -0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+0, 0x1.fffffffffffffp+1023, -0x1.ffffffffffffdp+1023),
    TEST_fff_f (fma, 0x1.fffffffffffffp+1023, 2.0, -0x1.fffffffffffffp+1023, 0x1.fffffffffffffp+1023),
    TEST_fff_f (fma, 0x1.6a09e667f3bccp-538, 0x1.6a09e667f3bccp-538, 0.0, 0.0, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.deadbeef2feedp-495, 0x1.deadbeef2feedp-495, -0x1.bf86a5786a574p-989, 0x0.0000042625a1fp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.deadbeef2feedp-503, 0x1.deadbeef2feedp-503, -0x1.bf86a5786a574p-1005, 0x0.0000000004262p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-537, 0x1p-538, 0x1p-1074, 0x0.0000000000002p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.7fffff8p-968, 0x1p-106, 0x0.000001p-1022, 0x0.0000010000001p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.4000004p-967, 0x1p-106, 0x0.000001p-1022, 0x0.0000010000003p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.4p-967, -0x1p-106, -0x0.000001p-1022, -0x0.0000010000002p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.19cab66d73e17p-959, 0x1.c7108a8c5ff51p-107, -0x0.80b0ad65d9b64p-1022, -0x0.80b0ad65d9d59p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.d2eaed6e8e9d3p-979, -0x1.4e066c62ac9ddp-63, -0x0.9245e6b003454p-1022, -0x0.9245c09c5fb5dp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.153d650bb9f06p-907, 0x1.2d01230d48407p-125, -0x0.b278d5acfc3cp-1022, -0x0.b22757123bbe9p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.fffffffffffffp-711, 0x1.fffffffffffffp-275, 0x1.fffffe00007ffp-983, 0x1.7ffffe00007ffp-983),
    TEST_fff_f (fma, 0x1.4p-1022, 0x1.0000000000002p-1, 0x1p-1024, 0x1.c000000000002p-1023, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-1022, 0x1.0000000000002p-1, -0x1p-1024, -0x1.c000000000002p-1023, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.ffffffffffffcp-1022, 0x1.0000000000001p-1, 0x1p-1074, 0x1p-1022, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1.ffffffffffffcp-1022, 0x1.0000000000001p-1, -0x1p-1074, -0x1p-1022, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1, 0x0.fffffffffffffp-1022, 0x1p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-1074, 0x1p-1, -0x0.fffffffffffffp-1022, -0x1p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1.1p-1, 0x0.fffffffffffffp-1022, 0x1p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-1074, 0x1.1p-1, -0x0.fffffffffffffp-1022, -0x1p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p1023, 0x1p1023),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p1023, 0x1p1023),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p1023, -0x1p1023),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p1023, -0x1p1023),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p-1022, 0x1p-1022),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p-1022, 0x1p-1022, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p-1022, -0x1p-1022, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p-1022, -0x1p-1022),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x0.fffffffffffffp-1022, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x0.fffffffffffffp-1022, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x0.fffffffffffffp-1022, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x0.fffffffffffffp-1022, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p-1074, 0x1p-1074, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p-1074, 0x1p-1074, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p-1074, -0x1p-1074, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p-1074, -0x1p-1074, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.fffffffffffff8p0, 0x0.fffffffffffff8p0, -0x0.fffffffffffffp0, 0x1p-106),
    TEST_fff_f (fma, 0x0.fffffffffffff8p0, -0x0.fffffffffffff8p0, 0x0.fffffffffffffp0, -0x1p-106),
    TEST_fff_f (fma, -0x0.fffffffffffff8p0, 0x0.fffffffffffff8p0, 0x0.fffffffffffffp0, -0x1p-106),
    TEST_fff_f (fma, -0x0.fffffffffffff8p0, -0x0.fffffffffffff8p0, -0x0.fffffffffffffp0, 0x1p-106),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, 0x1p1023, 0x1p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, 0x1p1023, 0x1p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, -0x1p1023, -0x1p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, -0x1p1023, -0x1p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, 0x1p970, 0x1p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, 0x1p970, 0x1p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, -0x1p970, -0x1p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, -0x1p970, -0x1p970),
#endif
#if defined (TEST_LDOUBLE) && LDBL_MANT_DIG == 64
    TEST_fff_f (fma, -0x8.03fcp+3696L, 0xf.fffffffffffffffp-6140L, 0x8.3ffffffffffffffp-2450L, -0x8.01ecp-2440L),
    TEST_fff_f (fma, 0x9.fcp+2033L, -0x8.000e1f000ff800fp-3613L, -0xf.fffffffffffc0ffp-1579L, -0xd.fc119fb093ed092p-1577L),
    TEST_fff_f (fma, 0xc.7fc000003ffffffp-1194L, 0x8.1e0003fffffffffp+15327L, -0x8.fffep+14072L, 0xc.ae9f164020effffp+14136L),
    TEST_fff_f (fma, -0x8.0001fc000000003p+1798L, 0xcp-2230L, 0x8.f7e000000000007p-468L, -0xc.0002f9ffee10404p-429L),
    TEST_fff_f (fma, 0xc.0000000000007ffp+10130L, -0x8.000000000000001p+4430L, 0xc.07000000001ffffp+14513L, -0xb.fffffffffffd7e4p+14563L),
    TEST_fff_f (fma, 0xb.ffffp-4777L, 0x8.000000fffffffffp-11612L, -0x0.3800fff8p-16385L, 0x5.c7fe80c7ffeffffp-16385L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.4p-16382L, 0x1.0000000000000004p-1L, 0x1p-16384L, 0x1.c000000000000004p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-16382L, 0x1.0000000000000004p-1L, -0x1p-16384L, -0x1.c000000000000004p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffffffffffffff8p-16382L, 0x1.0000000000000002p-1L, 0x1p-16445L, 0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1.fffffffffffffff8p-16382L, 0x1.0000000000000002p-1L, -0x1p-16445L, -0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-1L, 0x0.fffffffffffffffep-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16445L, 0x1p-1L, -0x0.fffffffffffffffep-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1.1p-1L, 0x0.fffffffffffffffep-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16445L, 0x1.1p-1L, -0x0.fffffffffffffffep-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p-16382L, 0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p-16382L, -0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x0.fffffffffffffffep-16382L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x0.fffffffffffffffep-16382L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x0.fffffffffffffffep-16382L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x0.fffffffffffffffep-16382L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p-16445L, 0x1p-16445L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p-16445L, 0x1p-16445L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p-16445L, -0x1p-16445L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p-16445L, -0x1p-16445L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.ffffffffffffffffp0L, 0x0.ffffffffffffffffp0L, -0x0.fffffffffffffffep0L, 0x1p-128L),
    TEST_fff_f (fma, 0x0.ffffffffffffffffp0L, -0x0.ffffffffffffffffp0L, 0x0.fffffffffffffffep0L, -0x1p-128L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffp0L, 0x0.ffffffffffffffffp0L, 0x0.fffffffffffffffep0L, -0x1p-128L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffp0L, -0x0.ffffffffffffffffp0L, -0x0.fffffffffffffffep0L, 0x1p-128L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, 0x1p16319L, 0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, 0x1p16319L, 0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, -0x1p16319L, -0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, -0x1p16319L, -0x1p16319L),
#endif
#if defined (TEST_LDOUBLE) && LDBL_MANT_DIG == 113
    TEST_fff_f (fma, 0x1.bb2de33e02ccbbfa6e245a7c1f71p-2584L, -0x1.6b500daf0580d987f1bc0cadfcddp-13777L, 0x1.613cd91d9fed34b33820e5ab9d8dp-16378L, -0x1.3a79fb50eb9ce887cffa0f09bd9fp-16360L),
    TEST_fff_f (fma, -0x1.f949b880cacb0f0c61540105321dp-5954L, -0x1.3876cec84b4140f3bd6198731b7ep-10525L, -0x0.a5dc1c6cfbc498c54fb0b504bf19p-16382L, -0x0.a5dc1c6cfbc498c54fb0b5038abbp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.0000fffffffffp-16221L, 0x1.0000001fffff8007fep-239L, 0x0.ff87ffffffffffffe000003fffffp-16382L, 0x0.ff87ffffffffffffdffc003bff7fp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.ac79c9376ef447f3827c9e9de008p-2228L, -0x1.5ba830022b6139e21fbe7270cad8p-6314L, 0x1.e8282b6a26bb6a9daf5c8e73e9f9p-8616L, 0x1.22f14a0253878a730cd1aee373adp-8541L),
    TEST_fff_f (fma, -0x1.c69749ec574caaa2ab8e97ddb9f3p+2652L, 0x1.f34235ff9d095449c29b4831b62dp+3311L, 0x1.fbe4302df23354dbd0c4d3cfe606p+5879L, -0x1.bb473bfdfb7a6e18886ce6e57eafp+5964L),
    TEST_fff_f (fma, -0x1.ca8835fc6ecfb5398625fc891be5p-1686L, 0x1.621e1972bbe2180e5be9dd7d8df5p-7671L, -0x1.7d2d21b73b52cf20dec2a83902a4p-9395L, -0x1.3d2322191c9c88bc68a62ab8042cp-9356L),
    TEST_fff_f (fma, -0x1.55cff679ec49c2541fab41fc843ep-11819L, 0x1.e60e9f464f9e8df0509647c7c971p+12325L, 0x1.eaa2a7649d765c2f564f7a5beca7p+454L, -0x1.447e29fa7e406a285f4e350fcf86p+507L),
    TEST_fff_f (fma, 0x1.f0e7b1454908576f2537d863cf9bp+11432L, 0x1.cdce52f09d4ca76e68706f34b5d5p-1417L, -0x1.2e986187c70f146235ea2066e486p+9979L, 0x1.c030dad3cc5643f3dd0f5619f661p+10016L),
    TEST_fff_f (fma, 0x1.f102f7da4a57a3a4aab620e29452p-3098L, -0x1.cc06a4ff40248f9e2dcc4b6afd84p-11727L, 0x1.d512a11126b5ac8ed8973b8580c8p-14849L, -0x1.be8f1cf737ab4d1c31c54f5ec23bp-14824L),
    TEST_fff_f (fma, -0x1.fc47ac7434b993cd8dcb2b431f25p-3816L, 0x1.fbc9750da8468852d84558e1db6dp-5773L, -0x1.00a98abf783f75c40fe5b7a37d86p-9607L, -0x1.f81917b166f45e763cfcc057e2adp-9588L),
    TEST_fff_f (fma, 0x1.00000000000007ffffffffffffffp-9045L, -0x1.ffffffffffff80000001ffffffffp+4773L, -0x1.f8p-4316L, -0x1.00000000000f88000000fffffdffp-4271L),
    TEST_fff_f (fma, 0x1.4e922764c90701d4a2f21d01893dp-8683L, -0x1.955a12e2d7c9447c27fa022fc865p+212L, -0x1.e9634462eaef96528b90b6944578p-8521L, -0x1.08e1783184a371943d3598e10865p-8470L),
    TEST_fff_f (fma, 0x1.801181509c03bdbef10d6165588cp-15131L, 0x1.ad86f8e57d3d40bfa8007780af63p-368L, -0x1.6e9df0dab1c9f1d7a6043c390741p-15507L, 0x1.417c9b2b15e2ad57dc9e0e920844p-15498L),
    TEST_fff_f (fma, 0x1.4p-16382L, 0x1.0000000000000000000000000002p-1L, 0x1p-16384L, 0x1.c000000000000000000000000002p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-16382L, 0x1.0000000000000000000000000002p-1L, -0x1p-16384L, -0x1.c000000000000000000000000002p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffffffffffffffffffffffffffcp-16382L, 0x1.0000000000000000000000000001p-1L, 0x1p-16494L, 0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1.fffffffffffffffffffffffffffcp-16382L, 0x1.0000000000000000000000000001p-1L, -0x1p-16494L, -0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-1L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16494L, 0x1p-1L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1.1p-1L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16494L, 0x1.1p-1L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p-16382L, 0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p-16382L, -0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p-16494L, 0x1p-16494L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p-16494L, 0x1p-16494L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p-16494L, -0x1p-16494L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p-16494L, -0x1p-16494L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffffp0L, 0x1p-226L),
    TEST_fff_f (fma, 0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffffp0L, -0x1p-226L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffffp0L, -0x1p-226L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffffp0L, 0x1p-226L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, 0x1p16319L, 0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, 0x1p16319L, 0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, -0x1p16319L, -0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, -0x1p16319L, -0x1p16319L),
#endif
    END_DATA (fma)
  };

static void
fma_test (void)
{
  START (fma);
  RUN_TEST_LOOP_fff_f (fma, fma_test_data, );
  END (fma);
}


static const struct test_fff_f_data fma_towardzero_test_data[] =
  {
    START_DATA (fma_towardzero),
    TEST_fff_f (fma, plus_zero, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, plus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, minus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, plus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, minus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, plus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, minus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, -1.0, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, -1.0, plus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, -1.0, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, -1.0, minus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, 1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, 1.0, minus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, -1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, -1.0, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, 1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, 1.0, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, -1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, -1.0, minus_zero, plus_zero),

    TEST_fff_f (fma, 1.0, 1.0, -1.0, plus_zero),
    TEST_fff_f (fma, 1.0, -1.0, 1.0, plus_zero),
    TEST_fff_f (fma, -1.0, 1.0, 1.0, plus_zero),
    TEST_fff_f (fma, -1.0, -1.0, -1.0, plus_zero),

    TEST_fff_f (fma, min_value, min_value, plus_zero, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, min_value, minus_zero, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, -min_value, plus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, -min_value, minus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, min_value, plus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, min_value, minus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, -min_value, plus_zero, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, -min_value, minus_zero, plus_zero, UNDERFLOW_EXCEPTION),

#if !(defined TEST_LDOUBLE && LDBL_MANT_DIG == 106) /* Bug 13304.  */
    TEST_fff_f (fma, max_value, max_value, min_value, max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, max_value, -min_value, max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, -max_value, min_value, -max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, -max_value, -min_value, -max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, max_value, min_value, -max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, max_value, -min_value, -max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, -max_value, min_value, max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, -max_value, -min_value, max_value, OVERFLOW_EXCEPTION),
#endif

#if defined (TEST_FLOAT) && FLT_MANT_DIG == 24
    TEST_fff_f (fma, 0x1.4p-126, 0x1.000004p-1, 0x1p-128, 0x1.c00004p-127, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-126, 0x1.000004p-1, -0x1p-128, -0x1.c00004p-127, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffff8p-126, 0x1.000002p-1, 0x1p-149, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.fffff8p-126, 0x1.000002p-1, -0x1p-149, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-1, 0x0.fffffep-126, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-149, 0x1p-1, -0x0.fffffep-126, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1.1p-1, 0x0.fffffep-126, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-149, 0x1.1p-1, -0x0.fffffep-126, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p127, 0x1p127),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p127, 0x0.ffffffp127),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p127, -0x0.ffffffp127),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p127, -0x1p127),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p-126, 0x1p-126),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p-126, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p-126, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p-126, -0x1p-126),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x0.fffffep-126, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x0.fffffep-126, 0x0.fffffcp-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x0.fffffep-126, -0x0.fffffcp-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x0.fffffep-126, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p-149, 0x1p-149, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p-149, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p-149, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p-149, -0x1p-149, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.fffp0, 0x0.fffp0, -0x0.ffep0, 0x1p-24),
    TEST_fff_f (fma, 0x0.fffp0, -0x0.fffp0, 0x0.ffep0, -0x1p-24),
    TEST_fff_f (fma, -0x0.fffp0, 0x0.fffp0, 0x0.ffep0, -0x1p-24),
    TEST_fff_f (fma, -0x0.fffp0, -0x0.fffp0, -0x0.ffep0, 0x1p-24),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, 0x1p127, 0x1p127),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, 0x1p127, 0x0.ffffffp127),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, -0x1p127, -0x0.ffffffp127),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, -0x1p127, -0x1p127),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, 0x1p103, 0x1p103),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, 0x1p103, 0x0.ffffffp103),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, -0x1p103, -0x0.ffffffp103),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, -0x1p103, -0x1p103),
#endif
#if defined (TEST_DOUBLE) && DBL_MANT_DIG == 53
    TEST_fff_f (fma, 0x1.4p-1022, 0x1.0000000000002p-1, 0x1p-1024, 0x1.c000000000002p-1023, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-1022, 0x1.0000000000002p-1, -0x1p-1024, -0x1.c000000000002p-1023, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.ffffffffffffcp-1022, 0x1.0000000000001p-1, 0x1p-1074, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.ffffffffffffcp-1022, 0x1.0000000000001p-1, -0x1p-1074, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1, 0x0.fffffffffffffp-1022, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-1074, 0x1p-1, -0x0.fffffffffffffp-1022, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1.1p-1, 0x0.fffffffffffffp-1022, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-1074, 0x1.1p-1, -0x0.fffffffffffffp-1022, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p1023, 0x1p1023),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p1023, 0x0.fffffffffffff8p1023),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p1023, -0x0.fffffffffffff8p1023),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p1023, -0x1p1023),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p-1022, 0x1p-1022),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p-1022, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p-1022, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p-1022, -0x1p-1022),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x0.fffffffffffffp-1022, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x0.fffffffffffffp-1022, 0x0.ffffffffffffep-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x0.fffffffffffffp-1022, -0x0.ffffffffffffep-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x0.fffffffffffffp-1022, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p-1074, 0x1p-1074, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p-1074, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p-1074, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p-1074, -0x1p-1074, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.fffffffffffff8p0, 0x0.fffffffffffff8p0, -0x0.fffffffffffffp0, 0x1p-106),
    TEST_fff_f (fma, 0x0.fffffffffffff8p0, -0x0.fffffffffffff8p0, 0x0.fffffffffffffp0, -0x1p-106),
    TEST_fff_f (fma, -0x0.fffffffffffff8p0, 0x0.fffffffffffff8p0, 0x0.fffffffffffffp0, -0x1p-106),
    TEST_fff_f (fma, -0x0.fffffffffffff8p0, -0x0.fffffffffffff8p0, -0x0.fffffffffffffp0, 0x1p-106),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, 0x1p1023, 0x1p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, 0x1p1023, 0x0.fffffffffffff8p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, -0x1p1023, -0x0.fffffffffffff8p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, -0x1p1023, -0x1p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, 0x1p970, 0x1p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, 0x1p970, 0x0.fffffffffffff8p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, -0x1p970, -0x0.fffffffffffff8p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, -0x1p970, -0x1p970),
#endif
#if defined (TEST_LDOUBLE) && LDBL_MANT_DIG == 64
    TEST_fff_f (fma, 0x1.4p-16382L, 0x1.0000000000000004p-1L, 0x1p-16384L, 0x1.c000000000000004p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-16382L, 0x1.0000000000000004p-1L, -0x1p-16384L, -0x1.c000000000000004p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffffffffffffff8p-16382L, 0x1.0000000000000002p-1L, 0x1p-16445L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.fffffffffffffff8p-16382L, 0x1.0000000000000002p-1L, -0x1p-16445L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-1L, 0x0.fffffffffffffffep-16382L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16445L, 0x1p-1L, -0x0.fffffffffffffffep-16382L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1.1p-1L, 0x0.fffffffffffffffep-16382L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16445L, 0x1.1p-1L, -0x0.fffffffffffffffep-16382L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p16383L, 0x0.ffffffffffffffffp16383L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p16383L, -0x0.ffffffffffffffffp16383L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p-16382L, 0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p-16382L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p-16382L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p-16382L, -0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x0.fffffffffffffffep-16382L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x0.fffffffffffffffep-16382L, 0x0.fffffffffffffffcp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x0.fffffffffffffffep-16382L, -0x0.fffffffffffffffcp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x0.fffffffffffffffep-16382L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p-16445L, 0x1p-16445L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p-16445L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p-16445L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p-16445L, -0x1p-16445L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.ffffffffffffffffp0L, 0x0.ffffffffffffffffp0L, -0x0.fffffffffffffffep0L, 0x1p-128L),
    TEST_fff_f (fma, 0x0.ffffffffffffffffp0L, -0x0.ffffffffffffffffp0L, 0x0.fffffffffffffffep0L, -0x1p-128L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffp0L, 0x0.ffffffffffffffffp0L, 0x0.fffffffffffffffep0L, -0x1p-128L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffp0L, -0x0.ffffffffffffffffp0L, -0x0.fffffffffffffffep0L, 0x1p-128L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, 0x1p16383L, 0x0.ffffffffffffffffp16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, -0x1p16383L, -0x0.ffffffffffffffffp16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, 0x1p16319L, 0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, 0x1p16319L, 0x0.ffffffffffffffffp16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, -0x1p16319L, -0x0.ffffffffffffffffp16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, -0x1p16319L, -0x1p16319L),
#endif
#if defined (TEST_LDOUBLE) && LDBL_MANT_DIG == 113
    TEST_fff_f (fma, 0x1.4p-16382L, 0x1.0000000000000000000000000002p-1L, 0x1p-16384L, 0x1.c000000000000000000000000002p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-16382L, 0x1.0000000000000000000000000002p-1L, -0x1p-16384L, -0x1.c000000000000000000000000002p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffffffffffffffffffffffffffcp-16382L, 0x1.0000000000000000000000000001p-1L, 0x1p-16494L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.fffffffffffffffffffffffffffcp-16382L, 0x1.0000000000000000000000000001p-1L, -0x1p-16494L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-1L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16494L, 0x1p-1L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1.1p-1L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16494L, 0x1.1p-1L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p16383L, 0x0.ffffffffffffffffffffffffffff8p16383L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p16383L, -0x0.ffffffffffffffffffffffffffff8p16383L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p-16382L, 0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p-16382L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p-16382L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p-16382L, -0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x0.fffffffffffffffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x0.fffffffffffffffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p-16494L, 0x1p-16494L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p-16494L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p-16494L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p-16494L, -0x1p-16494L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffffp0L, 0x1p-226L),
    TEST_fff_f (fma, 0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffffp0L, -0x1p-226L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffffp0L, -0x1p-226L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffffp0L, 0x1p-226L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, 0x1p16383L, 0x0.ffffffffffffffffffffffffffff8p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, -0x1p16383L, -0x0.ffffffffffffffffffffffffffff8p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, 0x1p16319L, 0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, 0x1p16319L, 0x0.ffffffffffffffffffffffffffff8p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, -0x1p16319L, -0x0.ffffffffffffffffffffffffffff8p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, -0x1p16319L, -0x1p16319L),
#endif
    END_DATA (fma_towardzero)
  };

static void
fma_test_towardzero (void)
{
  START (fma_towardzero);
  RUN_TEST_LOOP_fff_f (fma, fma_towardzero_test_data, FE_TOWARDZERO);
  END (fma_towardzero);
}


static const struct test_fff_f_data fma_downward_test_data[] =
  {
    START_DATA (fma_downward),
    TEST_fff_f (fma, plus_zero, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, plus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, plus_zero, minus_zero, plus_zero, minus_zero),
    TEST_fff_f (fma, plus_zero, minus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, plus_zero, plus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, plus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, minus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, 1.0, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, plus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, 1.0, minus_zero, plus_zero, minus_zero),
    TEST_fff_f (fma, 1.0, minus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, -1.0, plus_zero, plus_zero, minus_zero),
    TEST_fff_f (fma, -1.0, plus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, -1.0, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, -1.0, minus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, plus_zero, 1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, 1.0, minus_zero, minus_zero),
    TEST_fff_f (fma, plus_zero, -1.0, plus_zero, minus_zero),
    TEST_fff_f (fma, plus_zero, -1.0, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, 1.0, plus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, 1.0, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, -1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, -1.0, minus_zero, minus_zero),

    TEST_fff_f (fma, 1.0, 1.0, -1.0, minus_zero),
    TEST_fff_f (fma, 1.0, -1.0, 1.0, minus_zero),
    TEST_fff_f (fma, -1.0, 1.0, 1.0, minus_zero),
    TEST_fff_f (fma, -1.0, -1.0, -1.0, minus_zero),

    TEST_fff_f (fma, min_value, min_value, plus_zero, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, min_value, minus_zero, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, -min_value, plus_zero, -min_subnorm_value, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, -min_value, minus_zero, -min_subnorm_value, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, min_value, plus_zero, -min_subnorm_value, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, min_value, minus_zero, -min_subnorm_value, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, -min_value, plus_zero, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, -min_value, minus_zero, plus_zero, UNDERFLOW_EXCEPTION),

#if !(defined TEST_LDOUBLE && LDBL_MANT_DIG == 106) /* Bug 13304.  */
    TEST_fff_f (fma, max_value, max_value, min_value, max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, max_value, -min_value, max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, -max_value, min_value, minus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, -max_value, -min_value, minus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, max_value, min_value, minus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, max_value, -min_value, minus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, -max_value, min_value, max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, -max_value, -min_value, max_value, OVERFLOW_EXCEPTION),
#endif

#if defined (TEST_FLOAT) && FLT_MANT_DIG == 24
    TEST_fff_f (fma, 0x1.4p-126, 0x1.000004p-1, 0x1p-128, 0x1.c00004p-127, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-126, 0x1.000004p-1, -0x1p-128, -0x1.c00008p-127, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffff8p-126, 0x1.000002p-1, 0x1p-149, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.fffff8p-126, 0x1.000002p-1, -0x1p-149, -0x1p-126, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-149, 0x1p-1, 0x0.fffffep-126, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-149, 0x1p-1, -0x0.fffffep-126, -0x1p-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1.1p-1, 0x0.fffffep-126, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-149, 0x1.1p-1, -0x0.fffffep-126, -0x1p-126, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p127, 0x1p127),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p127, 0x0.ffffffp127),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p127, -0x1p127),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p127, -0x1.000002p127),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p-126, 0x1p-126),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p-126, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p-126, -0x1p-126, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p-126, -0x1.000002p-126),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x0.fffffep-126, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x0.fffffep-126, 0x0.fffffcp-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x0.fffffep-126, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x0.fffffep-126, -0x1p-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p-149, 0x1p-149, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p-149, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p-149, -0x1p-149, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p-149, -0x1p-148, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.fffp0, 0x0.fffp0, -0x0.ffep0, 0x1p-24),
    TEST_fff_f (fma, 0x0.fffp0, -0x0.fffp0, 0x0.ffep0, -0x1p-24),
    TEST_fff_f (fma, -0x0.fffp0, 0x0.fffp0, 0x0.ffep0, -0x1p-24),
    TEST_fff_f (fma, -0x0.fffp0, -0x0.fffp0, -0x0.ffep0, 0x1p-24),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, 0x1p127, 0x1p127),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, 0x1p127, 0x0.ffffffp127),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, -0x1p127, -0x1p127),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, -0x1p127, -0x1.000002p127),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, 0x1p103, 0x1p103),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, 0x1p103, 0x0.ffffffp103),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, -0x1p103, -0x1p103),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, -0x1p103, -0x1.000002p103),
#endif
#if defined (TEST_DOUBLE) && DBL_MANT_DIG == 53
    TEST_fff_f (fma, 0x1.4p-1022, 0x1.0000000000002p-1, 0x1p-1024, 0x1.c000000000002p-1023, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-1022, 0x1.0000000000002p-1, -0x1p-1024, -0x1.c000000000004p-1023, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.ffffffffffffcp-1022, 0x1.0000000000001p-1, 0x1p-1074, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.ffffffffffffcp-1022, 0x1.0000000000001p-1, -0x1p-1074, -0x1p-1022, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1, 0x0.fffffffffffffp-1022, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-1074, 0x1p-1, -0x0.fffffffffffffp-1022, -0x1p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1.1p-1, 0x0.fffffffffffffp-1022, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-1074, 0x1.1p-1, -0x0.fffffffffffffp-1022, -0x1p-1022, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p1023, 0x1p1023),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p1023, 0x0.fffffffffffff8p1023),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p1023, -0x1p1023),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p1023, -0x1.0000000000001p1023),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p-1022, 0x1p-1022),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p-1022, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p-1022, -0x1p-1022, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p-1022, -0x1.0000000000001p-1022),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x0.fffffffffffffp-1022, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x0.fffffffffffffp-1022, 0x0.ffffffffffffep-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x0.fffffffffffffp-1022, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x0.fffffffffffffp-1022, -0x1p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p-1074, 0x1p-1074, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p-1074, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p-1074, -0x1p-1074, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p-1074, -0x1p-1073, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.fffffffffffff8p0, 0x0.fffffffffffff8p0, -0x0.fffffffffffffp0, 0x1p-106),
    TEST_fff_f (fma, 0x0.fffffffffffff8p0, -0x0.fffffffffffff8p0, 0x0.fffffffffffffp0, -0x1p-106),
    TEST_fff_f (fma, -0x0.fffffffffffff8p0, 0x0.fffffffffffff8p0, 0x0.fffffffffffffp0, -0x1p-106),
    TEST_fff_f (fma, -0x0.fffffffffffff8p0, -0x0.fffffffffffff8p0, -0x0.fffffffffffffp0, 0x1p-106),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, 0x1p1023, 0x1p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, 0x1p1023, 0x0.fffffffffffff8p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, -0x1p1023, -0x1p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, -0x1p1023, -0x1.0000000000001p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, 0x1p970, 0x1p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, 0x1p970, 0x0.fffffffffffff8p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, -0x1p970, -0x1p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, -0x1p970, -0x1.0000000000001p970),
#endif
#if defined (TEST_LDOUBLE) && LDBL_MANT_DIG == 64
    TEST_fff_f (fma, 0x1.4p-16382L, 0x1.0000000000000004p-1L, 0x1p-16384L, 0x1.c000000000000004p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-16382L, 0x1.0000000000000004p-1L, -0x1p-16384L, -0x1.c000000000000008p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffffffffffffff8p-16382L, 0x1.0000000000000002p-1L, 0x1p-16445L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.fffffffffffffff8p-16382L, 0x1.0000000000000002p-1L, -0x1p-16445L, -0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-1L, 0x0.fffffffffffffffep-16382L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16445L, 0x1p-1L, -0x0.fffffffffffffffep-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1.1p-1L, 0x0.fffffffffffffffep-16382L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16445L, 0x1.1p-1L, -0x0.fffffffffffffffep-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p16383L, 0x0.ffffffffffffffffp16383L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p16383L, -0x1.0000000000000002p16383L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p-16382L, 0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p-16382L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p-16382L, -0x1.0000000000000002p-16382L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x0.fffffffffffffffep-16382L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x0.fffffffffffffffep-16382L, 0x0.fffffffffffffffcp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x0.fffffffffffffffep-16382L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x0.fffffffffffffffep-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p-16445L, 0x1p-16445L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p-16445L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p-16445L, -0x1p-16445L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p-16445L, -0x1p-16444L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.ffffffffffffffffp0L, 0x0.ffffffffffffffffp0L, -0x0.fffffffffffffffep0L, 0x1p-128L),
    TEST_fff_f (fma, 0x0.ffffffffffffffffp0L, -0x0.ffffffffffffffffp0L, 0x0.fffffffffffffffep0L, -0x1p-128L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffp0L, 0x0.ffffffffffffffffp0L, 0x0.fffffffffffffffep0L, -0x1p-128L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffp0L, -0x0.ffffffffffffffffp0L, -0x0.fffffffffffffffep0L, 0x1p-128L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, 0x1p16383L, 0x0.ffffffffffffffffp16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, -0x1p16383L, -0x1.0000000000000002p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, 0x1p16319L, 0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, 0x1p16319L, 0x0.ffffffffffffffffp16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, -0x1p16319L, -0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, -0x1p16319L, -0x1.0000000000000002p16319L),
#endif
#if defined (TEST_LDOUBLE) && LDBL_MANT_DIG == 113
    TEST_fff_f (fma, 0x1.4p-16382L, 0x1.0000000000000000000000000002p-1L, 0x1p-16384L, 0x1.c000000000000000000000000002p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-16382L, 0x1.0000000000000000000000000002p-1L, -0x1p-16384L, -0x1.c000000000000000000000000004p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffffffffffffffffffffffffffcp-16382L, 0x1.0000000000000000000000000001p-1L, 0x1p-16494L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.fffffffffffffffffffffffffffcp-16382L, 0x1.0000000000000000000000000001p-1L, -0x1p-16494L, -0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-1L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16494L, 0x1p-1L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1.1p-1L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16494L, 0x1.1p-1L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p16383L, 0x0.ffffffffffffffffffffffffffff8p16383L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p16383L, -0x1.0000000000000000000000000001p16383L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p-16382L, 0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p-16382L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p-16382L, -0x1.0000000000000000000000000001p-16382L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x0.fffffffffffffffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p-16494L, 0x1p-16494L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p-16494L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p-16494L, -0x1p-16494L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p-16494L, -0x1p-16493L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffffp0L, 0x1p-226L),
    TEST_fff_f (fma, 0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffffp0L, -0x1p-226L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffffp0L, -0x1p-226L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffffp0L, 0x1p-226L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, 0x1p16383L, 0x0.ffffffffffffffffffffffffffff8p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, -0x1p16383L, -0x1.0000000000000000000000000001p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, 0x1p16319L, 0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, 0x1p16319L, 0x0.ffffffffffffffffffffffffffff8p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, -0x1p16319L, -0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, -0x1p16319L, -0x1.0000000000000000000000000001p16319L),
#endif
    END_DATA (fma_downward)
  };

static void
fma_test_downward (void)
{
  START (fma_downward);
  RUN_TEST_LOOP_fff_f (fma, fma_downward_test_data, FE_DOWNWARD);
  END (fma_downward);
}


static const struct test_fff_f_data fma_upward_test_data[] =
  {
    START_DATA (fma_upward),
    TEST_fff_f (fma, plus_zero, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, plus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, minus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, plus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, minus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, plus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, 1.0, minus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, -1.0, plus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, -1.0, plus_zero, minus_zero, minus_zero),
    TEST_fff_f (fma, -1.0, minus_zero, plus_zero, plus_zero),
    TEST_fff_f (fma, -1.0, minus_zero, minus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, 1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, 1.0, minus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, -1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, plus_zero, -1.0, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, 1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, 1.0, minus_zero, minus_zero),
    TEST_fff_f (fma, minus_zero, -1.0, plus_zero, plus_zero),
    TEST_fff_f (fma, minus_zero, -1.0, minus_zero, plus_zero),

    TEST_fff_f (fma, 1.0, 1.0, -1.0, plus_zero),
    TEST_fff_f (fma, 1.0, -1.0, 1.0, plus_zero),
    TEST_fff_f (fma, -1.0, 1.0, 1.0, plus_zero),
    TEST_fff_f (fma, -1.0, -1.0, -1.0, plus_zero),

    TEST_fff_f (fma, min_value, min_value, plus_zero, min_subnorm_value, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, min_value, minus_zero, min_subnorm_value, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, -min_value, plus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, min_value, -min_value, minus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, min_value, plus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, min_value, minus_zero, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, -min_value, plus_zero, min_subnorm_value, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -min_value, -min_value, minus_zero, min_subnorm_value, UNDERFLOW_EXCEPTION),

#if !(defined TEST_LDOUBLE && LDBL_MANT_DIG == 106) /* Bug 13304.  */
    TEST_fff_f (fma, max_value, max_value, min_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, max_value, -min_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, -max_value, min_value, -max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, max_value, -max_value, -min_value, -max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, max_value, min_value, -max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, max_value, -min_value, -max_value, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, -max_value, min_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fff_f (fma, -max_value, -max_value, -min_value, plus_infty, OVERFLOW_EXCEPTION),
#endif

#if defined (TEST_FLOAT) && FLT_MANT_DIG == 24
    TEST_fff_f (fma, 0x1.4p-126, 0x1.000004p-1, 0x1p-128, 0x1.c00008p-127, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-126, 0x1.000004p-1, -0x1p-128, -0x1.c00004p-127, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffff8p-126, 0x1.000002p-1, 0x1p-149, 0x1p-126, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1.fffff8p-126, 0x1.000002p-1, -0x1p-149, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-1, 0x0.fffffep-126, 0x1p-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-149, 0x1p-1, -0x0.fffffep-126, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1.1p-1, 0x0.fffffep-126, 0x1p-126, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1p-149, 0x1.1p-1, -0x0.fffffep-126, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p127, 0x1.000002p127),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p127, 0x1p127),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p127, -0x0.ffffffp127),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p127, -0x1p127),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p-126, 0x1.000002p-126),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p-126, 0x1p-126, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p-126, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p-126, -0x1p-126),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x0.fffffep-126, 0x1p-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x0.fffffep-126, 0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x0.fffffep-126, -0x0.fffffcp-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x0.fffffep-126, -0x0.fffffep-126, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, 0x1p-149, 0x1p-148, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, 0x1p-149, 0x1p-149, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, 0x1p-149, -0x1p-149, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-149, -0x1p-149, -0x1p-149, -0x1p-149, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.fffp0, 0x0.fffp0, -0x0.ffep0, 0x1p-24),
    TEST_fff_f (fma, 0x0.fffp0, -0x0.fffp0, 0x0.ffep0, -0x1p-24),
    TEST_fff_f (fma, -0x0.fffp0, 0x0.fffp0, 0x0.ffep0, -0x1p-24),
    TEST_fff_f (fma, -0x0.fffp0, -0x0.fffp0, -0x0.ffep0, 0x1p-24),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, 0x1p127, 0x1.000002p127),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, 0x1p127, 0x1p127),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, -0x1p127, -0x0.ffffffp127),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, -0x1p127, -0x1p127),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, 0x1p103, 0x1.000002p103),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, 0x1p103, 0x1p103),
    TEST_fff_f (fma, 0x1.000002p-126, 0x1.000002p-26, -0x1p103, -0x0.ffffffp103),
    TEST_fff_f (fma, 0x1.000002p-126, -0x1.000002p-26, -0x1p103, -0x1p103),
#endif
#if defined (TEST_DOUBLE) && DBL_MANT_DIG == 53
    TEST_fff_f (fma, 0x1.4p-1022, 0x1.0000000000002p-1, 0x1p-1024, 0x1.c000000000004p-1023, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-1022, 0x1.0000000000002p-1, -0x1p-1024, -0x1.c000000000002p-1023, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.ffffffffffffcp-1022, 0x1.0000000000001p-1, 0x1p-1074, 0x1p-1022, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1.ffffffffffffcp-1022, 0x1.0000000000001p-1, -0x1p-1074, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1, 0x0.fffffffffffffp-1022, 0x1p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-1074, 0x1p-1, -0x0.fffffffffffffp-1022, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1.1p-1, 0x0.fffffffffffffp-1022, 0x1p-1022, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1p-1074, 0x1.1p-1, -0x0.fffffffffffffp-1022, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p1023, 0x1.0000000000001p1023),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p1023, 0x1p1023),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p1023, -0x0.fffffffffffff8p1023),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p1023, -0x1p1023),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p-1022, 0x1.0000000000001p-1022),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p-1022, 0x1p-1022, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p-1022, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p-1022, -0x1p-1022),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x0.fffffffffffffp-1022, 0x1p-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x0.fffffffffffffp-1022, 0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x0.fffffffffffffp-1022, -0x0.ffffffffffffep-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x0.fffffffffffffp-1022, -0x0.fffffffffffffp-1022, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, 0x1p-1074, 0x1p-1073, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, 0x1p-1074, 0x1p-1074, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, 0x1p-1074, -0x1p-1074, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-1074, -0x1p-1074, -0x1p-1074, -0x1p-1074, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.fffffffffffff8p0, 0x0.fffffffffffff8p0, -0x0.fffffffffffffp0, 0x1p-106),
    TEST_fff_f (fma, 0x0.fffffffffffff8p0, -0x0.fffffffffffff8p0, 0x0.fffffffffffffp0, -0x1p-106),
    TEST_fff_f (fma, -0x0.fffffffffffff8p0, 0x0.fffffffffffff8p0, 0x0.fffffffffffffp0, -0x1p-106),
    TEST_fff_f (fma, -0x0.fffffffffffff8p0, -0x0.fffffffffffff8p0, -0x0.fffffffffffffp0, 0x1p-106),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, 0x1p1023, 0x1.0000000000001p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, 0x1p1023, 0x1p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, -0x1p1023, -0x0.fffffffffffff8p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, -0x1p1023, -0x1p1023),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, 0x1p970, 0x1.0000000000001p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, 0x1p970, 0x1p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, 0x1.0000000000001p-55, -0x1p970, -0x0.fffffffffffff8p970),
    TEST_fff_f (fma, 0x1.0000000000001p-1022, -0x1.0000000000001p-55, -0x1p970, -0x1p970),
#endif
#if defined (TEST_LDOUBLE) && LDBL_MANT_DIG == 64
    TEST_fff_f (fma, 0x1.4p-16382L, 0x1.0000000000000004p-1L, 0x1p-16384L, 0x1.c000000000000008p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-16382L, 0x1.0000000000000004p-1L, -0x1p-16384L, -0x1.c000000000000004p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffffffffffffff8p-16382L, 0x1.0000000000000002p-1L, 0x1p-16445L, 0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1.fffffffffffffff8p-16382L, 0x1.0000000000000002p-1L, -0x1p-16445L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-1L, 0x0.fffffffffffffffep-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16445L, 0x1p-1L, -0x0.fffffffffffffffep-16382L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1.1p-1L, 0x0.fffffffffffffffep-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1p-16445L, 0x1.1p-1L, -0x0.fffffffffffffffep-16382L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p16383L, 0x1.0000000000000002p16383L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p16383L, -0x0.ffffffffffffffffp16383L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p-16382L, 0x1.0000000000000002p-16382L),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p-16382L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p-16382L, -0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x0.fffffffffffffffep-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x0.fffffffffffffffep-16382L, 0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x0.fffffffffffffffep-16382L, -0x0.fffffffffffffffcp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x0.fffffffffffffffep-16382L, -0x0.fffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, 0x1p-16445L, 0x1p-16444L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, 0x1p-16445L, 0x1p-16445L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, 0x1p-16445L, -0x1p-16445L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16445L, -0x1p-16445L, -0x1p-16445L, -0x1p-16445L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.ffffffffffffffffp0L, 0x0.ffffffffffffffffp0L, -0x0.fffffffffffffffep0L, 0x1p-128L),
    TEST_fff_f (fma, 0x0.ffffffffffffffffp0L, -0x0.ffffffffffffffffp0L, 0x0.fffffffffffffffep0L, -0x1p-128L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffp0L, 0x0.ffffffffffffffffp0L, 0x0.fffffffffffffffep0L, -0x1p-128L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffp0L, -0x0.ffffffffffffffffp0L, -0x0.fffffffffffffffep0L, 0x1p-128L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, 0x1p16383L, 0x1.0000000000000002p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, -0x1p16383L, -0x0.ffffffffffffffffp16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, 0x1p16319L, 0x1.0000000000000002p16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, 0x1p16319L, 0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, 0x1.0000000000000002p-66L, -0x1p16319L, -0x0.ffffffffffffffffp16319L),
    TEST_fff_f (fma, 0x1.0000000000000002p-16382L, -0x1.0000000000000002p-66L, -0x1p16319L, -0x1p16319L),
#endif
#if defined (TEST_LDOUBLE) && LDBL_MANT_DIG == 113
    TEST_fff_f (fma, 0x1.4p-16382L, 0x1.0000000000000000000000000002p-1L, 0x1p-16384L, 0x1.c000000000000000000000000004p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1.4p-16382L, 0x1.0000000000000000000000000002p-1L, -0x1p-16384L, -0x1.c000000000000000000000000002p-16383L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1.fffffffffffffffffffffffffffcp-16382L, 0x1.0000000000000000000000000001p-1L, 0x1p-16494L, 0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1.fffffffffffffffffffffffffffcp-16382L, 0x1.0000000000000000000000000001p-1L, -0x1p-16494L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-1L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, -0x1p-16494L, 0x1p-1L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1.1p-1L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, -0x1p-16494L, 0x1.1p-1L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p16383L, 0x1.0000000000000000000000000001p16383L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p16383L, -0x0.ffffffffffffffffffffffffffff8p16383L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p-16382L, 0x1.0000000000000000000000000001p-16382L),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION_BEFORE_ROUNDING),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p-16382L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p-16382L, -0x1p-16382L),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x1p-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x0.ffffffffffffffffffffffffffffp-16382L, 0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x0.fffffffffffffffffffffffffffep-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x0.ffffffffffffffffffffffffffffp-16382L, -0x0.ffffffffffffffffffffffffffffp-16382L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, 0x1p-16494L, 0x1p-16493L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, 0x1p-16494L, 0x1p-16494L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, 0x1p-16494L, -0x1p-16494L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x1p-16494L, -0x1p-16494L, -0x1p-16494L, -0x1p-16494L, UNDERFLOW_EXCEPTION),
    TEST_fff_f (fma, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffffp0L, 0x1p-226L),
    TEST_fff_f (fma, 0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffffp0L, -0x1p-226L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffffffffffffffffffffffffp0L, -0x1p-226L),
    TEST_fff_f (fma, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffffffffffffffffffffffffp0L, 0x1p-226L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, 0x1p16383L, 0x1.0000000000000000000000000001p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, 0x1p16383L, 0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, -0x1p16383L, -0x0.ffffffffffffffffffffffffffff8p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, -0x1p16383L, -0x1p16383L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, 0x1p16319L, 0x1.0000000000000000000000000001p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, 0x1p16319L, 0x1p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, 0x1.0000000000000000000000000001p-66L, -0x1p16319L, -0x0.ffffffffffffffffffffffffffff8p16319L),
    TEST_fff_f (fma, 0x1.0000000000000000000000000001p-16382L, -0x1.0000000000000000000000000001p-66L, -0x1p16319L, -0x1p16319L),
#endif
    END_DATA (fma_upward)
  };

static void
fma_test_upward (void)
{
  START (fma_upward);
  RUN_TEST_LOOP_fff_f (fma, fma_upward_test_data, FE_UPWARD);
  END (fma_upward);
}


static const struct test_ff_f_data fmax_test_data[] =
  {
    START_DATA (fmax),
    TEST_ff_f (fmax, 0, 0, 0),
    TEST_ff_f (fmax, minus_zero, minus_zero, minus_zero),
    TEST_ff_f (fmax, 9, 0, 9),
    TEST_ff_f (fmax, 0, 9, 9),
    TEST_ff_f (fmax, -9, 0, 0),
    TEST_ff_f (fmax, 0, -9, 0),

    TEST_ff_f (fmax, plus_infty, 9, plus_infty),
    TEST_ff_f (fmax, 0, plus_infty, plus_infty),
    TEST_ff_f (fmax, -9, plus_infty, plus_infty),
    TEST_ff_f (fmax, plus_infty, -9, plus_infty),

    TEST_ff_f (fmax, minus_infty, 9, 9),
    TEST_ff_f (fmax, minus_infty, -9, -9),
    TEST_ff_f (fmax, 9, minus_infty, 9),
    TEST_ff_f (fmax, -9, minus_infty, -9),

    TEST_ff_f (fmax, 0, qnan_value, 0),
    TEST_ff_f (fmax, 9, qnan_value, 9),
    TEST_ff_f (fmax, -9, qnan_value, -9),
    TEST_ff_f (fmax, qnan_value, 0, 0),
    TEST_ff_f (fmax, qnan_value, 9, 9),
    TEST_ff_f (fmax, qnan_value, -9, -9),
    TEST_ff_f (fmax, plus_infty, qnan_value, plus_infty),
    TEST_ff_f (fmax, minus_infty, qnan_value, minus_infty),
    TEST_ff_f (fmax, qnan_value, plus_infty, plus_infty),
    TEST_ff_f (fmax, qnan_value, minus_infty, minus_infty),
    TEST_ff_f (fmax, qnan_value, qnan_value, qnan_value),
    END_DATA (fmax)
  };

static void
fmax_test (void)
{
  START (fmax);
  RUN_TEST_LOOP_ff_f (fmax, fmax_test_data, );
  END (fmax);
}


static const struct test_ff_f_data fmin_test_data[] =
  {
    START_DATA (fmin),
    TEST_ff_f (fmin, 0, 0, 0),
    TEST_ff_f (fmin, minus_zero, minus_zero, minus_zero),
    TEST_ff_f (fmin, 9, 0, 0),
    TEST_ff_f (fmin, 0, 9, 0),
    TEST_ff_f (fmin, -9, 0, -9),
    TEST_ff_f (fmin, 0, -9, -9),

    TEST_ff_f (fmin, plus_infty, 9, 9),
    TEST_ff_f (fmin, 9, plus_infty, 9),
    TEST_ff_f (fmin, plus_infty, -9, -9),
    TEST_ff_f (fmin, -9, plus_infty, -9),
    TEST_ff_f (fmin, minus_infty, 9, minus_infty),
    TEST_ff_f (fmin, minus_infty, -9, minus_infty),
    TEST_ff_f (fmin, 9, minus_infty, minus_infty),
    TEST_ff_f (fmin, -9, minus_infty, minus_infty),

    TEST_ff_f (fmin, 0, qnan_value, 0),
    TEST_ff_f (fmin, 9, qnan_value, 9),
    TEST_ff_f (fmin, -9, qnan_value, -9),
    TEST_ff_f (fmin, qnan_value, 0, 0),
    TEST_ff_f (fmin, qnan_value, 9, 9),
    TEST_ff_f (fmin, qnan_value, -9, -9),
    TEST_ff_f (fmin, plus_infty, qnan_value, plus_infty),
    TEST_ff_f (fmin, minus_infty, qnan_value, minus_infty),
    TEST_ff_f (fmin, qnan_value, plus_infty, plus_infty),
    TEST_ff_f (fmin, qnan_value, minus_infty, minus_infty),
    TEST_ff_f (fmin, qnan_value, qnan_value, qnan_value),
    END_DATA (fmin)
  };

static void
fmin_test (void)
{
  START (fmin);
  RUN_TEST_LOOP_ff_f (fmin, fmin_test_data, );
  END (fmin);
}


static const struct test_ff_f_data fmod_test_data[] =
  {
    START_DATA (fmod),
    /* fmod (+0, y) == +0 for y != 0.  */
    TEST_ff_f (fmod, 0, 3, 0),

    /* fmod (-0, y) == -0 for y != 0.  */
    TEST_ff_f (fmod, minus_zero, 3, minus_zero),

    /* fmod (+inf, y) == qNaN plus invalid exception.  */
    TEST_ff_f (fmod, plus_infty, 3, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    /* fmod (-inf, y) == qNaN plus invalid exception.  */
    TEST_ff_f (fmod, minus_infty, 3, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    /* fmod (x, +0) == qNaN plus invalid exception.  */
    TEST_ff_f (fmod, 3, 0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    /* fmod (x, -0) == qNaN plus invalid exception.  */
    TEST_ff_f (fmod, 3, minus_zero, qnan_value, INVALID_EXCEPTION),

    /* fmod (x, +inf) == x for x not infinite.  */
    TEST_ff_f (fmod, 3.0, plus_infty, 3.0),
    /* fmod (x, -inf) == x for x not infinite.  */
    TEST_ff_f (fmod, 3.0, minus_infty, 3.0),

    TEST_ff_f (fmod, qnan_value, qnan_value, qnan_value),

    TEST_ff_f (fmod, 6.5, 2.25L, 2.0L),
    TEST_ff_f (fmod, -6.5, 2.25L, -2.0L),
    TEST_ff_f (fmod, 6.5, -2.25L, 2.0L),
    TEST_ff_f (fmod, -6.5, -2.25L, -2.0L),

    TEST_ff_f (fmod, 0x0.fffffep-126L, 0x1p-149L, plus_zero),
#ifndef TEST_FLOAT
    TEST_ff_f (fmod, 0x0.fffffffffffffp-1022L, 0x1p-1074L, plus_zero),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_ff_f (fmod, 0x0.fffffffffffffffep-16382L, 0x1p-16445L, plus_zero),
#endif
    END_DATA (fmod)
  };

static void
fmod_test (void)
{
  errno = 0;
  FUNC(fmod) (6.5, 2.3L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (fmod);
  RUN_TEST_LOOP_ff_f (fmod, fmod_test_data, );
  END (fmod);
}


static const struct test_f_i_data fpclassify_test_data[] =
  {
    START_DATA (fpclassify),
    TEST_f_i (fpclassify, qnan_value, FP_NAN),
    TEST_f_i (fpclassify, plus_infty, FP_INFINITE),
    TEST_f_i (fpclassify, minus_infty, FP_INFINITE),
    TEST_f_i (fpclassify, plus_zero, FP_ZERO),
    TEST_f_i (fpclassify, minus_zero, FP_ZERO),
    TEST_f_i (fpclassify, 1000, FP_NORMAL),
    TEST_f_i (fpclassify, min_subnorm_value, FP_SUBNORMAL),
    END_DATA (fpclassify)
  };

static void
fpclassify_test (void)
{
  START (fpclassify);
  RUN_TEST_LOOP_f_i_tg (fpclassify, fpclassify_test_data, );
  END (fpclassify);
}


static const struct test_f_f1_data frexp_test_data[] =
  {
    START_DATA (frexp),
    TEST_fI_f1 (frexp, plus_infty, plus_infty, IGNORE),
    TEST_fI_f1 (frexp, minus_infty, minus_infty, IGNORE),
    TEST_fI_f1 (frexp, qnan_value, qnan_value, IGNORE),

    TEST_fI_f1 (frexp, 0.0, 0.0, 0.0),
    TEST_fI_f1 (frexp, minus_zero, minus_zero, 0.0),

    TEST_fI_f1 (frexp, 12.8L, 0.8L, 4),
    TEST_fI_f1 (frexp, -27.34L, -0.854375L, 5),
    END_DATA (frexp)
  };

static void
frexp_test (void)
{
  int x;

  START (frexp);
  RUN_TEST_LOOP_fI_f1 (frexp, frexp_test_data, , x);
  END (frexp);
}


static const struct test_f_f1_data gamma_test_data[] =
  {
    START_DATA (gamma),
    TEST_f_f1 (gamma, plus_infty, plus_infty, 1),
    TEST_f_f1 (gamma, 0, plus_infty, 1, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f1 (gamma, -3, plus_infty, IGNORE, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f1 (gamma, minus_infty, plus_infty, IGNORE),
    TEST_f_f1 (gamma, qnan_value, qnan_value, IGNORE),

    TEST_f_f1 (gamma, 1, 0, 1),
    TEST_f_f1 (gamma, 3, M_LN2l, 1),

    TEST_f_f1 (gamma, 0.5, M_LOG_SQRT_PIl, 1),
    TEST_f_f1 (gamma, -0.5, M_LOG_2_SQRT_PIl, -1),
    END_DATA (gamma)
  };

static void
gamma_test (void)
{
  errno = 0;
  FUNC(gamma) (1);

  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (gamma);
  RUN_TEST_LOOP_f_f1 (gamma, gamma_test_data, , signgam);
  END (gamma);
}

static const struct test_ff_f_data hypot_test_data[] =
  {
    START_DATA (hypot),
    TEST_ff_f (hypot, plus_infty, 1, plus_infty),
    TEST_ff_f (hypot, minus_infty, 1, plus_infty),

#ifndef TEST_INLINE
    TEST_ff_f (hypot, plus_infty, qnan_value, plus_infty),
    TEST_ff_f (hypot, minus_infty, qnan_value, plus_infty),
    TEST_ff_f (hypot, qnan_value, plus_infty, plus_infty),
    TEST_ff_f (hypot, qnan_value, minus_infty, plus_infty),
#endif

    TEST_ff_f (hypot, qnan_value, qnan_value, qnan_value),

    /* hypot (x,y) == hypot (+-x, +-y)  */
    TEST_ff_f (hypot, 0.7L, 12.4L, 12.419742348374220601176836866763271L),
    TEST_ff_f (hypot, -0.7L, 12.4L, 12.419742348374220601176836866763271L),
    TEST_ff_f (hypot, 0.7L, -12.4L, 12.419742348374220601176836866763271L),
    TEST_ff_f (hypot, -0.7L, -12.4L, 12.419742348374220601176836866763271L),
    TEST_ff_f (hypot, 12.4L, 0.7L, 12.419742348374220601176836866763271L),
    TEST_ff_f (hypot, -12.4L, 0.7L, 12.419742348374220601176836866763271L),
    TEST_ff_f (hypot, 12.4L, -0.7L, 12.419742348374220601176836866763271L),
    TEST_ff_f (hypot, -12.4L, -0.7L, 12.419742348374220601176836866763271L),

    /*  hypot (x,0) == fabs (x)  */
    TEST_ff_f (hypot, 0.75L, 0, 0.75L),
    TEST_ff_f (hypot, -0.75L, 0, 0.75L),
    TEST_ff_f (hypot, -5.7e7, 0, 5.7e7L),

    TEST_ff_f (hypot, 0.75L, 1.25L, 1.45773797371132511771853821938639577L),

    TEST_ff_f (hypot, 1.0L, 0x1p-61L, 1.0L),
    TEST_ff_f (hypot, 0x1p+0L, 0x1.fp-129L, 0x1p+0L),
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_ff_f (hypot, 0x1.23456789abcdef0123456789ab8p-500L, 0x1.23456789abcdef0123456789ab8p-500L, 4.9155782399407039128612180934736799735113e-151L),
#endif

#if !(defined TEST_FLOAT && defined TEST_INLINE)
    TEST_ff_f (hypot, 0x3p125L, 0x4p125L, 0x5p125L),
    TEST_ff_f (hypot, 0x1.234566p-126L, 0x1.234566p-126L, 1.891441686191081936598531534017449451173e-38L),
#endif

#if !defined TEST_FLOAT && !(defined TEST_DOUBLE && defined TEST_INLINE)
    TEST_ff_f (hypot, 0x3p1021L, 0x4p1021L, 0x5p1021L),
    TEST_ff_f (hypot, 0x1p+0L, 0x0.3ep-1022L, 0x1p+0L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384 && !defined TEST_INLINE
    TEST_ff_f (hypot, 0x3p16381L, 0x4p16381L, 0x5p16381L),
#endif
    END_DATA (hypot)
  };

static void
hypot_test (void)
{
  errno = 0;
  FUNC(hypot) (0.7L, 12.4L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (hypot);
  RUN_TEST_LOOP_ff_f (hypot, hypot_test_data, );
  END (hypot);
}


static const struct test_f_i_data ilogb_test_data[] =
  {
    START_DATA (ilogb),
    TEST_f_i (ilogb, 1, 0),
    TEST_f_i (ilogb, M_El, 1),
    TEST_f_i (ilogb, 1024, 10),
    TEST_f_i (ilogb, -2000, 10),

    /* ilogb (0.0) == FP_ILOGB0 plus invalid exception  */
    TEST_f_i (ilogb, 0.0, FP_ILOGB0, INVALID_EXCEPTION|ERRNO_EDOM),
    /* ilogb (qNaN) == FP_ILOGBNAN plus invalid exception  */
    TEST_f_i (ilogb, qnan_value, FP_ILOGBNAN, INVALID_EXCEPTION|ERRNO_EDOM),
    /* ilogb (inf) == INT_MAX plus invalid exception  */
    TEST_f_i (ilogb, plus_infty, INT_MAX, INVALID_EXCEPTION|ERRNO_EDOM),
    /* ilogb (-inf) == INT_MAX plus invalid exception  */
    TEST_f_i (ilogb, minus_infty, INT_MAX, INVALID_EXCEPTION|ERRNO_EDOM),
    END_DATA (ilogb)
  };

static void
ilogb_test (void)
{
  START (ilogb);
  RUN_TEST_LOOP_f_i (ilogb, ilogb_test_data, );
  END (ilogb);
}

static void
isfinite_test (void)
{
  START (isfinite);

  TEST_f_b (isfinite, 0, 1);
  TEST_f_b (isfinite, minus_zero, 1);
  TEST_f_b (isfinite, 10, 1);
  TEST_f_b (isfinite, min_subnorm_value, 1);
  TEST_f_b (isfinite, plus_infty, 0);
  TEST_f_b (isfinite, minus_infty, 0);
  TEST_f_b (isfinite, qnan_value, 0);

  END (isfinite);
}

static void
isgreater_test (void)
{
  START (isgreater);

  TEST_ff_i (isgreater, minus_zero, minus_zero, 0);
  TEST_ff_i (isgreater, minus_zero, plus_zero, 0);
  TEST_ff_i (isgreater, minus_zero, (FLOAT) 1, 0);
  TEST_ff_i (isgreater, minus_zero, qnan_value, 0);
  TEST_ff_i (isgreater, plus_zero, minus_zero, 0);
  TEST_ff_i (isgreater, plus_zero, plus_zero, 0);
  TEST_ff_i (isgreater, plus_zero, (FLOAT) 1, 0);
  TEST_ff_i (isgreater, plus_zero, qnan_value, 0);
  TEST_ff_i (isgreater, (FLOAT) 1, minus_zero, 1);
  TEST_ff_i (isgreater, (FLOAT) 1, plus_zero, 1);
  TEST_ff_i (isgreater, (FLOAT) 1, (FLOAT) 1, 0);
  TEST_ff_i (isgreater, (FLOAT) 1, qnan_value, 0);
  TEST_ff_i (isgreater, qnan_value, minus_zero, 0);
  TEST_ff_i (isgreater, qnan_value, plus_zero, 0);
  TEST_ff_i (isgreater, qnan_value, (FLOAT) 1, 0);
  TEST_ff_i (isgreater, qnan_value, qnan_value, 0);

  END (isgreater);
}

static void
isgreaterequal_test (void)
{
  START (isgreaterequal);

  TEST_ff_i (isgreaterequal, minus_zero, minus_zero, 1);
  TEST_ff_i (isgreaterequal, minus_zero, plus_zero, 1);
  TEST_ff_i (isgreaterequal, minus_zero, (FLOAT) 1, 0);
  TEST_ff_i (isgreaterequal, minus_zero, qnan_value, 0);
  TEST_ff_i (isgreaterequal, plus_zero, minus_zero, 1);
  TEST_ff_i (isgreaterequal, plus_zero, plus_zero, 1);
  TEST_ff_i (isgreaterequal, plus_zero, (FLOAT) 1, 0);
  TEST_ff_i (isgreaterequal, plus_zero, qnan_value, 0);
  TEST_ff_i (isgreaterequal, (FLOAT) 1, minus_zero, 1);
  TEST_ff_i (isgreaterequal, (FLOAT) 1, plus_zero, 1);
  TEST_ff_i (isgreaterequal, (FLOAT) 1, (FLOAT) 1, 1);
  TEST_ff_i (isgreaterequal, (FLOAT) 1, qnan_value, 0);
  TEST_ff_i (isgreaterequal, qnan_value, minus_zero, 0);
  TEST_ff_i (isgreaterequal, qnan_value, plus_zero, 0);
  TEST_ff_i (isgreaterequal, qnan_value, (FLOAT) 1, 0);
  TEST_ff_i (isgreaterequal, qnan_value, qnan_value, 0);

  END (isgreaterequal);
}

static void
isinf_test (void)
{
  START (isinf);

  TEST_f_b (isinf, 0, 0);
  TEST_f_b (isinf, minus_zero, 0);
  TEST_f_b (isinf, 10, 0);
  TEST_f_b (isinf, min_subnorm_value, 0);
  TEST_f_b (isinf, plus_infty, 1);
  TEST_f_b (isinf, minus_infty, 1);
  TEST_f_b (isinf, qnan_value, 0);

  END (isinf);
}

static void
isless_test (void)
{
  START (isless);

  TEST_ff_i (isless, minus_zero, minus_zero, 0);
  TEST_ff_i (isless, minus_zero, plus_zero, 0);
  TEST_ff_i (isless, minus_zero, (FLOAT) 1, 1);
  TEST_ff_i (isless, minus_zero, qnan_value, 0);
  TEST_ff_i (isless, plus_zero, minus_zero, 0);
  TEST_ff_i (isless, plus_zero, plus_zero, 0);
  TEST_ff_i (isless, plus_zero, (FLOAT) 1, 1);
  TEST_ff_i (isless, plus_zero, qnan_value, 0);
  TEST_ff_i (isless, (FLOAT) 1, minus_zero, 0);
  TEST_ff_i (isless, (FLOAT) 1, plus_zero, 0);
  TEST_ff_i (isless, (FLOAT) 1, (FLOAT) 1, 0);
  TEST_ff_i (isless, (FLOAT) 1, qnan_value, 0);
  TEST_ff_i (isless, qnan_value, minus_zero, 0);
  TEST_ff_i (isless, qnan_value, plus_zero, 0);
  TEST_ff_i (isless, qnan_value, (FLOAT) 1, 0);
  TEST_ff_i (isless, qnan_value, qnan_value, 0);

  END (isless);
}

static void
islessequal_test (void)
{
  START (islessequal);

  TEST_ff_i (islessequal, minus_zero, minus_zero, 1);
  TEST_ff_i (islessequal, minus_zero, plus_zero, 1);
  TEST_ff_i (islessequal, minus_zero, (FLOAT) 1, 1);
  TEST_ff_i (islessequal, minus_zero, qnan_value, 0);
  TEST_ff_i (islessequal, plus_zero, minus_zero, 1);
  TEST_ff_i (islessequal, plus_zero, plus_zero, 1);
  TEST_ff_i (islessequal, plus_zero, (FLOAT) 1, 1);
  TEST_ff_i (islessequal, plus_zero, qnan_value, 0);
  TEST_ff_i (islessequal, (FLOAT) 1, minus_zero, 0);
  TEST_ff_i (islessequal, (FLOAT) 1, plus_zero, 0);
  TEST_ff_i (islessequal, (FLOAT) 1, (FLOAT) 1, 1);
  TEST_ff_i (islessequal, (FLOAT) 1, qnan_value, 0);
  TEST_ff_i (islessequal, qnan_value, minus_zero, 0);
  TEST_ff_i (islessequal, qnan_value, plus_zero, 0);
  TEST_ff_i (islessequal, qnan_value, (FLOAT) 1, 0);
  TEST_ff_i (islessequal, qnan_value, qnan_value, 0);

  END (islessequal);
}

static void
islessgreater_test (void)
{
  START (islessgreater);

  TEST_ff_i (islessgreater, minus_zero, minus_zero, 0);
  TEST_ff_i (islessgreater, minus_zero, plus_zero, 0);
  TEST_ff_i (islessgreater, minus_zero, (FLOAT) 1, 1);
  TEST_ff_i (islessgreater, minus_zero, qnan_value, 0);
  TEST_ff_i (islessgreater, plus_zero, minus_zero, 0);
  TEST_ff_i (islessgreater, plus_zero, plus_zero, 0);
  TEST_ff_i (islessgreater, plus_zero, (FLOAT) 1, 1);
  TEST_ff_i (islessgreater, plus_zero, qnan_value, 0);
  TEST_ff_i (islessgreater, (FLOAT) 1, minus_zero, 1);
  TEST_ff_i (islessgreater, (FLOAT) 1, plus_zero, 1);
  TEST_ff_i (islessgreater, (FLOAT) 1, (FLOAT) 1, 0);
  TEST_ff_i (islessgreater, (FLOAT) 1, qnan_value, 0);
  TEST_ff_i (islessgreater, qnan_value, minus_zero, 0);
  TEST_ff_i (islessgreater, qnan_value, plus_zero, 0);
  TEST_ff_i (islessgreater, qnan_value, (FLOAT) 1, 0);
  TEST_ff_i (islessgreater, qnan_value, qnan_value, 0);

  END (islessgreater);
}

static void
isnan_test (void)
{
  START (isnan);

  TEST_f_b (isnan, 0, 0);
  TEST_f_b (isnan, minus_zero, 0);
  TEST_f_b (isnan, 10, 0);
  TEST_f_b (isnan, min_subnorm_value, 0);
  TEST_f_b (isnan, plus_infty, 0);
  TEST_f_b (isnan, minus_infty, 0);
  TEST_f_b (isnan, qnan_value, 1);

  END (isnan);
}

static void
isnormal_test (void)
{
  START (isnormal);

  TEST_f_b (isnormal, 0, 0);
  TEST_f_b (isnormal, minus_zero, 0);
  TEST_f_b (isnormal, 10, 1);
  TEST_f_b (isnormal, min_subnorm_value, 0);
  TEST_f_b (isnormal, plus_infty, 0);
  TEST_f_b (isnormal, minus_infty, 0);
  TEST_f_b (isnormal, qnan_value, 0);

  END (isnormal);
}

static void
issignaling_test (void)
{
  START (issignaling);

  TEST_f_b (issignaling, 0, 0);
  TEST_f_b (issignaling, minus_zero, 0);
  TEST_f_b (issignaling, 10, 0);
  TEST_f_b (issignaling, min_subnorm_value, 0);
  TEST_f_b (issignaling, plus_infty, 0);
  TEST_f_b (issignaling, minus_infty, 0);
  TEST_f_b (issignaling, qnan_value, 0);

  END (issignaling);
}

static void
isunordered_test (void)
{
  START (isunordered);

  TEST_ff_i (isunordered, minus_zero, minus_zero, 0);
  TEST_ff_i (isunordered, minus_zero, plus_zero, 0);
  TEST_ff_i (isunordered, minus_zero, (FLOAT) 1, 0);
  TEST_ff_i (isunordered, minus_zero, qnan_value, 1);
  TEST_ff_i (isunordered, plus_zero, minus_zero, 0);
  TEST_ff_i (isunordered, plus_zero, plus_zero, 0);
  TEST_ff_i (isunordered, plus_zero, (FLOAT) 1, 0);
  TEST_ff_i (isunordered, plus_zero, qnan_value, 1);
  TEST_ff_i (isunordered, (FLOAT) 1, minus_zero, 0);
  TEST_ff_i (isunordered, (FLOAT) 1, plus_zero, 0);
  TEST_ff_i (isunordered, (FLOAT) 1, (FLOAT) 1, 0);
  TEST_ff_i (isunordered, (FLOAT) 1, qnan_value, 1);
  TEST_ff_i (isunordered, qnan_value, minus_zero, 1);
  TEST_ff_i (isunordered, qnan_value, plus_zero, 1);
  TEST_ff_i (isunordered, qnan_value, (FLOAT) 1, 1);
  TEST_ff_i (isunordered, qnan_value, qnan_value, 1);

  END (isunordered);
}

static const struct test_f_f_data j0_test_data[] =
  {
    START_DATA (j0),
    /* j0 is the Bessel function of the first kind of order 0 */
    TEST_f_f (j0, qnan_value, qnan_value),
    TEST_f_f (j0, plus_infty, 0),
    TEST_f_f (j0, -1.0, 0.765197686557966551449717526102663221L),
    TEST_f_f (j0, 0.0, 1.0),
    TEST_f_f (j0, 0.125L, 0.996097563041985204620768999453174712L),
    TEST_f_f (j0, 0.75L, 0.864242275166648623555731103820923211L),
    TEST_f_f (j0, 1.0, 0.765197686557966551449717526102663221L),
    TEST_f_f (j0, 1.5, 0.511827671735918128749051744283411720L),
    TEST_f_f (j0, 2.0, 0.223890779141235668051827454649948626L),
    TEST_f_f (j0, 8.0, 0.171650807137553906090869407851972001L),
    TEST_f_f (j0, 10.0, -0.245935764451348335197760862485328754L),
    TEST_f_f (j0, 4.0, -3.9714980986384737228659076845169804197562E-1L),
    TEST_f_f (j0, -4.0, -3.9714980986384737228659076845169804197562E-1L),

    TEST_f_f (j0, 0x1.d7ce3ap+107L, 2.775523647291230802651040996274861694514e-17L),

#ifndef TEST_FLOAT
    TEST_f_f (j0, -0x1.001000001p+593L, -3.927269966354206207832593635798954916263e-90L),
    TEST_f_f (j0, 0x1p1023L, -1.5665258060609012834424478437196679802783e-155L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_f_f (j0, 0x1p16382L, -1.2193782500509000574176799046642541129387e-2466L),
    TEST_f_f (j0, 0x1p16383L, 9.5859502826270374691362975419147645151233e-2467L),
#endif
    END_DATA (j0)
  };

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
  RUN_TEST_LOOP_f_f (j0, j0_test_data, );
  END (j0);
}


static const struct test_f_f_data j1_test_data[] =
  {
    START_DATA (j1),
    /* j1 is the Bessel function of the first kind of order 1 */
    TEST_f_f (j1, qnan_value, qnan_value),
    TEST_f_f (j1, plus_infty, 0),

    TEST_f_f (j1, -1.0, -0.440050585744933515959682203718914913L),
    TEST_f_f (j1, 0.0, 0.0),
    TEST_f_f (j1, 0.125L, 0.0623780091344946810942311355879361177L),
    TEST_f_f (j1, 0.75L, 0.349243602174862192523281016426251335L),
    TEST_f_f (j1, 1.0, 0.440050585744933515959682203718914913L),
    TEST_f_f (j1, 1.5, 0.557936507910099641990121213156089400L),
    TEST_f_f (j1, 2.0, 0.576724807756873387202448242269137087L),
    TEST_f_f (j1, 8.0, 0.234636346853914624381276651590454612L),
    TEST_f_f (j1, 10.0, 0.0434727461688614366697487680258592883L),

    TEST_f_f (j1, 0x1.3ffp+74L, 1.818984347516051243459364437186082741567e-12L),

#ifndef TEST_FLOAT
    TEST_f_f (j1, 0x1.ff00000000002p+840L, 1.846591691699331493194965158699937660696e-127L),
    TEST_f_f (j1, 0x1p1023L, 8.2687542933709649327986678723012001545638e-155L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_f_f (j1, 0x1p16382L, 8.0839224448726336195866026476176740513439e-2467L),
    TEST_f_f (j1, 0x1p16383L, -3.8895531955766020648617743624167352352217e-2467L),
#endif
    END_DATA (j1)
  };

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

  START (j1);
  RUN_TEST_LOOP_f_f (j1, j1_test_data, );
  END (j1);
}

static const struct test_if_f_data jn_test_data[] =
  {
    START_DATA (jn),
    /* jn is the Bessel function of the first kind of order n.  */
    /* jn (0, x) == j0 (x)  */
    TEST_if_f (jn, 0, qnan_value, qnan_value),
    TEST_if_f (jn, 0, plus_infty, 0),
    TEST_if_f (jn, 0, -1.0, 0.765197686557966551449717526102663221L),
    TEST_if_f (jn, 0, 0.0, 1.0),
    TEST_if_f (jn, 0, 0.125L, 0.996097563041985204620768999453174712L),
    TEST_if_f (jn, 0, 0.75L, 0.864242275166648623555731103820923211L),
    TEST_if_f (jn, 0, 1.0, 0.765197686557966551449717526102663221L),
    TEST_if_f (jn, 0, 1.5, 0.511827671735918128749051744283411720L),
    TEST_if_f (jn, 0, 2.0, 0.223890779141235668051827454649948626L),
    TEST_if_f (jn, 0, 8.0, 0.171650807137553906090869407851972001L),
    TEST_if_f (jn, 0, 10.0, -0.245935764451348335197760862485328754L),
    TEST_if_f (jn, 0, 4.0, -3.9714980986384737228659076845169804197562E-1L),
    TEST_if_f (jn, 0, -4.0, -3.9714980986384737228659076845169804197562E-1L),

    /* jn (1, x) == j1 (x)  */
    TEST_if_f (jn, 1, qnan_value, qnan_value),
    TEST_if_f (jn, 1, plus_infty, 0),
    TEST_if_f (jn, 1, -1.0, -0.440050585744933515959682203718914913L),
    TEST_if_f (jn, 1, 0.0, 0.0),
    TEST_if_f (jn, 1, 0.125L, 0.0623780091344946810942311355879361177L),
    TEST_if_f (jn, 1, 0.75L, 0.349243602174862192523281016426251335L),
    TEST_if_f (jn, 1, 1.0, 0.440050585744933515959682203718914913L),
    TEST_if_f (jn, 1, 1.5, 0.557936507910099641990121213156089400L),
    TEST_if_f (jn, 1, 2.0, 0.576724807756873387202448242269137087L),
    TEST_if_f (jn, 1, 8.0, 0.234636346853914624381276651590454612L),
    TEST_if_f (jn, 1, 10.0, 0.0434727461688614366697487680258592883L),

    /* jn (3, x)  */
    TEST_if_f (jn, 3, qnan_value, qnan_value),
    TEST_if_f (jn, 3, plus_infty, 0),

    TEST_if_f (jn, 3, -1.0, -0.0195633539826684059189053216217515083L),
    TEST_if_f (jn, 3, 0.0, 0.0),
    TEST_if_f (jn, 3, 0.125L, 0.406503832554912875023029337653442868e-4L),
    TEST_if_f (jn, 3, 0.75L, 0.848438342327410884392755236884386804e-2L),
    TEST_if_f (jn, 3, 1.0, 0.0195633539826684059189053216217515083L),
    TEST_if_f (jn, 3, 2.0, 0.128943249474402051098793332969239835L),
    TEST_if_f (jn, 3, 10.0, 0.0583793793051868123429354784103409563L),

    /*  jn (10, x)  */
    TEST_if_f (jn, 10, qnan_value, qnan_value),
    TEST_if_f (jn, 10, plus_infty, 0),

    TEST_if_f (jn, 10, -1.0, 0.263061512368745320699785368779050294e-9L),
    TEST_if_f (jn, 10, 0.0, 0.0),
    TEST_if_f (jn, 10, 0.125L, 0.250543369809369890173993791865771547e-18L),
    TEST_if_f (jn, 10, 0.75L, 0.149621713117596814698712483621682835e-10L),
    TEST_if_f (jn, 10, 1.0, 0.263061512368745320699785368779050294e-9L),
    TEST_if_f (jn, 10, 2.0, 0.251538628271673670963516093751820639e-6L),
    TEST_if_f (jn, 10, 10.0, 0.207486106633358857697278723518753428L),

    /* BZ #11589 .*/
    TEST_if_f (jn, 2, 2.4048255576957729L, 0.43175480701968038399746111312430703L),
    TEST_if_f (jn, 3, 2.4048255576957729L, 0.19899990535769083404042146764530813L),
    TEST_if_f (jn, 4, 2.4048255576957729L, 0.647466661641779720084932282551219891E-1L),
    TEST_if_f (jn, 5, 2.4048255576957729L, 0.163892432048058525099230549946147698E-1L),
    TEST_if_f (jn, 6, 2.4048255576957729L, 0.34048184720278336646673682895929161E-2L),
    TEST_if_f (jn, 7, 2.4048255576957729L, 0.60068836573295394221291569249883076E-3L),
    TEST_if_f (jn, 8, 2.4048255576957729L, 0.92165786705344923232879022467054148E-4L),
    TEST_if_f (jn, 9, 2.4048255576957729L, 0.12517270977961513005428966643852564E-4L),

    /* Bug 14155: spurious exception may occur.  */
    TEST_if_f (jn, 2, 0x1.ffff62p+99L, -4.43860668048170034334926693188979974489e-16L, UNDERFLOW_EXCEPTION_OK),
    END_DATA (jn)
  };

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

  START (jn);
  RUN_TEST_LOOP_if_f (jn, jn_test_data, );
  END (jn);
}


static const struct test_fi_f_data ldexp_test_data[] =
  {
    START_DATA (ldexp),
    TEST_fi_f (ldexp, 0, 0, 0),
    TEST_fi_f (ldexp, minus_zero, 0, minus_zero),

    TEST_fi_f (ldexp, plus_infty, 1, plus_infty),
    TEST_fi_f (ldexp, minus_infty, 1, minus_infty),
    TEST_fi_f (ldexp, qnan_value, 1, qnan_value),

    TEST_fi_f (ldexp, 0.8L, 4, 12.8L),
    TEST_fi_f (ldexp, -0.854375L, 5, -27.34L),

    /* ldexp (x, 0) == x.  */
    TEST_fi_f (ldexp, 1.0L, 0L, 1.0L),
    END_DATA (ldexp)
  };

static void
ldexp_test (void)
{
  START (ldexp);
  RUN_TEST_LOOP_fi_f (ldexp, ldexp_test_data, );
  END (ldexp);
}


static const struct test_f_f1_data lgamma_test_data[] =
  {
    START_DATA (lgamma),
    TEST_f_f1 (lgamma, plus_infty, plus_infty, 1),
    TEST_f_f1 (lgamma, 0, plus_infty, 1, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f1 (lgamma, minus_zero, plus_infty, -1, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f1 (lgamma, qnan_value, qnan_value, IGNORE),

    /* lgamma (x) == +inf plus divide by zero exception for integer x <= 0.  */
    TEST_f_f1 (lgamma, -3, plus_infty, IGNORE, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f1 (lgamma, minus_infty, plus_infty, IGNORE),
    TEST_f_f1 (lgamma, -max_value, plus_infty, IGNORE, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f1 (lgamma, max_value, plus_infty, 1, OVERFLOW_EXCEPTION),

    TEST_f_f1 (lgamma, 1, 0, 1),

    TEST_f_f1 (lgamma, 3, M_LN2l, 1),

    TEST_f_f1 (lgamma, 0.5, M_LOG_SQRT_PIl, 1),
    TEST_f_f1 (lgamma, -0.5, M_LOG_2_SQRT_PIl, -1),
    TEST_f_f1 (lgamma, 0.7L, 0.260867246531666514385732417016759578L, 1),
    TEST_f_f1 (lgamma, 1.2L, -0.853740900033158497197028392998854470e-1L, 1),
    END_DATA (lgamma)
  };

static void
lgamma_test (void)
{
  errno = 0;
  FUNC(lgamma) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (lgamma);
  RUN_TEST_LOOP_f_f1 (lgamma, lgamma_test_data, , signgam);
  END (lgamma);
}


static void
lrint_test (void)
{
  /* XXX this test is incomplete.  We need to have a way to specifiy
     the rounding method and test the critical cases.  So far, only
     unproblematic numbers are tested.  */
  /* TODO: missing +/-Inf as well as qNaN tests.  */

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
  /* TODO: missing +/-Inf as well as qNaN tests.  */

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


static const struct test_f_f_data log_test_data[] =
  {
    START_DATA (log),
    TEST_f_f (log, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f (log, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),

    TEST_f_f (log, 1, 0),

    TEST_f_f (log, -1, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log, -max_value, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log, minus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log, plus_infty, plus_infty),
    TEST_f_f (log, qnan_value, qnan_value),

    TEST_f_f (log, M_El, 1),
    TEST_f_f (log, M_1_DIV_El, -1),
    TEST_f_f (log, 2, M_LN2l),
    TEST_f_f (log, 10, M_LN10l),
    TEST_f_f (log, 0.75L, -0.287682072451780927439219005993827432L),
    END_DATA (log)
  };

static void
log_test (void)
{
  errno = 0;
  FUNC(log) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;
  START (log);
  RUN_TEST_LOOP_f_f (log, log_test_data, );
  END (log);
}


static const struct test_f_f_data log10_test_data[] =
  {
    START_DATA (log10),
    TEST_f_f (log10, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f (log10, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),

    TEST_f_f (log10, 1, 0),

    /* log10 (x) == qNaN plus invalid exception if x < 0.  */
    TEST_f_f (log10, -1, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log10, -max_value, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log10, minus_infty, qnan_value, INVALID_EXCEPTION),

    TEST_f_f (log10, plus_infty, plus_infty),
    TEST_f_f (log10, qnan_value, qnan_value),

    TEST_f_f (log10, 0.1L, -1),
    TEST_f_f (log10, 10.0, 1),
    TEST_f_f (log10, 100.0, 2),
    TEST_f_f (log10, 10000.0, 4),
    TEST_f_f (log10, M_El, M_LOG10El),
    TEST_f_f (log10, 0.75L, -0.124938736608299953132449886193870744L),
    END_DATA (log10)
  };

static void
log10_test (void)
{
  errno = 0;
  FUNC(log10) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (log10);
  RUN_TEST_LOOP_f_f (log10, log10_test_data, );
  END (log10);
}


static const struct test_f_f_data log1p_test_data[] =
  {
    START_DATA (log1p),
    TEST_f_f (log1p, 0, 0),
    TEST_f_f (log1p, minus_zero, minus_zero),

    TEST_f_f (log1p, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f (log1p, -2, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log1p, -max_value, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log1p, minus_infty, qnan_value, INVALID_EXCEPTION),

    TEST_f_f (log1p, plus_infty, plus_infty),
    TEST_f_f (log1p, qnan_value, qnan_value),

    TEST_f_f (log1p, M_El - 1.0, 1),

    TEST_f_f (log1p, -0.25L, -0.287682072451780927439219005993827432L),
    TEST_f_f (log1p, -0.875, -2.07944154167983592825169636437452970L),
    END_DATA (log1p)
  };

static void
log1p_test (void)
{
  errno = 0;
  FUNC(log1p) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (log1p);
  RUN_TEST_LOOP_f_f (log1p, log1p_test_data, );
  END (log1p);
}


static const struct test_f_f_data log2_test_data[] =
  {
    START_DATA (log2),
    TEST_f_f (log2, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f (log2, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),

    TEST_f_f (log2, 1, 0),

    TEST_f_f (log2, -1, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log2, -max_value, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log2, minus_infty, qnan_value, INVALID_EXCEPTION),

    TEST_f_f (log2, plus_infty, plus_infty),
    TEST_f_f (log2, qnan_value, qnan_value),

    TEST_f_f (log2, M_El, M_LOG2El),
    TEST_f_f (log2, 2.0, 1),
    TEST_f_f (log2, 16.0, 4),
    TEST_f_f (log2, 256.0, 8),
    TEST_f_f (log2, 0.75L, -.415037499278843818546261056052183492L),
    END_DATA (log2)
  };

static void
log2_test (void)
{
  errno = 0;
  FUNC(log2) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (log2);
  RUN_TEST_LOOP_f_f (log2, log2_test_data, );
  END (log2);
}


static const struct test_f_f_data logb_test_data[] =
  {
    START_DATA (logb),
    TEST_f_f (logb, plus_infty, plus_infty),
    TEST_f_f (logb, minus_infty, plus_infty),

    TEST_f_f (logb, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),

    TEST_f_f (logb, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f (logb, qnan_value, qnan_value),

    TEST_f_f (logb, 1, 0),
    TEST_f_f (logb, M_El, 1),
    TEST_f_f (logb, 1024, 10),
    TEST_f_f (logb, -2000, 10),

    TEST_f_f (logb, 0x0.1p-127, -131),
    TEST_f_f (logb, 0x0.01p-127, -135),
    TEST_f_f (logb, 0x0.011p-127, -135),
#ifndef TEST_FLOAT
    TEST_f_f (logb, 0x0.8p-1022, -1023),
    TEST_f_f (logb, 0x0.1p-1022, -1026),
    TEST_f_f (logb, 0x0.00111p-1022, -1034),
    TEST_f_f (logb, 0x0.00001p-1022, -1042),
    TEST_f_f (logb, 0x0.000011p-1022, -1042),
    TEST_f_f (logb, 0x0.0000000000001p-1022, -1074),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP - LDBL_MANT_DIG <= -16400
    TEST_f_f (logb, 0x1p-16400L, -16400),
    TEST_f_f (logb, 0x.00000000001p-16382L, -16426),
#endif
    END_DATA (logb)
  };

static void
logb_test (void)
{
  START (logb);
  RUN_TEST_LOOP_f_f (logb, logb_test_data, );
  END (logb);
}

static const struct test_f_f_data logb_downward_test_data[] =
  {
    START_DATA (logb_downward),
    /* IEEE 754-2008 says (section 5.3.3) that "logB(1) is +0.".  Libm
       should not return -0 from logb in any rounding mode.  PowerPC32 has
       failed with this test for power4 logb (and logbl on all PowerPC
       platforms) in the past due to instruction selection.  GCC PR 52775
       provides the availability of the fcfid insn in 32-bit mode which
       eliminates the use of fsub in this instance and prevents the negative
       signed 0.0.  */

    /* BZ #887  */
    TEST_f_f (logb, 1.000e+0, plus_zero),
    END_DATA (logb_downward)
  };

static void
logb_test_downward (void)
{
  errno = 0;

  FUNC(logb) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (logb_downward);
  RUN_TEST_LOOP_f_f (logb, logb_downward_test_data, FE_DOWNWARD);
  END (logb_downward);
}

static void
lround_test (void)
{
  /* TODO: missing +/-Inf as well as qNaN tests.  */

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
  /* TODO: missing +/-Inf as well as qNaN tests.  */

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

static const struct test_fF_f1_data modf_test_data[] =
  {
    START_DATA (modf),
    TEST_fF_f1 (modf, plus_infty, 0, plus_infty),
    TEST_fF_f1 (modf, minus_infty, minus_zero, minus_infty),
    TEST_fF_f1 (modf, qnan_value, qnan_value, qnan_value),
    TEST_fF_f1 (modf, 0, 0, 0),
    TEST_fF_f1 (modf, 1.5, 0.5, 1),
    TEST_fF_f1 (modf, 2.5, 0.5, 2),
    TEST_fF_f1 (modf, -2.5, -0.5, -2),
    TEST_fF_f1 (modf, 20, 0, 20),
    TEST_fF_f1 (modf, 21, 0, 21),
    TEST_fF_f1 (modf, 89.5, 0.5, 89),
    END_DATA (modf)
  };

static void
modf_test (void)
{
  FLOAT x;

  START (modf);
  RUN_TEST_LOOP_fF_f1 (modf, modf_test_data, , x);
  END (modf);
}


static const struct test_f_f_data nearbyint_test_data[] =
  {
    START_DATA (nearbyint),

    TEST_f_f (nearbyint, 0.0, 0.0),
    TEST_f_f (nearbyint, minus_zero, minus_zero),
    TEST_f_f (nearbyint, plus_infty, plus_infty),
    TEST_f_f (nearbyint, minus_infty, minus_infty),
    TEST_f_f (nearbyint, qnan_value, qnan_value),

    /* Subnormal values */
    TEST_f_f (nearbyint, -8.98847e+307, -8.98847e+307),
    TEST_f_f (nearbyint, -4.45015e-308, minus_zero),

    /* Default rounding mode is round to nearest.  */
    TEST_f_f (nearbyint, 0.5, 0.0),
    TEST_f_f (nearbyint, 1.5, 2.0),
    TEST_f_f (nearbyint, -0.5, minus_zero),
    TEST_f_f (nearbyint, -1.5, -2.0),

    TEST_f_f (nearbyint, 262144.75, 262145.0),
    TEST_f_f (nearbyint, 262142.75, 262143.0),
    TEST_f_f (nearbyint, 524286.75, 524287.0),
    TEST_f_f (nearbyint, 524288.75, 524289.0),

    TEST_f_f (nearbyint, 1048576.75, 1048577.0),
    TEST_f_f (nearbyint, 2097152.75, 2097153.0),
    TEST_f_f (nearbyint, 2492472.75, 2492473.0),
    TEST_f_f (nearbyint, 2886220.75, 2886221.0),
    TEST_f_f (nearbyint, 3058792.75, 3058793.0),
    TEST_f_f (nearbyint, -1048576.75, -1048577.0),
    TEST_f_f (nearbyint, -2097152.75, -2097153.0),
    TEST_f_f (nearbyint, -2492472.75, -2492473.0),
    TEST_f_f (nearbyint, -2886220.75, -2886221.0),
    TEST_f_f (nearbyint, -3058792.75, -3058793.0),
#ifndef TEST_FLOAT
    TEST_f_f (nearbyint, 70368744177664.75, 70368744177665.0),
    TEST_f_f (nearbyint, 140737488355328.75, 140737488355329.0),
    TEST_f_f (nearbyint, 281474976710656.75, 281474976710657.0),
    TEST_f_f (nearbyint, 562949953421312.75, 562949953421313.0),
    TEST_f_f (nearbyint, 1125899906842624.75, 1125899906842625.0),
    TEST_f_f (nearbyint, -70368744177664.75, -70368744177665.0),
    TEST_f_f (nearbyint, -140737488355328.75, -140737488355329.0),
    TEST_f_f (nearbyint, -281474976710656.75, -281474976710657.0),
    TEST_f_f (nearbyint, -562949953421312.75, -562949953421313.0),
    TEST_f_f (nearbyint, -1125899906842624.75, -1125899906842625.0),
#endif

    END_DATA (nearbyint)
  };

static void
nearbyint_test (void)
{
  START (nearbyint);
  RUN_TEST_LOOP_f_f (nearbyint, nearbyint_test_data, );
  END (nearbyint);
}

static const struct test_ff_f_data nextafter_test_data[] =
  {
    START_DATA (nextafter),

    TEST_ff_f (nextafter, 0, 0, 0),
    TEST_ff_f (nextafter, minus_zero, 0, 0),
    TEST_ff_f (nextafter, 0, minus_zero, minus_zero),
    TEST_ff_f (nextafter, minus_zero, minus_zero, minus_zero),

    TEST_ff_f (nextafter, 9, 9, 9),
    TEST_ff_f (nextafter, -9, -9, -9),
    TEST_ff_f (nextafter, plus_infty, plus_infty, plus_infty),
    TEST_ff_f (nextafter, minus_infty, minus_infty, minus_infty),

    TEST_ff_f (nextafter, qnan_value, 1.1L, qnan_value),
    TEST_ff_f (nextafter, 1.1L, qnan_value, qnan_value),
    TEST_ff_f (nextafter, qnan_value, qnan_value, qnan_value),

    TEST_ff_f (nextafter, max_value, plus_infty, plus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (nextafter, -max_value, minus_infty, minus_infty, OVERFLOW_EXCEPTION),

#ifdef TEST_LDOUBLE
    // XXX Enable once gcc is fixed.
    //TEST_ff_f (nextafter, 0x0.00000040000000000000p-16385L, -0.1L, 0x0.0000003ffffffff00000p-16385L),
#endif

    /* XXX We need the hexadecimal FP number representation here for further
       tests.  */
    END_DATA (nextafter)
  };

static void
nextafter_test (void)
{

  START (nextafter);
  RUN_TEST_LOOP_ff_f (nextafter, nextafter_test_data, );
  END (nextafter);
}


static const struct test_ff_f_data_nexttoward nexttoward_test_data[] =
  {
    START_DATA (nexttoward),
    TEST_ff_f (nexttoward, 0, 0, 0),
    TEST_ff_f (nexttoward, minus_zero, 0, 0),
    TEST_ff_f (nexttoward, 0, minus_zero, minus_zero),
    TEST_ff_f (nexttoward, minus_zero, minus_zero, minus_zero),

    TEST_ff_f (nexttoward, 9, 9, 9),
    TEST_ff_f (nexttoward, -9, -9, -9),
    TEST_ff_f (nexttoward, plus_infty, plus_infty, plus_infty),
    TEST_ff_f (nexttoward, minus_infty, minus_infty, minus_infty),

    TEST_ff_f (nexttoward, qnan_value, 1.1L, qnan_value),
    TEST_ff_f (nexttoward, 1.1L, qnan_value, qnan_value),
    TEST_ff_f (nexttoward, qnan_value, qnan_value, qnan_value),

#ifdef TEST_FLOAT
    TEST_ff_f (nexttoward, 1.0, 1.1L, 0x1.000002p0),
    TEST_ff_f (nexttoward, 1.0, LDBL_MAX, 0x1.000002p0),
    TEST_ff_f (nexttoward, 1.0, 0x1.0000000000001p0, 0x1.000002p0),
    TEST_ff_f (nexttoward, 1.0, 0.9L, 0x0.ffffffp0),
    TEST_ff_f (nexttoward, 1.0, -LDBL_MAX, 0x0.ffffffp0),
    TEST_ff_f (nexttoward, 1.0, 0x0.fffffffffffff8p0, 0x0.ffffffp0),
    TEST_ff_f (nexttoward, -1.0, -1.1L, -0x1.000002p0),
    TEST_ff_f (nexttoward, -1.0, -LDBL_MAX, -0x1.000002p0),
    TEST_ff_f (nexttoward, -1.0, -0x1.0000000000001p0, -0x1.000002p0),
    TEST_ff_f (nexttoward, -1.0, -0.9L, -0x0.ffffffp0),
    TEST_ff_f (nexttoward, -1.0, LDBL_MAX, -0x0.ffffffp0),
    TEST_ff_f (nexttoward, -1.0, -0x0.fffffffffffff8p0, -0x0.ffffffp0),
    TEST_ff_f (nexttoward, -0x1.3p-145, -0xap-148L, -0x1.4p-145, UNDERFLOW_EXCEPTION),
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (nexttoward, 1.0, 0x1.000000000000002p0L, 0x1.000002p0),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffp0L, 0x0.ffffffp0),
    TEST_ff_f (nexttoward, -1.0, -0x1.000000000000002p0L, -0x1.000002p0),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffp0L, -0x0.ffffffp0),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (nexttoward, 1.0, 0x1.000000000000000000000000008p0L, 0x1.000002p0),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffcp0L, 0x0.ffffffp0),
    TEST_ff_f (nexttoward, -1.0, -0x1.000000000000000000000000008p0L, -0x1.000002p0),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffcp0L, -0x0.ffffffp0),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (nexttoward, 1.0, 0x1.0000000000000000000000000001p0L, 0x1.000002p0),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffp0),
    TEST_ff_f (nexttoward, -1.0, -0x1.0000000000000000000000000001p0L, -0x1.000002p0),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffp0),
# endif
#endif
#ifdef TEST_DOUBLE
    TEST_ff_f (nexttoward, 1.0, 1.1L, 0x1.0000000000001p0),
    TEST_ff_f (nexttoward, 1.0, LDBL_MAX, 0x1.0000000000001p0),
    TEST_ff_f (nexttoward, 1.0, 0x1.0000000000001p0, 0x1.0000000000001p0),
    TEST_ff_f (nexttoward, 1.0, 0.9L, 0x0.fffffffffffff8p0),
    TEST_ff_f (nexttoward, 1.0, -LDBL_MAX, 0x0.fffffffffffff8p0),
    TEST_ff_f (nexttoward, 1.0, 0x0.fffffffffffff8p0, 0x0.fffffffffffff8p0),
    TEST_ff_f (nexttoward, -1.0, -1.1L, -0x1.0000000000001p0),
    TEST_ff_f (nexttoward, -1.0, -LDBL_MAX, -0x1.0000000000001p0),
    TEST_ff_f (nexttoward, -1.0, -0x1.0000000000001p0, -0x1.0000000000001p0),
    TEST_ff_f (nexttoward, -1.0, -0.9L, -0x0.fffffffffffff8p0),
    TEST_ff_f (nexttoward, -1.0, LDBL_MAX, -0x0.fffffffffffff8p0),
    TEST_ff_f (nexttoward, -1.0, -0x0.fffffffffffff8p0, -0x0.fffffffffffff8p0),
    TEST_ff_f (nexttoward, -1.0, -0x8.00346dc5d6388p-3L, -0x1.0000000000001p0),
    TEST_ff_f (nexttoward, 0x1p-1074, 0x1p-1073L, 0x1p-1073, UNDERFLOW_EXCEPTION),
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (nexttoward, 1.0, 0x1.000000000000002p0L, 0x1.0000000000001p0),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffp0L, 0x0.fffffffffffff8p0),
    TEST_ff_f (nexttoward, -1.0, -0x1.000000000000002p0L, -0x1.0000000000001p0),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffp0L, -0x0.fffffffffffff8p0),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (nexttoward, 1.0, 0x1.000000000000000000000000008p0L, 0x1.0000000000001p0),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffcp0L, 0x0.fffffffffffff8p0),
    TEST_ff_f (nexttoward, -1.0, -0x1.000000000000000000000000008p0L, -0x1.0000000000001p0),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffcp0L, -0x0.fffffffffffff8p0),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (nexttoward, 1.0, 0x1.0000000000000000000000000001p0L, 0x1.0000000000001p0),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.fffffffffffff8p0),
    TEST_ff_f (nexttoward, -1.0, -0x1.0000000000000000000000000001p0L, -0x1.0000000000001p0),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.fffffffffffff8p0),
# endif
#endif
    END_DATA (nexttoward)
  };

static void
nexttoward_test (void)
{
  START (nexttoward);
  RUN_TEST_LOOP_ff_f (nexttoward, nexttoward_test_data, );
  END (nexttoward);
}


static const struct test_ff_f_data pow_test_data[] =
  {
    START_DATA (pow),
    TEST_ff_f (pow, 0, 0, 1),
    TEST_ff_f (pow, 0, minus_zero, 1),
    TEST_ff_f (pow, minus_zero, 0, 1),
    TEST_ff_f (pow, minus_zero, minus_zero, 1),

    TEST_ff_f (pow, 10, 0, 1),
    TEST_ff_f (pow, 10, minus_zero, 1),
    TEST_ff_f (pow, -10, 0, 1),
    TEST_ff_f (pow, -10, minus_zero, 1),

    TEST_ff_f (pow, qnan_value, 0, 1),
    TEST_ff_f (pow, qnan_value, minus_zero, 1),

#ifndef TEST_INLINE
    TEST_ff_f (pow, 1.1L, plus_infty, plus_infty),
    TEST_ff_f (pow, plus_infty, plus_infty, plus_infty),
    TEST_ff_f (pow, -1.1L, plus_infty, plus_infty),
    TEST_ff_f (pow, minus_infty, plus_infty, plus_infty),

    TEST_ff_f (pow, 0.9L, plus_infty, 0),
    TEST_ff_f (pow, 1e-7L, plus_infty, 0),
    TEST_ff_f (pow, -0.9L, plus_infty, 0),
    TEST_ff_f (pow, -1e-7L, plus_infty, 0),

    TEST_ff_f (pow, 1.1L, minus_infty, 0),
    TEST_ff_f (pow, plus_infty, minus_infty, 0),
    TEST_ff_f (pow, -1.1L, minus_infty, 0),
    TEST_ff_f (pow, minus_infty, minus_infty, 0),

    TEST_ff_f (pow, 0.9L, minus_infty, plus_infty),
    TEST_ff_f (pow, 1e-7L, minus_infty, plus_infty),
    TEST_ff_f (pow, -0.9L, minus_infty, plus_infty),
    TEST_ff_f (pow, -1e-7L, minus_infty, plus_infty),

    TEST_ff_f (pow, plus_infty, 1e-7L, plus_infty),
    TEST_ff_f (pow, plus_infty, 1, plus_infty),
    TEST_ff_f (pow, plus_infty, 1e7L, plus_infty),
    TEST_ff_f (pow, plus_infty, min_subnorm_value, plus_infty),

    TEST_ff_f (pow, plus_infty, -1e-7L, 0),
    TEST_ff_f (pow, plus_infty, -1, 0),
    TEST_ff_f (pow, plus_infty, -1e7L, 0),
    TEST_ff_f (pow, plus_infty, -min_subnorm_value, 0),

    TEST_ff_f (pow, minus_infty, 1, minus_infty),
    TEST_ff_f (pow, minus_infty, 11, minus_infty),
    TEST_ff_f (pow, minus_infty, 1001, minus_infty),

    TEST_ff_f (pow, minus_infty, 2, plus_infty),
    TEST_ff_f (pow, minus_infty, 12, plus_infty),
    TEST_ff_f (pow, minus_infty, 1002, plus_infty),
    TEST_ff_f (pow, minus_infty, 0.1L, plus_infty),
    TEST_ff_f (pow, minus_infty, 1.1L, plus_infty),
    TEST_ff_f (pow, minus_infty, 11.1L, plus_infty),
    TEST_ff_f (pow, minus_infty, 1001.1L, plus_infty),
    TEST_ff_f (pow, minus_infty, min_subnorm_value, plus_infty),

    TEST_ff_f (pow, minus_infty, -1, minus_zero),
    TEST_ff_f (pow, minus_infty, -11, minus_zero),
    TEST_ff_f (pow, minus_infty, -1001, minus_zero),

    TEST_ff_f (pow, minus_infty, -2, 0),
    TEST_ff_f (pow, minus_infty, -12, 0),
    TEST_ff_f (pow, minus_infty, -1002, 0),
    TEST_ff_f (pow, minus_infty, -0.1L, 0),
    TEST_ff_f (pow, minus_infty, -1.1L, 0),
    TEST_ff_f (pow, minus_infty, -11.1L, 0),
    TEST_ff_f (pow, minus_infty, -1001.1L, 0),
    TEST_ff_f (pow, minus_infty, -min_subnorm_value, 0),
#endif

    TEST_ff_f (pow, qnan_value, qnan_value, qnan_value),
    TEST_ff_f (pow, 0, qnan_value, qnan_value),
    TEST_ff_f (pow, 1, qnan_value, 1),
    TEST_ff_f (pow, -1, qnan_value, qnan_value),
    TEST_ff_f (pow, qnan_value, 1, qnan_value),
    TEST_ff_f (pow, qnan_value, -1, qnan_value),

    /* pow (x, qNaN) == qNaN.  */
    TEST_ff_f (pow, 3.0, qnan_value, qnan_value),
    TEST_ff_f (pow, minus_zero, qnan_value, qnan_value),
    TEST_ff_f (pow, plus_infty, qnan_value, qnan_value),
    TEST_ff_f (pow, -3.0, qnan_value, qnan_value),
    TEST_ff_f (pow, minus_infty, qnan_value, qnan_value),

    TEST_ff_f (pow, qnan_value, 3.0, qnan_value),
    TEST_ff_f (pow, qnan_value, -3.0, qnan_value),
    TEST_ff_f (pow, qnan_value, plus_infty, qnan_value),
    TEST_ff_f (pow, qnan_value, minus_infty, qnan_value),
    TEST_ff_f (pow, qnan_value, 2.5, qnan_value),
    TEST_ff_f (pow, qnan_value, -2.5, qnan_value),
    TEST_ff_f (pow, qnan_value, min_subnorm_value, qnan_value),
    TEST_ff_f (pow, qnan_value, -min_subnorm_value, qnan_value),

    TEST_ff_f (pow, 1, plus_infty, 1),
    TEST_ff_f (pow, -1, plus_infty, 1),
    TEST_ff_f (pow, 1, minus_infty, 1),
    TEST_ff_f (pow, -1, minus_infty, 1),
    TEST_ff_f (pow, 1, 1, 1),
    TEST_ff_f (pow, 1, -1, 1),
    TEST_ff_f (pow, 1, 1.25, 1),
    TEST_ff_f (pow, 1, -1.25, 1),
    TEST_ff_f (pow, 1, 0x1p62L, 1),
    TEST_ff_f (pow, 1, 0x1p63L, 1),
    TEST_ff_f (pow, 1, 0x1p64L, 1),
    TEST_ff_f (pow, 1, 0x1p72L, 1),
    TEST_ff_f (pow, 1, min_subnorm_value, 1),
    TEST_ff_f (pow, 1, -min_subnorm_value, 1),

    /* pow (x, +-0) == 1.  */
    TEST_ff_f (pow, plus_infty, 0, 1),
    TEST_ff_f (pow, plus_infty, minus_zero, 1),
    TEST_ff_f (pow, minus_infty, 0, 1),
    TEST_ff_f (pow, minus_infty, minus_zero, 1),
    TEST_ff_f (pow, 32.75L, 0, 1),
    TEST_ff_f (pow, 32.75L, minus_zero, 1),
    TEST_ff_f (pow, -32.75L, 0, 1),
    TEST_ff_f (pow, -32.75L, minus_zero, 1),
    TEST_ff_f (pow, 0x1p72L, 0, 1),
    TEST_ff_f (pow, 0x1p72L, minus_zero, 1),
    TEST_ff_f (pow, 0x1p-72L, 0, 1),
    TEST_ff_f (pow, 0x1p-72L, minus_zero, 1),

    TEST_ff_f (pow, -0.1L, 1.1L, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -0.1L, -1.1L, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -10.1L, 1.1L, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -10.1L, -1.1L, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -1.01L, min_subnorm_value, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -1.01L, -min_subnorm_value, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -1.0L, min_subnorm_value, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -1.0L, -min_subnorm_value, qnan_value, INVALID_EXCEPTION),

    TEST_ff_f (pow, 0, -1, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, 0, -11, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, 0, -0xffffff, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, 0, -0x1.fffffffffffffp+52L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, 0, -0x1.fffffffffffffffep+63L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, 0, -0x1.ffffffffffffffffffffffffff8p+105L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, 0, -0x1.ffffffffffffffffffffffffffffp+112L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
# endif
#endif
    TEST_ff_f (pow, minus_zero, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -11L, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -0xffffff, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -0x1fffffe, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, minus_zero, -0x1.fffffffffffffp+52L, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -0x1.fffffffffffffp+53L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, minus_zero, -0x1.fffffffffffffffep+63L, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -0x1.fffffffffffffffep+64L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, minus_zero, -0x1.ffffffffffffffffffffffffff8p+105L, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -0x1.ffffffffffffffffffffffffff8p+106L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, minus_zero, -0x1.ffffffffffffffffffffffffffffp+112L, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -0x1.ffffffffffffffffffffffffffffp+113L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
# endif
#endif

    TEST_ff_f (pow, 0, -2, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, 0, -11.1L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, 0, -min_subnorm_value, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, 0, -0x1p24, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, 0, -0x1p127, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, 0, -max_value, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -2, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -11.1L, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -min_subnorm_value, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -0x1p24, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -0x1p127, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_ff_f (pow, minus_zero, -max_value, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),

    TEST_ff_f (pow, 0x1p72L, 0x1p72L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, 10, -0x1p72L, 0, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, max_value, max_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, 10, -max_value, 0, UNDERFLOW_EXCEPTION),

    TEST_ff_f (pow, 0, 1, 0),
    TEST_ff_f (pow, 0, 11, 0),

    TEST_ff_f (pow, minus_zero, 1, minus_zero),
    TEST_ff_f (pow, minus_zero, 11, minus_zero),

    TEST_ff_f (pow, 0, 2, 0),
    TEST_ff_f (pow, 0, 11.1L, 0),

    TEST_ff_f (pow, minus_zero, 2, 0),
    TEST_ff_f (pow, minus_zero, 11.1L, 0),
    TEST_ff_f (pow, 0, plus_infty, 0),
    TEST_ff_f (pow, minus_zero, plus_infty, 0),
    TEST_ff_f (pow, 0, minus_infty, plus_infty, DIVIDE_BY_ZERO_EXCEPTION_OK),
    TEST_ff_f (pow, minus_zero, minus_infty, plus_infty, DIVIDE_BY_ZERO_EXCEPTION_OK),

#ifndef TEST_INLINE
    /* pow (x, +inf) == +inf for |x| > 1.  */
    TEST_ff_f (pow, 1.5, plus_infty, plus_infty),

    /* pow (x, +inf) == +0 for |x| < 1.  */
    TEST_ff_f (pow, 0.5, plus_infty, 0.0),

    /* pow (x, -inf) == +0 for |x| > 1.  */
    TEST_ff_f (pow, 1.5, minus_infty, 0.0),

    /* pow (x, -inf) == +inf for |x| < 1.  */
    TEST_ff_f (pow, 0.5, minus_infty, plus_infty),
#endif

    /* pow (+inf, y) == +inf for y > 0.  */
    TEST_ff_f (pow, plus_infty, 2, plus_infty),
    TEST_ff_f (pow, plus_infty, 0xffffff, plus_infty),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, plus_infty, 0x1.fffffffffffffp+52L, plus_infty),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, plus_infty, 0x1.fffffffffffffffep+63L, plus_infty),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, plus_infty, 0x1.ffffffffffffffffffffffffff8p+105L, plus_infty),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, plus_infty, 0x1.ffffffffffffffffffffffffffffp+112L, plus_infty),
# endif
#endif
    TEST_ff_f (pow, plus_infty, 0x1p24, plus_infty),
    TEST_ff_f (pow, plus_infty, 0x1p127, plus_infty),
    TEST_ff_f (pow, plus_infty, max_value, plus_infty),

    /* pow (+inf, y) == +0 for y < 0.  */
    TEST_ff_f (pow, plus_infty, -1, 0.0),
    TEST_ff_f (pow, plus_infty, -0xffffff, 0.0),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, plus_infty, -0x1.fffffffffffffp+52L, 0.0),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, plus_infty, -0x1.fffffffffffffffep+63L, 0.0),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, plus_infty, -0x1.ffffffffffffffffffffffffff8p+105L, 0.0),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, plus_infty, -0x1.ffffffffffffffffffffffffffffp+112L, 0.0),
# endif
#endif
    TEST_ff_f (pow, plus_infty, -0x1p24, 0.0),
    TEST_ff_f (pow, plus_infty, -0x1p127, 0.0),
    TEST_ff_f (pow, plus_infty, -max_value, 0.0),

    /* pow (-inf, y) == -inf for y an odd integer > 0.  */
    TEST_ff_f (pow, minus_infty, 27, minus_infty),
    TEST_ff_f (pow, minus_infty, 0xffffff, minus_infty),
    TEST_ff_f (pow, minus_infty, 0x1fffffe, plus_infty),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, minus_infty, 0x1.fffffffffffffp+52L, minus_infty),
    TEST_ff_f (pow, minus_infty, 0x1.fffffffffffffp+53L, plus_infty),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, minus_infty, 0x1.fffffffffffffffep+63L, minus_infty),
    TEST_ff_f (pow, minus_infty, 0x1.fffffffffffffffep+64L, plus_infty),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, minus_infty, 0x1.ffffffffffffffffffffffffff8p+105L, minus_infty),
    TEST_ff_f (pow, minus_infty, 0x1.ffffffffffffffffffffffffff8p+106L, plus_infty),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, minus_infty, 0x1.ffffffffffffffffffffffffffffp+112L, minus_infty),
    TEST_ff_f (pow, minus_infty, 0x1.ffffffffffffffffffffffffffffp+113L, plus_infty),
# endif
#endif

    /* pow (-inf, y) == +inf for y > 0 and not an odd integer.  */
    TEST_ff_f (pow, minus_infty, 28, plus_infty),
    TEST_ff_f (pow, minus_infty, 0x1p24, plus_infty),
    TEST_ff_f (pow, minus_infty, 0x1p127, plus_infty),
    TEST_ff_f (pow, minus_infty, max_value, plus_infty),

    /* pow (-inf, y) == -0 for y an odd integer < 0. */
    TEST_ff_f (pow, minus_infty, -3, minus_zero),
    TEST_ff_f (pow, minus_infty, -0xffffff, minus_zero),
    TEST_ff_f (pow, minus_infty, -0x1fffffe, plus_zero),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, minus_infty, -0x1.fffffffffffffp+52L, minus_zero),
    TEST_ff_f (pow, minus_infty, -0x1.fffffffffffffp+53L, plus_zero),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, minus_infty, -0x1.fffffffffffffffep+63L, minus_zero),
    TEST_ff_f (pow, minus_infty, -0x1.fffffffffffffffep+64L, plus_zero),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, minus_infty, -0x1.ffffffffffffffffffffffffff8p+105L, minus_zero),
    TEST_ff_f (pow, minus_infty, -0x1.ffffffffffffffffffffffffff8p+106L, plus_zero),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, minus_infty, -0x1.ffffffffffffffffffffffffffffp+112L, minus_zero),
    TEST_ff_f (pow, minus_infty, -0x1.ffffffffffffffffffffffffffffp+113L, plus_zero),
# endif
#endif
    /* pow (-inf, y) == +0 for y < 0 and not an odd integer.  */
    TEST_ff_f (pow, minus_infty, -2.0, 0.0),
    TEST_ff_f (pow, minus_infty, -0x1p24, 0.0),
    TEST_ff_f (pow, minus_infty, -0x1p127, 0.0),
    TEST_ff_f (pow, minus_infty, -max_value, 0.0),

    /* pow (+0, y) == +0 for y an odd integer > 0.  */
    TEST_ff_f (pow, 0.0, 27, 0.0),
    TEST_ff_f (pow, 0.0, 0xffffff, 0.0),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, 0.0, 0x1.fffffffffffffp+52L, 0.0),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, 0.0, 0x1.fffffffffffffffep+63L, 0.0),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, 0.0, 0x1.ffffffffffffffffffffffffff8p+105L, 0.0),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, 0.0, 0x1.ffffffffffffffffffffffffffffp+112L, 0.0),
# endif
#endif

    /* pow (-0, y) == -0 for y an odd integer > 0.  */
    TEST_ff_f (pow, minus_zero, 27, minus_zero),
    TEST_ff_f (pow, minus_zero, 0xffffff, minus_zero),
    TEST_ff_f (pow, minus_zero, 0x1fffffe, plus_zero),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, minus_zero, 0x1.fffffffffffffp+52L, minus_zero),
    TEST_ff_f (pow, minus_zero, 0x1.fffffffffffffp+53L, plus_zero),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, minus_zero, 0x1.fffffffffffffffep+63L, minus_zero),
    TEST_ff_f (pow, minus_zero, 0x1.fffffffffffffffep+64L, plus_zero),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, minus_zero, 0x1.ffffffffffffffffffffffffff8p+105L, minus_zero),
    TEST_ff_f (pow, minus_zero, 0x1.ffffffffffffffffffffffffff8p+106L, plus_zero),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, minus_zero, 0x1.ffffffffffffffffffffffffffffp+112L, minus_zero),
    TEST_ff_f (pow, minus_zero, 0x1.ffffffffffffffffffffffffffffp+113L, plus_zero),
# endif
#endif

    /* pow (+0, y) == +0 for y > 0 and not an odd integer.  */
    TEST_ff_f (pow, 0.0, 4, 0.0),
    TEST_ff_f (pow, 0.0, 0x1p24, 0.0),
    TEST_ff_f (pow, 0.0, 0x1p127, 0.0),
    TEST_ff_f (pow, 0.0, max_value, 0.0),
    TEST_ff_f (pow, 0.0, min_subnorm_value, 0.0),

    /* pow (-0, y) == +0 for y > 0 and not an odd integer.  */
    TEST_ff_f (pow, minus_zero, 4, 0.0),
    TEST_ff_f (pow, minus_zero, 0x1p24, 0.0),
    TEST_ff_f (pow, minus_zero, 0x1p127, 0.0),
    TEST_ff_f (pow, minus_zero, max_value, 0.0),
    TEST_ff_f (pow, minus_zero, min_subnorm_value, 0.0),

    TEST_ff_f (pow, 16, 0.25L, 2),
    TEST_ff_f (pow, 0x1p64L, 0.125L, 256),
    TEST_ff_f (pow, 2, 4, 16),
    TEST_ff_f (pow, 256, 8, 0x1p64L),

    TEST_ff_f (pow, 0.75L, 1.25L, 0.697953644326574699205914060237425566L),

#if defined TEST_DOUBLE || defined TEST_LDOUBLE
    TEST_ff_f (pow, -7.49321e+133, -9.80818e+16, 0, UNDERFLOW_EXCEPTION),
#endif

    TEST_ff_f (pow, -1.0, -0xffffff, -1.0),
    TEST_ff_f (pow, -1.0, -0x1fffffe, 1.0),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, -1.0, -0x1.fffffffffffffp+52L, -1.0),
    TEST_ff_f (pow, -1.0, -0x1.fffffffffffffp+53L, 1.0),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, -1.0, -0x1.fffffffffffffffep+63L, -1.0),
    TEST_ff_f (pow, -1.0, -0x1.fffffffffffffffep+64L, 1.0),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, -1.0, -0x1.ffffffffffffffffffffffffff8p+105L, -1.0),
    TEST_ff_f (pow, -1.0, -0x1.ffffffffffffffffffffffffff8p+106L, 1.0),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, -1.0, -0x1.ffffffffffffffffffffffffffffp+112L, -1.0),
    TEST_ff_f (pow, -1.0, -0x1.ffffffffffffffffffffffffffffp+113L, 1.0),
# endif
#endif
    TEST_ff_f (pow, -1.0, -max_value, 1.0),

    TEST_ff_f (pow, -1.0, 0xffffff, -1.0),
    TEST_ff_f (pow, -1.0, 0x1fffffe, 1.0),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, -1.0, 0x1.fffffffffffffp+52L, -1.0),
    TEST_ff_f (pow, -1.0, 0x1.fffffffffffffp+53L, 1.0),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, -1.0, 0x1.fffffffffffffffep+63L, -1.0),
    TEST_ff_f (pow, -1.0, 0x1.fffffffffffffffep+64L, 1.0),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, -1.0, 0x1.ffffffffffffffffffffffffff8p+105L, -1.0),
    TEST_ff_f (pow, -1.0, 0x1.ffffffffffffffffffffffffff8p+106L, 1.0),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, -1.0, 0x1.ffffffffffffffffffffffffffffp+112L, -1.0),
    TEST_ff_f (pow, -1.0, 0x1.ffffffffffffffffffffffffffffp+113L, 1.0),
# endif
#endif
    TEST_ff_f (pow, -1.0, max_value, 1.0),

    TEST_ff_f (pow, -2.0, 126, 0x1p126),
    TEST_ff_f (pow, -2.0, 127, -0x1p127),
    /* Allow inexact results for float to be considered to underflow.  */
    TEST_ff_f (pow, -2.0, -126, 0x1p-126, UNDERFLOW_EXCEPTION_OK_FLOAT),
    TEST_ff_f (pow, -2.0, -127, -0x1p-127, UNDERFLOW_EXCEPTION_OK_FLOAT),

    TEST_ff_f (pow, -2.0, -0xffffff, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -2.0, -0x1fffffe, plus_zero, UNDERFLOW_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, -2.0, -0x1.fffffffffffffp+52L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -2.0, -0x1.fffffffffffffp+53L, plus_zero, UNDERFLOW_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, -2.0, -0x1.fffffffffffffffep+63L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -2.0, -0x1.fffffffffffffffep+64L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, -2.0, -0x1.ffffffffffffffffffffffffff8p+105L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -2.0, -0x1.ffffffffffffffffffffffffff8p+106L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, -2.0, -0x1.ffffffffffffffffffffffffffffp+112L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -2.0, -0x1.ffffffffffffffffffffffffffffp+113L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
#endif
    TEST_ff_f (pow, -2.0, -max_value, plus_zero, UNDERFLOW_EXCEPTION),

    TEST_ff_f (pow, -2.0, 0xffffff, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -2.0, 0x1fffffe, plus_infty, OVERFLOW_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, -2.0, 0x1.fffffffffffffp+52L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -2.0, 0x1.fffffffffffffp+53L, plus_infty, OVERFLOW_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, -2.0, 0x1.fffffffffffffffep+63L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -2.0, 0x1.fffffffffffffffep+64L, plus_infty, OVERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, -2.0, 0x1.ffffffffffffffffffffffffff8p+105L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -2.0, 0x1.ffffffffffffffffffffffffff8p+106L, plus_infty, OVERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, -2.0, 0x1.ffffffffffffffffffffffffffffp+112L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -2.0, 0x1.ffffffffffffffffffffffffffffp+113L, plus_infty, OVERFLOW_EXCEPTION),
# endif
#endif
    TEST_ff_f (pow, -2.0, max_value, plus_infty, OVERFLOW_EXCEPTION),

    TEST_ff_f (pow, -max_value, 0.5, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -max_value, 1.5, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -max_value, 1000.5, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -max_value, -2, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, -3, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, 2, plus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, 3, minus_infty, OVERFLOW_EXCEPTION),

    TEST_ff_f (pow, -max_value, -0xffffff, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, -0x1fffffe, plus_zero, UNDERFLOW_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, -max_value, -0x1.fffffffffffffp+52L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, -0x1.fffffffffffffp+53L, plus_zero, UNDERFLOW_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, -max_value, -0x1.fffffffffffffffep+63L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, -0x1.fffffffffffffffep+64L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, -max_value, -0x1.ffffffffffffffffffffffffff8p+105L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, -0x1.ffffffffffffffffffffffffff8p+106L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, -max_value, -0x1.ffffffffffffffffffffffffffffp+112L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, -0x1.ffffffffffffffffffffffffffffp+113L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
#endif
    TEST_ff_f (pow, -max_value, -max_value, plus_zero, UNDERFLOW_EXCEPTION),

    TEST_ff_f (pow, -max_value, 0xffffff, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, 0x1fffffe, plus_infty, OVERFLOW_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, -max_value, 0x1.fffffffffffffp+52L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, 0x1.fffffffffffffp+53L, plus_infty, OVERFLOW_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, -max_value, 0x1.fffffffffffffffep+63L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, 0x1.fffffffffffffffep+64L, plus_infty, OVERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, -max_value, 0x1.ffffffffffffffffffffffffff8p+105L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, 0x1.ffffffffffffffffffffffffff8p+106L, plus_infty, OVERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, -max_value, 0x1.ffffffffffffffffffffffffffffp+112L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -max_value, 0x1.ffffffffffffffffffffffffffffp+113L, plus_infty, OVERFLOW_EXCEPTION),
# endif
#endif
    TEST_ff_f (pow, -max_value, max_value, plus_infty, OVERFLOW_EXCEPTION),

    TEST_ff_f (pow, -0.5, 126, 0x1p-126),
    TEST_ff_f (pow, -0.5, 127, -0x1p-127),
    TEST_ff_f (pow, -0.5, -126, 0x1p126),
    TEST_ff_f (pow, -0.5, -127, -0x1p127),

    TEST_ff_f (pow, -0.5, -0xffffff, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -0.5, -0x1fffffe, plus_infty, OVERFLOW_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, -0.5, -0x1.fffffffffffffp+52L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -0.5, -0x1.fffffffffffffp+53L, plus_infty, OVERFLOW_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, -0.5, -0x1.fffffffffffffffep+63L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -0.5, -0x1.fffffffffffffffep+64L, plus_infty, OVERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, -0.5, -0x1.ffffffffffffffffffffffffff8p+105L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -0.5, -0x1.ffffffffffffffffffffffffff8p+106L, plus_infty, OVERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, -0.5, -0x1.ffffffffffffffffffffffffffffp+112L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -0.5, -0x1.ffffffffffffffffffffffffffffp+113L, plus_infty, OVERFLOW_EXCEPTION),
# endif
#endif
    TEST_ff_f (pow, -0.5, -max_value, plus_infty, OVERFLOW_EXCEPTION),

    TEST_ff_f (pow, -0.5, 0xffffff, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -0.5, 0x1fffffe, plus_zero, UNDERFLOW_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, -0.5, 0x1.fffffffffffffp+52L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -0.5, 0x1.fffffffffffffp+53L, plus_zero, UNDERFLOW_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, -0.5, 0x1.fffffffffffffffep+63L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -0.5, 0x1.fffffffffffffffep+64L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, -0.5, 0x1.ffffffffffffffffffffffffff8p+105L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -0.5, 0x1.ffffffffffffffffffffffffff8p+106L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, -0.5, 0x1.ffffffffffffffffffffffffffffp+112L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -0.5, 0x1.ffffffffffffffffffffffffffffp+113L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
#endif
    TEST_ff_f (pow, -0.5, max_value, plus_zero, UNDERFLOW_EXCEPTION),

    TEST_ff_f (pow, -min_value, 0.5, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -min_value, 1.5, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -min_value, 1000.5, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (pow, -min_value, -2, plus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, -3, minus_infty, OVERFLOW_EXCEPTION),
    /* Allow inexact results to be considered to underflow.  */
    TEST_ff_f (pow, -min_value, 1, -min_value, UNDERFLOW_EXCEPTION_OK),
    TEST_ff_f (pow, -min_value, 2, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, 3, minus_zero, UNDERFLOW_EXCEPTION),

    TEST_ff_f (pow, -min_value, -0xffffff, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, -0x1fffffe, plus_infty, OVERFLOW_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, -min_value, -0x1.fffffffffffffp+52L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, -0x1.fffffffffffffp+53L, plus_infty, OVERFLOW_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, -min_value, -0x1.fffffffffffffffep+63L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, -0x1.fffffffffffffffep+64L, plus_infty, OVERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, -min_value, -0x1.ffffffffffffffffffffffffff8p+105L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, -0x1.ffffffffffffffffffffffffff8p+106L, plus_infty, OVERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, -min_value, -0x1.ffffffffffffffffffffffffffffp+112L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, -0x1.ffffffffffffffffffffffffffffp+113L, plus_infty, OVERFLOW_EXCEPTION),
# endif
#endif
    TEST_ff_f (pow, -min_value, -max_value, plus_infty, OVERFLOW_EXCEPTION),

    TEST_ff_f (pow, -min_value, 0xffffff, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, 0x1fffffe, plus_zero, UNDERFLOW_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_ff_f (pow, -min_value, 0x1.fffffffffffffp+52L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, 0x1.fffffffffffffp+53L, plus_zero, UNDERFLOW_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (pow, -min_value, 0x1.fffffffffffffffep+63L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, 0x1.fffffffffffffffep+64L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (pow, -min_value, 0x1.ffffffffffffffffffffffffff8p+105L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, 0x1.ffffffffffffffffffffffffff8p+106L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, -min_value, 0x1.ffffffffffffffffffffffffffffp+112L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_ff_f (pow, -min_value, 0x1.ffffffffffffffffffffffffffffp+113L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
#endif
    TEST_ff_f (pow, -min_value, max_value, plus_zero, UNDERFLOW_EXCEPTION),

    TEST_ff_f (pow, 0x0.ffffffp0, 10, 0.999999403953712118183885036774764444747L),
    TEST_ff_f (pow, 0x0.ffffffp0, 100, 0.999994039553108359406305079606228341585L),
    TEST_ff_f (pow, 0x0.ffffffp0, 1000, 0.9999403971297699052276650144650733772182L),
    TEST_ff_f (pow, 0x0.ffffffp0, 0x1p24, 0.3678794302077803437135155590023422899744L),
    TEST_ff_f (pow, 0x0.ffffffp0, 0x1p30, 1.603807831524924233828134753069728224044e-28L),
    TEST_ff_f (pow, 0x0.ffffffp0, 0x1.234566p30, 2.374884712135295099971443365381007297732e-32L),
    TEST_ff_f (pow, 0x0.ffffffp0, -10, 1.000000596046643153205170848674671339688L),
    TEST_ff_f (pow, 0x0.ffffffp0, -100, 1.000005960482418779499387594989252621451L),
    TEST_ff_f (pow, 0x0.ffffffp0, -1000, 1.000059606422943986382898964231519867906L),
    TEST_ff_f (pow, 0x0.ffffffp0, -0x1p24, 2.7182819094701610539628664526874952929416L),
    TEST_ff_f (pow, 0x0.ffffffp0, -0x1p30, 6.2351609734265057988914412331288163636075e+27L),
    TEST_ff_f (pow, 0x0.ffffffp0, -0x1.234566p30, 4.2107307141696353498921307077142537353515e+31L),
    TEST_ff_f (pow, 0x1.000002p0, 0x1p24, 7.3890552180866447284268641248075832310141L),
    TEST_ff_f (pow, 0x1.000002p0, 0x1.234566p29, 4.2107033006507495188536371520637025716256e+31L),
    TEST_ff_f (pow, 0x1.000002p0, -0x1.234566p29, 2.3749001736727769098946062325205705312166e-32L),

#if !defined TEST_FLOAT
    TEST_ff_f (pow, 0x0.fffffffffffff8p0L, 0x1.23456789abcdfp62L, 1.0118762747827252817436395051178295138220e-253L),
    TEST_ff_f (pow, 0x0.fffffffffffff8p0L, -0x1.23456789abcdfp62L, 9.8826311568054561811190162420900667121992e+252L),
    TEST_ff_f (pow, 0x1.0000000000001p0L, 0x1.23456789abcdfp61L, 9.8826311568044974397135026217687399395481e+252L),
    TEST_ff_f (pow, 0x1.0000000000001p0L, -0x1.23456789abcdfp61L, 1.0118762747828234466621210689458255908670e-253L),
#endif

#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 64 && LDBL_MAX_EXP >= 16384
    TEST_ff_f (pow, 0x0.ffffffffffffffffp0L, 0x1.23456789abcdef0ep77L, 1.2079212226420368189981778807634890018840e-4048L),
    TEST_ff_f (pow, 0x0.ffffffffffffffffp0L, -0x1.23456789abcdef0ep77L, 8.2786855736563746280496724205839522148001e+4047L),
    TEST_ff_f (pow, 0x1.0000000000000002p0L, 0x1.23456789abcdef0ep76L, 8.2786855736563683535324500168799315131570e+4047L),
    TEST_ff_f (pow, 0x1.0000000000000002p0L, -0x1.23456789abcdef0ep76L, 1.2079212226420377344964713407722652880280e-4048L),
#endif

#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 113
    TEST_ff_f (pow, 0x0.ffffffffffffffffffffffffffff8p0L, 0x1.23456789abcdef0123456789abcdp126L, 1.2079212226420440237790185999151440179953e-4048L),
    TEST_ff_f (pow, 0x0.ffffffffffffffffffffffffffff8p0L, -0x1.23456789abcdef0123456789abcdp126L, 8.2786855736563252489063231915535105363602e+4047L),
    TEST_ff_f (pow, 0x1.0000000000000000000000000001p0L, 0x1.23456789abcdef0123456789abcdp125L, 8.2786855736563252489063231915423647547782e+4047L),
    TEST_ff_f (pow, 0x1.0000000000000000000000000001p0L, -0x1.23456789abcdef0123456789abcdp125L, 1.2079212226420440237790185999167702696503e-4048L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_ff_f (pow, 1e4932L, 0.75L, 1e3699L),
    TEST_ff_f (pow, 1e4928L, 0.75L, 1e3696L),
    TEST_ff_f (pow, 1e4924L, 0.75L, 1e3693L),
    TEST_ff_f (pow, 1e4920L, 0.75L, 1e3690L),
    TEST_ff_f (pow, 10.0L, 4932.0L, 1e4932L),
    TEST_ff_f (pow, 10.0L, 4931.0L, 1e4931L),
    TEST_ff_f (pow, 10.0L, 4930.0L, 1e4930L),
    TEST_ff_f (pow, 10.0L, 4929.0L, 1e4929L),
    TEST_ff_f (pow, 10.0L, -4931.0L, 1e-4931L),
    TEST_ff_f (pow, 10.0L, -4930.0L, 1e-4930L),
    TEST_ff_f (pow, 10.0L, -4929.0L, 1e-4929L),
    TEST_ff_f (pow, 1e27L, 182.0L, 1e4914L),
    TEST_ff_f (pow, 1e27L, -182.0L, 1e-4914L),
#endif

    TEST_ff_f (pow, min_subnorm_value, min_subnorm_value, 1.0L),
    TEST_ff_f (pow, min_subnorm_value, -min_subnorm_value, 1.0L),
    TEST_ff_f (pow, max_value, min_subnorm_value, 1.0L),
    TEST_ff_f (pow, max_value, -min_subnorm_value, 1.0L),
    TEST_ff_f (pow, 0.99L, min_subnorm_value, 1.0L),
    TEST_ff_f (pow, 0.99L, -min_subnorm_value, 1.0L),
    TEST_ff_f (pow, 1.01L, min_subnorm_value, 1.0L),
    TEST_ff_f (pow, 1.01L, -min_subnorm_value, 1.0L),

    TEST_ff_f (pow, 2.0L, -100000.0L, plus_zero, UNDERFLOW_EXCEPTION),
    END_DATA (pow)
  };

static void
pow_test (void)
{

  errno = 0;
  FUNC(pow) (0, 0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (pow);
  RUN_TEST_LOOP_ff_f (pow, pow_test_data, );
  END (pow);
}


static const struct test_ff_f_data pow_tonearest_test_data[] =
  {
    START_DATA (pow_tonearest),
    TEST_ff_f (pow, 1.0625L, 1.125L, 1.070582293028761362162622578677070098674L),
    TEST_ff_f (pow, 1.5L, 1.03125L, 1.519127098714743184071644334163037684948L),
    END_DATA (pow_tonearest)
  };

static void
pow_test_tonearest (void)
{
  errno = 0;
  FUNC(pow) (0, 0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (pow_tonearest);
  RUN_TEST_LOOP_ff_f (pow, pow_tonearest_test_data, FE_TONEAREST);
  END (pow_tonearest);
}


static const struct test_ff_f_data pow_towardzero_test_data[] =
  {
    START_DATA (pow_towardzero),
    TEST_ff_f (pow, 1.0625L, 1.125L, 1.070582293028761362162622578677070098674L),
    TEST_ff_f (pow, 1.5L, 1.03125L, 1.519127098714743184071644334163037684948L),
    END_DATA (pow_towardzero)
  };

static void
pow_test_towardzero (void)
{
  errno = 0;
  FUNC(pow) (0, 0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (pow_towardzero);
  RUN_TEST_LOOP_ff_f (pow, pow_towardzero_test_data, FE_TOWARDZERO);
  END (pow_towardzero);
}


static const struct test_ff_f_data pow_downward_test_data[] =
  {
    START_DATA (pow_downward),
    TEST_ff_f (pow, 1.0625L, 1.125L, 1.070582293028761362162622578677070098674L),
    TEST_ff_f (pow, 1.5L, 1.03125L, 1.519127098714743184071644334163037684948L),
    END_DATA (pow_downward)
  };

static void
pow_test_downward (void)
{
  errno = 0;
  FUNC(pow) (0, 0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (pow_downward);
  RUN_TEST_LOOP_ff_f (pow, pow_downward_test_data, FE_DOWNWARD);
  END (pow_downward);
}


static const struct test_ff_f_data pow_upward_test_data[] =
  {
    START_DATA (pow_upward),
    TEST_ff_f (pow, 1.0625L, 1.125L, 1.070582293028761362162622578677070098674L),
    TEST_ff_f (pow, 1.5L, 1.03125L, 1.519127098714743184071644334163037684948L),
    END_DATA (pow_upward)
  };

static void
pow_test_upward (void)
{
  errno = 0;
  FUNC(pow) (0, 0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (pow_upward);
  RUN_TEST_LOOP_ff_f (pow, pow_upward_test_data, FE_UPWARD);
  END (pow_upward);
}


static const struct test_ff_f_data remainder_test_data[] =
  {
    START_DATA (remainder),
    TEST_ff_f (remainder, 1, 0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, 1, minus_zero, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, minus_zero, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 1, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 2, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, minus_zero, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 1, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 2, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, qnan_value, qnan_value, qnan_value, ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 0, qnan_value, qnan_value, ERRNO_UNCHANGED),
    TEST_ff_f (remainder, qnan_value, 0, qnan_value, ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 7.0, plus_infty, 7.0, ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 7.0, minus_infty, 7.0, ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 1.625, 1.0, -0.375),
    TEST_ff_f (remainder, -1.625, 1.0, 0.375),
    TEST_ff_f (remainder, 1.625, -1.0, -0.375),
    TEST_ff_f (remainder, -1.625, -1.0, 0.375),
    TEST_ff_f (remainder, 5.0, 2.0, 1.0),
    TEST_ff_f (remainder, 3.0, 2.0, -1.0),
    END_DATA (remainder)
  };

static void
remainder_test (void)
{
  errno = 0;
  FUNC(remainder) (1.625, 1.0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (remainder);
  RUN_TEST_LOOP_ff_f (remainder, remainder_test_data, );
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

  TEST_ffI_f1 (remquo, 1, 0, qnan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, 1, minus_zero, qnan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, plus_infty, 1, qnan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, minus_infty, 1, qnan_value, IGNORE, INVALID_EXCEPTION);
  TEST_ffI_f1 (remquo, qnan_value, qnan_value, qnan_value, IGNORE);

  TEST_ffI_f1 (remquo, 1.625, 1.0, -0.375, 2);
  TEST_ffI_f1 (remquo, -1.625, 1.0, 0.375, -2);
  TEST_ffI_f1 (remquo, 1.625, -1.0, -0.375, -2);
  TEST_ffI_f1 (remquo, -1.625, -1.0, 0.375, 2);

  TEST_ffI_f1 (remquo, 5, 2, 1, 2);
  TEST_ffI_f1 (remquo, 3, 2, -1, 2);

  END (remquo);
}

static const struct test_f_f_data rint_test_data[] =
  {
    START_DATA (rint),
    /* TODO: missing qNaN tests.  */

    TEST_f_f (rint, 0.0, 0.0),
    TEST_f_f (rint, minus_zero, minus_zero),
    TEST_f_f (rint, plus_infty, plus_infty),
    TEST_f_f (rint, minus_infty, minus_infty),

    /* Default rounding mode is round to even.  */
    TEST_f_f (rint, 0.5, 0.0),
    TEST_f_f (rint, 1.5, 2.0),
    TEST_f_f (rint, 2.5, 2.0),
    TEST_f_f (rint, 3.5, 4.0),
    TEST_f_f (rint, 4.5, 4.0),
    TEST_f_f (rint, -0.5, -0.0),
    TEST_f_f (rint, -1.5, -2.0),
    TEST_f_f (rint, -2.5, -2.0),
    TEST_f_f (rint, -3.5, -4.0),
    TEST_f_f (rint, -4.5, -4.0),
    TEST_f_f (rint, 0.1, 0.0),
    TEST_f_f (rint, 0.25, 0.0),
    TEST_f_f (rint, 0.625, 1.0),
    TEST_f_f (rint, -0.1, -0.0),
    TEST_f_f (rint, -0.25, -0.0),
    TEST_f_f (rint, -0.625, -1.0),
    TEST_f_f (rint, 262144.75, 262145.0),
    TEST_f_f (rint, 262142.75, 262143.0),
    TEST_f_f (rint, 524286.75, 524287.0),
    TEST_f_f (rint, 524288.75, 524289.0),
    TEST_f_f (rint, 1048576.75, 1048577.0),
    TEST_f_f (rint, 2097152.75, 2097153.0),
    TEST_f_f (rint, -1048576.75, -1048577.0),
    TEST_f_f (rint, -2097152.75, -2097153.0),
#ifndef TEST_FLOAT
    TEST_f_f (rint, 70368744177664.75, 70368744177665.0),
    TEST_f_f (rint, 140737488355328.75, 140737488355329.0),
    TEST_f_f (rint, 281474976710656.75, 281474976710657.0),
    TEST_f_f (rint, 562949953421312.75, 562949953421313.0),
    TEST_f_f (rint, 1125899906842624.75, 1125899906842625.0),
    TEST_f_f (rint, -70368744177664.75, -70368744177665.0),
    TEST_f_f (rint, -140737488355328.75, -140737488355329.0),
    TEST_f_f (rint, -281474976710656.75, -281474976710657.0),
    TEST_f_f (rint, -562949953421312.75, -562949953421313.0),
    TEST_f_f (rint, -1125899906842624.75, -1125899906842625.0),
#endif
#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (rint, 4503599627370495.5L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.75L, 4503599627370497.0L),
    TEST_f_f (rint, 4503599627370497.5L, 4503599627370498.0L),

# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370495.0L),
    TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370497.0L),
# endif

    TEST_f_f (rint, -4503599627370495.5L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.75L, -4503599627370497.0L),
    TEST_f_f (rint, -4503599627370497.5L, -4503599627370498.0L),

# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370495.0L),
    TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370497.0L),

    TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L),
    TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L),
    TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740993.0L),
    TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740994.0L),

    TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L),
    TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L),
    TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740993.0L),
    TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740994.0L),
# endif

    TEST_f_f (rint, 9007199254740991.5L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740992.25L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740992.5L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740992.75L, 9007199254740993.0L),
    TEST_f_f (rint, 9007199254740993.5L, 9007199254740994.0L),

    TEST_f_f (rint, -9007199254740991.5L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740992.25L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740992.5L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740992.75L, -9007199254740993.0L),
    TEST_f_f (rint, -9007199254740993.5L, -9007199254740994.0L),

    TEST_f_f (rint, 72057594037927935.5L, 72057594037927936.0L),
    TEST_f_f (rint, 72057594037927936.25L, 72057594037927936.0L),
    TEST_f_f (rint, 72057594037927936.5L, 72057594037927936.0L),
    TEST_f_f (rint, 72057594037927936.75L, 72057594037927937.0L),
    TEST_f_f (rint, 72057594037927937.5L, 72057594037927938.0L),

    TEST_f_f (rint, -72057594037927935.5L, -72057594037927936.0L),
    TEST_f_f (rint, -72057594037927936.25L, -72057594037927936.0L),
    TEST_f_f (rint, -72057594037927936.5L, -72057594037927936.0L),
    TEST_f_f (rint, -72057594037927936.75L, -72057594037927937.0L),
    TEST_f_f (rint, -72057594037927937.5L, -72057594037927938.0L),

    TEST_f_f (rint, 10141204801825835211973625643007.5L, 10141204801825835211973625643008.0L),
    TEST_f_f (rint, 10141204801825835211973625643008.25L, 10141204801825835211973625643008.0L),
    TEST_f_f (rint, 10141204801825835211973625643008.5L, 10141204801825835211973625643008.0L),
    TEST_f_f (rint, 10141204801825835211973625643008.75L, 10141204801825835211973625643009.0L),
    TEST_f_f (rint, 10141204801825835211973625643009.5L, 10141204801825835211973625643010.0L),
#endif
    END_DATA (rint)
  };

static void
rint_test (void)
{
  START (rint);
  RUN_TEST_LOOP_f_f (rint, rint_test_data, );
  END (rint);
}

static const struct test_f_f_data rint_tonearest_test_data[] =
  {
    START_DATA (rint_tonearest),
    TEST_f_f (rint, 2.0, 2.0),
    TEST_f_f (rint, 1.5, 2.0),
    TEST_f_f (rint, 1.0, 1.0),
    TEST_f_f (rint, 0.5, 0.0),
    TEST_f_f (rint, 0.0, 0.0),
    TEST_f_f (rint, minus_zero, minus_zero),
    TEST_f_f (rint, -0.5, -0.0),
    TEST_f_f (rint, -1.0, -1.0),
    TEST_f_f (rint, -1.5, -2.0),
    TEST_f_f (rint, -2.0, -2.0),
    TEST_f_f (rint, 0.1, 0.0),
    TEST_f_f (rint, 0.25, 0.0),
    TEST_f_f (rint, 0.625, 1.0),
    TEST_f_f (rint, -0.1, -0.0),
    TEST_f_f (rint, -0.25, -0.0),
    TEST_f_f (rint, -0.625, -1.0),
    TEST_f_f (rint, 1048576.75, 1048577.0),
    TEST_f_f (rint, 2097152.75, 2097153.0),
    TEST_f_f (rint, -1048576.75, -1048577.0),
    TEST_f_f (rint, -2097152.75, -2097153.0),
#ifndef TEST_FLOAT
    TEST_f_f (rint, 70368744177664.75, 70368744177665.0),
    TEST_f_f (rint, 140737488355328.75, 140737488355329.0),
    TEST_f_f (rint, 281474976710656.75, 281474976710657.0),
    TEST_f_f (rint, 562949953421312.75, 562949953421313.0),
    TEST_f_f (rint, 1125899906842624.75, 1125899906842625.0),
    TEST_f_f (rint, -70368744177664.75, -70368744177665.0),
    TEST_f_f (rint, -140737488355328.75, -140737488355329.0),
    TEST_f_f (rint, -281474976710656.75, -281474976710657.0),
    TEST_f_f (rint, -562949953421312.75, -562949953421313.0),
    TEST_f_f (rint, -1125899906842624.75, -1125899906842625.0),
#endif
#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (rint, 4503599627370495.5L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.75L, 4503599627370497.0L),
    TEST_f_f (rint, 4503599627370497.5L, 4503599627370498.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370495.0L),
    TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370497.0L),
# endif
    TEST_f_f (rint, -4503599627370495.5L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.75L, -4503599627370497.0L),
    TEST_f_f (rint, -4503599627370497.5L, -4503599627370498.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370495.0L),
    TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370497.0L),

    TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L),
    TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L),
    TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740993.0L),
    TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740994.0L),

    TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L),
    TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L),
    TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740993.0L),
    TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740994.0L),
# endif
#endif
    END_DATA (rint_tonearest)
  };

static void
rint_test_tonearest (void)
{
  START (rint_tonearest);
  RUN_TEST_LOOP_f_f (rint, rint_tonearest_test_data, FE_TONEAREST);
  END (rint_tonearest);
}

static const struct test_f_f_data rint_towardzero_test_data[] =
  {
    START_DATA (rint_towardzero),
    TEST_f_f (rint, 2.0, 2.0),
    TEST_f_f (rint, 1.5, 1.0),
    TEST_f_f (rint, 1.0, 1.0),
    TEST_f_f (rint, 0.5, 0.0),
    TEST_f_f (rint, 0.0, 0.0),
    TEST_f_f (rint, minus_zero, minus_zero),
    TEST_f_f (rint, -0.5, -0.0),
    TEST_f_f (rint, -1.0, -1.0),
    TEST_f_f (rint, -1.5, -1.0),
    TEST_f_f (rint, -2.0, -2.0),
    TEST_f_f (rint, 0.1, 0.0),
    TEST_f_f (rint, 0.25, 0.0),
    TEST_f_f (rint, 0.625, 0.0),
    TEST_f_f (rint, -0.1, -0.0),
    TEST_f_f (rint, -0.25, -0.0),
    TEST_f_f (rint, -0.625, -0.0),
    TEST_f_f (rint, 1048576.75, 1048576.0),
    TEST_f_f (rint, 2097152.75, 2097152.0),
    TEST_f_f (rint, -1048576.75, -1048576.0),
    TEST_f_f (rint, -2097152.75, -2097152.0),
#ifndef TEST_FLOAT
    TEST_f_f (rint, 70368744177664.75, 70368744177664.0),
    TEST_f_f (rint, 140737488355328.75, 140737488355328.0),
    TEST_f_f (rint, 281474976710656.75, 281474976710656.0),
    TEST_f_f (rint, 562949953421312.75, 562949953421312.0),
    TEST_f_f (rint, 1125899906842624.75, 1125899906842624.0),
    TEST_f_f (rint, -70368744177664.75, -70368744177664.0),
    TEST_f_f (rint, -140737488355328.75, -140737488355328.0),
    TEST_f_f (rint, -281474976710656.75, -281474976710656.0),
    TEST_f_f (rint, -562949953421312.75, -562949953421312.0),
    TEST_f_f (rint, -1125899906842624.75, -1125899906842624.0),
#endif
#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (rint, 4503599627370495.5L, 4503599627370495.0L),
    TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.75L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370497.5L, 4503599627370497.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370494.0L),
    TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370495.0L),
    TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370496.0L),
# endif
    TEST_f_f (rint, -4503599627370495.5L, -4503599627370495.0L),
    TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.75L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370497.5L, -4503599627370497.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370494.0L),
    TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370495.0L),
    TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370496.0L),

    TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L),
    TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L),
    TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740991.0L),
    TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740993.0L),

    TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L),
    TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L),
    TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740991.0L),
    TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740993.0L),
# endif
#endif
    END_DATA (rint_towardzero)
  };

static void
rint_test_towardzero (void)
{
  START (rint_towardzero);
  RUN_TEST_LOOP_f_f (rint, rint_towardzero_test_data, FE_TOWARDZERO);
  END (rint_towardzero);
}

static const struct test_f_f_data rint_downward_test_data[] =
  {
    START_DATA (rint_downward),
    TEST_f_f (rint, 2.0, 2.0),
    TEST_f_f (rint, 1.5, 1.0),
    TEST_f_f (rint, 1.0, 1.0),
    TEST_f_f (rint, 0.5, 0.0),
    TEST_f_f (rint, 0.0, 0.0),
    TEST_f_f (rint, minus_zero, minus_zero),
    TEST_f_f (rint, -0.5, -1.0),
    TEST_f_f (rint, -1.0, -1.0),
    TEST_f_f (rint, -1.5, -2.0),
    TEST_f_f (rint, -2.0, -2.0),
    TEST_f_f (rint, 0.1, 0.0),
    TEST_f_f (rint, 0.25, 0.0),
    TEST_f_f (rint, 0.625, 0.0),
    TEST_f_f (rint, -0.1, -1.0),
    TEST_f_f (rint, -0.25, -1.0),
    TEST_f_f (rint, -0.625, -1.0),
    TEST_f_f (rint, 1048576.75, 1048576.0),
    TEST_f_f (rint, 2097152.75, 2097152.0),
    TEST_f_f (rint, -1048576.75, -1048577.0),
    TEST_f_f (rint, -2097152.75, -2097153.0),
#ifndef TEST_FLOAT
    TEST_f_f (rint, 70368744177664.75, 70368744177664.0),
    TEST_f_f (rint, 140737488355328.75, 140737488355328.0),
    TEST_f_f (rint, 281474976710656.75, 281474976710656.0),
    TEST_f_f (rint, 562949953421312.75, 562949953421312.0),
    TEST_f_f (rint, 1125899906842624.75, 1125899906842624.0),
    TEST_f_f (rint, -70368744177664.75, -70368744177665.0),
    TEST_f_f (rint, -140737488355328.75, -140737488355329.0),
    TEST_f_f (rint, -281474976710656.75, -281474976710657.0),
    TEST_f_f (rint, -562949953421312.75, -562949953421313.0),
    TEST_f_f (rint, -1125899906842624.75, -1125899906842625.0),
#endif
#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (rint, 4503599627370495.5L, 4503599627370495.0L),
    TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.75L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370497.5L, 4503599627370497.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370494.0L),
    TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370495.0L),
    TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370496.0L),
# endif
    TEST_f_f (rint, -4503599627370495.5L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.25L, -4503599627370497.0L),
    TEST_f_f (rint, -4503599627370496.5L, -4503599627370497.0L),
    TEST_f_f (rint, -4503599627370496.75L, -4503599627370497.0L),
    TEST_f_f (rint, -4503599627370497.5L, -4503599627370498.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370495.0L),
    TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370497.0L),

    TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L),
    TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L),
    TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740991.0L),
    TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740993.0L),

    TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740993.0L),
    TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740994.0L),
    TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740993.0L),
    TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740994.0L),
# endif
#endif
    END_DATA (rint_downward)
  };

static void
rint_test_downward (void)
{
  START (rint_downward);
  RUN_TEST_LOOP_f_f (rint, rint_downward_test_data, FE_DOWNWARD);
  END (rint_downward);
}

static const struct test_f_f_data rint_upward_test_data[] =
  {
    START_DATA (rint_upward),
    TEST_f_f (rint, 2.0, 2.0),
    TEST_f_f (rint, 1.5, 2.0),
    TEST_f_f (rint, 1.0, 1.0),
    TEST_f_f (rint, 0.5, 1.0),
    TEST_f_f (rint, 0.0, 0.0),
    TEST_f_f (rint, minus_zero, minus_zero),
    TEST_f_f (rint, -0.5, -0.0),
    TEST_f_f (rint, -1.0, -1.0),
    TEST_f_f (rint, -1.5, -1.0),
    TEST_f_f (rint, -2.0, -2.0),
    TEST_f_f (rint, 0.1, 1.0),
    TEST_f_f (rint, 0.25, 1.0),
    TEST_f_f (rint, 0.625, 1.0),
    TEST_f_f (rint, -0.1, -0.0),
    TEST_f_f (rint, -0.25, -0.0),
    TEST_f_f (rint, -0.625, -0.0),
    TEST_f_f (rint, 1048576.75, 1048577.0),
    TEST_f_f (rint, 2097152.75, 2097153.0),
    TEST_f_f (rint, -1048576.75, -1048576.0),
    TEST_f_f (rint, -2097152.75, -2097152.0),
#ifndef TEST_FLOAT
    TEST_f_f (rint, 70368744177664.75, 70368744177665.0),
    TEST_f_f (rint, 140737488355328.75, 140737488355329.0),
    TEST_f_f (rint, 281474976710656.75, 281474976710657.0),
    TEST_f_f (rint, 562949953421312.75, 562949953421313.0),
    TEST_f_f (rint, 1125899906842624.75, 1125899906842625.0),
    TEST_f_f (rint, -70368744177664.75, -70368744177664.0),
    TEST_f_f (rint, -140737488355328.75, -140737488355328.0),
    TEST_f_f (rint, -281474976710656.75, -281474976710656.0),
    TEST_f_f (rint, -562949953421312.75, -562949953421312.0),
    TEST_f_f (rint, -1125899906842624.75, -1125899906842624.0),
#endif
#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (rint, 4503599627370495.5L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.25L, 4503599627370497.0L),
    TEST_f_f (rint, 4503599627370496.5L, 4503599627370497.0L),
    TEST_f_f (rint, 4503599627370496.75L, 4503599627370497.0L),
    TEST_f_f (rint, 4503599627370497.5L, 4503599627370498.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370495.0L),
    TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370496.0L),
    TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370497.0L),
# endif
    TEST_f_f (rint, -4503599627370495.5L, -4503599627370495.0L),
    TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370496.75L, -4503599627370496.0L),
    TEST_f_f (rint, -4503599627370497.5L, -4503599627370497.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370494.0L),
    TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370495.0L),
    TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370496.0L),

    TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740993.0L),
    TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740994.0L),
    TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740992.0L),
    TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740993.0L),
    TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740994.0L),

    TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L),
    TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L),
    TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740991.0L),
    TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740992.0L),
    TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740993.0L),
# endif
#endif
    END_DATA (rint_upward)
  };

static void
rint_test_upward (void)
{
  START (rint_upward);
  RUN_TEST_LOOP_f_f (rint, rint_upward_test_data, FE_UPWARD);
  END (rint_upward);
}

static const struct test_f_f_data round_test_data[] =
  {
    START_DATA (round),
    /* TODO: missing +/-Inf as well as qNaN tests.  */

    TEST_f_f (round, 0, 0),
    TEST_f_f (round, minus_zero, minus_zero),
    TEST_f_f (round, 0.2L, 0.0),
    TEST_f_f (round, -0.2L, minus_zero),
    TEST_f_f (round, 0.5, 1.0),
    TEST_f_f (round, -0.5, -1.0),
    TEST_f_f (round, 0.8L, 1.0),
    TEST_f_f (round, -0.8L, -1.0),
    TEST_f_f (round, 1.5, 2.0),
    TEST_f_f (round, -1.5, -2.0),
    TEST_f_f (round, 0.1, 0.0),
    TEST_f_f (round, 0.25, 0.0),
    TEST_f_f (round, 0.625, 1.0),
    TEST_f_f (round, -0.1, -0.0),
    TEST_f_f (round, -0.25, -0.0),
    TEST_f_f (round, -0.625, -1.0),
    TEST_f_f (round, 2097152.5, 2097153),
    TEST_f_f (round, -2097152.5, -2097153),

#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (round, 4503599627370495.5L, 4503599627370496.0L),
    TEST_f_f (round, 4503599627370496.25L, 4503599627370496.0L),
    TEST_f_f (round, 4503599627370496.5L, 4503599627370497.0L),
    TEST_f_f (round, 4503599627370496.75L, 4503599627370497.0L),
    TEST_f_f (round, 4503599627370497.5L, 4503599627370498.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (round, 4503599627370494.5000000000001L, 4503599627370495.0L),
    TEST_f_f (round, 4503599627370495.5000000000001L, 4503599627370496.0L),
    TEST_f_f (round, 4503599627370496.5000000000001L, 4503599627370497.0L),
# endif

    TEST_f_f (round, -4503599627370495.5L, -4503599627370496.0L),
    TEST_f_f (round, -4503599627370496.25L, -4503599627370496.0L),
    TEST_f_f (round, -4503599627370496.5L, -4503599627370497.0L),
    TEST_f_f (round, -4503599627370496.75L, -4503599627370497.0L),
    TEST_f_f (round, -4503599627370497.5L, -4503599627370498.0L),
# if LDBL_MANT_DIG > 100
    TEST_f_f (round, -4503599627370494.5000000000001L, -4503599627370495.0L),
    TEST_f_f (round, -4503599627370495.5000000000001L, -4503599627370496.0L),
    TEST_f_f (round, -4503599627370496.5000000000001L, -4503599627370497.0L),
# endif

    TEST_f_f (round, 9007199254740991.5L, 9007199254740992.0L),
    TEST_f_f (round, 9007199254740992.25L, 9007199254740992.0L),
    TEST_f_f (round, 9007199254740992.5L, 9007199254740993.0L),
    TEST_f_f (round, 9007199254740992.75L, 9007199254740993.0L),
    TEST_f_f (round, 9007199254740993.5L, 9007199254740994.0L),

    TEST_f_f (round, -9007199254740991.5L, -9007199254740992.0L),
    TEST_f_f (round, -9007199254740992.25L, -9007199254740992.0L),
    TEST_f_f (round, -9007199254740992.5L, -9007199254740993.0L),
    TEST_f_f (round, -9007199254740992.75L, -9007199254740993.0L),
    TEST_f_f (round, -9007199254740993.5L, -9007199254740994.0L),

# if LDBL_MANT_DIG > 100
    TEST_f_f (round, 9007199254740991.0000000000001L, 9007199254740991.0L),
    TEST_f_f (round, 9007199254740992.0000000000001L, 9007199254740992.0L),
    TEST_f_f (round, 9007199254740993.0000000000001L, 9007199254740993.0L),
    TEST_f_f (round, 9007199254740991.5000000000001L, 9007199254740992.0L),
    TEST_f_f (round, 9007199254740992.5000000000001L, 9007199254740993.0L),
    TEST_f_f (round, 9007199254740993.5000000000001L, 9007199254740994.0L),

    TEST_f_f (round, -9007199254740991.0000000000001L, -9007199254740991.0L),
    TEST_f_f (round, -9007199254740992.0000000000001L, -9007199254740992.0L),
    TEST_f_f (round, -9007199254740993.0000000000001L, -9007199254740993.0L),
    TEST_f_f (round, -9007199254740991.5000000000001L, -9007199254740992.0L),
    TEST_f_f (round, -9007199254740992.5000000000001L, -9007199254740993.0L),
    TEST_f_f (round, -9007199254740993.5000000000001L, -9007199254740994.0L),
# endif

    TEST_f_f (round, 72057594037927935.5L, 72057594037927936.0L),
    TEST_f_f (round, 72057594037927936.25L, 72057594037927936.0L),
    TEST_f_f (round, 72057594037927936.5L, 72057594037927937.0L),
    TEST_f_f (round, 72057594037927936.75L, 72057594037927937.0L),
    TEST_f_f (round, 72057594037927937.5L, 72057594037927938.0L),

    TEST_f_f (round, -72057594037927935.5L, -72057594037927936.0L),
    TEST_f_f (round, -72057594037927936.25L, -72057594037927936.0L),
    TEST_f_f (round, -72057594037927936.5L, -72057594037927937.0L),
    TEST_f_f (round, -72057594037927936.75L, -72057594037927937.0L),
    TEST_f_f (round, -72057594037927937.5L, -72057594037927938.0L),

    TEST_f_f (round, 10141204801825835211973625643007.5L, 10141204801825835211973625643008.0L),
    TEST_f_f (round, 10141204801825835211973625643008.25L, 10141204801825835211973625643008.0L),
    TEST_f_f (round, 10141204801825835211973625643008.5L, 10141204801825835211973625643009.0L),
    TEST_f_f (round, 10141204801825835211973625643008.75L, 10141204801825835211973625643009.0L),
    TEST_f_f (round, 10141204801825835211973625643009.5L, 10141204801825835211973625643010.0L),
#endif
    END_DATA (round)
  };

static void
round_test (void)
{
  START (round);
  RUN_TEST_LOOP_f_f (round, round_test_data, );
  END (round);
}


static const struct test_ff_f_data scalb_test_data[] =
  {
    START_DATA (scalb),
    TEST_ff_f (scalb, 2.0, 0.5, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (scalb, 3.0, -2.5, qnan_value, INVALID_EXCEPTION),

    TEST_ff_f (scalb, 0, qnan_value, qnan_value),
    TEST_ff_f (scalb, 1, qnan_value, qnan_value),

    TEST_ff_f (scalb, 1, 0, 1),
    TEST_ff_f (scalb, -1, 0, -1),

    TEST_ff_f (scalb, 0, plus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (scalb, minus_zero, plus_infty, qnan_value, INVALID_EXCEPTION),

    TEST_ff_f (scalb, 0, 2, 0),
    TEST_ff_f (scalb, minus_zero, -4, minus_zero),
    TEST_ff_f (scalb, 0, 0, 0),
    TEST_ff_f (scalb, minus_zero, 0, minus_zero),
    TEST_ff_f (scalb, 0, -1, 0),
    TEST_ff_f (scalb, minus_zero, -10, minus_zero),
    TEST_ff_f (scalb, 0, minus_infty, 0),
    TEST_ff_f (scalb, minus_zero, minus_infty, minus_zero),

    TEST_ff_f (scalb, plus_infty, -1, plus_infty),
    TEST_ff_f (scalb, minus_infty, -10, minus_infty),
    TEST_ff_f (scalb, plus_infty, 0, plus_infty),
    TEST_ff_f (scalb, minus_infty, 0, minus_infty),
    TEST_ff_f (scalb, plus_infty, 2, plus_infty),
    TEST_ff_f (scalb, minus_infty, 100, minus_infty),

    TEST_ff_f (scalb, 0.1L, minus_infty, 0.0),
    TEST_ff_f (scalb, -0.1L, minus_infty, minus_zero),

    TEST_ff_f (scalb, 1, plus_infty, plus_infty),
    TEST_ff_f (scalb, -1, plus_infty, minus_infty),
    TEST_ff_f (scalb, plus_infty, plus_infty, plus_infty),
    TEST_ff_f (scalb, minus_infty, plus_infty, minus_infty),

    TEST_ff_f (scalb, plus_infty, minus_infty, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (scalb, minus_infty, minus_infty, qnan_value, INVALID_EXCEPTION),

    TEST_ff_f (scalb, qnan_value, 1, qnan_value),
    TEST_ff_f (scalb, 1, qnan_value, qnan_value),
    TEST_ff_f (scalb, qnan_value, 0, qnan_value),
    TEST_ff_f (scalb, 0, qnan_value, qnan_value),
    TEST_ff_f (scalb, qnan_value, plus_infty, qnan_value),
    TEST_ff_f (scalb, plus_infty, qnan_value, qnan_value),
    TEST_ff_f (scalb, qnan_value, qnan_value, qnan_value),

    TEST_ff_f (scalb, 0.8L, 4, 12.8L),
    TEST_ff_f (scalb, -0.854375L, 5, -27.34L),
    END_DATA (scalb)
  };

static void
scalb_test (void)
{

  START (scalb);
  RUN_TEST_LOOP_ff_f (scalb, scalb_test_data, );
  END (scalb);
}


static const struct test_fi_f_data scalbn_test_data[] =
  {
    START_DATA (scalbn),
    TEST_fi_f (scalbn, 0, 0, 0),
    TEST_fi_f (scalbn, minus_zero, 0, minus_zero),

    TEST_fi_f (scalbn, plus_infty, 1, plus_infty),
    TEST_fi_f (scalbn, minus_infty, 1, minus_infty),
    TEST_fi_f (scalbn, qnan_value, 1, qnan_value),

    TEST_fi_f (scalbn, 0.8L, 4, 12.8L),
    TEST_fi_f (scalbn, -0.854375L, 5, -27.34L),

    TEST_fi_f (scalbn, 1, 0L, 1),

    TEST_fi_f (scalbn, 1, INT_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, 1, INT_MIN, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, max_value, INT_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, max_value, INT_MIN, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, min_value, INT_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, min_value, INT_MIN, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, min_value / 4, INT_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, min_value / 4, INT_MIN, plus_zero, UNDERFLOW_EXCEPTION),
    END_DATA (scalbn)
  };

static void
scalbn_test (void)
{

  START (scalbn);
  RUN_TEST_LOOP_fi_f (scalbn, scalbn_test_data, );
  END (scalbn);
}


static const struct test_fl_f_data scalbln_test_data[] =
  {
    START_DATA (scalbln),
    TEST_fl_f (scalbln, 0, 0, 0),
    TEST_fl_f (scalbln, minus_zero, 0, minus_zero),

    TEST_fl_f (scalbln, plus_infty, 1, plus_infty),
    TEST_fl_f (scalbln, minus_infty, 1, minus_infty),
    TEST_fl_f (scalbln, qnan_value, 1, qnan_value),

    TEST_fl_f (scalbln, 0.8L, 4, 12.8L),
    TEST_fl_f (scalbln, -0.854375L, 5, -27.34L),

    TEST_fl_f (scalbln, 1, 0L, 1),

    TEST_fl_f (scalbln, 1, INT_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, 1, INT_MIN, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, INT_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, INT_MIN, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, INT_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, INT_MIN, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, INT_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, INT_MIN, plus_zero, UNDERFLOW_EXCEPTION),

    TEST_fl_f (scalbln, 1, LONG_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, 1, LONG_MIN, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, LONG_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, LONG_MIN, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, LONG_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, LONG_MIN, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, LONG_MAX, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, LONG_MIN, plus_zero, UNDERFLOW_EXCEPTION),

#if LONG_MAX >= 0x100000000
    TEST_fl_f (scalbln, 1, 0x88000000L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, 1, -0x88000000L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, 0x88000000L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, -0x88000000L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, 0x88000000L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, -0x88000000L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, 0x88000000L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, -0x88000000L, plus_zero, UNDERFLOW_EXCEPTION),
#endif
    END_DATA (scalbln)
  };

static void
scalbln_test (void)
{

  START (scalbln);
  RUN_TEST_LOOP_fl_f (scalbln, scalbln_test_data, );
  END (scalbln);
}


static void
signbit_test (void)
{
  /* TODO: missing qNaN tests.  */

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


static const struct test_f_f_data sin_test_data[] =
  {
    START_DATA (sin),
    TEST_f_f (sin, 0, 0),
    TEST_f_f (sin, minus_zero, minus_zero),
    TEST_f_f (sin, plus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sin, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sin, qnan_value, qnan_value, ERRNO_UNCHANGED),

    TEST_f_f (sin, M_PI_6l, 0.5),
    TEST_f_f (sin, -M_PI_6l, -0.5),
    TEST_f_f (sin, M_PI_2l, 1),
    TEST_f_f (sin, -M_PI_2l, -1),
    TEST_f_f (sin, 0.75L, 0.681638760023334166733241952779893935L),

    TEST_f_f (sin, 0x1p65, -0.047183876212354673805106149805700013943218L),
    TEST_f_f (sin, -0x1p65, 0.047183876212354673805106149805700013943218L),

    TEST_f_f (sin, 0x1.7f4134p+103, -6.6703229329788657073304190650534846045235e-08L),

#ifdef TEST_DOUBLE
    TEST_f_f (sin, 0.80190127184058835, 0.71867942238767868),
    TEST_f_f (sin, 2.522464e-1, 2.4957989804940911e-1),
#endif

#ifndef TEST_FLOAT
    TEST_f_f (sin, 1e22, -0.8522008497671888017727058937530293682618L),
    TEST_f_f (sin, 0x1p1023, 0.5631277798508840134529434079444683477104L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_f_f (sin, 0x1p16383L, 0.3893629985894208126948115852610595405563L),
#endif

    TEST_f_f (sin, 0x1p+120, 3.77820109360752022655548470056922991960587e-01L),
    TEST_f_f (sin, 0x1p+127, 6.23385512955870240370428801097126489001833e-01L),
    TEST_f_f (sin, 0x1.fffff8p+127, 4.85786063130487339701113680434728152037092e-02L),
    TEST_f_f (sin, 0x1.fffffep+127, -5.21876523333658540551505357019806722935726e-01L),
    TEST_f_f (sin, 0x1p+50, 4.96396515208940840876821859865411368093356e-01L),
    TEST_f_f (sin, 0x1p+28, -9.86198211836975655703110310527108292055548e-01L),
    END_DATA (sin)
  };

static void
sin_test (void)
{
  errno = 0;
  FUNC(sin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sin);
  RUN_TEST_LOOP_f_f (sin, sin_test_data, );
  END (sin);
}


static const struct test_f_f_data sin_tonearest_test_data[] =
  {
    START_DATA (sin_tonearest),
    TEST_f_f (sin, 1, 0.8414709848078965066525023216302989996226L),
    TEST_f_f (sin, 2, 0.9092974268256816953960198659117448427023L),
    TEST_f_f (sin, 3, 0.1411200080598672221007448028081102798469L),
    TEST_f_f (sin, 4, -0.7568024953079282513726390945118290941359L),
    TEST_f_f (sin, 5, -0.9589242746631384688931544061559939733525L),
    TEST_f_f (sin, 6, -0.2794154981989258728115554466118947596280L),
    TEST_f_f (sin, 7, 0.6569865987187890903969990915936351779369L),
    TEST_f_f (sin, 8, 0.9893582466233817778081235982452886721164L),
    TEST_f_f (sin, 9, 0.4121184852417565697562725663524351793439L),
    TEST_f_f (sin, 10, -0.5440211108893698134047476618513772816836L),
    END_DATA (sin_tonearest)
  };

static void
sin_test_tonearest (void)
{
  errno = 0;
  FUNC(sin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sin_tonearest);
  RUN_TEST_LOOP_f_f (sin, sin_tonearest_test_data, FE_TONEAREST);
  END (sin_tonearest);
}


static const struct test_f_f_data sin_towardzero_test_data[] =
  {
    START_DATA (sin_towardzero),
    TEST_f_f (sin, 1, 0.8414709848078965066525023216302989996226L),
    TEST_f_f (sin, 2, 0.9092974268256816953960198659117448427023L),
    TEST_f_f (sin, 3, 0.1411200080598672221007448028081102798469L),
    TEST_f_f (sin, 4, -0.7568024953079282513726390945118290941359L),
    TEST_f_f (sin, 5, -0.9589242746631384688931544061559939733525L),
    TEST_f_f (sin, 6, -0.2794154981989258728115554466118947596280L),
    TEST_f_f (sin, 7, 0.6569865987187890903969990915936351779369L),
    TEST_f_f (sin, 8, 0.9893582466233817778081235982452886721164L),
    TEST_f_f (sin, 9, 0.4121184852417565697562725663524351793439L),
    TEST_f_f (sin, 10, -0.5440211108893698134047476618513772816836L),
    END_DATA (sin_towardzero)
  };

static void
sin_test_towardzero (void)
{
  errno = 0;
  FUNC(sin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sin_towardzero);
  RUN_TEST_LOOP_f_f (sin, sin_towardzero_test_data, FE_TOWARDZERO);
  END (sin_towardzero);
}


static const struct test_f_f_data sin_downward_test_data[] =
  {
    START_DATA (sin_downward),
    TEST_f_f (sin, 1, 0.8414709848078965066525023216302989996226L),
    TEST_f_f (sin, 2, 0.9092974268256816953960198659117448427023L),
    TEST_f_f (sin, 3, 0.1411200080598672221007448028081102798469L),
    TEST_f_f (sin, 4, -0.7568024953079282513726390945118290941359L),
    TEST_f_f (sin, 5, -0.9589242746631384688931544061559939733525L),
    TEST_f_f (sin, 6, -0.2794154981989258728115554466118947596280L),
    TEST_f_f (sin, 7, 0.6569865987187890903969990915936351779369L),
    TEST_f_f (sin, 8, 0.9893582466233817778081235982452886721164L),
    TEST_f_f (sin, 9, 0.4121184852417565697562725663524351793439L),
    TEST_f_f (sin, 10, -0.5440211108893698134047476618513772816836L),
    END_DATA (sin_downward)
  };

static void
sin_test_downward (void)
{
  errno = 0;
  FUNC(sin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sin_downward);
  RUN_TEST_LOOP_f_f (sin, sin_downward_test_data, FE_DOWNWARD);
  END (sin_downward);
}


static const struct test_f_f_data sin_upward_test_data[] =
  {
    START_DATA (sin_upward),
    TEST_f_f (sin, 1, 0.8414709848078965066525023216302989996226L),
    TEST_f_f (sin, 2, 0.9092974268256816953960198659117448427023L),
    TEST_f_f (sin, 3, 0.1411200080598672221007448028081102798469L),
    TEST_f_f (sin, 4, -0.7568024953079282513726390945118290941359L),
    TEST_f_f (sin, 5, -0.9589242746631384688931544061559939733525L),
    TEST_f_f (sin, 6, -0.2794154981989258728115554466118947596280L),
    TEST_f_f (sin, 7, 0.6569865987187890903969990915936351779369L),
    TEST_f_f (sin, 8, 0.9893582466233817778081235982452886721164L),
    TEST_f_f (sin, 9, 0.4121184852417565697562725663524351793439L),
    TEST_f_f (sin, 10, -0.5440211108893698134047476618513772816836L),
    END_DATA (sin_upward)
  };

static void
sin_test_upward (void)
{
  errno = 0;
  FUNC(sin) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sin_upward);
  RUN_TEST_LOOP_f_f (sin, sin_upward_test_data, FE_UPWARD);
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
  TEST_extra (sincos, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION);
  TEST_extra (sincos, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION);
  TEST_extra (sincos, qnan_value, qnan_value, qnan_value);

  /* The value of M_PI_2l is never exactly PI/2, and therefore the
     answer is never exactly zero. The answer is equal to the error
     in rounding PI/2 for the type used.  Thus the answer is unique
     to each type.  */
#ifdef TEST_FLOAT
  /* 32-bit float.  */
  TEST_extra (sincos, M_PI_2l, 1, -0x1.777a5cp-25L);
#endif
#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MANT_DIG == 53)
  /* 64-bit double or 64-bit long double.  */
  TEST_extra (sincos, M_PI_2l, 1, 0x1.1a62633145c07p-54L);
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG == 64
  /* 96-bit long double.  */
  TEST_extra (sincos, M_PI_2l, 1, -0xe.ce675d1fc8f8cbbp-69L);
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG == 106
  /* 128-bit IBM long double.  */
  TEST_extra (sincos, M_PI_2l, 1, 0x1.c1cd129024e088a67cc74020bcp-107L);
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG == 113
  /* 128-bit long double.  */
  TEST_extra (sincos, M_PI_2l, 1, 0x1.cd129024e088a67cc74020bbea64p-115L);
#endif

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

  TEST_extra (sincos, 0x1p+120, 3.77820109360752022655548470056922991960587e-01L, -9.25879022854837867303861764107414946730833e-01L);
  TEST_extra (sincos, 0x1p+127, 6.23385512955870240370428801097126489001833e-01L, 7.81914638714960072263910298466369236613162e-01L);
  TEST_extra (sincos, 0x1.fffff8p+127, 4.85786063130487339701113680434728152037092e-02L, 9.98819362551949040703862043664101081064641e-01L);
  TEST_extra (sincos, 0x1.fffffep+127, -5.21876523333658540551505357019806722935726e-01L, 8.53021039830304158051791467692161107353094e-01L);
  TEST_extra (sincos, 0x1p+50, 4.96396515208940840876821859865411368093356e-01L, 8.68095904660550604334592502063501320395739e-01L);
  TEST_extra (sincos, 0x1p+28, -9.86198211836975655703110310527108292055548e-01L, -1.65568979490578758865468278195361551113358e-01L);

  END (sincos);
}

static const struct test_f_f_data sinh_test_data[] =
  {
    START_DATA (sinh),
    TEST_f_f (sinh, 0, 0),
    TEST_f_f (sinh, minus_zero, minus_zero),

#ifndef TEST_INLINE
    TEST_f_f (sinh, plus_infty, plus_infty),
    TEST_f_f (sinh, minus_infty, minus_infty),
#endif
    TEST_f_f (sinh, qnan_value, qnan_value),

    TEST_f_f (sinh, 0.75L, 0.822316731935829980703661634446913849L),
    TEST_f_f (sinh, 0x8p-32L, 1.86264514923095703232705808926175479e-9L),
    END_DATA (sinh)
  };

static void
sinh_test (void)
{
  errno = 0;
  FUNC(sinh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sinh);
  RUN_TEST_LOOP_f_f (sinh, sinh_test_data, );
  END (sinh);
}


static const struct test_f_f_data sinh_tonearest_test_data[] =
  {
    START_DATA (sinh_tonearest),
    TEST_f_f (sinh, 22, 1792456423.065795780701106568345764104225L),
    TEST_f_f (sinh, 23, 4872401723.124451299966006944252978187305L),
    TEST_f_f (sinh, 24, 13244561064.92173614705070540368454568168L),
    END_DATA (sinh_tonearest)
  };

static void
sinh_test_tonearest (void)
{
  errno = 0;
  FUNC(sinh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sinh_tonearest);
  RUN_TEST_LOOP_f_f (sinh, sinh_tonearest_test_data, FE_TONEAREST);
  END (sinh_tonearest);
}


static const struct test_f_f_data sinh_towardzero_test_data[] =
  {
    START_DATA (sinh_towardzero),
    TEST_f_f (sinh, 22, 1792456423.065795780701106568345764104225L),
    TEST_f_f (sinh, 23, 4872401723.124451299966006944252978187305L),
    TEST_f_f (sinh, 24, 13244561064.92173614705070540368454568168L),
    END_DATA (sinh_towardzero)
  };

static void
sinh_test_towardzero (void)
{
  errno = 0;
  FUNC(sinh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sinh_towardzero);
  RUN_TEST_LOOP_f_f (sinh, sinh_towardzero_test_data, FE_TOWARDZERO);
  END (sinh_towardzero);
}


static const struct test_f_f_data sinh_downward_test_data[] =
  {
    START_DATA (sinh_downward),
    TEST_f_f (sinh, 22, 1792456423.065795780701106568345764104225L),
    TEST_f_f (sinh, 23, 4872401723.124451299966006944252978187305L),
    TEST_f_f (sinh, 24, 13244561064.92173614705070540368454568168L),
    END_DATA (sinh_downward)
  };

static void
sinh_test_downward (void)
{
  errno = 0;
  FUNC(sinh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sinh_downward);
  RUN_TEST_LOOP_f_f (sinh, sinh_downward_test_data, FE_DOWNWARD);
  END (sinh_downward);
}


static const struct test_f_f_data sinh_upward_test_data[] =
  {
    START_DATA (sinh_upward),
    TEST_f_f (sinh, 22, 1792456423.065795780701106568345764104225L),
    TEST_f_f (sinh, 23, 4872401723.124451299966006944252978187305L),
    TEST_f_f (sinh, 24, 13244561064.92173614705070540368454568168L),
    END_DATA (sinh_upward)
  };

static void
sinh_test_upward (void)
{
  errno = 0;
  FUNC(sinh) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sinh_upward);
  RUN_TEST_LOOP_f_f (sinh, sinh_upward_test_data, FE_UPWARD);
  END (sinh_upward);
}


static const struct test_f_f_data sqrt_test_data[] =
  {
    START_DATA (sqrt),
    TEST_f_f (sqrt, 0, 0),
    TEST_f_f (sqrt, qnan_value, qnan_value),
    TEST_f_f (sqrt, plus_infty, plus_infty),

    TEST_f_f (sqrt, minus_zero, minus_zero),

    /* sqrt (x) == qNaN plus invalid exception for x < 0.  */
    TEST_f_f (sqrt, -1, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (sqrt, -max_value, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (sqrt, minus_infty, qnan_value, INVALID_EXCEPTION),

    TEST_f_f (sqrt, 2209, 47),
    TEST_f_f (sqrt, 4, 2),
    TEST_f_f (sqrt, 2, M_SQRT2l),
    TEST_f_f (sqrt, 0.25, 0.5),
    TEST_f_f (sqrt, 6642.25, 81.5),
    TEST_f_f (sqrt, 15190.5625L, 123.25L),
    TEST_f_f (sqrt, 0.75L, 0.866025403784438646763723170752936183L),
    END_DATA (sqrt)
  };

static void
sqrt_test (void)
{
  errno = 0;
  FUNC(sqrt) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (sqrt);
  RUN_TEST_LOOP_f_f (sqrt, sqrt_test_data, );
  END (sqrt);
}


static const struct test_f_f_data tan_test_data[] =
  {
    START_DATA (tan),
    TEST_f_f (tan, 0, 0),
    TEST_f_f (tan, minus_zero, minus_zero),
    TEST_f_f (tan, plus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (tan, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (tan, qnan_value, qnan_value, ERRNO_UNCHANGED),

    TEST_f_f (tan, M_PI_4l, 1),
    TEST_f_f (tan, 0.75L, 0.931596459944072461165202756573936428L),

    TEST_f_f (tan, 0x1p65, -0.0472364872359047946798414219288370688827L),
    TEST_f_f (tan, -0x1p65, 0.0472364872359047946798414219288370688827L),

    TEST_f_f (tan, 0xc.9p-4, 0.9995162902115457818029468900654150261381L),
    TEST_f_f (tan, 0xc.908p-4, 0.9997603425502441410973077452249560802034L),
    TEST_f_f (tan, 0xc.90cp-4, 0.9998823910588060302788513970802357770031L),
    TEST_f_f (tan, 0xc.90ep-4, 0.9999434208994808753305784795924711152508L),
    TEST_f_f (tan, 0xc.90fp-4, 0.9999739372166156702433266059635165160515L),
    TEST_f_f (tan, 0xc.90f8p-4, 0.9999891957244072765118898375645469865764L),
    TEST_f_f (tan, 0xc.90fcp-4, 0.9999968250656122402859679132395522927393L),
    TEST_f_f (tan, 0xc.90fdp-4, 0.9999987324100083358016192309006353329444L),
    TEST_f_f (tan, 0xc.90fd8p-4, 0.9999996860835706212861509874451585282616L),
    TEST_f_f (tan, 0xc.90fdap-4, 0.9999999245021033010474530133665235922808L),
    TEST_f_f (tan, 0xc.ap-4, 1.0073556597407272165371804539701396631519L),
    TEST_f_f (tan, 0xc.98p-4, 1.0034282930863044654045449407466962736255L),
    TEST_f_f (tan, 0xc.94p-4, 1.0014703786820082237342656561856877993328L),
    TEST_f_f (tan, 0xc.92p-4, 1.0004928571392300571266638743539017593717L),
    TEST_f_f (tan, 0xc.91p-4, 1.0000044544650244953647966900221905361131L),
    TEST_f_f (tan, 0xc.90fep-4, 1.0000006397580424009014454926842136804016L),
    TEST_f_f (tan, 0xc.90fdcp-4, 1.0000001629206928242190327320047489394217L),
    TEST_f_f (tan, 0xc.90fdbp-4, 1.0000000437113909572052640953950483705005L),

    TEST_f_f (tan, -0xc.9p-4, -0.9995162902115457818029468900654150261381L),
    TEST_f_f (tan, -0xc.908p-4, -0.9997603425502441410973077452249560802034L),
    TEST_f_f (tan, -0xc.90cp-4, -0.9998823910588060302788513970802357770031L),
    TEST_f_f (tan, -0xc.90ep-4, -0.9999434208994808753305784795924711152508L),
    TEST_f_f (tan, -0xc.90fp-4, -0.9999739372166156702433266059635165160515L),
    TEST_f_f (tan, -0xc.90f8p-4, -0.9999891957244072765118898375645469865764L),
    TEST_f_f (tan, -0xc.90fcp-4, -0.9999968250656122402859679132395522927393L),
    TEST_f_f (tan, -0xc.90fdp-4, -0.9999987324100083358016192309006353329444L),
    TEST_f_f (tan, -0xc.90fd8p-4, -0.9999996860835706212861509874451585282616L),
    TEST_f_f (tan, -0xc.90fdap-4, -0.9999999245021033010474530133665235922808L),
    TEST_f_f (tan, -0xc.ap-4, -1.0073556597407272165371804539701396631519L),
    TEST_f_f (tan, -0xc.98p-4, -1.0034282930863044654045449407466962736255L),
    TEST_f_f (tan, -0xc.94p-4, -1.0014703786820082237342656561856877993328L),
    TEST_f_f (tan, -0xc.92p-4, -1.0004928571392300571266638743539017593717L),
    TEST_f_f (tan, -0xc.91p-4, -1.0000044544650244953647966900221905361131L),
    TEST_f_f (tan, -0xc.90fep-4, -1.0000006397580424009014454926842136804016L),
    TEST_f_f (tan, -0xc.90fdcp-4, -1.0000001629206928242190327320047489394217L),
    TEST_f_f (tan, -0xc.90fdbp-4, -1.0000000437113909572052640953950483705005L),

#ifndef TEST_FLOAT
    TEST_f_f (tan, 1e22, -1.628778225606898878549375936939548513545L),
    TEST_f_f (tan, 0x1p1023, -0.6814476476066215012854144040167365190368L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_f_f (tan, 0x1p16383L, 0.422722393732022337800504160054440141575L),
#endif
    END_DATA (tan)
  };

static void
tan_test (void)
{
  errno = 0;
  FUNC(tan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tan);
  RUN_TEST_LOOP_f_f (tan, tan_test_data, );
  END (tan);
}


static const struct test_f_f_data tan_tonearest_test_data[] =
  {
    START_DATA (tan_tonearest),
    TEST_f_f (tan, 1, 1.5574077246549022305069748074583601730873L),
    TEST_f_f (tan, 2, -2.1850398632615189916433061023136825434320L),
    TEST_f_f (tan, 3, -0.1425465430742778052956354105339134932261L),
    TEST_f_f (tan, 4, 1.1578212823495775831373424182673239231198L),
    TEST_f_f (tan, 5, -3.3805150062465856369827058794473439087096L),
    TEST_f_f (tan, 6, -0.2910061913847491570536995888681755428312L),
    TEST_f_f (tan, 7, 0.8714479827243187364564508896003135663222L),
    TEST_f_f (tan, 8, -6.7997114552203786999252627596086333648814L),
    TEST_f_f (tan, 9, -0.4523156594418098405903708757987855343087L),
    TEST_f_f (tan, 10, 0.6483608274590866712591249330098086768169L),
    END_DATA (tan_tonearest)
  };

static void
tan_test_tonearest (void)
{
  errno = 0;
  FUNC(tan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tan_tonearest);
  RUN_TEST_LOOP_f_f (tan, tan_tonearest_test_data, FE_TONEAREST);
  END (tan_tonearest);
}


static const struct test_f_f_data tan_towardzero_test_data[] =
  {
    START_DATA (tan_towardzero),
    TEST_f_f (tan, 1, 1.5574077246549022305069748074583601730873L),
    TEST_f_f (tan, 2, -2.1850398632615189916433061023136825434320L),
    TEST_f_f (tan, 3, -0.1425465430742778052956354105339134932261L),
    TEST_f_f (tan, 4, 1.1578212823495775831373424182673239231198L),
    TEST_f_f (tan, 5, -3.3805150062465856369827058794473439087096L),
    TEST_f_f (tan, 6, -0.2910061913847491570536995888681755428312L),
    TEST_f_f (tan, 7, 0.8714479827243187364564508896003135663222L),
    TEST_f_f (tan, 8, -6.7997114552203786999252627596086333648814L),
    TEST_f_f (tan, 9, -0.4523156594418098405903708757987855343087L),
    TEST_f_f (tan, 10, 0.6483608274590866712591249330098086768169L),
    END_DATA (tan_towardzero)
  };

static void
tan_test_towardzero (void)
{
  errno = 0;
  FUNC(tan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tan_towardzero);
  RUN_TEST_LOOP_f_f (tan, tan_towardzero_test_data, FE_TOWARDZERO);
  END (tan_towardzero);
}


static const struct test_f_f_data tan_downward_test_data[] =
  {
    START_DATA (tan_downward),
    TEST_f_f (tan, 1, 1.5574077246549022305069748074583601730873L),
    TEST_f_f (tan, 2, -2.1850398632615189916433061023136825434320L),
    TEST_f_f (tan, 3, -0.1425465430742778052956354105339134932261L),
    TEST_f_f (tan, 4, 1.1578212823495775831373424182673239231198L),
    TEST_f_f (tan, 5, -3.3805150062465856369827058794473439087096L),
    TEST_f_f (tan, 6, -0.2910061913847491570536995888681755428312L),
    TEST_f_f (tan, 7, 0.8714479827243187364564508896003135663222L),
    TEST_f_f (tan, 8, -6.7997114552203786999252627596086333648814L),
    TEST_f_f (tan, 9, -0.4523156594418098405903708757987855343087L),
    TEST_f_f (tan, 10, 0.6483608274590866712591249330098086768169L),
    END_DATA (tan_downward)
  };

static void
tan_test_downward (void)
{
  errno = 0;
  FUNC(tan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tan_downward);
  RUN_TEST_LOOP_f_f (tan, tan_downward_test_data, FE_DOWNWARD);
  END (tan_downward);
}


static const struct test_f_f_data tan_upward_test_data[] =
  {
    START_DATA (tan_upward),
    TEST_f_f (tan, 1, 1.5574077246549022305069748074583601730873L),
    TEST_f_f (tan, 2, -2.1850398632615189916433061023136825434320L),
    TEST_f_f (tan, 3, -0.1425465430742778052956354105339134932261L),
    TEST_f_f (tan, 4, 1.1578212823495775831373424182673239231198L),
    TEST_f_f (tan, 5, -3.3805150062465856369827058794473439087096L),
    TEST_f_f (tan, 6, -0.2910061913847491570536995888681755428312L),
    TEST_f_f (tan, 7, 0.8714479827243187364564508896003135663222L),
    TEST_f_f (tan, 8, -6.7997114552203786999252627596086333648814L),
    TEST_f_f (tan, 9, -0.4523156594418098405903708757987855343087L),
    TEST_f_f (tan, 10, 0.6483608274590866712591249330098086768169L),
    END_DATA (tan_upward)
  };

static void
tan_test_upward (void)
{
  errno = 0;
  FUNC(tan) (0);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tan_upward);
  RUN_TEST_LOOP_f_f (tan, tan_upward_test_data, FE_UPWARD);
  END (tan_upward);
}


static const struct test_f_f_data tanh_test_data[] =
  {
    START_DATA (tanh),
    TEST_f_f (tanh, 0, 0),
    TEST_f_f (tanh, minus_zero, minus_zero),

#ifndef TEST_INLINE
    TEST_f_f (tanh, plus_infty, 1),
    TEST_f_f (tanh, minus_infty, -1),
#endif
    TEST_f_f (tanh, qnan_value, qnan_value),

    TEST_f_f (tanh, 0.75L, 0.635148952387287319214434357312496495L),
    TEST_f_f (tanh, -0.75L, -0.635148952387287319214434357312496495L),

    TEST_f_f (tanh, 1.0L, 0.7615941559557648881194582826047935904L),
    TEST_f_f (tanh, -1.0L, -0.7615941559557648881194582826047935904L),

    /* 2^-57  */
    TEST_f_f (tanh, 0x1p-57L, 6.938893903907228377647697925567626953125e-18L),
    END_DATA (tanh)
  };

static void
tanh_test (void)
{
  errno = 0;
  FUNC(tanh) (0.7L);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tanh);
  RUN_TEST_LOOP_f_f (tanh, tanh_test_data, );
  END (tanh);
}

static const struct test_f_f_data tgamma_test_data[] =
  {
    START_DATA (tgamma),
    TEST_f_f (tgamma, plus_infty, plus_infty),
    TEST_f_f (tgamma, max_value, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (tgamma, 0, plus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f (tgamma, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    /* tgamma (x) == qNaN plus invalid exception for integer x <= 0.  */
    TEST_f_f (tgamma, -2, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (tgamma, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (tgamma, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (tgamma, qnan_value, qnan_value),

    TEST_f_f (tgamma, 0.5, M_SQRT_PIl),
    TEST_f_f (tgamma, -0.5, -M_2_SQRT_PIl),

    TEST_f_f (tgamma, 1, 1),
    TEST_f_f (tgamma, 2, 1),
    TEST_f_f (tgamma, 3, 2),
    TEST_f_f (tgamma, 4, 6),
    TEST_f_f (tgamma, 5, 24),
    TEST_f_f (tgamma, 6, 120),
    TEST_f_f (tgamma, 7, 720),
    TEST_f_f (tgamma, 8, 5040),
    TEST_f_f (tgamma, 9,  40320),
    TEST_f_f (tgamma, 10, 362880),

    TEST_f_f (tgamma, 0.7L, 1.29805533264755778568117117915281162L),
    TEST_f_f (tgamma, 1.2L, 0.918168742399760610640951655185830401L),

    TEST_f_f (tgamma, 1.5L, 8.8622692545275801364908374167057259139877e-01L),
    TEST_f_f (tgamma, 2.5L, 1.3293403881791370204736256125058588870982e+00L),
    TEST_f_f (tgamma, 3.5L, 3.3233509704478425511840640312646472177454e+00L),
    TEST_f_f (tgamma, 4.5L, 1.1631728396567448929144224109426265262109e+01L),
    TEST_f_f (tgamma, 5.5L, 5.2342777784553520181149008492418193679490e+01L),
    TEST_f_f (tgamma, 6.5L, 2.8788527781504436099631954670830006523720e+02L),
    TEST_f_f (tgamma, 7.5L, 1.8712543057977883464760770536039504240418e+03L),
    TEST_f_f (tgamma, 8.5L, 1.4034407293483412598570577902029628180313e+04L),
    TEST_f_f (tgamma, 9.5L, 1.1929246199460900708784991216725183953266e+05L),
    TEST_f_f (tgamma, -1.5L, 2.3632718012073547030642233111215269103967e+00L),
    TEST_f_f (tgamma, -2.5L, -9.4530872048294188122568932444861076415869e-01L),
    TEST_f_f (tgamma, -3.5L, 2.7008820585226910892162552127103164690248e-01L),
    TEST_f_f (tgamma, -4.5L, -6.0019601300504246427027893615784810422774e-02L),
    TEST_f_f (tgamma, -5.5L, 1.0912654781909862986732344293779056440504e-02L),
    TEST_f_f (tgamma, -6.5L, -1.6788699664476712287280529682737009908468e-03L),
    TEST_f_f (tgamma, -7.5L, 2.2384932885968949716374039576982679877958e-04L),
    TEST_f_f (tgamma, -8.5L, -2.6335215159963470254557693619979623385833e-05L),
    TEST_f_f (tgamma, -9.5L, 2.7721279115751021320587045915768024616666e-06L),
    TEST_f_f (tgamma, 0x1p-24L, 1.6777215422784394050795179874582764575261e+07L),
    TEST_f_f (tgamma, -0x1p-24L, -1.6777216577215723853867349114260580375249e+07L),
    TEST_f_f (tgamma, 0x1p-53L, 9.0071992547409914227843350984672492007618e+15L),
    TEST_f_f (tgamma, -0x1p-53L, -9.0071992547409925772156649015329704137860e+15L),
    TEST_f_f (tgamma, 0x1p-64L, 1.8446744073709551615422784335098467139447e+19L),
    TEST_f_f (tgamma, -0x1p-64L, -1.8446744073709551616577215664901532860660e+19L),
    TEST_f_f (tgamma, 0x1p-106L, 8.1129638414606681695789005144063422784335e+31L),
    TEST_f_f (tgamma, -0x1p-106L, -8.1129638414606681695789005144064577215665e+31L),
    TEST_f_f (tgamma, 0x1p-113L, 1.0384593717069655257060992658440191422784e+34L),
    TEST_f_f (tgamma, -0x1p-113L, -1.0384593717069655257060992658440192577216e+34L),
    TEST_f_f (tgamma, 0x1p-127L, 1.7014118346046923173168730371588410572742e+38L),
    TEST_f_f (tgamma, -0x1p-127L, -1.7014118346046923173168730371588410572858e+38L),
#ifdef TEST_FLOAT
    TEST_f_f (tgamma, 0x1p-128L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x1p-128L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (tgamma, 0x1p-149L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x1p-149L, minus_infty, OVERFLOW_EXCEPTION),
#else
    TEST_f_f (tgamma, 0x1p-128L, 3.4028236692093846346337460743176821145542e+38L),
    TEST_f_f (tgamma, -0x1p-128L, -3.4028236692093846346337460743176821145658e+38L),
    TEST_f_f (tgamma, 0x1p-149L, 7.1362384635297994052914298472474756819137e+44L),
    TEST_f_f (tgamma, -0x1p-149L, -7.1362384635297994052914298472474756819137e+44L),
#endif
#ifndef TEST_FLOAT
    TEST_f_f (tgamma, 0x1p-1023L, 8.9884656743115795386465259539451236680899e+307L),
    TEST_f_f (tgamma, -0x1p-1023L, -8.9884656743115795386465259539451236680899e+307L),
# if !defined TEST_LDOUBLE || LDBL_MAX_EXP <= 1024
    TEST_f_f (tgamma, 0x1p-1024L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x1p-1024L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (tgamma, 0x1p-1074L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x1p-1074L, minus_infty, OVERFLOW_EXCEPTION),
# else
    TEST_f_f (tgamma, 0x1p-1024L, 1.7976931348623159077293051907890247336180e+308L),
    TEST_f_f (tgamma, -0x1p-1024L, -1.7976931348623159077293051907890247336180e+308L),
    TEST_f_f (tgamma, 0x1p-1074L, 2.0240225330731061835249534671891730704956e+323L),
    TEST_f_f (tgamma, -0x1p-1074L, -2.0240225330731061835249534671891730704956e+323L),
# endif
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_f_f (tgamma, 0x1p-16383L, 5.9486574767861588254287966331400356538172e+4931L),
    TEST_f_f (tgamma, -0x1p-16383L, -5.9486574767861588254287966331400356538172e+4931L),
    TEST_f_f (tgamma, 0x1p-16384L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x1p-16384L, minus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (tgamma, 0x1p-16445L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x1p-16445L, minus_infty, OVERFLOW_EXCEPTION),
# if LDBL_MANT_DIG >= 113
    TEST_f_f (tgamma, 0x1p-16494L, plus_infty, OVERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x1p-16494L, minus_infty, OVERFLOW_EXCEPTION),
# endif
#endif
    TEST_f_f (tgamma, 0x8.00001p0L, 5.0400096882277802019946778420223050233915e+03L),
    TEST_f_f (tgamma, 0x7.fffff8p0L, 5.0399951558933225045148935487583089307135e+03L),
    TEST_f_f (tgamma, 0x7.000008p0L, 7.2000064296977505705636258629805621178587e+02L),
    TEST_f_f (tgamma, 0x6.fffff8p0L, 7.1999935703082425988147448928288557689866e+02L),
    TEST_f_f (tgamma, 0x6.000008p0L, 1.2000009762487825358530770343720418162783e+02L),
    TEST_f_f (tgamma, 0x5.fffff8p0L, 1.1999990237520611552119807476573441975106e+02L),
    TEST_f_f (tgamma, 0x5.000008p0L, 2.4000017236155647574166073485628713443799e+01L),
    TEST_f_f (tgamma, 0x4.fffff8p0L, 2.3999982763857938712639837029547357501709e+01L),
    TEST_f_f (tgamma, 0x4.000008p0L, 6.0000035937827461765660468073471093546129e+00L),
    TEST_f_f (tgamma, 0x3.fffffcp0L, 5.9999982031095793171233994481968816873643e+00L),
    TEST_f_f (tgamma, 0x3.000004p0L, 2.0000004400179308360529417942462250547999e+00L),
    TEST_f_f (tgamma, 0x2.fffffcp0L, 1.9999995599822108706107786027549565954046e+00L),
    TEST_f_f (tgamma, 0x2.000004p0L, 1.0000001007996638509889062631687945799175e+00L),
    TEST_f_f (tgamma, 0x1.fffffep0L, 9.9999994960018563231526611134590489120697e-01L),
    TEST_f_f (tgamma, 0x1.000002p0L, 9.9999993119054472483596471908942669644327e-01L),
    TEST_f_f (tgamma, 0x0.ffffffp0L, 1.0000000344047381790797322460568297132998e+00L),
    TEST_f_f (tgamma, -0x0.ffffffp0L, -1.6777216422784419250710305882992376932423e+07L),
    TEST_f_f (tgamma, -0x1.000002p0L, 8.3886075772158332060084424806449513922858e+06L),
    TEST_f_f (tgamma, -0x1.fffffep0L, 4.1943044613922792026014320172298377770578e+06L),
    TEST_f_f (tgamma, -0x2.000004p0L, -2.0971515386080557574407223895988378776747e+06L),
    TEST_f_f (tgamma, -0x2.fffffcp0L, -6.9905087601970247876992248591045142913324e+05L),
    TEST_f_f (tgamma, -0x3.000004p0L, 6.9905045731381300146131914617735687322025e+05L),
    TEST_f_f (tgamma, -0x3.fffffcp0L, 1.7476272942159602684441970627092458855771e+05L),
    TEST_f_f (tgamma, -0x4.000008p0L, -8.7381270578483499672965708923121931082305e+04L),
    TEST_f_f (tgamma, -0x4.fffff8p0L, -1.7476280884325863043793087474680780379554e+04L),
    TEST_f_f (tgamma, -0x5.000008p0L, 1.7476252449031389167286893378510439443844e+04L),
    TEST_f_f (tgamma, -0x5.fffff8p0L, 2.9127137122026653716311560165769071985443e+03L),
    TEST_f_f (tgamma, -0x6.000008p0L, -2.9127085100239567622341538102130981196910e+03L),
    TEST_f_f (tgamma, -0x6.fffff8p0L, -4.1610198723079349791939054365613377035519e+02L),
    TEST_f_f (tgamma, -0x7.000008p0L, 4.1610118737306415004517215226199741948733e+02L),
    TEST_f_f (tgamma, -0x7.fffff8p0L, 5.2012751504050764429534086402871289946986e+01L),
    TEST_f_f (tgamma, -0x8.00001p0L, -2.6006296115134418896533598545925084576702e+01L),
    TEST_f_f (tgamma, -0x9.fffffp0L, 2.8896008370721717567612135720915723136310e-01L),
    TEST_f_f (tgamma, -0xa.00001p0L, -2.8895878754728051776830454190076999107021e-01L),
    TEST_f_f (tgamma, -0x13.ffffep0L, 2.1550026214525536756224040483579183652119e-13L),
    TEST_f_f (tgamma, -0x14.00002p0L, -2.1549777908265594916405421768142757507179e-13L),
    TEST_f_f (tgamma, -0x1d.ffffep0L, 1.9765721589464867957912772592816027583176e-27L),
    TEST_f_f (tgamma, -0x1e.00002p0L, -1.9765463890341964384070157599286498212650e-27L),
    TEST_f_f (tgamma, -0x27.ffffcp0L, 3.2129279441390812141195076945616975790225e-43L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_f_f (tgamma, -0x28.00004p0L, -3.2128372159115252365699015758097981155793e-43L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_f_f (tgamma, -0x28.ffffcp0L, -7.8364103489619817539676737414096652170685e-45L, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_f_f (tgamma, -0x29.00004p0L, 7.8361876024016854597745353972619195760515e-45L, UNDERFLOW_EXCEPTION_FLOAT),
#ifdef TEST_FLOAT
    TEST_f_f (tgamma, -0x29.ffffcp0L, plus_zero, UNDERFLOW_EXCEPTION_FLOAT),
    TEST_f_f (tgamma, -0x2a.00004p0L, minus_zero, UNDERFLOW_EXCEPTION_FLOAT),
#else
    TEST_f_f (tgamma, -0x29.ffffcp0L, 1.8658121573125798145204120066590953505132e-46L),
    TEST_f_f (tgamma, -0x2a.00004p0L, -1.8657587834931410688246126853566488626385e-46L),
#endif
#ifndef TEST_FLOAT
    TEST_f_f (tgamma, 0x8.0000000000008p0L, 5.0400000000000180457125667322294144477136e+03L),
    TEST_f_f (tgamma, 0x7.ffffffffffffcp0L, 5.0399999999999909771437166339103165198442e+03L),
    TEST_f_f (tgamma, 0x7.0000000000004p0L, 7.2000000000000119762397445457359071259652e+02L),
    TEST_f_f (tgamma, 0x6.ffffffffffffcp0L, 7.1999999999999880237602554542848858572672e+02L),
    TEST_f_f (tgamma, 0x6.0000000000004p0L, 1.2000000000000018184042734842640022086408e+02L),
    TEST_f_f (tgamma, 0x5.ffffffffffffcp0L, 1.1999999999999981815957265157389249327533e+02L),
    TEST_f_f (tgamma, 0x5.0000000000004p0L, 2.4000000000000032104829055124673225982803e+01L),
    TEST_f_f (tgamma, 0x4.ffffffffffffcp0L, 2.3999999999999967895170944875373910918544e+01L),
    TEST_f_f (tgamma, 0x4.0000000000004p0L, 6.0000000000000066939396342309789716341613e+00L),
    TEST_f_f (tgamma, 0x3.ffffffffffffep0L, 5.9999999999999966530301828845138185025345e+00L),
    TEST_f_f (tgamma, 0x3.0000000000002p0L, 2.0000000000000008195971324717875960213536e+00L),
    TEST_f_f (tgamma, 0x2.ffffffffffffep0L, 1.9999999999999991804028675282128956223990e+00L),
    TEST_f_f (tgamma, 0x2.0000000000002p0L, 1.0000000000000001877539613108624482361963e+00L),
    TEST_f_f (tgamma, 0x1.fffffffffffffp0L, 9.9999999999999990612301934456883679778984e-01L),
    TEST_f_f (tgamma, 0x1.0000000000001p0L, 9.9999999999999987183237573039992418700531e-01L),
    TEST_f_f (tgamma, 0x0.fffffffffffff8p0L, 1.0000000000000000640838121348000744796665e+00L),
    TEST_f_f (tgamma, -0x0.fffffffffffff8p0L, -9.0071992547409924227843350984672961392521e+15L),
    TEST_f_f (tgamma, -0x1.0000000000001p0L, 4.5035996273704955772156649015331740980405e+15L),
    TEST_f_f (tgamma, -0x1.fffffffffffffp0L, 2.2517998136852484613921675492337776673289e+15L),
    TEST_f_f (tgamma, -0x2.0000000000002p0L, -1.1258999068426235386078324507668462444260e+15L),
    TEST_f_f (tgamma, -0x2.ffffffffffffep0L, -3.7529996894754154268627807196691509198813e+14L),
    TEST_f_f (tgamma, -0x3.0000000000002p0L, 3.7529996894754112398038859470009084971438e+14L),
    TEST_f_f (tgamma, -0x3.ffffffffffffep0L, 9.3824992236885396088236184658402406857503e+13L),
    TEST_f_f (tgamma, -0x4.0000000000004p0L, -4.6912496118442603911763815341745722862351e+13L),
    TEST_f_f (tgamma, -0x4.ffffffffffffcp0L, -9.3824992236885475509805702650262155809819e+12L),
    TEST_f_f (tgamma, -0x5.0000000000004p0L, 9.3824992236885191156860964016850034672946e+12L),
    TEST_f_f (tgamma, -0x5.ffffffffffffcp0L, 1.5637498706147581566449098589862357835505e+12L),
    TEST_f_f (tgamma, -0x6.0000000000004p0L, -1.5637498706147529544662012521330708016396e+12L),
    TEST_f_f (tgamma, -0x6.ffffffffffffcp0L, -2.2339283865925119357965832452642909859289e+11L),
    TEST_f_f (tgamma, -0x7.0000000000004p0L, 2.2339283865925039372192897706214475877342e+11L),
    TEST_f_f (tgamma, -0x7.ffffffffffffcp0L, 2.7924104832406402297655703264222230055898e+10L),
    TEST_f_f (tgamma, -0x8.0000000000008p0L, -1.3962052416203121511868106259843527348026e+10L),
    TEST_f_f (tgamma, -0x9.ffffffffffff8p0L, 1.5513391573559147700413058496716749249803e+08L),
    TEST_f_f (tgamma, -0xa.0000000000008p0L, -1.5513391573559018084419393002828541166901e+08L),
    TEST_f_f (tgamma, -0x13.ffffffffffffp0L, 1.1569515572952029402736625857313236848570e-04L),
    TEST_f_f (tgamma, -0x14.000000000001p0L, -1.1569515572951781096476686854873801225397e-04L),
    TEST_f_f (tgamma, -0x1d.ffffffffffffp0L, 1.0611571800204311628217068863959963842891e-18L),
    TEST_f_f (tgamma, -0x1e.000000000001p0L, -1.0611571800204053929094168642022073530425e-18L),
    TEST_f_f (tgamma, -0x27.fffffffffffep0L, 1.7249032006742266376460389310340465554361e-34L),
    TEST_f_f (tgamma, -0x28.000000000002p0L, -1.7249032006741359094184881234822934593822e-34L),
    TEST_f_f (tgamma, -0x28.fffffffffffep0L, -4.2070809772542120404320040128839297118648e-36L),
    TEST_f_f (tgamma, -0x29.000000000002p0L, 4.2070809772539892938717205103652583609422e-36L),
    TEST_f_f (tgamma, -0x29.fffffffffffep0L, 1.0016859469652887505173040814397197718981e-37L),
    TEST_f_f (tgamma, -0x2a.000000000002p0L, -1.0016859469652353766978684241048308120274e-37L),
    TEST_f_f (tgamma, -0x31.fffffffffffep0L, 4.6273774273632946947805289899230181990085e-51L),
    TEST_f_f (tgamma, -0x32.000000000002p0L, -4.6273774273630367887073532197576655720178e-51L),
    TEST_f_f (tgamma, -0x63.fffffffffffcp0L, 7.5400833348840965463348754984345825364294e-145L),
    TEST_f_f (tgamma, -0x64.000000000004p0L, -7.5400833348831085791638490135462230991587e-145L),
    TEST_f_f (tgamma, -0x95.fffffffffff8p0L, 6.1582369322723207086020016423767264008839e-250L),
    TEST_f_f (tgamma, -0x96.000000000008p0L, -6.1582369322705655439003240743176243138734e-250L),
    TEST_f_f (tgamma, -0xb4.fffffffffff8p0L, -9.6760879059917574597728750098636253931457e-319L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xb5.000000000008p0L, 9.6760879059888966544677044221698800670218e-319L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xb5.fffffffffff8p0L, 5.3165318164789884455066481673086605454904e-321L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xb6.000000000008p0L, -5.3165318164774149139661976747137185876909e-321L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xb6.fffffffffff8p0L, -2.9052086428846935908287469917922960610289e-323L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xb7.000000000008p0L, 2.9052086428838328351126988236541632950925e-323L, UNDERFLOW_EXCEPTION_DOUBLE),
# if !defined TEST_LDOUBLE || LDBL_MAX_EXP <= 1024
    TEST_f_f (tgamma, -0xb7.fffffffffff8p0L, plus_zero, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xb8.000000000008p0L, minus_zero, UNDERFLOW_EXCEPTION_DOUBLE),
# else
    TEST_f_f (tgamma, -0xb7.fffffffffff8p0L, 1.5789177406982032823826953250736039527543e-325L),
    TEST_f_f (tgamma, -0xb8.000000000008p0L, -1.5789177406977349925854817486109369828857e-325L),
# endif
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_f_f (tgamma, 0x8.00000000000000000000000004p0L, 5.0400000000000000000000000000020034765587e+03L),
    TEST_f_f (tgamma, 0x7.fffffffffffffffffffffffffep0L, 5.0399999999999999999999999999989982617206e+03L),
    TEST_f_f (tgamma, 0x7.00000000000000000000000002p0L, 7.2000000000000000000000000000013296297113e+02L),
    TEST_f_f (tgamma, 0x6.fffffffffffffffffffffffffep0L, 7.1999999999999999999999999999986703702887e+02L),
    TEST_f_f (tgamma, 0x6.00000000000000000000000002p0L, 1.2000000000000000000000000000002018834292e+02L),
    TEST_f_f (tgamma, 0x5.fffffffffffffffffffffffffep0L, 1.1999999999999999999999999999997981165708e+02L),
    TEST_f_f (tgamma, 0x5.00000000000000000000000002p0L, 2.4000000000000000000000000000003564352042e+01L),
    TEST_f_f (tgamma, 0x4.fffffffffffffffffffffffffep0L, 2.3999999999999999999999999999996435647958e+01L),
    TEST_f_f (tgamma, 0x4.00000000000000000000000002p0L, 6.0000000000000000000000000000007431765907e+00L),
    TEST_f_f (tgamma, 0x3.ffffffffffffffffffffffffffp0L, 5.9999999999999999999999999999996284117046e+00L),
    TEST_f_f (tgamma, 0x3.00000000000000000000000001p0L, 2.0000000000000000000000000000000909935607e+00L),
    TEST_f_f (tgamma, 0x2.ffffffffffffffffffffffffffp0L, 1.9999999999999999999999999999999090064393e+00L),
    TEST_f_f (tgamma, 0x2.00000000000000000000000001p0L, 1.0000000000000000000000000000000208448771e+00L),
    TEST_f_f (tgamma, 0x1.ffffffffffffffffffffffffff8p0L, 9.9999999999999999999999999999998957756146e-01L),
    TEST_f_f (tgamma, 0x1.000000000000000000000000008p0L, 9.9999999999999999999999999999998577053525e-01L),
    TEST_f_f (tgamma, 0x0.ffffffffffffffffffffffffffcp0L, 1.0000000000000000000000000000000071147324e+00L),
    TEST_f_f (tgamma, -0x0.ffffffffffffffffffffffffffcp0L, -8.1129638414606681695789005144064422784335e+31L),
    TEST_f_f (tgamma, -0x1.000000000000000000000000008p0L, 4.0564819207303340847894502572031577215665e+31L),
    TEST_f_f (tgamma, -0x1.ffffffffffffffffffffffffff8p0L, 2.0282409603651670423947251286016461392168e+31L),
    TEST_f_f (tgamma, -0x2.00000000000000000000000001p0L, -1.0141204801825835211973625643007538607832e+31L),
    TEST_f_f (tgamma, -0x2.ffffffffffffffffffffffffffp0L, -3.3804016006086117373245418810028760196114e+30L),
    TEST_f_f (tgamma, -0x3.00000000000000000000000001p0L, 3.3804016006086117373245418810024573137219e+30L),
    TEST_f_f (tgamma, -0x3.ffffffffffffffffffffffffffp0L, 8.4510040015215293433113547025072942156952e+29L),
    TEST_f_f (tgamma, -0x4.00000000000000000000000002p0L, -4.2255020007607646716556773512527057843048e+29L),
    TEST_f_f (tgamma, -0x4.fffffffffffffffffffffffffep0L, -8.4510040015215293433113547025080884313904e+28L),
    TEST_f_f (tgamma, -0x5.00000000000000000000000002p0L, 8.4510040015215293433113547025052449019430e+28L),
    TEST_f_f (tgamma, -0x5.fffffffffffffffffffffffffep0L, 1.4085006669202548905518924504180378867132e+28L),
    TEST_f_f (tgamma, -0x6.00000000000000000000000002p0L, -1.4085006669202548905518924504175176688423e+28L),
    TEST_f_f (tgamma, -0x6.fffffffffffffffffffffffffep0L, -2.0121438098860784150741320720257967542615e+27L),
    TEST_f_f (tgamma, -0x7.00000000000000000000000002p0L, 2.0121438098860784150741320720249968965322e+27L),
    TEST_f_f (tgamma, -0x7.fffffffffffffffffffffffffep0L, 2.5151797623575980188426650900322769448110e+26L),
    TEST_f_f (tgamma, -0x8.00000000000000000000000004p0L, -1.2575898811787990094213325450153421028080e+26L),
    TEST_f_f (tgamma, -0x9.fffffffffffffffffffffffffcp0L, 1.3973220901986655660237028277960625420495e+24L),
    TEST_f_f (tgamma, -0xa.00000000000000000000000004p0L, -1.3973220901986655660237028277947663821128e+24L),
    TEST_f_f (tgamma, -0x13.fffffffffffffffffffffffff8p0L, 1.0420893204640670202556853709074896123293e+12L),
    TEST_f_f (tgamma, -0x14.00000000000000000000000008p0L, -1.0420893204640670202556853709050065497299e+12L),
    TEST_f_f (tgamma, -0x1d.fffffffffffffffffffffffff8p0L, 9.5580541610429641982963434151488827190079e-03L),
    TEST_f_f (tgamma, -0x1e.00000000000000000000000008p0L, -9.5580541610429641982963434151231128067179e-03L),
    TEST_f_f (tgamma, -0x27.fffffffffffffffffffffffffp0L, 1.5536546823612837336494536911280147806523e-18L),
    TEST_f_f (tgamma, -0x28.0000000000000000000000001p0L, -1.5536546823612837336494536911189419578973e-18L),
    TEST_f_f (tgamma, -0x28.fffffffffffffffffffffffffp0L, -3.7894016642958139845108626612879138384405e-20L),
    TEST_f_f (tgamma, -0x29.0000000000000000000000001p0L, 3.7894016642958139845108626612656391824122e-20L),
    TEST_f_f (tgamma, -0x29.fffffffffffffffffffffffffp0L, 9.0223849149900332964544349078285357440663e-22L),
    TEST_f_f (tgamma, -0x2a.0000000000000000000000001p0L, -9.0223849149900332964544349077751619246306e-22L),
    TEST_f_f (tgamma, -0x31.fffffffffffffffffffffffffp0L, 4.1679710515150795310771069868348482819424e-35L),
    TEST_f_f (tgamma, -0x32.0000000000000000000000001p0L, -4.1679710515150795310771069868090576746248e-35L),
    TEST_f_f (tgamma, -0x63.ffffffffffffffffffffffffep0L, 6.7915032994648558610510614163560656864280e-129L),
    TEST_f_f (tgamma, -0x64.0000000000000000000000002p0L, -6.7915032994648558610510614162572689693253e-129L),
    TEST_f_f (tgamma, -0x95.ffffffffffffffffffffffffcp0L, 5.5468467106873795353190582463444660395353e-234L),
    TEST_f_f (tgamma, -0x96.0000000000000000000000004p0L, -5.5468467106873795353190582461689495693675e-234L),
    TEST_f_f (tgamma, -0xb4.ffffffffffffffffffffffffcp0L, -8.7154451775644399729745472278589884205029e-303L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_f_f (tgamma, -0xb5.0000000000000000000000004p0L, 8.7154451775644399729745472275729078899858e-303L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_f_f (tgamma, -0xb5.ffffffffffffffffffffffffcp0L, 4.7887061415189230620739270482742524864974e-305L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_f_f (tgamma, -0xb6.0000000000000000000000004p0L, -4.7887061415189230620739270481168993324524e-305L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_f_f (tgamma, -0xb6.ffffffffffffffffffffffffcp0L, -2.6167793123054224382917634143575601407067e-307L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_f_f (tgamma, -0xb7.0000000000000000000000004p0L, 2.6167793123054224382917634142714845691019e-307L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
    TEST_f_f (tgamma, -0xb7.ffffffffffffffffffffffffcp0L, 1.4221626697312078468976975078030462044826e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xb8.0000000000000000000000004p0L, -1.4221626697312078468976975077562172247612e-309L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xbb.ffffffffffffffffffffffffcp0L, 1.1756150745511026776007338998283058108448e-318L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xbc.0000000000000000000000004p0L, -1.1756150745511026776007338997894360215107e-318L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xbc.ffffffffffffffffffffffffcp0L, -6.2201855796354639026493857133773832713743e-321L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xbd.0000000000000000000000004p0L, 6.2201855796354639026493857131715153125175e-321L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xbd.ffffffffffffffffffffffffcp0L, 3.2737818840186652119207293228302560914295e-323L, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xbe.0000000000000000000000004p0L, -3.2737818840186652119207293227217957948294e-323L, UNDERFLOW_EXCEPTION_DOUBLE),
# if LDBL_MAX_EXP <= 1024
    TEST_f_f (tgamma, -0xbe.ffffffffffffffffffffffffcp0L, minus_zero, UNDERFLOW_EXCEPTION_DOUBLE),
    TEST_f_f (tgamma, -0xbf.0000000000000000000000004p0L, plus_zero, UNDERFLOW_EXCEPTION_DOUBLE),
# else
    TEST_f_f (tgamma, -0xbe.ffffffffffffffffffffffffcp0L, -1.7140219288055838805867692789687487834686e-325L),
    TEST_f_f (tgamma, -0xbf.0000000000000000000000004p0L, 1.7140219288055838805867692789119066543632e-325L),
# endif
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_f_f (tgamma, 0x8.000000000000001p0L, 5.0400000000000000088113830892247051102283e+03L),
    TEST_f_f (tgamma, 0x7.fffffffffffffff8p0L, 5.0399999999999999955943084553876474508520e+03L),
    TEST_f_f (tgamma, 0x7.0000000000000008p0L, 7.2000000000000000058477733127664675369681e+02L),
    TEST_f_f (tgamma, 0x6.fffffffffffffff8p0L, 7.1999999999999999941522266872335324679893e+02L),
    TEST_f_f (tgamma, 0x6.0000000000000008p0L, 1.2000000000000000008878927116622375680433e+02L),
    TEST_f_f (tgamma, 0x5.fffffffffffffff8p0L, 1.1999999999999999991121072883377624326546e+02L),
    TEST_f_f (tgamma, 0x5.0000000000000008p0L, 2.4000000000000000015676186062072582846211e+01L),
    TEST_f_f (tgamma, 0x4.fffffffffffffff8p0L, 2.3999999999999999984323813937927417165027e+01L),
    TEST_f_f (tgamma, 0x4.0000000000000008p0L, 6.0000000000000000032685252120268430507939e+00L),
    TEST_f_f (tgamma, 0x3.fffffffffffffffcp0L, 5.9999999999999999983657373939865784753909e+00L),
    TEST_f_f (tgamma, 0x3.0000000000000004p0L, 2.0000000000000000004001939123397399171482e+00L),
    TEST_f_f (tgamma, 0x2.fffffffffffffffcp0L, 1.9999999999999999995998060876602600829690e+00L),
    TEST_f_f (tgamma, 0x2.0000000000000004p0L, 1.0000000000000000000916767389213195151634e+00L),
    TEST_f_f (tgamma, 0x1.fffffffffffffffep0L, 9.9999999999999999995416163053934024243282e-01L),
    TEST_f_f (tgamma, 0x1.0000000000000002p0L, 9.9999999999999999993741815221210931418291e-01L),
    TEST_f_f (tgamma, 0x0.ffffffffffffffffp0L, 1.0000000000000000000312909238939453429173e+00L),
    TEST_f_f (tgamma, -0x0.ffffffffffffffffp0L, -1.8446744073709551616422784335098467139470e+19L),
    TEST_f_f (tgamma, -0x1.0000000000000002p0L, 9.2233720368547758075772156649015328607596e+18L),
    TEST_f_f (tgamma, -0x1.fffffffffffffffep0L, 4.6116860184273879044613921675492335697983e+18L),
    TEST_f_f (tgamma, -0x2.0000000000000004p0L, -2.3058430092136939515386078324507664305064e+18L),
    TEST_f_f (tgamma, -0x2.fffffffffffffffcp0L, -7.6861433640456465087601961140530007887063e+17L),
    TEST_f_f (tgamma, -0x3.0000000000000004p0L, 7.6861433640456465045731372192803325462836e+17L),
    TEST_f_f (tgamma, -0x3.fffffffffffffffcp0L, 1.9215358410114116272942156951799168638773e+17L),
    TEST_f_f (tgamma, -0x4.0000000000000008p0L, -9.6076792050570581270578430482008313684602e+16L),
    TEST_f_f (tgamma, -0x4.fffffffffffffff8p0L, -1.9215358410114116280884313903598337283601e+16L),
    TEST_f_f (tgamma, -0x5.0000000000000008p0L, 1.9215358410114116252449019429734996071487e+16L),
    TEST_f_f (tgamma, -0x5.fffffffffffffff8p0L, 3.2025597350190193803788671320812043622696e+15L),
    TEST_f_f (tgamma, -0x6.0000000000000008p0L, -3.2025597350190193751766884234743511972877e+15L),
    TEST_f_f (tgamma, -0x6.fffffffffffffff8p0L, -4.5750853357414562579675426149912896787735e+14L),
    TEST_f_f (tgamma, -0x7.0000000000000008p0L, 4.5750853357414562499689653215166468353753e+14L),
    TEST_f_f (tgamma, -0x7.fffffffffffffff8p0L, 5.7188566696768203227694481100089533685959e+13L),
    TEST_f_f (tgamma, -0x8.000000000000001p0L, -2.8594283348384101534210280804672371201060e+13L),
    TEST_f_f (tgamma, -0x9.fffffffffffffffp0L, 3.1771425942649001828476427167843945971988e+11L),
    TEST_f_f (tgamma, -0xa.000000000000001p0L, -3.1771425942649001698860433502350057763905e+11L),
    TEST_f_f (tgamma, -0x13.ffffffffffffffep0L, 2.3694367893405502075347562184931828448654e-01L),
    TEST_f_f (tgamma, -0x14.000000000000002p0L, -2.3694367893405501827041302245929389013031e-01L),
    TEST_f_f (tgamma, -0x1d.ffffffffffffffep0L, 2.1732499046818166459536268654187775086902e-15L),
    TEST_f_f (tgamma, -0x1e.000000000000002p0L, -2.1732499046818166201837145753965837196590e-15L),
    TEST_f_f (tgamma, -0x27.ffffffffffffffcp0L, 3.5326017549807232935581894777156474496719e-31L),
    TEST_f_f (tgamma, -0x28.000000000000004p0L, -3.5326017549807232028299619269080956965758e-31L),
    TEST_f_f (tgamma, -0x28.ffffffffffffffcp0L, -8.6161018414163982777002940498289948893044e-33L),
    TEST_f_f (tgamma, -0x29.000000000000004p0L, 8.6161018414163980549537337663264762179535e-33L),
    TEST_f_f (tgamma, -0x29.ffffffffffffffcp0L, 2.0514528193848567329552463626090806737389e-34L),
    TEST_f_f (tgamma, -0x2a.000000000000004p0L, -2.0514528193848566795814269269517457847791e-34L),
    TEST_f_f (tgamma, -0x31.ffffffffffffffcp0L, 9.4768689712397635680446279661359728835046e-48L),
    TEST_f_f (tgamma, -0x32.000000000000004p0L, -9.4768689712397633101385547903658075308777e-48L),
    TEST_f_f (tgamma, -0x63.ffffffffffffff8p0L, 1.5442090669841618542494279375256856430049e-141L),
    TEST_f_f (tgamma, -0x64.000000000000008p0L, -1.5442090669841617554527108348771968070612e-141L),
    TEST_f_f (tgamma, -0x95.ffffffffffffffp0L, 1.2612069237291916400144732227892704713839e-246L),
    TEST_f_f (tgamma, -0x96.00000000000001p0L, -1.2612069237291914644980030550324645611752e-246L),
    TEST_f_f (tgamma, -0xb4.ffffffffffffffp0L, -1.9816628031468191243385005680879281767694e-315L),
    TEST_f_f (tgamma, -0xb5.00000000000001p0L, 1.9816628031468188382579700510291588022368e-315L),
    TEST_f_f (tgamma, -0xb5.ffffffffffffffp0L, 1.0888257160147357826865964233809723297472e-317L),
    TEST_f_f (tgamma, -0xb6.00000000000001p0L, -1.0888257160147356253334423783317128355514e-317L),
    TEST_f_f (tgamma, -0xb6.ffffffffffffffp0L, -5.9498673006269714905418984659220067091260e-320L),
    TEST_f_f (tgamma, -0xb7.00000000000001p0L, 5.9498673006269706297861824177538685763601e-320L),
    TEST_f_f (tgamma, -0xb7.ffffffffffffffp0L, 3.2336235329494410277123118903958061569834e-322L),
    TEST_f_f (tgamma, -0xb8.00000000000001p0L, -3.2336235329494405594225146768193434900135e-322L),
    TEST_f_f (tgamma, -0xbb.ffffffffffffffp0L, 2.6730392040715350119087465463119939092815e-331L),
    TEST_f_f (tgamma, -0xbc.00000000000001p0L, -2.6730392040715346232108532050343031951651e-331L),
    TEST_f_f (tgamma, -0xbc.ffffffffffffffp0L, -1.4143064571807063556111222197839950086445e-333L),
    TEST_f_f (tgamma, -0xbd.00000000000001p0L, 1.4143064571807061497431633629389135273431e-333L),
    TEST_f_f (tgamma, -0xbd.ffffffffffffffp0L, 7.4437181956879281879706555863416819210399e-336L),
    TEST_f_f (tgamma, -0xbe.00000000000001p0L, -7.4437181956879271033676895858841525581153e-336L),
    TEST_f_f (tgamma, -0xbe.ffffffffffffffp0L, -3.8972346574282346536709453101948570578636e-338L),
    TEST_f_f (tgamma, -0xbf.00000000000001p0L, 3.8972346574282340852496542564155275274974e-338L),
    TEST_f_f (tgamma, -0xf9.ffffffffffffffp0L, 2.2289142548411573883553287678043297937797e-476L),
    TEST_f_f (tgamma, -0xfa.00000000000001p0L, -2.2289142548411570466476165308364665814265e-476L),
    TEST_f_f (tgamma, -0x1f3.fffffffffffffep0L, 2.9528489142763141594943668922610562530068e-1118L),
    TEST_f_f (tgamma, -0x1f4.00000000000002p0L, -2.9528489142763131406565394149878256133744e-1118L),
    TEST_f_f (tgamma, -0x2ed.fffffffffffffcp0L, 6.9801511765871818502006905472380418430269e-1817L),
    TEST_f_f (tgamma, -0x2ee.00000000000004p0L, -6.9801511765871767194421856376592926002995e-1817L),
    TEST_f_f (tgamma, -0x3e7.fffffffffffffcp0L, 4.4768809295877296071892611539415773519036e-2552L),
    TEST_f_f (tgamma, -0x3e8.00000000000004p0L, -4.4768809295877261735541135972060089530309e-2552L),
    TEST_f_f (tgamma, -0x4e1.fffffffffffff8p0L, 5.4651488569236507565341414077911790252381e-3315L),
    TEST_f_f (tgamma, -0x4e2.00000000000008p0L, -5.4651488569236421026544487194247355967789e-3315L),
    TEST_f_f (tgamma, -0x5db.fffffffffffff8p0L, 1.8718211510339187689122114747834510481993e-4099L),
    TEST_f_f (tgamma, -0x5dc.00000000000008p0L, -1.8718211510339157291960718369454861898499e-4099L),
    TEST_f_f (tgamma, -0x6d5.fffffffffffff8p0L, 4.2925786447266492555651378780094011518063e-4902L),
    TEST_f_f (tgamma, -0x6d6.00000000000008p0L, -4.2925786447266421378134368786479937285900e-4902L),
    TEST_f_f (tgamma, -0x6e2.fffffffffffff8p0L, -2.8229173528168668283609231628661510187536e-4944L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e3.00000000000008p0L, 2.8229173528168621428945467532322397890424e-4944L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e3.fffffffffffff8p0L, 1.6002932839097884515524894602387278842659e-4947L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e4.00000000000008p0L, -1.6002932839097857951909742110188634438017e-4947L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e4.fffffffffffff8p0L, -9.0668174725767051085164382743343408095146e-4951L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e5.00000000000008p0L, 9.0668174725766900571689083050287838090189e-4951L, UNDERFLOW_EXCEPTION),
# if LDBL_MANT_DIG <= 64
    TEST_f_f (tgamma, -0x6e5.fffffffffffff8p0L, plus_zero, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e6.00000000000008p0L, minus_zero, UNDERFLOW_EXCEPTION),
# else
    TEST_f_f (tgamma, -0x6e5.fffffffffffff8p0L, 5.1340982290921319983501912122292601780326e-4954L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e6.00000000000008p0L, -5.1340982290921234748578193083614111497314e-4954L, UNDERFLOW_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_f_f (tgamma, 0x8.0000000000000000000000000008p0L, 5.0400000000000000000000000000000156521606e+03L),
    TEST_f_f (tgamma, 0x7.fffffffffffffffffffffffffffcp0L, 5.0399999999999999999999999999999921739197e+03L),
    TEST_f_f (tgamma, 0x7.0000000000000000000000000004p0L, 7.2000000000000000000000000000000103877321e+02L),
    TEST_f_f (tgamma, 0x6.fffffffffffffffffffffffffffcp0L, 7.1999999999999999999999999999999896122679e+02L),
    TEST_f_f (tgamma, 0x6.0000000000000000000000000004p0L, 1.2000000000000000000000000000000015772143e+02L),
    TEST_f_f (tgamma, 0x5.fffffffffffffffffffffffffffcp0L, 1.1999999999999999999999999999999984227857e+02L),
    TEST_f_f (tgamma, 0x5.0000000000000000000000000004p0L, 2.4000000000000000000000000000000027846500e+01L),
    TEST_f_f (tgamma, 0x4.fffffffffffffffffffffffffffcp0L, 2.3999999999999999999999999999999972153500e+01L),
    TEST_f_f (tgamma, 0x4.0000000000000000000000000004p0L, 6.0000000000000000000000000000000058060671e+00L),
    TEST_f_f (tgamma, 0x3.fffffffffffffffffffffffffffep0L, 5.9999999999999999999999999999999970969664e+00L),
    TEST_f_f (tgamma, 0x3.0000000000000000000000000002p0L, 2.0000000000000000000000000000000007108872e+00L),
    TEST_f_f (tgamma, 0x2.fffffffffffffffffffffffffffep0L, 1.9999999999999999999999999999999992891128e+00L),
    TEST_f_f (tgamma, 0x2.0000000000000000000000000002p0L, 1.0000000000000000000000000000000001628506e+00L),
    TEST_f_f (tgamma, 0x1.ffffffffffffffffffffffffffffp0L, 9.9999999999999999999999999999999991857470e-01L),
    TEST_f_f (tgamma, 0x1.0000000000000000000000000001p0L, 9.9999999999999999999999999999999988883231e-01L),
    TEST_f_f (tgamma, 0x0.ffffffffffffffffffffffffffff8p0L, 1.0000000000000000000000000000000000555838e+00L),
    TEST_f_f (tgamma, -0x0.ffffffffffffffffffffffffffff8p0L, -1.0384593717069655257060992658440192422784e+34L),
    TEST_f_f (tgamma, -0x1.0000000000000000000000000001p0L, 5.1922968585348276285304963292200955772157e+33L),
    TEST_f_f (tgamma, -0x1.ffffffffffffffffffffffffffffp0L, 2.5961484292674138142652481646100484613922e+33L),
    TEST_f_f (tgamma, -0x2.0000000000000000000000000002p0L, -1.2980742146337069071326240823050235386078e+33L),
    TEST_f_f (tgamma, -0x2.fffffffffffffffffffffffffffep0L, -4.3269140487790230237754136076834154268628e+32L),
    TEST_f_f (tgamma, -0x3.0000000000000000000000000002p0L, 4.3269140487790230237754136076834112398039e+32L),
    TEST_f_f (tgamma, -0x3.fffffffffffffffffffffffffffep0L, 1.0817285121947557559438534019208539608824e+32L),
    TEST_f_f (tgamma, -0x4.0000000000000000000000000004p0L, -5.4086425609737787797192670096042603911764e+31L),
    TEST_f_f (tgamma, -0x4.fffffffffffffffffffffffffffcp0L, -1.0817285121947557559438534019208547550981e+31L),
    TEST_f_f (tgamma, -0x5.0000000000000000000000000004p0L, 1.0817285121947557559438534019208519115686e+31L),
    TEST_f_f (tgamma, -0x5.fffffffffffffffffffffffffffcp0L, 1.8028808536579262599064223365347581566449e+30L),
    TEST_f_f (tgamma, -0x6.0000000000000000000000000004p0L, -1.8028808536579262599064223365347529544662e+30L),
    TEST_f_f (tgamma, -0x6.fffffffffffffffffffffffffffcp0L, -2.5755440766541803712948890521925119357966e+29L),
    TEST_f_f (tgamma, -0x7.0000000000000000000000000004p0L, 2.5755440766541803712948890521925039372193e+29L),
    TEST_f_f (tgamma, -0x7.fffffffffffffffffffffffffffcp0L, 3.2194300958177254641186113152406402297656e+28L),
    TEST_f_f (tgamma, -0x8.0000000000000000000000000008p0L, -1.6097150479088627320593056576203121511868e+28L),
    TEST_f_f (tgamma, -0x9.fffffffffffffffffffffffffff8p0L, 1.7885722754542919245103396195781369922635e+26L),
    TEST_f_f (tgamma, -0xa.0000000000000000000000000008p0L, -1.7885722754542919245103396195781240306642e+26L),
    TEST_f_f (tgamma, -0x13.fffffffffffffffffffffffffffp0L, 1.3338743301940057859272772747600099590309e+14L),
    TEST_f_f (tgamma, -0x14.000000000000000000000000001p0L, -1.3338743301940057859272772747599851284049e+14L),
    TEST_f_f (tgamma, -0x1d.fffffffffffffffffffffffffffp0L, 1.2234309326134994173819319571374205986026e+00L),
    TEST_f_f (tgamma, -0x1e.000000000000000000000000001p0L, -1.2234309326134994173819319571373948286903e+00L),
    TEST_f_f (tgamma, -0x27.ffffffffffffffffffffffffffep0L, 1.9886779934224431790713007246380976767855e-16L),
    TEST_f_f (tgamma, -0x28.000000000000000000000000002p0L, -1.9886779934224431790713007246380069485580e-16L),
    TEST_f_f (tgamma, -0x28.ffffffffffffffffffffffffffep0L, -4.8504341302986419001739042064343853066259e-18L),
    TEST_f_f (tgamma, -0x29.000000000000000000000000002p0L, 4.8504341302986419001739042064341625600656e-18L),
    TEST_f_f (tgamma, -0x29.ffffffffffffffffffffffffffep0L, 1.1548652691187242619461676681986633377063e-19L),
    TEST_f_f (tgamma, -0x2a.000000000000000000000000002p0L, -1.1548652691187242619461676681986099638869e-19L),
    TEST_f_f (tgamma, -0x31.ffffffffffffffffffffffffffep0L, 5.3350029459393017997786969431322287652396e-33L),
    TEST_f_f (tgamma, -0x32.000000000000000000000000002p0L, -5.3350029459393017997786969431319708591664e-33L),
    TEST_f_f (tgamma, -0x63.ffffffffffffffffffffffffffcp0L, 8.6931242233150155021453586128730281632676e-127L),
    TEST_f_f (tgamma, -0x64.000000000000000000000000004p0L, -8.6931242233150155021453586128720401960966e-127L),
    TEST_f_f (tgamma, -0x95.ffffffffffffffffffffffffff8p0L, 7.0999637896798458052083945552094635720486e-232L),
    TEST_f_f (tgamma, -0x96.000000000000000000000000008p0L, -7.0999637896798458052083945552077084073470e-232L),
    TEST_f_f (tgamma, -0xb4.ffffffffffffffffffffffffff8p0L, -1.1155769827282483165407420451477844041365e-300L),
    TEST_f_f (tgamma, -0xb5.000000000000000000000000008p0L, 1.1155769827282483165407420451474983236060e-300L),
    TEST_f_f (tgamma, -0xb5.ffffffffffffffffffffffffff8p0L, 6.1295438611442215194546266216911239298981e-303L),
    TEST_f_f (tgamma, -0xb6.000000000000000000000000008p0L, -6.1295438611442215194546266216895503983577e-303L),
    TEST_f_f (tgamma, -0xb6.ffffffffffffffffffffffffff8p0L, -3.3494775197509407210134571703230189921356e-305L),
    TEST_f_f (tgamma, -0xb7.000000000000000000000000008p0L, 3.3494775197509407210134571703221582364195e-305L),
    TEST_f_f (tgamma, -0xb7.ffffffffffffffffffffffffff8p0L, 1.8203682172559460440290528099581627396147e-307L),
    TEST_f_f (tgamma, -0xb8.000000000000000000000000008p0L, -1.8203682172559460440290528099576944498174e-307L),
    TEST_f_f (tgamma, -0xbb.ffffffffffffffffffffffffff8p0L, 1.5047872954254114273289393917555491216542e-316L),
    TEST_f_f (tgamma, -0xbc.000000000000000000000000008p0L, -1.5047872954254114273289393917551604237609e-316L),
    TEST_f_f (tgamma, -0xbc.ffffffffffffffffffffffffff8p0L, -7.9618375419333937953912137129923244334851e-319L),
    TEST_f_f (tgamma, -0xbd.000000000000000000000000008p0L, 7.9618375419333937953912137129902657538965e-319L),
    TEST_f_f (tgamma, -0xbd.ffffffffffffffffffffffffff8p0L, 4.1904408115438914712585335331538555086887e-321L),
    TEST_f_f (tgamma, -0xbe.000000000000000000000000008p0L, -4.1904408115438914712585335331527709057227e-321L),
    TEST_f_f (tgamma, -0xbe.ffffffffffffffffffffffffff8p0L, -2.1939480688711473671510646770439036908579e-323L),
    TEST_f_f (tgamma, -0xbf.000000000000000000000000008p0L, 2.1939480688711473671510646770433352695669e-323L),
    TEST_f_f (tgamma, -0xf9.ffffffffffffffffffffffffff8p0L, 1.2547671759429278005937024349858508569625e-461L),
    TEST_f_f (tgamma, -0xfa.000000000000000000000000008p0L, -1.2547671759429278005937024349855091492502e-461L),
    TEST_f_f (tgamma, -0x1f3.ffffffffffffffffffffffffffp0L, 1.6623061587520224800948170558276526986560e-1103L),
    TEST_f_f (tgamma, -0x1f4.00000000000000000000000001p0L, -1.6623061587520224800948170558266338608285e-1103L),
    TEST_f_f (tgamma, -0x2ed.fffffffffffffffffffffffffep0L, 3.9294757797334687313030998663853566763041e-1802L),
    TEST_f_f (tgamma, -0x2ee.00000000000000000000000002p0L, -3.9294757797334687313030998663802259177992e-1802L),
    TEST_f_f (tgamma, -0x3e7.fffffffffffffffffffffffffep0L, 2.5202599107841713834679953735597552855978e-2537L),
    TEST_f_f (tgamma, -0x3e8.00000000000000000000000002p0L, -2.5202599107841713834679953735563216504503e-2537L),
    TEST_f_f (tgamma, -0x4e1.fffffffffffffffffffffffffcp0L, 3.0766052944457032773369415396747635242705e-3300L),
    TEST_f_f (tgamma, -0x4e2.00000000000000000000000004p0L, -3.0766052944457032773369415396661096445778e-3300L),
    TEST_f_f (tgamma, -0x5db.fffffffffffffffffffffffffcp0L, 1.0537416297875703295453200836588944487917e-4084L),
    TEST_f_f (tgamma, -0x5dc.00000000000000000000000004p0L, -1.0537416297875703295453200836558547326520e-4084L),
    TEST_f_f (tgamma, -0x6d5.fffffffffffffffffffffffffcp0L, 2.4165069481061837867659591369998698607387e-4887L),
    TEST_f_f (tgamma, -0x6d6.00000000000000000000000004p0L, -2.4165069481061837867659591369927521090377e-4887L),
    TEST_f_f (tgamma, -0x6e2.fffffffffffffffffffffffffcp0L, -1.5891611922804672355414813803008070609431e-4929L),
    TEST_f_f (tgamma, -0x6e3.00000000000000000000000004p0L, 1.5891611922804672355414813802961215945667e-4929L),
    TEST_f_f (tgamma, -0x6e3.fffffffffffffffffffffffffcp0L, 9.0088502963745308137272187091882496519917e-4933L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e4.00000000000000000000000004p0L, -9.0088502963745308137272187091616860368392e-4933L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e4.fffffffffffffffffffffffffcp0L, -5.1041644738665896961627301468488672286746e-4936L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e5.00000000000000000000000004p0L, 5.1041644738665896961627301468338158811446e-4936L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e5.fffffffffffffffffffffffffcp0L, 2.8902403589278537350864836618623260468143e-4939L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6e6.00000000000000000000000004p0L, -2.8902403589278537350864836618538025544424e-4939L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6eb.fffffffffffffffffffffffffcp0L, 9.4152481960798287090819640211017219042636e-4959L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6ec.00000000000000000000000004p0L, -9.4152481960798287090819640210739431797168e-4959L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6ec.fffffffffffffffffffffffffcp0L, -5.3103486723518492437010513373388172315540e-4962L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6ed.00000000000000000000000004p0L, 5.3103486723518492437010513373231484108489e-4962L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6ed.fffffffffffffffffffffffffcp0L, 2.9934321715624854812294539669328172615019e-4965L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6ee.00000000000000000000000004p0L, -2.9934321715624854812294539669239841152754e-4965L, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6ee.fffffffffffffffffffffffffcp0L, minus_zero, UNDERFLOW_EXCEPTION),
    TEST_f_f (tgamma, -0x6ef.00000000000000000000000004p0L, plus_zero, UNDERFLOW_EXCEPTION),
# endif
#endif
    TEST_f_f (tgamma, -0x1.0a32a2p+5L, 1.8125267978155035272941154746083439329912e-37L),
#ifndef TEST_FLOAT
    TEST_f_f (tgamma, -0x1.5800000080001p+7L, -3.1439271448823567326093363350637118195240e-304L, UNDERFLOW_EXCEPTION_LDOUBLE_IBM),
#endif
    TEST_f_f (tgamma, 18.5L, 1.4986120533153361177371791123515513270334e+15L),
    TEST_f_f (tgamma, 19.5L, 2.7724322986333718178137813578503699550119e+16L),
    TEST_f_f (tgamma, 23.5L, 5.3613035875444147334274983856108155717836e+21L),
    TEST_f_f (tgamma, 29.5L, 1.6348125198274266444378807806868221866931e+30L),
    TEST_f_f (tgamma, 30.5L, 4.8226969334909086010917483030261254507447e+31L),
    TEST_f_f (tgamma, 31.5L, 1.4709225647147271233329832324229682624771e+33L),
    TEST_f_f (tgamma, 32.5L, 4.6334060788513904384988971821323500268029e+34L),
    TEST_f_f (tgamma, 33.5L, 1.5058569756267018925121415841930137587110e+36L),
    TEST_f_f (tgamma, 34.5L, 5.0446208683494513399156743070465960916817e+37L),
    TEST_f_f (tgamma, 0x2.30a43cp+4L, 3.4027979115654976101247558405326779640190e+38L),
#ifdef TEST_FLOAT
    TEST_f_f (tgamma, 0x2.30a44p+4L, plus_infty, OVERFLOW_EXCEPTION),
#else
    TEST_f_f (tgamma, 0x2.30a44p+4L, 3.4028438913396451054667218138127983367181e+38L),
#endif
#ifndef TEST_FLOAT
    TEST_f_f (tgamma, 0xa.b9fd72b0fb238p+4L, 1.7976931348622298700886249281842651388250e+308L),
# if !defined TEST_LDOUBLE || LDBL_MAX_EXP <= 1024
    TEST_f_f (tgamma, 0xa.b9fd72b0fb24p+4L, plus_infty, OVERFLOW_EXCEPTION),
# else
    TEST_f_f (tgamma, 0xa.b9fd72b0fb24p+4L, 1.7976931348624926129589466917917870782484e+308L),
# endif
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 106
    TEST_f_f (tgamma, 0xa.b9fd72b0fb23a9ddbf0d3804f4p+4L, 1.7976931348623158079372897140307719334857e+308L),
# if LDBL_MAX_EXP <= 1024
    TEST_f_f (tgamma, 0xa.b9fd72b0fb23a9ddbf0d3804f8p+4L, plus_infty, OVERFLOW_EXCEPTION),
# else
    TEST_f_f (tgamma, 0xa.b9fd72b0fb23a9ddbf0d3804f8p+4L, 1.7976931348623158079372897140599422519044e+308L),
# endif
#endif
#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_f_f (tgamma, 0x6.db8c603359a97108p+8L, 1.1897314953572317517071551278058233700012e+4932L),
    TEST_f_f (tgamma, 0x6.db8c603359a9711p+8L, plus_infty, OVERFLOW_EXCEPTION),
# if LDBL_MANT_DIG >= 113
    TEST_f_f (tgamma, 0x6.db8c603359a971081bc4a2e9dfdp+8L, 1.1897314953572317650857593266265995494998e+4932L),
    TEST_f_f (tgamma, 0x6.db8c603359a971081bc4a2e9dfd4p+8L, plus_infty, OVERFLOW_EXCEPTION),
# endif
#endif
    END_DATA (tgamma)
  };

static void
tgamma_test (void)
{
  errno = 0;
  FUNC(tgamma) (1);
  if (errno == ENOSYS)
    /* Function not implemented.  */
    return;

  START (tgamma);
  RUN_TEST_LOOP_f_f (tgamma, tgamma_test_data, );
  END (tgamma);
}


static const struct test_f_f_data trunc_test_data[] =
  {
    START_DATA (trunc),
    TEST_f_f (trunc, plus_infty, plus_infty),
    TEST_f_f (trunc, minus_infty, minus_infty),
    TEST_f_f (trunc, qnan_value, qnan_value),

    TEST_f_f (trunc, 0, 0),
    TEST_f_f (trunc, minus_zero, minus_zero),
    TEST_f_f (trunc, 0.1, 0),
    TEST_f_f (trunc, 0.25, 0),
    TEST_f_f (trunc, 0.625, 0),
    TEST_f_f (trunc, -0.1, minus_zero),
    TEST_f_f (trunc, -0.25, minus_zero),
    TEST_f_f (trunc, -0.625, minus_zero),
    TEST_f_f (trunc, 1, 1),
    TEST_f_f (trunc, -1, -1),
    TEST_f_f (trunc, 1.625, 1),
    TEST_f_f (trunc, -1.625, -1),

    TEST_f_f (trunc, 1048580.625L, 1048580L),
    TEST_f_f (trunc, -1048580.625L, -1048580L),

    TEST_f_f (trunc, 8388610.125L, 8388610.0L),
    TEST_f_f (trunc, -8388610.125L, -8388610.0L),

    TEST_f_f (trunc, 4294967296.625L, 4294967296.0L),
    TEST_f_f (trunc, -4294967296.625L, -4294967296.0L),

#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (trunc, 4503599627370495.5L, 4503599627370495.0L),
    TEST_f_f (trunc, 4503599627370496.25L, 4503599627370496.0L),
    TEST_f_f (trunc, 4503599627370496.5L, 4503599627370496.0L),
    TEST_f_f (trunc, 4503599627370496.75L, 4503599627370496.0L),
    TEST_f_f (trunc, 4503599627370497.5L, 4503599627370497.0L),

# if LDBL_MANT_DIG > 100
    TEST_f_f (trunc, 4503599627370494.5000000000001L, 4503599627370494.0L),
    TEST_f_f (trunc, 4503599627370495.5000000000001L, 4503599627370495.0L),
    TEST_f_f (trunc, 4503599627370496.5000000000001L, 4503599627370496.0L),
# endif

    TEST_f_f (trunc, -4503599627370495.5L, -4503599627370495.0L),
    TEST_f_f (trunc, -4503599627370496.25L, -4503599627370496.0L),
    TEST_f_f (trunc, -4503599627370496.5L, -4503599627370496.0L),
    TEST_f_f (trunc, -4503599627370496.75L, -4503599627370496.0L),
    TEST_f_f (trunc, -4503599627370497.5L, -4503599627370497.0L),

# if LDBL_MANT_DIG > 100
    TEST_f_f (trunc, -4503599627370494.5000000000001L, -4503599627370494.0L),
    TEST_f_f (trunc, -4503599627370495.5000000000001L, -4503599627370495.0L),
    TEST_f_f (trunc, -4503599627370496.5000000000001L, -4503599627370496.0L),
# endif

    TEST_f_f (trunc, 9007199254740991.5L, 9007199254740991.0L),
    TEST_f_f (trunc, 9007199254740992.25L, 9007199254740992.0L),
    TEST_f_f (trunc, 9007199254740992.5L, 9007199254740992.0L),
    TEST_f_f (trunc, 9007199254740992.75L, 9007199254740992.0L),
    TEST_f_f (trunc, 9007199254740993.5L, 9007199254740993.0L),

# if LDBL_MANT_DIG > 100
    TEST_f_f (trunc, 9007199254740991.0000000000001L, 9007199254740991.0L),
    TEST_f_f (trunc, 9007199254740992.0000000000001L, 9007199254740992.0L),
    TEST_f_f (trunc, 9007199254740993.0000000000001L, 9007199254740993.0L),
    TEST_f_f (trunc, 9007199254740991.5000000000001L, 9007199254740991.0L),
    TEST_f_f (trunc, 9007199254740992.5000000000001L, 9007199254740992.0L),
    TEST_f_f (trunc, 9007199254740993.5000000000001L, 9007199254740993.0L),
# endif

    TEST_f_f (trunc, -9007199254740991.5L, -9007199254740991.0L),
    TEST_f_f (trunc, -9007199254740992.25L, -9007199254740992.0L),
    TEST_f_f (trunc, -9007199254740992.5L, -9007199254740992.0L),
    TEST_f_f (trunc, -9007199254740992.75L, -9007199254740992.0L),
    TEST_f_f (trunc, -9007199254740993.5L, -9007199254740993.0L),

# if LDBL_MANT_DIG > 100
    TEST_f_f (trunc, -9007199254740991.0000000000001L, -9007199254740991.0L),
    TEST_f_f (trunc, -9007199254740992.0000000000001L, -9007199254740992.0L),
    TEST_f_f (trunc, -9007199254740993.0000000000001L, -9007199254740993.0L),
    TEST_f_f (trunc, -9007199254740991.5000000000001L, -9007199254740991.0L),
    TEST_f_f (trunc, -9007199254740992.5000000000001L, -9007199254740992.0L),
    TEST_f_f (trunc, -9007199254740993.5000000000001L, -9007199254740993.0L),
# endif

    TEST_f_f (trunc, 72057594037927935.5L, 72057594037927935.0L),
    TEST_f_f (trunc, 72057594037927936.25L, 72057594037927936.0L),
    TEST_f_f (trunc, 72057594037927936.5L, 72057594037927936.0L),
    TEST_f_f (trunc, 72057594037927936.75L, 72057594037927936.0L),
    TEST_f_f (trunc, 72057594037927937.5L, 72057594037927937.0L),

    TEST_f_f (trunc, -72057594037927935.5L, -72057594037927935.0L),
    TEST_f_f (trunc, -72057594037927936.25L, -72057594037927936.0L),
    TEST_f_f (trunc, -72057594037927936.5L, -72057594037927936.0L),
    TEST_f_f (trunc, -72057594037927936.75L, -72057594037927936.0L),
    TEST_f_f (trunc, -72057594037927937.5L, -72057594037927937.0L),

    TEST_f_f (trunc, 10141204801825835211973625643007.5L, 10141204801825835211973625643007.0L),
    TEST_f_f (trunc, 10141204801825835211973625643008.25L, 10141204801825835211973625643008.0L),
    TEST_f_f (trunc, 10141204801825835211973625643008.5L, 10141204801825835211973625643008.0L),
    TEST_f_f (trunc, 10141204801825835211973625643008.75L, 10141204801825835211973625643008.0L),
    TEST_f_f (trunc, 10141204801825835211973625643009.5L, 10141204801825835211973625643009.0L),
#endif
    END_DATA (trunc)
  };

static void
trunc_test (void)
{
  START (trunc);
  RUN_TEST_LOOP_f_f (trunc, trunc_test_data, );
  END (trunc);
}

static const struct test_f_f_data y0_test_data[] =
  {
    START_DATA (y0),
    /* y0 is the Bessel function of the second kind of order 0 */
    TEST_f_f (y0, -1.0, minus_infty, INVALID_EXCEPTION),
    TEST_f_f (y0, -max_value, minus_infty, INVALID_EXCEPTION),
    TEST_f_f (y0, 0.0, minus_infty),
    TEST_f_f (y0, qnan_value, qnan_value),
    TEST_f_f (y0, plus_infty, 0),

    TEST_f_f (y0, 0.125L, -1.38968062514384052915582277745018693L),
    TEST_f_f (y0, 0.75L, -0.137172769385772397522814379396581855L),
    TEST_f_f (y0, 1.0, 0.0882569642156769579829267660235151628L),
    TEST_f_f (y0, 1.5, 0.382448923797758843955068554978089862L),
    TEST_f_f (y0, 2.0, 0.510375672649745119596606592727157873L),
    TEST_f_f (y0, 8.0, 0.223521489387566220527323400498620359L),
    TEST_f_f (y0, 10.0, 0.0556711672835993914244598774101900481L),

    TEST_f_f (y0, 0x1.3ffp+74L, 1.818984347516051243459467456433028748678e-12L),

#ifndef TEST_FLOAT
    TEST_f_f (y0, 0x1.ff00000000002p+840L, 1.846591691699331493194965158699937660696e-127L),
    TEST_f_f (y0, 0x1p1023L, 8.2687542933709649327986678723012001545638e-155L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_f_f (y0, 0x1p16382L, 8.0839224448726336195866026476176740513439e-2467L),
    TEST_f_f (y0, 0x1p16383L, -3.8895531955766020648617743624167352352217e-2467L),
#endif

    TEST_f_f (y0, 0x1p-10L, -4.4865150767109739412411806297168793661098L),
    TEST_f_f (y0, 0x1p-20L, -8.8992283012125827603076426611387876938160L),
    TEST_f_f (y0, 0x1p-30L, -1.3311940304267782826037118027401817264906e+1L),
    TEST_f_f (y0, 0x1p-40L, -1.7724652307320814696990854700366226762563e+1L),
    TEST_f_f (y0, 0x1p-50L, -2.2137364310373846564919987139743760738155e+1L),
    TEST_f_f (y0, 0x1p-60L, -2.6550076313426878432849115782108205929120e+1L),
    TEST_f_f (y0, 0x1p-70L, -3.0962788316479910300778244424468159753887e+1L),
    TEST_f_f (y0, 0x1p-80L, -3.5375500319532942168707373066828113573541e+1L),
    TEST_f_f (y0, 0x1p-90L, -3.9788212322585974036636501709188067393195e+1L),
    TEST_f_f (y0, 0x1p-100L, -4.420092432563900590456563035154802121284e+1L),
    TEST_f_f (y0, 0x1p-110L, -4.861363632869203777249475899390797503250e+1L),
    END_DATA (y0)
  };

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

  START (y0);
  RUN_TEST_LOOP_f_f (y0, y0_test_data, );
  END (y0);
}


static const struct test_f_f_data y1_test_data[] =
  {
    START_DATA (y1),
    /* y1 is the Bessel function of the second kind of order 1 */
    TEST_f_f (y1, -1.0, minus_infty, INVALID_EXCEPTION),
    TEST_f_f (y1, -max_value, minus_infty, INVALID_EXCEPTION),
    TEST_f_f (y1, 0.0, minus_infty),
    TEST_f_f (y1, plus_infty, 0),
    TEST_f_f (y1, qnan_value, qnan_value),

    TEST_f_f (y1, 0.125L, -5.19993611253477499595928744876579921L),
    TEST_f_f (y1, 0.75L, -1.03759455076928541973767132140642198L),
    TEST_f_f (y1, 1.0, -0.781212821300288716547150000047964821L),
    TEST_f_f (y1, 1.5, -0.412308626973911295952829820633445323L),
    TEST_f_f (y1, 2.0, -0.107032431540937546888370772277476637L),
    TEST_f_f (y1, 8.0, -0.158060461731247494255555266187483550L),
    TEST_f_f (y1, 10.0, 0.249015424206953883923283474663222803L),

    TEST_f_f (y1, 0x1.27e204p+99L, -8.881610148467797208469612080785210013461e-16L),

#ifndef TEST_FLOAT
    TEST_f_f (y1, 0x1.001000001p+593L, 3.927269966354206207832593635798954916263e-90L),
    TEST_f_f (y1, 0x1p1023L, 1.5665258060609012834424478437196679802783e-155L),
#endif

#if defined TEST_LDOUBLE && LDBL_MAX_EXP >= 16384
    TEST_f_f (y1, 0x1p16382L, 1.2193782500509000574176799046642541129387e-2466L),
    TEST_f_f (y1, 0x1p16383L, -9.5859502826270374691362975419147645151233e-2467L),
#endif

    TEST_f_f (y1, 0x1p-10L, -6.5190099301063115047395187618929589514382e+02L),
    TEST_f_f (y1, 0x1p-20L, -6.6754421443450423911167962313100637952285e+05L),
    TEST_f_f (y1, 0x1p-30L, -6.8356527557643159612937462812258975438856e+08L),
    TEST_f_f (y1, 0x1p-40L, -6.9997084219026594793707805777425993547887e+11L),
    TEST_f_f (y1, 0x1p-50L, -7.1677014240283233068755952926181262431559e+14L),
    TEST_f_f (y1, 0x1p-60L, -7.3397262582050030662406095795388448059822e+17L),
    TEST_f_f (y1, 0x1p-70L, -7.5158796884019231398303842094477769620063e+20L),
    TEST_f_f (y1, 0x1p-80L, -7.6962608009235692951863134304745236090943e+23L),
    TEST_f_f (y1, 0x1p-90L, -7.8809710601457349582707849528059121757126e+26L),
    TEST_f_f (y1, 0x1p-100L, -8.0701143655892325972692837916732540679297e+29L),
    TEST_f_f (y1, 0x1p-110L, -8.2637971103633741796037466026734121655600e+32L),
    END_DATA (y1)
  };

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

  START (y1);
  RUN_TEST_LOOP_f_f (y1, y1_test_data, );
  END (y1);
}


static const struct test_if_f_data yn_test_data[] =
  {
    START_DATA (yn),
    /* yn is the Bessel function of the second kind of order n */
    /* yn (0, x) == y0 (x)  */
    TEST_if_f (yn, 0, -1.0, minus_infty, INVALID_EXCEPTION),
    TEST_if_f (yn, 0, -max_value, minus_infty, INVALID_EXCEPTION),
    TEST_if_f (yn, 0, 0.0, minus_infty),
    TEST_if_f (yn, 0, qnan_value, qnan_value),
    TEST_if_f (yn, 0, plus_infty, 0),

    TEST_if_f (yn, 0, 0.125L, -1.38968062514384052915582277745018693L),
    TEST_if_f (yn, 0, 0.75L, -0.137172769385772397522814379396581855L),
    TEST_if_f (yn, 0, 1.0, 0.0882569642156769579829267660235151628L),
    TEST_if_f (yn, 0, 1.5, 0.382448923797758843955068554978089862L),
    TEST_if_f (yn, 0, 2.0, 0.510375672649745119596606592727157873L),
    TEST_if_f (yn, 0, 8.0, 0.223521489387566220527323400498620359L),
    TEST_if_f (yn, 0, 10.0, 0.0556711672835993914244598774101900481L),

    /* yn (1, x) == y1 (x)  */
    TEST_if_f (yn, 1, -1.0, minus_infty, INVALID_EXCEPTION),
    TEST_if_f (yn, 1, 0.0, minus_infty),
    TEST_if_f (yn, 1, plus_infty, 0),
    TEST_if_f (yn, 1, qnan_value, qnan_value),

    TEST_if_f (yn, 1, 0.125L, -5.19993611253477499595928744876579921L),
    TEST_if_f (yn, 1, 0.75L, -1.03759455076928541973767132140642198L),
    TEST_if_f (yn, 1, 1.0, -0.781212821300288716547150000047964821L),
    TEST_if_f (yn, 1, 1.5, -0.412308626973911295952829820633445323L),
    TEST_if_f (yn, 1, 2.0, -0.107032431540937546888370772277476637L),
    TEST_if_f (yn, 1, 8.0, -0.158060461731247494255555266187483550L),
    TEST_if_f (yn, 1, 10.0, 0.249015424206953883923283474663222803L),

    /* yn (3, x)  */
    TEST_if_f (yn, 3, plus_infty, 0),
    TEST_if_f (yn, 3, qnan_value, qnan_value),

    TEST_if_f (yn, 3, 0.125L, -2612.69757350066712600220955744091741L),
    TEST_if_f (yn, 3, 0.75L, -12.9877176234475433186319774484809207L),
    TEST_if_f (yn, 3, 1.0, -5.82151760596472884776175706442981440L),
    TEST_if_f (yn, 3, 2.0, -1.12778377684042778608158395773179238L),
    TEST_if_f (yn, 3, 10.0, -0.251362657183837329779204747654240998L),

    /* yn (10, x)  */
    TEST_if_f (yn, 10, plus_infty, 0),
    TEST_if_f (yn, 10, qnan_value, qnan_value),

    TEST_if_f (yn, 10, 0.125L, -127057845771019398.252538486899753195L),
    TEST_if_f (yn, 10, 0.75L, -2133501638.90573424452445412893839236L),
    TEST_if_f (yn, 10, 1.0, -121618014.278689189288130426667971145L),
    TEST_if_f (yn, 10, 2.0, -129184.542208039282635913145923304214L),
    TEST_if_f (yn, 10, 10.0, -0.359814152183402722051986577343560609L),

    /* Check whether yn returns correct value for LDBL_MIN, DBL_MIN,
       and FLT_MIN.  See Bug 14173.  */
    TEST_if_f (yn, 10, min_value, minus_infty, OVERFLOW_EXCEPTION),

    TEST_if_f (yn, 10, min_value, minus_infty, OVERFLOW_EXCEPTION|ERRNO_ERANGE),
    END_DATA (yn)
  };

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

  START (yn);
  RUN_TEST_LOOP_if_f (yn, yn_test_data, );
  END (yn);
}


static const struct test_f_f_data significand_test_data[] =
  {
    START_DATA (significand),
    /* significand returns the mantissa of the exponential representation.  */
    /* TODO: missing +/-Inf as well as qNaN tests.  */
    TEST_f_f (significand, 4.0, 1.0),
    TEST_f_f (significand, 6.0, 1.5),
    TEST_f_f (significand, 8.0, 1.0),
    END_DATA (significand)
  };

static void
significand_test (void)
{
  START (significand);
  RUN_TEST_LOOP_f_f (significand, significand_test_data, );
  END (significand);
}


static void
initialize (void)
{
  fpstack_test ("start *init*");

  /* Clear all exceptions.  From now on we must not get random exceptions.  */
  feclearexcept (FE_ALL_EXCEPT);
  errno = 0;

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
  { "output-dir", 'o', "DIR", 0,
    "Directory where generated files will be placed"},
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
    case 'o':
      output_dir = (char *) malloc (strlen (arg) + 1);
      if (output_dir != NULL)
	strcpy (output_dir, arg);
      else
        return errno;
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
  char *ulps_file_path;
  size_t dir_len = 0;

  verbose = 1;
  output_ulps = 0;
  output_max_error = 1;
  output_points = 1;
  output_dir = NULL;
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
      if (output_dir != NULL)
	dir_len = strlen (output_dir);
      ulps_file_path = (char *) malloc (dir_len + strlen (ulps_file_name) + 1);
      if (ulps_file_path == NULL)
        {
	  perror ("can't allocate path for `ULPs' file: ");
	  exit (1);
        }
      sprintf (ulps_file_path, "%s%s", output_dir == NULL ? "" : output_dir, ulps_file_name);
      ulps_file = fopen (ulps_file_path, "a");
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
  finite_test ();
  fpclassify_test ();
  isfinite_test ();
  isinf_test ();
  isnan_test ();
  isnormal_test ();
  issignaling_test ();
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
  fma_test_towardzero ();
  fma_test_downward ();
  fma_test_upward ();

  /* Comparison macros:  */
  isgreater_test ();
  isgreaterequal_test ();
  isless_test ();
  islessequal_test ();
  islessgreater_test ();
  isunordered_test ();

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
  ctan_test_tonearest ();
  ctan_test_towardzero ();
  ctan_test_downward ();
  ctan_test_upward ();
  ctanh_test ();
  ctanh_test_tonearest ();
  ctanh_test_towardzero ();
  ctanh_test_downward ();
  ctanh_test_upward ();

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
  printf ("  %d test cases plus %d tests for exception flags and\n"
	  "    %d tests for errno executed.\n",
	  noTests, noExcTests, noErrnoTests);
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
