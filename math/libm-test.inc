/* Copyright (C) 1997-2014 Free Software Foundation, Inc.
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
   cbrt, ceil, copysign, cos, cosh, drem, erf, erfc, exp, exp10, exp2, expm1,
   fabs, fdim, finite, floor, fma, fmax, fmin, fmod, fpclassify,
   frexp, gamma, hypot,
   ilogb, isfinite, isinf, isnan, isnormal, issignaling,
   isless, islessequal, isgreater, isgreaterequal, islessgreater, isunordered,
   j0, j1, jn,
   ldexp, lgamma, log, log10, log1p, log2, logb,
   modf, nearbyint, nextafter, nexttoward,
   pow, pow10, remainder, remquo, rint, lrint, llrint,
   round, lround, llround,
   scalb, scalbn, scalbln, signbit, sin, sincos, sinh, sqrt, tan, tanh, tgamma, trunc,
   y0, y1, yn, significand

   and for the following complex math functions:
   cabs, cacos, cacosh, carg, casin, casinh, catan, catanh,
   ccos, ccosh, cexp, cimag, clog, clog10, conj, cpow, cproj, creal,
   csin, csinh, csqrt, ctan, ctanh.

   At the moment the following functions and macros aren't tested:
   lgamma_r,
   nan.

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
#include <math-tests.h>

/* Structure for ulp data for a test, a function, or the real or
   imaginary part of a function.  */
struct ulp_data
{
  const char *name;
  FLOAT max_ulp;
};

/* This header defines test_ulps, func_ulps, func_real_ulps and
   func_imag_ulps arrays.  */
#include "libm-test-ulps.h"

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
#define INEXACT_EXCEPTION		0x10
/* The next flags signals that those exceptions are allowed but not required.   */
#define INVALID_EXCEPTION_OK		0x20
#define DIVIDE_BY_ZERO_EXCEPTION_OK	0x40
#define OVERFLOW_EXCEPTION_OK		0x80
#define UNDERFLOW_EXCEPTION_OK		0x100
/* For "inexact" exceptions, the default is allowed but not required
   unless INEXACT_EXCEPTION or NO_INEXACT_EXCEPTION is specified.  */
#define NO_INEXACT_EXCEPTION		0x200
#define EXCEPTIONS_OK INVALID_EXCEPTION_OK+DIVIDE_BY_ZERO_EXCEPTION_OK
/* Some special test flags, passed together with exceptions.  */
#define IGNORE_ZERO_INF_SIGN		0x400
#define TEST_NAN_SIGN			0x800
#define NO_TEST_INLINE			0x1000
#define XFAIL_TEST			0x2000
/* Indicate errno settings required or disallowed.  */
#define ERRNO_UNCHANGED			0x4000
#define ERRNO_EDOM			0x8000
#define ERRNO_ERANGE			0x10000
/* Flags generated by gen-libm-test.pl, not entered here manually.  */
#define IGNORE_RESULT			0x20000

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

/* Inline tests disabled for particular types.  */
#ifdef TEST_FLOAT
# define NO_TEST_INLINE_FLOAT	NO_TEST_INLINE
#else
# define NO_TEST_INLINE_FLOAT	0
#endif
#ifdef TEST_DOUBLE
# define NO_TEST_INLINE_DOUBLE	NO_TEST_INLINE
#else
# define NO_TEST_INLINE_DOUBLE	0
#endif

/* Conditions used by tests generated by gen-auto-libm-tests.c.  */
#ifdef TEST_FLOAT
# define TEST_COND_flt_32	1
#else
# define TEST_COND_flt_32	0
#endif
#if defined TEST_DOUBLE || (defined TEST_LDOUBLE && LDBL_MANT_DIG == 53)
# define TEST_COND_dbl_64	1
#else
# define TEST_COND_dbl_64	0
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG == 64 && LDBL_MIN_EXP == -16381
# define TEST_COND_ldbl_96_intel	1
#else
# define TEST_COND_ldbl_96_intel	0
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG == 64 && LDBL_MIN_EXP == -16382
# define TEST_COND_ldbl_96_m68k	1
#else
# define TEST_COND_ldbl_96_m68k	0
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG == 113
# define TEST_COND_ldbl_128	1
#else
# define TEST_COND_ldbl_128	0
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG == 106
# define TEST_COND_ldbl_128ibm	1
#else
# define TEST_COND_ldbl_128ibm	0
#endif
#if LONG_MAX == 0x7fffffff
# define TEST_COND_long32	1
# define TEST_COND_long64	0
#else
# define TEST_COND_long32	0
# define TEST_COND_long64	1
#endif
#define TEST_COND_before_rounding	(!TININESS_AFTER_ROUNDING)
#define TEST_COND_after_rounding	TININESS_AFTER_ROUNDING

#ifdef __x86_64__
# define TEST_COND_x86_64	1
#else
# define TEST_COND_x86_64	0
#endif

#ifdef __i386__
# define TEST_COND_x86		1
#else
# define TEST_COND_x86		0
#endif

/* Various constants (we must supply them precalculated for accuracy).  */
#define M_PI_6l			.52359877559829887307710723054658383L
#define M_PI_34l		2.356194490192344928846982537459627163L	/* 3*pi/4 */
#define M_PI_34_LOG10El		1.023282265381381010614337719073516828L
#define M_PI2_LOG10El		0.682188176920920673742891812715677885L
#define M_PI4_LOG10El		0.341094088460460336871445906357838943L
#define M_PI_LOG10El		1.364376353841841347485783625431355770L

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

#define plus_zero	CHOOSE (0.0L, 0.0, 0.0f,	\
				0.0L, 0.0, 0.0f)
#define minus_zero	CHOOSE (-0.0L, -0.0, -0.0f,	\
				-0.0L, -0.0, -0.0f)
#define plus_infty	CHOOSE (HUGE_VALL, HUGE_VAL, HUGE_VALF, \
				HUGE_VALL, HUGE_VAL, HUGE_VALF)
#define minus_infty	CHOOSE (-HUGE_VALL, -HUGE_VAL, -HUGE_VALF,	\
				-HUGE_VALL, -HUGE_VAL, -HUGE_VALF)
#define qnan_value	FUNC (__builtin_nan) ("")
#define max_value	CHOOSE (LDBL_MAX, DBL_MAX, FLT_MAX,	\
				LDBL_MAX, DBL_MAX, FLT_MAX)
#define min_value	CHOOSE (LDBL_MIN, DBL_MIN, FLT_MIN,	\
				LDBL_MIN, DBL_MIN, FLT_MIN)
#define min_subnorm_value	CHOOSE (__LDBL_DENORM_MIN__,	\
					__DBL_DENORM_MIN__,	\
					__FLT_DENORM_MIN__,	\
					__LDBL_DENORM_MIN__,	\
					__DBL_DENORM_MIN__,	\
					__FLT_DENORM_MIN__)

static FLOAT max_error, real_max_error, imag_max_error;


#define BUILD_COMPLEX(real, imag) \
  ({ __complex__ FLOAT __retval;					      \
     __real__ __retval = (real);					      \
     __imag__ __retval = (imag);					      \
     __retval; })

#define MANT_DIG CHOOSE ((LDBL_MANT_DIG-1), (DBL_MANT_DIG-1), (FLT_MANT_DIG-1),  \
			 (LDBL_MANT_DIG-1), (DBL_MANT_DIG-1), (FLT_MANT_DIG-1))
#define MIN_EXP CHOOSE ((LDBL_MIN_EXP-1), (DBL_MIN_EXP-1), (FLT_MIN_EXP-1),	\
			(LDBL_MIN_EXP-1), (DBL_MIN_EXP-1), (FLT_MIN_EXP-1))

/* Compare KEY (a string, with the name of a test or a function) with
   ULP (a pointer to a struct ulp_data structure), returning a value
   less than, equal to or greater than zero for use in bsearch.  */

static int
compare_ulp_data (const void *key, const void *ulp)
{
  const char *keystr = key;
  const struct ulp_data *ulpdat = ulp;
  return strcmp (keystr, ulpdat->name);
}

/* Return the ulps for NAME in array DATA with NMEMB elements, or 0 if
   no ulps listed.  */

static FLOAT
find_ulps (const char *name, const struct ulp_data *data, size_t nmemb)
{
  const struct ulp_data *entry = bsearch (name, data, nmemb, sizeof (*data),
					  compare_ulp_data);
  if (entry == NULL)
    return 0;
  else
    return entry->max_ulp;
}

/* Return the ulps for test NAME.  */

static FLOAT
find_test_ulps (const char *name)
{
  return find_ulps (name, test_ulps,
		    sizeof (test_ulps) / sizeof (test_ulps[0]));
}

/* Return the ulps for real function NAME.  */

static FLOAT
find_function_ulps (const char *name)
{
  return find_ulps (name, func_ulps,
		    sizeof (func_ulps) / sizeof (func_ulps[0]));
}

/* Return the ulps for complex function NAME.  */

static __complex__ FLOAT
find_complex_function_ulps (const char *name)
{
  FLOAT ulp_real = find_ulps (name, func_real_ulps,
			      (sizeof (func_real_ulps)
			       / sizeof (func_real_ulps[0])));
  FLOAT ulp_imag = find_ulps (name, func_imag_ulps,
			      (sizeof (func_imag_ulps)
			       / sizeof (func_imag_ulps[0])));
  return BUILD_COMPLEX (ulp_real, ulp_imag);
}

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
print_max_error (const char *func_name)
{
  FLOAT allowed = find_function_ulps (func_name);
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
print_complex_max_error (const char *func_name)
{
  __complex__ FLOAT allowed = find_complex_function_ulps (func_name);
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
  if (EXCEPTION_TESTS (FLOAT))
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
	test_single_exception (test_name, exception,
			       INVALID_EXCEPTION, FE_INVALID,
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
#ifdef FE_INEXACT
      if ((exception & (INEXACT_EXCEPTION | NO_INEXACT_EXCEPTION)) != 0)
	test_single_exception (test_name, exception, INEXACT_EXCEPTION,
			       FE_INEXACT, "Inexact");
#endif
    }
  feclearexcept (FE_ALL_EXCEPT);
}

/* Test whether errno for TEST_NAME, set to ERRNO_VALUE, has value
   EXPECTED_VALUE (description EXPECTED_NAME).  */
static void
test_single_errno (const char *test_name, int errno_value,
		   int expected_value, const char *expected_name)
{
#ifndef TEST_INLINE
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
#endif
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

/* Returns the number of ulps that GIVEN is away from EXPECTED.  */
#define ULPDIFF(given, expected) \
	(FUNC(fabs) ((given) - (expected)) / ulp (expected))

/* Returns the size of an ulp for VALUE.  */
static FLOAT
ulp (FLOAT value)
{
  FLOAT ulp;

  switch (fpclassify (value))
    {
      case FP_ZERO:
	/* We compute the distance to the next FP which is the same as the
	   value of the smallest subnormal number. Previously we used
	   2^(-MANT_DIG) which is too large a value to be useful. Note that we
	   can't use ilogb(0), since that isn't a valid thing to do. As a point
	   of comparison Java's ulp returns the next normal value e.g.
	   2^(1 - MAX_EXP) for ulp(0), but that is not what we want for
	   glibc.  */
	/* Fall through...  */
      case FP_SUBNORMAL:
        /* The next closest subnormal value is a constant distance away.  */
	ulp = FUNC(ldexp) (1.0, MIN_EXP - MANT_DIG);
	break;

      case FP_NORMAL:
	ulp = FUNC(ldexp) (1.0, FUNC(ilogb) (value) - MANT_DIG);
	break;

      default:
	/* It should never happen. */
	abort ();
	break;
    }
  return ulp;
}

static void
check_float_internal (const char *test_name, FLOAT computed, FLOAT expected,
		      int exceptions,
		      FLOAT *curr_max_error)
{
  int ok = 0;
  int print_diff = 0;
  FLOAT diff = 0;
  FLOAT ulps = 0;
  int errno_value = errno;

  test_exceptions (test_name, exceptions);
  test_errno (test_name, errno_value, exceptions);
  if (exceptions & IGNORE_RESULT)
    goto out;
  FLOAT max_ulp = find_test_ulps (test_name);
  if (issignaling (computed) && issignaling (expected))
    {
      if ((exceptions & TEST_NAN_SIGN) != 0
	  && signbit (computed) != signbit (expected))
	{
	  ok = 0;
	  printf ("signaling NaN has wrong sign.\n");
	}
      else
	ok = 1;
    }
  else if (issignaling (computed) || issignaling (expected))
    ok = 0;
  else if (isnan (computed) && isnan (expected))
    {
      if ((exceptions & TEST_NAN_SIGN) != 0
	  && signbit (computed) != signbit (expected))
	{
	  ok = 0;
	  printf ("quiet NaN has wrong sign.\n");
	}
      else
	ok = 1;
    }
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
      ulps = ULPDIFF (computed, expected);
      set_max_error (ulps, curr_max_error);
      print_diff = 1;
      if ((exceptions & IGNORE_ZERO_INF_SIGN) == 0
	  && computed == 0.0 && expected == 0.0
	  && signbit(computed) != signbit (expected))
	ok = 0;
      else if (ulps <= 0.5 || (ulps <= max_ulp && !ignore_max_ulp))
	ok = 1;
      else
	{
	  ok = 0;
	  print_ulps (test_name, ulps);
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
	  printf (" ulp       : % .4" PRINTF_NEXPR "\n", ulps);
	  printf (" max.ulp   : % .4" PRINTF_NEXPR "\n", max_ulp);
	}
    }
  update_stats (ok);

 out:
  fpstack_test (test_name);
  errno = 0;
}


static void
check_float (const char *test_name, FLOAT computed, FLOAT expected,
	     int exceptions)
{
  check_float_internal (test_name, computed, expected,
			exceptions, &max_error);
}


static void
check_complex (const char *test_name, __complex__ FLOAT computed,
	       __complex__ FLOAT expected,
	       int exception)
{
  FLOAT part_comp, part_exp;
  char *str;

  if (asprintf (&str, "Real part of: %s", test_name) == -1)
    abort ();

  part_comp = __real__ computed;
  part_exp = __real__ expected;

  check_float_internal (str, part_comp, part_exp,
			exception, &real_max_error);
  free (str);

  if (asprintf (&str, "Imaginary part of: %s", test_name) == -1)
    abort ();

  part_comp = __imag__ computed;
  part_exp = __imag__ expected;

  /* Don't check again for exceptions or errno, just pass through the
     other relevant flags.  */
  check_float_internal (str, part_comp, part_exp,
			exception & (IGNORE_ZERO_INF_SIGN
				     | TEST_NAN_SIGN
				     | IGNORE_RESULT),
			&imag_max_error);
  free (str);
}


/* Check that computed and expected values are equal (int values).  */
static void
check_int (const char *test_name, int computed, int expected,
	   int exceptions)
{
  int ok = 0;
  int errno_value = errno;

  test_exceptions (test_name, exceptions);
  test_errno (test_name, errno_value, exceptions);
  if (exceptions & IGNORE_RESULT)
    goto out;
  noTests++;
  if (computed == expected)
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
 out:
  fpstack_test (test_name);
  errno = 0;
}


/* Check that computed and expected values are equal (long int values).  */
static void
check_long (const char *test_name, long int computed, long int expected,
	    int exceptions)
{
  int ok = 0;
  int errno_value = errno;

  test_exceptions (test_name, exceptions);
  test_errno (test_name, errno_value, exceptions);
  if (exceptions & IGNORE_RESULT)
    goto out;
  noTests++;
  if (computed == expected)
    ok = 1;

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
 out:
  fpstack_test (test_name);
  errno = 0;
}


/* Check that computed value is true/false.  */
static void
check_bool (const char *test_name, int computed, int expected,
	    int exceptions)
{
  int ok = 0;
  int errno_value = errno;

  test_exceptions (test_name, exceptions);
  test_errno (test_name, errno_value, exceptions);
  if (exceptions & IGNORE_RESULT)
    goto out;
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
 out:
  fpstack_test (test_name);
  errno = 0;
}


/* check that computed and expected values are equal (long int values) */
static void
check_longlong (const char *test_name, long long int computed,
		long long int expected,
		int exceptions)
{
  int ok = 0;
  int errno_value = errno;

  test_exceptions (test_name, exceptions);
  test_errno (test_name, errno_value, exceptions);
  if (exceptions & IGNORE_RESULT)
    goto out;
  noTests++;
  if (computed == expected)
    ok = 1;

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
 out:
  fpstack_test (test_name);
  errno = 0;
}

/* Return whether a test with flags EXCEPTIONS should be run.  */
static int
enable_test (int exceptions)
{
  if (exceptions & XFAIL_TEST)
    return 0;
#ifdef TEST_INLINE
  if (exceptions & NO_TEST_INLINE)
    return 0;
#endif
  return 1;
}

/* Structures for each kind of test.  */
struct test_f_f_data
{
  const char *arg_str;
  FLOAT arg;
  FLOAT expected;
  int exceptions;
};
struct test_ff_f_data
{
  const char *arg_str;
  FLOAT arg1, arg2;
  FLOAT expected;
  int exceptions;
};
struct test_ff_f_data_nexttoward
{
  const char *arg_str;
  FLOAT arg1;
  long double arg2;
  FLOAT expected;
  int exceptions;
};
struct test_fi_f_data
{
  const char *arg_str;
  FLOAT arg1;
  int arg2;
  FLOAT expected;
  int exceptions;
};
struct test_fl_f_data
{
  const char *arg_str;
  FLOAT arg1;
  long int arg2;
  FLOAT expected;
  int exceptions;
};
struct test_if_f_data
{
  const char *arg_str;
  int arg1;
  FLOAT arg2;
  FLOAT expected;
  int exceptions;
};
struct test_fff_f_data
{
  const char *arg_str;
  FLOAT arg1, arg2, arg3;
  FLOAT expected;
  int exceptions;
};
struct test_c_f_data
{
  const char *arg_str;
  FLOAT argr, argc;
  FLOAT expected;
  int exceptions;
};
/* Used for both RUN_TEST_LOOP_f_f1 and RUN_TEST_LOOP_fI_f1.  */
struct test_f_f1_data
{
  const char *arg_str;
  FLOAT arg;
  FLOAT expected;
  int exceptions;
  int extra_test;
  int extra_expected;
};
struct test_fF_f1_data
{
  const char *arg_str;
  FLOAT arg;
  FLOAT expected;
  int exceptions;
  int extra_test;
  FLOAT extra_expected;
};
struct test_ffI_f1_data
{
  const char *arg_str;
  FLOAT arg1, arg2;
  FLOAT expected;
  int exceptions;
  int extra_test;
  int extra_expected;
};
struct test_c_c_data
{
  const char *arg_str;
  FLOAT argr, argc;
  FLOAT expr, expc;
  int exceptions;
};
struct test_cc_c_data
{
  const char *arg_str;
  FLOAT arg1r, arg1c, arg2r, arg2c;
  FLOAT expr, expc;
  int exceptions;
};
/* Used for all of RUN_TEST_LOOP_f_i, RUN_TEST_LOOP_f_i_tg,
   RUN_TEST_LOOP_f_b and RUN_TEST_LOOP_f_b_tg.  */
struct test_f_i_data
{
  const char *arg_str;
  FLOAT arg;
  int expected;
  int exceptions;
};
struct test_ff_i_data
{
  const char *arg_str;
  FLOAT arg1, arg2;
  int expected;
  int exceptions;
};
struct test_f_l_data
{
  const char *arg_str;
  FLOAT arg;
  long int expected;
  int exceptions;
};
struct test_f_L_data
{
  const char *arg_str;
  FLOAT arg;
  long long int expected;
  int exceptions;
};
struct test_fFF_11_data
{
  const char *arg_str;
  FLOAT arg;
  int exceptions;
  int extra1_test;
  FLOAT extra1_expected;
  int extra2_test;
  FLOAT extra2_expected;
};

/* Set the rounding mode, or restore the saved value.  */
#define IF_ROUND_INIT_	/* Empty.  */
#define IF_ROUND_INIT_FE_DOWNWARD		\
  int save_round_mode = fegetround ();		\
  if (ROUNDING_TESTS (FLOAT, FE_DOWNWARD)	\
      && fesetround (FE_DOWNWARD) == 0)
#define IF_ROUND_INIT_FE_TONEAREST		\
  int save_round_mode = fegetround ();		\
  if (ROUNDING_TESTS (FLOAT, FE_TONEAREST)	\
      && fesetround (FE_TONEAREST) == 0)
#define IF_ROUND_INIT_FE_TOWARDZERO		\
  int save_round_mode = fegetround ();		\
  if (ROUNDING_TESTS (FLOAT, FE_TOWARDZERO)	\
      && fesetround (FE_TOWARDZERO) == 0)
#define IF_ROUND_INIT_FE_UPWARD			\
  int save_round_mode = fegetround ();		\
  if (ROUNDING_TESTS (FLOAT, FE_UPWARD)		\
      && fesetround (FE_UPWARD) == 0)
#define ROUND_RESTORE_	/* Empty.  */
#define ROUND_RESTORE_FE_DOWNWARD		\
  fesetround (save_round_mode)
#define ROUND_RESTORE_FE_TONEAREST		\
  fesetround (save_round_mode)
#define ROUND_RESTORE_FE_TOWARDZERO		\
  fesetround (save_round_mode)
#define ROUND_RESTORE_FE_UPWARD			\
  fesetround (save_round_mode)

/* Common setup for an individual test.  */
#define COMMON_TEST_SETUP(ARG_STR)					\
  char *test_name;							\
  if (asprintf (&test_name, "%s (%s)", this_func, (ARG_STR)) == -1)	\
    abort ()

/* Setup for a test with an extra output.  */
#define EXTRA_OUTPUT_TEST_SETUP(ARG_STR, N)			\
  char *extra##N##_name;					\
  if (asprintf (&extra##N##_name, "%s (%s) extra output " #N,	\
		this_func, (ARG_STR)) == -1)			\
    abort ()

/* Common cleanup after an individual test.  */
#define COMMON_TEST_CLEANUP			\
  free (test_name)

/* Cleanup for a test with an extra output.  */
#define EXTRA_OUTPUT_TEST_CLEANUP(N)		\
  free (extra##N##_name)

/* Run an individual test, including any required setup and checking
   of results, or loop over all tests in an array.  */
#define RUN_TEST_f_f(ARG_STR, FUNC_NAME, ARG, EXPECTED,			\
		     EXCEPTIONS)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_float (test_name, FUNC (FUNC_NAME) (ARG), EXPECTED,	\
		     EXCEPTIONS);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_f ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].expected, (ARRAY)[i].exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_2_f(ARG_STR, FUNC_NAME, ARG1, ARG2, EXPECTED,	\
		     EXCEPTIONS)				\
  do								\
    if (enable_test (EXCEPTIONS))				\
      {								\
	COMMON_TEST_SETUP (ARG_STR);				\
	check_float (test_name, FUNC (FUNC_NAME) (ARG1, ARG2),	\
		     EXPECTED, EXCEPTIONS);			\
	COMMON_TEST_CLEANUP;					\
      }								\
  while (0)
#define RUN_TEST_LOOP_2_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_2_f ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg1,	\
		    (ARRAY)[i].arg2, (ARRAY)[i].expected,		\
		    (ARRAY)[i].exceptions);				\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_ff_f RUN_TEST_2_f
#define RUN_TEST_LOOP_ff_f RUN_TEST_LOOP_2_f
#define RUN_TEST_fi_f RUN_TEST_2_f
#define RUN_TEST_LOOP_fi_f RUN_TEST_LOOP_2_f
#define RUN_TEST_fl_f RUN_TEST_2_f
#define RUN_TEST_LOOP_fl_f RUN_TEST_LOOP_2_f
#define RUN_TEST_if_f RUN_TEST_2_f
#define RUN_TEST_LOOP_if_f RUN_TEST_LOOP_2_f
#define RUN_TEST_fff_f(ARG_STR, FUNC_NAME, ARG1, ARG2, ARG3,		\
		       EXPECTED, EXCEPTIONS)				\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_float (test_name, FUNC (FUNC_NAME) (ARG1, ARG2, ARG3),	\
		     EXPECTED, EXCEPTIONS);				\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_fff_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_fff_f ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg1,	\
		      (ARRAY)[i].arg2, (ARRAY)[i].arg3,			\
		      (ARRAY)[i].expected, (ARRAY)[i].exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_c_f(ARG_STR, FUNC_NAME, ARG1, ARG2, EXPECTED,		\
		     EXCEPTIONS)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_float (test_name,						\
		     FUNC (FUNC_NAME) (BUILD_COMPLEX (ARG1, ARG2)),	\
		     EXPECTED, EXCEPTIONS);				\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_c_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_c_f ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].argr,	\
		    (ARRAY)[i].argc, (ARRAY)[i].expected,		\
		    (ARRAY)[i].exceptions);				\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_f1(ARG_STR, FUNC_NAME, ARG, EXPECTED,		\
		      EXCEPTIONS, EXTRA_VAR, EXTRA_TEST,		\
		      EXTRA_EXPECTED)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	(EXTRA_VAR) = (EXTRA_EXPECTED) == 0 ? 1 : 0;			\
	check_float (test_name, FUNC (FUNC_NAME) (ARG), EXPECTED,	\
		     EXCEPTIONS);					\
	EXTRA_OUTPUT_TEST_SETUP (ARG_STR, 1);				\
	if (EXTRA_TEST)							\
	  check_int (extra1_name, EXTRA_VAR, EXTRA_EXPECTED, 0);	\
	EXTRA_OUTPUT_TEST_CLEANUP (1);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_f1(FUNC_NAME, ARRAY, ROUNDING_MODE, EXTRA_VAR)	\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_f1 ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		     (ARRAY)[i].expected, (ARRAY)[i].exceptions,	\
		     EXTRA_VAR, (ARRAY)[i].extra_test,			\
		     (ARRAY)[i].extra_expected);			\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_fF_f1(ARG_STR, FUNC_NAME, ARG, EXPECTED,		\
		       EXCEPTIONS, EXTRA_VAR, EXTRA_TEST,		\
		       EXTRA_EXPECTED)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	(EXTRA_VAR) = (EXTRA_EXPECTED) == 0 ? 1 : 0;			\
	check_float (test_name, FUNC (FUNC_NAME) (ARG, &(EXTRA_VAR)),	\
		     EXPECTED, EXCEPTIONS);				\
	EXTRA_OUTPUT_TEST_SETUP (ARG_STR, 1);				\
	if (EXTRA_TEST)							\
	  check_float (extra1_name, EXTRA_VAR, EXTRA_EXPECTED, 0);	\
	EXTRA_OUTPUT_TEST_CLEANUP (1);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_fF_f1(FUNC_NAME, ARRAY, ROUNDING_MODE, EXTRA_VAR)	\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_fF_f1 ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		      (ARRAY)[i].expected, (ARRAY)[i].exceptions,	\
		      EXTRA_VAR, (ARRAY)[i].extra_test,			\
		      (ARRAY)[i].extra_expected);			\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_fI_f1(ARG_STR, FUNC_NAME, ARG, EXPECTED,		\
		       EXCEPTIONS, EXTRA_VAR, EXTRA_TEST,		\
		       EXTRA_EXPECTED)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	(EXTRA_VAR) = (EXTRA_EXPECTED) == 0 ? 1 : 0;			\
	check_float (test_name, FUNC (FUNC_NAME) (ARG, &(EXTRA_VAR)),	\
		     EXPECTED, EXCEPTIONS);				\
	EXTRA_OUTPUT_TEST_SETUP (ARG_STR, 1);				\
	if (EXTRA_TEST)							\
	  check_int (extra1_name, EXTRA_VAR, EXTRA_EXPECTED, 0);	\
	EXTRA_OUTPUT_TEST_CLEANUP (1);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_fI_f1(FUNC_NAME, ARRAY, ROUNDING_MODE, EXTRA_VAR)	\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_fI_f1 ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		      (ARRAY)[i].expected, (ARRAY)[i].exceptions,	\
		      EXTRA_VAR, (ARRAY)[i].extra_test,			\
		      (ARRAY)[i].extra_expected);			\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_ffI_f1(ARG_STR, FUNC_NAME, ARG1, ARG2, EXPECTED,	\
			EXCEPTIONS, EXTRA_VAR, EXTRA_TEST,		\
			EXTRA_EXPECTED)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	(EXTRA_VAR) = (EXTRA_EXPECTED) == 0 ? 1 : 0;			\
	check_float (test_name,						\
		     FUNC (FUNC_NAME) (ARG1, ARG2, &(EXTRA_VAR)),	\
		     EXPECTED, EXCEPTIONS);				\
	EXTRA_OUTPUT_TEST_SETUP (ARG_STR, 1);				\
	if (EXTRA_TEST)							\
	  check_int (extra1_name, EXTRA_VAR, EXTRA_EXPECTED, 0);	\
	EXTRA_OUTPUT_TEST_CLEANUP (1);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_ffI_f1(FUNC_NAME, ARRAY, ROUNDING_MODE,		\
			     EXTRA_VAR)					\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_ffI_f1 ((ARRAY)[i].arg_str, FUNC_NAME,			\
		       (ARRAY)[i].arg1, (ARRAY)[i].arg2,		\
		       (ARRAY)[i].expected, (ARRAY)[i].exceptions,	\
		       EXTRA_VAR, (ARRAY)[i].extra_test,		\
		       (ARRAY)[i].extra_expected);			\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_c_c(ARG_STR, FUNC_NAME, ARGR, ARGC, EXPR, EXPC,	\
		     EXCEPTIONS)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_complex (test_name,					\
		       FUNC (FUNC_NAME) (BUILD_COMPLEX (ARGR, ARGC)),	\
		       BUILD_COMPLEX (EXPR, EXPC), EXCEPTIONS);		\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_c_c(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_c_c ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].argr,	\
		    (ARRAY)[i].argc, (ARRAY)[i].expr, (ARRAY)[i].expc,	\
		    (ARRAY)[i].exceptions);				\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_cc_c(ARG_STR, FUNC_NAME, ARG1R, ARG1C, ARG2R, ARG2C,	\
		      EXPR, EXPC, EXCEPTIONS)				\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_complex (test_name,					\
		       FUNC (FUNC_NAME) (BUILD_COMPLEX (ARG1R, ARG1C),	\
					 BUILD_COMPLEX (ARG2R, ARG2C)),	\
		       BUILD_COMPLEX (EXPR, EXPC), EXCEPTIONS);		\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_cc_c(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_cc_c ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg1r,	\
		     (ARRAY)[i].arg1c, (ARRAY)[i].arg2r,		\
		     (ARRAY)[i].arg2c, (ARRAY)[i].expr,			\
		     (ARRAY)[i].expc, (ARRAY)[i].exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_i(ARG_STR, FUNC_NAME, ARG, EXPECTED, EXCEPTIONS)	\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_int (test_name, FUNC (FUNC_NAME) (ARG), EXPECTED,		\
		   EXCEPTIONS);						\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_i(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_i ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].expected, (ARRAY)[i].exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_i_tg(ARG_STR, FUNC_NAME, ARG, EXPECTED,		\
			EXCEPTIONS)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_int (test_name, FUNC_NAME (ARG), EXPECTED, EXCEPTIONS);	\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_i_tg(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_i_tg ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		       (ARRAY)[i].expected, (ARRAY)[i].exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_ff_i_tg(ARG_STR, FUNC_NAME, ARG1, ARG2, EXPECTED,	\
			 EXCEPTIONS)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_int (test_name, FUNC_NAME (ARG1, ARG2), EXPECTED,		\
		   EXCEPTIONS);						\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_ff_i_tg(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_ff_i_tg ((ARRAY)[i].arg_str, FUNC_NAME,		\
			(ARRAY)[i].arg1, (ARRAY)[i].arg2,		\
			(ARRAY)[i].expected, (ARRAY)[i].exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_b(ARG_STR, FUNC_NAME, ARG, EXPECTED, EXCEPTIONS)	\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_bool (test_name, FUNC (FUNC_NAME) (ARG), EXPECTED,	\
		    EXCEPTIONS);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_b(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_b ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].expected, (ARRAY)[i].exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_b_tg(ARG_STR, FUNC_NAME, ARG, EXPECTED,		\
			EXCEPTIONS)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_bool (test_name, FUNC_NAME (ARG), EXPECTED, EXCEPTIONS);	\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_b_tg(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_b_tg ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		       (ARRAY)[i].expected, (ARRAY)[i].exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_l(ARG_STR, FUNC_NAME, ARG, EXPECTED, EXCEPTIONS)	\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_long (test_name, FUNC (FUNC_NAME) (ARG), EXPECTED,	\
		    EXCEPTIONS);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_l(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_l ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].expected, (ARRAY)[i].exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_L(ARG_STR, FUNC_NAME, ARG, EXPECTED, EXCEPTIONS)	\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_longlong (test_name, FUNC (FUNC_NAME) (ARG), EXPECTED,	\
			EXCEPTIONS);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_L(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_L ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].expected, (ARRAY)[i].exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_fFF_11(ARG_STR, FUNC_NAME, ARG, EXCEPTIONS,		\
			EXTRA1_VAR, EXTRA1_TEST,			\
			EXTRA1_EXPECTED, EXTRA2_VAR,			\
			EXTRA2_TEST, EXTRA2_EXPECTED)			\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	FUNC (FUNC_NAME) (ARG, &(EXTRA1_VAR), &(EXTRA2_VAR));		\
	EXTRA_OUTPUT_TEST_SETUP (ARG_STR, 1);				\
	if (EXTRA1_TEST)						\
	  check_float (extra1_name, EXTRA1_VAR, EXTRA1_EXPECTED,	\
		       EXCEPTIONS);					\
	EXTRA_OUTPUT_TEST_CLEANUP (1);					\
	EXTRA_OUTPUT_TEST_SETUP (ARG_STR, 2);				\
	if (EXTRA2_TEST)						\
	  check_float (extra2_name, EXTRA2_VAR, EXTRA2_EXPECTED, 0);	\
	EXTRA_OUTPUT_TEST_CLEANUP (2);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_fFF_11(FUNC_NAME, ARRAY, ROUNDING_MODE,		\
			     EXTRA1_VAR, EXTRA2_VAR)			\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_fFF_11 ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		       (ARRAY)[i].exceptions,				\
		       EXTRA1_VAR, (ARRAY)[i].extra1_test,		\
		       (ARRAY)[i].extra1_expected,			\
		       EXTRA2_VAR,					\
		       (ARRAY)[i].extra2_test,				\
		       (ARRAY)[i].extra2_expected);			\
  ROUND_RESTORE_ ## ROUNDING_MODE

/* Start and end the tests for a given function.  */
#define START(FUNC)				\
  const char *this_func = #FUNC;		\
  init_max_error ()
#define END					\
  print_max_error (this_func)
#define END_COMPLEX				\
  print_complex_max_error (this_func)

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
    TEST_f_f (acos, plus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acos, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acos, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* |x| > 1: */
    TEST_f_f (acos, 1.125L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acos, -1.125L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acos, max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acos, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),

    AUTO_TESTS_f_f (acos, tonearest),
  };

static void
acos_test (void)
{
  START (acos);
  RUN_TEST_LOOP_f_f (acos, acos_test_data, );
  END;
}


static const struct test_f_f_data acos_tonearest_test_data[] =
  {
    AUTO_TESTS_f_f (acos, tonearest),
  };

static void
acos_test_tonearest (void)
{
  START (acos_tonearest);
  RUN_TEST_LOOP_f_f (acos, acos_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_f_f_data acos_towardzero_test_data[] =
  {
    AUTO_TESTS_f_f (acos, towardzero),
  };

static void
acos_test_towardzero (void)
{
  START (acos_towardzero);
  RUN_TEST_LOOP_f_f (acos, acos_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_f_f_data acos_downward_test_data[] =
  {
    AUTO_TESTS_f_f (acos, downward),
  };

static void
acos_test_downward (void)
{
  START (acos_downward);
  RUN_TEST_LOOP_f_f (acos, acos_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_f_f_data acos_upward_test_data[] =
  {
    AUTO_TESTS_f_f (acos, upward),
  };

static void
acos_test_upward (void)
{
  START (acos_upward);
  RUN_TEST_LOOP_f_f (acos, acos_upward_test_data, FE_UPWARD);
  END;
}

static const struct test_f_f_data acosh_test_data[] =
  {
    TEST_f_f (acosh, plus_infty, plus_infty),
    TEST_f_f (acosh, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acosh, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* x < 1:  */
    TEST_f_f (acosh, 0.75L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acosh, min_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acosh, min_subnorm_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acosh, plus_zero, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acosh, minus_zero, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acosh, -min_subnorm_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acosh, -min_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acosh, -1.125L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (acosh, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),

    AUTO_TESTS_f_f (acosh, tonearest),
  };

static void
acosh_test (void)
{
  START (acosh);
  RUN_TEST_LOOP_f_f (acosh, acosh_test_data, );
  END;
}

static const struct test_f_f_data asin_test_data[] =
  {
    TEST_f_f (asin, plus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (asin, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (asin, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* asin x == qNaN plus invalid exception for |x| > 1.  */
    TEST_f_f (asin, 1.125L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (asin, -1.125L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (asin, max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (asin, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),

    AUTO_TESTS_f_f (asin, tonearest),
  };

static void
asin_test (void)
{
  START (asin);
  RUN_TEST_LOOP_f_f (asin, asin_test_data, );
  END;
}


static const struct test_f_f_data asin_tonearest_test_data[] =
  {
    AUTO_TESTS_f_f (asin, tonearest),
  };

static void
asin_test_tonearest (void)
{
  START (asin_tonearest);
  RUN_TEST_LOOP_f_f (asin, asin_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_f_f_data asin_towardzero_test_data[] =
  {
    AUTO_TESTS_f_f (asin, towardzero),
  };

static void
asin_test_towardzero (void)
{
  START (asin_towardzero);
  RUN_TEST_LOOP_f_f (asin, asin_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_f_f_data asin_downward_test_data[] =
  {
    AUTO_TESTS_f_f (asin, downward),
  };

static void
asin_test_downward (void)
{
  START (asin_downward);
  RUN_TEST_LOOP_f_f (asin, asin_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_f_f_data asin_upward_test_data[] =
  {
    AUTO_TESTS_f_f (asin, upward),
  };

static void
asin_test_upward (void)
{
  START (asin_upward);
  RUN_TEST_LOOP_f_f (asin, asin_upward_test_data, FE_UPWARD);
  END;
}

static const struct test_f_f_data asinh_test_data[] =
  {
    TEST_f_f (asinh, plus_infty, plus_infty, NO_TEST_INLINE),
    TEST_f_f (asinh, minus_infty, minus_infty, NO_TEST_INLINE),
    TEST_f_f (asinh, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    AUTO_TESTS_f_f (asinh, tonearest),
  };

static void
asinh_test (void)
{
  START (asinh);
  RUN_TEST_LOOP_f_f (asinh, asinh_test_data, );
  END;
}

static const struct test_f_f_data atan_test_data[] =
  {
    TEST_f_f (atan, plus_infty, M_PI_2l),
    TEST_f_f (atan, minus_infty, -M_PI_2l),
    TEST_f_f (atan, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (atan, tonearest),
  };

static void
atan_test (void)
{
  START (atan);
  RUN_TEST_LOOP_f_f (atan, atan_test_data, );
  END;
}



static const struct test_f_f_data atanh_test_data[] =
  {
    TEST_f_f (atanh, 1, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (atanh, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (atanh, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* atanh (x) == qNaN plus invalid exception if |x| > 1.  */
    TEST_f_f (atanh, 1.125L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (atanh, -1.125L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (atanh, max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (atanh, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (atanh, plus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (atanh, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),

    AUTO_TESTS_f_f (atanh, tonearest),
  };

static void
atanh_test (void)
{
  START (atanh);
  RUN_TEST_LOOP_f_f (atanh, atanh_test_data, );
  END;
}

static const struct test_ff_f_data atan2_test_data[] =
  {
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
    TEST_ff_f (atan2, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_ff_f (atan2, tonearest),
  };

static void
atan2_test (void)
{
  START (atan2);
  RUN_TEST_LOOP_ff_f (atan2, atan2_test_data, );
  END;
}

static const struct test_c_f_data cabs_test_data[] =
  {
    /* cabs (x + iy) is specified as hypot (x,y) */

    /* cabs (+inf + i x) == +inf.  */
    TEST_c_f (cabs, plus_infty, 1.0, plus_infty),
    /* cabs (-inf + i x) == +inf.  */
    TEST_c_f (cabs, minus_infty, 1.0, plus_infty),

    TEST_c_f (cabs, minus_infty, qnan_value, plus_infty),
    TEST_c_f (cabs, minus_infty, qnan_value, plus_infty),

    TEST_c_f (cabs, qnan_value, qnan_value, qnan_value),

    AUTO_TESTS_c_f (cabs, tonearest),
  };

static void
cabs_test (void)
{
  START (cabs);
  RUN_TEST_LOOP_c_f (cabs, cabs_test_data, );
  END;
}


static const struct test_c_c_data cacos_test_data[] =
  {
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
  };

static void
cacos_test (void)
{
  START (cacos);
  RUN_TEST_LOOP_c_c (cacos, cacos_test_data, );
  END_COMPLEX;
}

static const struct test_c_c_data cacosh_test_data[] =
  {
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
  };

static void
cacosh_test (void)
{
  START (cacosh);
  RUN_TEST_LOOP_c_c (cacosh, cacosh_test_data, );
  END_COMPLEX;
}


static const struct test_c_f_data carg_test_data[] =
  {
    /* carg (x + iy) is specified as atan2 (y, x) */

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

    AUTO_TESTS_c_f (carg, tonearest),
  };

static void
carg_test (void)
{
  START (carg);
  RUN_TEST_LOOP_c_f (carg, carg_test_data, );
  END;
}

static const struct test_c_c_data casin_test_data[] =
  {
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
    /* Bug 15319: underflow exception may be missing.  */
    TEST_c_c (casin, 0x1.fp-16385L, 0x1p-63L, 8.142593549724601460479922838826119584369e-4933L, 1.084202172485504434007452800869941711424e-19L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casin, 0x1.fp-16385L, -0x1p-63L, 8.142593549724601460479922838826119584369e-4933L, -1.084202172485504434007452800869941711424e-19L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casin, -0x1.fp-16385L, 0x1p-63L, -8.142593549724601460479922838826119584369e-4933L, 1.084202172485504434007452800869941711424e-19L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casin, -0x1.fp-16385L, -0x1p-63L, -8.142593549724601460479922838826119584369e-4933L, -1.084202172485504434007452800869941711424e-19L, UNDERFLOW_EXCEPTION_OK),
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
    /* Bug 15319: underflow exception may be missing.  */
    TEST_c_c (casin, 0x1.fp-16385L, 0x1p-112L, 8.142593549724601460479922838826119584417e-4933L, 1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casin, 0x1.fp-16385L, -0x1p-112L, 8.142593549724601460479922838826119584417e-4933L, -1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casin, -0x1.fp-16385L, 0x1p-112L, -8.142593549724601460479922838826119584417e-4933L, 1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casin, -0x1.fp-16385L, -0x1p-112L, -8.142593549724601460479922838826119584417e-4933L, -1.925929944387235853055977942584927318538e-34L, UNDERFLOW_EXCEPTION_OK),
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
  };

static void
casin_test (void)
{
  START (casin);
  RUN_TEST_LOOP_c_c (casin, casin_test_data, );
  END_COMPLEX;
}


static const struct test_c_c_data casinh_test_data[] =
  {
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
    /* Bug 15319: underflow exception may be missing.  */
    TEST_c_c (casinh, 0x1p-63L, 0x1.fp-16385L, 1.084202172485504434007452800869941711424e-19L, 8.142593549724601460479922838826119584369e-4933L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casinh, -0x1p-63L, 0x1.fp-16385L, -1.084202172485504434007452800869941711424e-19L, 8.142593549724601460479922838826119584369e-4933L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casinh, 0x1p-63L, -0x1.fp-16385L, 1.084202172485504434007452800869941711424e-19L, -8.142593549724601460479922838826119584369e-4933L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casinh, -0x1p-63L, -0x1.fp-16385L, -1.084202172485504434007452800869941711424e-19L, -8.142593549724601460479922838826119584369e-4933L, UNDERFLOW_EXCEPTION_OK),
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
    /* Bug 15319: underflow exception may be missing.  */
    TEST_c_c (casinh, 0x1p-112L, 0x1.fp-16385L, 1.925929944387235853055977942584927318538e-34L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casinh, -0x1p-112L, 0x1.fp-16385L, -1.925929944387235853055977942584927318538e-34L, 8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casinh, 0x1p-112L, -0x1.fp-16385L, 1.925929944387235853055977942584927318538e-34L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION_OK),
    TEST_c_c (casinh, -0x1p-112L, -0x1.fp-16385L, -1.925929944387235853055977942584927318538e-34L, -8.142593549724601460479922838826119584417e-4933L, UNDERFLOW_EXCEPTION_OK),
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
  };

static void
casinh_test (void)
{
  START (casinh);
  RUN_TEST_LOOP_c_c (casinh, casinh_test_data, );
  END_COMPLEX;
}


static const struct test_c_c_data catan_test_data[] =
  {
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
  };

static void
catan_test (void)
{
  START (catan);
  RUN_TEST_LOOP_c_c (catan, catan_test_data, );
  END_COMPLEX;
}

static const struct test_c_c_data catanh_test_data[] =
  {
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
  };

static void
catanh_test (void)
{
  START (catanh);
  RUN_TEST_LOOP_c_c (catanh, catanh_test_data, );
  END_COMPLEX;
}

static const struct test_f_f_data cbrt_test_data[] =
  {
    TEST_f_f (cbrt, plus_infty, plus_infty),
    TEST_f_f (cbrt, minus_infty, minus_infty),
    TEST_f_f (cbrt, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (cbrt, tonearest),
  };

static void
cbrt_test (void)
{
  START (cbrt);
  RUN_TEST_LOOP_f_f (cbrt, cbrt_test_data, );
  END;
}


static const struct test_c_c_data ccos_test_data[] =
  {
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

    AUTO_TESTS_c_c (ccos, tonearest),
  };

static void
ccos_test (void)
{
  START (ccos);
  RUN_TEST_LOOP_c_c (ccos, ccos_test_data, );
  END_COMPLEX;
}


static const struct test_c_c_data ccosh_test_data[] =
  {
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

    AUTO_TESTS_c_c (ccosh, tonearest),
  };

static void
ccosh_test (void)
{
  START (ccosh);
  RUN_TEST_LOOP_c_c (ccosh, ccosh_test_data, );
  END_COMPLEX;
}


static const struct test_f_f_data ceil_test_data[] =
  {
    TEST_f_f (ceil, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (ceil, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_f_f (ceil, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (ceil, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (ceil, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* Bug 15479: spurious "inexact" exception may occur.  */
    TEST_f_f (ceil, M_PIl, 4.0),
    TEST_f_f (ceil, -M_PIl, -3.0),
    TEST_f_f (ceil, min_subnorm_value, 1.0),
    TEST_f_f (ceil, min_value, 1.0),
    TEST_f_f (ceil, 0.1, 1.0),
    TEST_f_f (ceil, 0.25, 1.0),
    TEST_f_f (ceil, 0.625, 1.0),
    TEST_f_f (ceil, max_value, max_value),
    TEST_f_f (ceil, -min_subnorm_value, minus_zero),
    TEST_f_f (ceil, -min_value, minus_zero),
    TEST_f_f (ceil, -0.1, minus_zero),
    TEST_f_f (ceil, -0.25, minus_zero),
    TEST_f_f (ceil, -0.625, minus_zero),
    TEST_f_f (ceil, -max_value, -max_value),

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
  };

static void
ceil_test (void)
{
  START (ceil);
  RUN_TEST_LOOP_f_f (ceil, ceil_test_data, );
  END;
}


static const struct test_c_c_data cexp_test_data[] =
  {
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

    TEST_c_c (cexp, qnan_value, 0.0, qnan_value, 0.0),
    TEST_c_c (cexp, qnan_value, minus_zero, qnan_value, minus_zero),
    TEST_c_c (cexp, qnan_value, 1.0, qnan_value, qnan_value, INVALID_EXCEPTION_OK),

    TEST_c_c (cexp, qnan_value, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (cexp, 0, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (cexp, 1, qnan_value, qnan_value, qnan_value, INVALID_EXCEPTION_OK),
    TEST_c_c (cexp, qnan_value, qnan_value, qnan_value, qnan_value),

    AUTO_TESTS_c_c (cexp, tonearest),
  };

static void
cexp_test (void)
{
  START (cexp);
  RUN_TEST_LOOP_c_c (cexp, cexp_test_data, );
  END_COMPLEX;
}


static const struct test_c_f_data cimag_test_data[] =
  {
    TEST_c_f (cimag, 1.0, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_f (cimag, 1.0, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_f (cimag, 1.0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_c_f (cimag, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_c_f (cimag, 1.0, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_c_f (cimag, 1.0, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_c_f (cimag, 2.0, 3.0, 3.0, NO_INEXACT_EXCEPTION),
  };

static void
cimag_test (void)
{
  START (cimag);
  RUN_TEST_LOOP_c_f (cimag, cimag_test_data, );
  END;
}

static const struct test_c_c_data clog_test_data[] =
  {
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

    AUTO_TESTS_c_c (clog, tonearest),
  };

static void
clog_test (void)
{
  START (clog);
  RUN_TEST_LOOP_c_c (clog, clog_test_data, );
  END_COMPLEX;
}


static const struct test_c_c_data clog10_test_data[] =
  {
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
  };

static void
clog10_test (void)
{
  START (clog10);
  RUN_TEST_LOOP_c_c (clog10, clog10_test_data, );
  END_COMPLEX;
}


static const struct test_c_c_data conj_test_data[] =
  {
    TEST_c_c (conj, 0.0, 0.0, 0.0, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_c (conj, 0.0, minus_zero, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (conj, qnan_value, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_c_c (conj, plus_infty, minus_infty, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_c_c (conj, plus_infty, plus_infty, plus_infty, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_c_c (conj, 1.0, 2.0, 1.0, -2.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (conj, 3.0, -4.0, 3.0, 4.0, NO_INEXACT_EXCEPTION),
  };

static void
conj_test (void)
{
  START (conj);
  RUN_TEST_LOOP_c_c (conj, conj_test_data, );
  END_COMPLEX;
}


static const struct test_ff_f_data copysign_test_data[] =
  {
    TEST_ff_f (copysign, 0, 4, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, 0, -4, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, minus_zero, 4, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, minus_zero, -4, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_ff_f (copysign, plus_infty, 0, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, plus_infty, minus_zero, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, minus_infty, 0, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, minus_infty, minus_zero, minus_infty, NO_INEXACT_EXCEPTION),

    TEST_ff_f (copysign, 0, plus_infty, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, 0, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, minus_zero, plus_infty, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, minus_zero, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_ff_f (copysign, 0, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, 0, -qnan_value, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, minus_zero, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, minus_zero, -qnan_value, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_ff_f (copysign, qnan_value, 0, qnan_value, NO_INEXACT_EXCEPTION|TEST_NAN_SIGN),
    TEST_ff_f (copysign, qnan_value, minus_zero, -qnan_value, NO_INEXACT_EXCEPTION|TEST_NAN_SIGN),
    TEST_ff_f (copysign, -qnan_value, 0, qnan_value, NO_INEXACT_EXCEPTION|TEST_NAN_SIGN),
    TEST_ff_f (copysign, -qnan_value, minus_zero, -qnan_value, NO_INEXACT_EXCEPTION|TEST_NAN_SIGN),

    TEST_ff_f (copysign, min_value, min_subnorm_value, min_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, min_value, -min_subnorm_value, -min_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, -min_value, min_subnorm_value, min_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, -min_value, -min_subnorm_value, -min_value, NO_INEXACT_EXCEPTION),

    TEST_ff_f (copysign, min_subnorm_value, max_value, min_subnorm_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, min_subnorm_value, -max_value, -min_subnorm_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, -min_subnorm_value, max_value, min_subnorm_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, -min_subnorm_value, -max_value, -min_subnorm_value, NO_INEXACT_EXCEPTION),

    TEST_ff_f (copysign, max_value, min_value, max_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, max_value, -min_value, -max_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, -max_value, min_value, max_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (copysign, -max_value, -min_value, -max_value, NO_INEXACT_EXCEPTION),
  };

static void
copysign_test (void)
{
  START (copysign);
  RUN_TEST_LOOP_ff_f (copysign, copysign_test_data, );
  END;
}


static const struct test_f_f_data cos_test_data[] =
  {
    TEST_f_f (cos, plus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (cos, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (cos, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    AUTO_TESTS_f_f (cos, tonearest),
  };

static void
cos_test (void)
{
  START (cos);
  RUN_TEST_LOOP_f_f (cos, cos_test_data, );
  END;
}


static const struct test_f_f_data cos_tonearest_test_data[] =
  {
    AUTO_TESTS_f_f (cos, tonearest),
  };

static void
cos_test_tonearest (void)
{
  START (cos_tonearest);
  RUN_TEST_LOOP_f_f (cos, cos_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_f_f_data cos_towardzero_test_data[] =
  {
    AUTO_TESTS_f_f (cos, towardzero),
  };

static void
cos_test_towardzero (void)
{
  START (cos_towardzero);
  RUN_TEST_LOOP_f_f (cos, cos_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_f_f_data cos_downward_test_data[] =
  {
    AUTO_TESTS_f_f (cos, downward),
  };

static void
cos_test_downward (void)
{
  START (cos_downward);
  RUN_TEST_LOOP_f_f (cos, cos_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_f_f_data cos_upward_test_data[] =
  {
    AUTO_TESTS_f_f (cos, upward),
  };

static void
cos_test_upward (void)
{
  START (cos_upward);
  RUN_TEST_LOOP_f_f (cos, cos_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_f_f_data cosh_test_data[] =
  {
    TEST_f_f (cosh, plus_infty, plus_infty, NO_TEST_INLINE),
    TEST_f_f (cosh, minus_infty, plus_infty, NO_TEST_INLINE),
    TEST_f_f (cosh, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (cosh, tonearest),
  };

static void
cosh_test (void)
{
  START (cosh);
  RUN_TEST_LOOP_f_f (cosh, cosh_test_data, );
  END;
}


static const struct test_f_f_data cosh_tonearest_test_data[] =
  {
    AUTO_TESTS_f_f (cosh, tonearest),
  };

static void
cosh_test_tonearest (void)
{
  START (cosh_tonearest);
  RUN_TEST_LOOP_f_f (cosh, cosh_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_f_f_data cosh_towardzero_test_data[] =
  {
    AUTO_TESTS_f_f (cosh, towardzero),
  };

static void
cosh_test_towardzero (void)
{
  START (cosh_towardzero);
  RUN_TEST_LOOP_f_f (cosh, cosh_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_f_f_data cosh_downward_test_data[] =
  {
    AUTO_TESTS_f_f (cosh, downward),
  };

static void
cosh_test_downward (void)
{
  START (cosh_downward);
  RUN_TEST_LOOP_f_f (cosh, cosh_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_f_f_data cosh_upward_test_data[] =
  {
    AUTO_TESTS_f_f (cosh, upward),
  };

static void
cosh_test_upward (void)
{
  START (cosh_upward);
  RUN_TEST_LOOP_f_f (cosh, cosh_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_cc_c_data cpow_test_data[] =
  {
    TEST_cc_c (cpow, qnan_value, qnan_value, qnan_value, qnan_value, qnan_value, qnan_value),

    AUTO_TESTS_cc_c (cpow, tonearest),
  };

static void
cpow_test (void)
{
  START (cpow);
  RUN_TEST_LOOP_cc_c (cpow, cpow_test_data, );
  END_COMPLEX;
}


static const struct test_c_c_data cproj_test_data[] =
  {
    TEST_c_c (cproj, 0.0, 0.0, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_zero, minus_zero, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, 0.0, minus_zero, 0.0, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_zero, 0.0, minus_zero, 0.0, NO_INEXACT_EXCEPTION),

    TEST_c_c (cproj, qnan_value, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    TEST_c_c (cproj, plus_zero, qnan_value, plus_zero, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_zero, qnan_value, minus_zero, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, qnan_value, plus_zero, qnan_value, plus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, qnan_value, minus_zero, qnan_value, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_c_c (cproj, 1.0, qnan_value, 1.0, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, -1.0, qnan_value, -1.0, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, qnan_value, 1.0, qnan_value, 1.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, qnan_value, -1.0, qnan_value, -1.0, NO_INEXACT_EXCEPTION),

    TEST_c_c (cproj, plus_infty, plus_infty, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, plus_infty, minus_infty, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_infty, plus_infty, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_infty, minus_infty, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_c_c (cproj, plus_infty, plus_zero, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, plus_infty, minus_zero, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_infty, plus_zero, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_infty, minus_zero, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_c_c (cproj, plus_zero, plus_infty, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, plus_zero, minus_infty, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_zero, plus_infty, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_zero, minus_infty, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_c_c (cproj, plus_infty, 1.0, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, plus_infty, -1.0, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_infty, 1.0, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_infty, -1.0, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_c_c (cproj, 1.0, plus_infty, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, 1.0, minus_infty, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, -1.0, plus_infty, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, -1.0, minus_infty, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_c_c (cproj, plus_infty, qnan_value, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, plus_infty, -qnan_value, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_infty, qnan_value, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, minus_infty, -qnan_value, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_c_c (cproj, qnan_value, plus_infty, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, qnan_value, minus_infty, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, -qnan_value, plus_infty, plus_infty, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, -qnan_value, minus_infty, plus_infty, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_c_c (cproj, 1.0, 0.0, 1.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_c (cproj, 2.0, 3.0, 2.0, 3.0, NO_INEXACT_EXCEPTION),
  };

static void
cproj_test (void)
{
  START (cproj);
  RUN_TEST_LOOP_c_c (cproj, cproj_test_data, );
  END_COMPLEX;
}


static const struct test_c_f_data creal_test_data[] =
  {
    TEST_c_f (creal, 0.0, 1.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_c_f (creal, minus_zero, 1.0, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_c_f (creal, qnan_value, 1.0, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_c_f (creal, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_c_f (creal, plus_infty, 1.0, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_c_f (creal, minus_infty, 1.0, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_c_f (creal, 2.0, 3.0, 2.0, NO_INEXACT_EXCEPTION),
  };

static void
creal_test (void)
{
  START (creal);
  RUN_TEST_LOOP_c_f (creal, creal_test_data, );
  END;
}

static const struct test_c_c_data csin_test_data[] =
  {
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
  };

static void
csin_test (void)
{
  START (csin);
  RUN_TEST_LOOP_c_c (csin, csin_test_data, );
  END_COMPLEX;
}


static const struct test_c_c_data csinh_test_data[] =
  {
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
  };

static void
csinh_test (void)
{
  START (csinh);
  RUN_TEST_LOOP_c_c (csinh, csinh_test_data, );
  END_COMPLEX;
}


static const struct test_c_c_data csqrt_test_data[] =
  {
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

    AUTO_TESTS_c_c (csqrt, tonearest),
  };

static void
csqrt_test (void)
{
  START (csqrt);
  RUN_TEST_LOOP_c_c (csqrt, csqrt_test_data, );
  END_COMPLEX;
}

static const struct test_c_c_data ctan_test_data[] =
  {
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

    AUTO_TESTS_c_c (ctan, tonearest),
  };

static void
ctan_test (void)
{
  START (ctan);
  RUN_TEST_LOOP_c_c (ctan, ctan_test_data, );
  END_COMPLEX;
}


static const struct test_c_c_data ctan_tonearest_test_data[] =
  {
    AUTO_TESTS_c_c (ctan, tonearest),
  };

static void
ctan_test_tonearest (void)
{
  START (ctan_tonearest);
  RUN_TEST_LOOP_c_c (ctan, ctan_tonearest_test_data, FE_TONEAREST);
  END_COMPLEX;
}


static const struct test_c_c_data ctan_towardzero_test_data[] =
  {
    AUTO_TESTS_c_c (ctan, towardzero),
  };

static void
ctan_test_towardzero (void)
{
  START (ctan_towardzero);
  RUN_TEST_LOOP_c_c (ctan, ctan_towardzero_test_data, FE_TOWARDZERO);
  END_COMPLEX;
}


static const struct test_c_c_data ctan_downward_test_data[] =
  {
    AUTO_TESTS_c_c (ctan, downward),
  };

static void
ctan_test_downward (void)
{
  START (ctan_downward);
  RUN_TEST_LOOP_c_c (ctan, ctan_downward_test_data, FE_DOWNWARD);
  END_COMPLEX;
}


static const struct test_c_c_data ctan_upward_test_data[] =
  {
    AUTO_TESTS_c_c (ctan, upward),
  };

static void
ctan_test_upward (void)
{
  START (ctan_upward);
  RUN_TEST_LOOP_c_c (ctan, ctan_upward_test_data, FE_UPWARD);
  END_COMPLEX;
}


static const struct test_c_c_data ctanh_test_data[] =
  {
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

    AUTO_TESTS_c_c (ctanh, tonearest),
  };

static void
ctanh_test (void)
{
  START (ctanh);
  RUN_TEST_LOOP_c_c (ctanh, ctanh_test_data, );
  END_COMPLEX;
}


static const struct test_c_c_data ctanh_tonearest_test_data[] =
  {
    AUTO_TESTS_c_c (ctanh, tonearest),
  };

static void
ctanh_test_tonearest (void)
{
  START (ctanh_tonearest);
  RUN_TEST_LOOP_c_c (ctanh, ctanh_tonearest_test_data, FE_TONEAREST);
  END_COMPLEX;
}


static const struct test_c_c_data ctanh_towardzero_test_data[] =
  {
    AUTO_TESTS_c_c (ctanh, towardzero),
  };

static void
ctanh_test_towardzero (void)
{
  START (ctanh_towardzero);
  RUN_TEST_LOOP_c_c (ctanh, ctanh_towardzero_test_data, FE_TOWARDZERO);
  END_COMPLEX;
}


static const struct test_c_c_data ctanh_downward_test_data[] =
  {
    AUTO_TESTS_c_c (ctanh, downward),
  };

static void
ctanh_test_downward (void)
{
  START (ctanh_downward);
  RUN_TEST_LOOP_c_c (ctanh, ctanh_downward_test_data, FE_DOWNWARD);
  END_COMPLEX;
}


static const struct test_c_c_data ctanh_upward_test_data[] =
  {
    AUTO_TESTS_c_c (ctanh, upward),
  };

static void
ctanh_test_upward (void)
{
  START (ctanh_upward);
  RUN_TEST_LOOP_c_c (ctanh, ctanh_upward_test_data, FE_UPWARD);
  END_COMPLEX;
}


static const struct test_f_f_data erf_test_data[] =
  {
    TEST_f_f (erf, plus_infty, 1),
    TEST_f_f (erf, minus_infty, -1),
    TEST_f_f (erf, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (erf, tonearest),
  };

static void
erf_test (void)
{
  START (erf);
  RUN_TEST_LOOP_f_f (erf, erf_test_data, );
  END;
}


static const struct test_f_f_data erfc_test_data[] =
  {
    TEST_f_f (erfc, plus_infty, 0.0),
    TEST_f_f (erfc, minus_infty, 2.0),
    TEST_f_f (erfc, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (erfc, tonearest),
  };

static void
erfc_test (void)
{
  START (erfc);
  RUN_TEST_LOOP_f_f (erfc, erfc_test_data, );
  END;
}


static const struct test_f_f_data exp_test_data[] =
  {
    TEST_f_f (exp, plus_infty, plus_infty, NO_TEST_INLINE),
    TEST_f_f (exp, minus_infty, 0, NO_TEST_INLINE),
    TEST_f_f (exp, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (exp, tonearest),
  };

static void
exp_test (void)
{
  START (exp);
  RUN_TEST_LOOP_f_f (exp, exp_test_data, );
  END;
}


static const struct test_f_f_data exp_tonearest_test_data[] =
  {
    AUTO_TESTS_f_f (exp, tonearest),
  };

static void
exp_test_tonearest (void)
{
  START (exp_tonearest);
  RUN_TEST_LOOP_f_f (exp, exp_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_f_f_data exp_towardzero_test_data[] =
  {
    AUTO_TESTS_f_f (exp, towardzero),
  };

static void
exp_test_towardzero (void)
{
  START (exp_towardzero);
  RUN_TEST_LOOP_f_f (exp, exp_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_f_f_data exp_downward_test_data[] =
  {
    AUTO_TESTS_f_f (exp, downward),
  };

static void
exp_test_downward (void)
{
  START (exp_downward);
  RUN_TEST_LOOP_f_f (exp, exp_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_f_f_data exp_upward_test_data[] =
  {
    AUTO_TESTS_f_f (exp, upward),
  };

static void
exp_test_upward (void)
{
  START (exp_upward);
  RUN_TEST_LOOP_f_f (exp, exp_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_f_f_data exp10_test_data[] =
  {
    TEST_f_f (exp10, plus_infty, plus_infty),
    TEST_f_f (exp10, minus_infty, 0),
    TEST_f_f (exp10, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (exp10, tonearest),
  };

static void
exp10_test (void)
{
  START (exp10);
  RUN_TEST_LOOP_f_f (exp10, exp10_test_data, );
  END;
}


static const struct test_f_f_data exp10_tonearest_test_data[] =
  {
    AUTO_TESTS_f_f (exp10, tonearest),
  };

static void
exp10_test_tonearest (void)
{
  START (exp10_tonearest);
  RUN_TEST_LOOP_f_f (exp10, exp10_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_f_f_data exp10_towardzero_test_data[] =
  {
    AUTO_TESTS_f_f (exp10, towardzero),
  };

static void
exp10_test_towardzero (void)
{
  START (exp10_towardzero);
  RUN_TEST_LOOP_f_f (exp10, exp10_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_f_f_data exp10_downward_test_data[] =
  {
    AUTO_TESTS_f_f (exp10, downward),
  };

static void
exp10_test_downward (void)
{
  START (exp10_downward);
  RUN_TEST_LOOP_f_f (exp10, exp10_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_f_f_data exp10_upward_test_data[] =
  {
    AUTO_TESTS_f_f (exp10, upward),
  };

static void
exp10_test_upward (void)
{
  START (exp10_upward);
  RUN_TEST_LOOP_f_f (exp10, exp10_upward_test_data, FE_UPWARD);
  END;
}

static void
pow10_test (void)
{
  START (pow10);
  /* pow10 uses the same test data as exp10.  */
  RUN_TEST_LOOP_f_f (pow10, exp10_test_data, );
  END;
}


static const struct test_f_f_data exp2_test_data[] =
  {
    TEST_f_f (exp2, plus_infty, plus_infty),
    TEST_f_f (exp2, minus_infty, 0),
    TEST_f_f (exp2, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (exp2, tonearest),
  };

static void
exp2_test (void)
{
  START (exp2);
  RUN_TEST_LOOP_f_f (exp2, exp2_test_data, );
  END;
}


static const struct test_f_f_data expm1_test_data[] =
  {
    TEST_f_f (expm1, plus_infty, plus_infty, NO_TEST_INLINE),
    TEST_f_f (expm1, minus_infty, -1, NO_TEST_INLINE),
    TEST_f_f (expm1, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (expm1, tonearest),
  };

static void
expm1_test (void)
{
  START (expm1);
  RUN_TEST_LOOP_f_f (expm1, expm1_test_data, );
  END;
}


static const struct test_f_f_data expm1_tonearest_test_data[] =
  {
    AUTO_TESTS_f_f (expm1, tonearest),
  };

static void
expm1_test_tonearest (void)
{
  START (expm1_tonearest);
  RUN_TEST_LOOP_f_f (expm1, expm1_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_f_f_data expm1_towardzero_test_data[] =
  {
    AUTO_TESTS_f_f (expm1, towardzero),
  };

static void
expm1_test_towardzero (void)
{
  START (expm1_towardzero);
  RUN_TEST_LOOP_f_f (expm1, expm1_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_f_f_data expm1_downward_test_data[] =
  {
    AUTO_TESTS_f_f (expm1, downward),
  };

static void
expm1_test_downward (void)
{
  START (expm1_downward);
  RUN_TEST_LOOP_f_f (expm1, expm1_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_f_f_data expm1_upward_test_data[] =
  {
    AUTO_TESTS_f_f (expm1, upward),
  };

static void
expm1_test_upward (void)
{
  START (expm1_upward);
  RUN_TEST_LOOP_f_f (expm1, expm1_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_f_f_data fabs_test_data[] =
  {
    TEST_f_f (fabs, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_f (fabs, minus_zero, 0, NO_INEXACT_EXCEPTION),

    TEST_f_f (fabs, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (fabs, minus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (fabs, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|TEST_NAN_SIGN),
    TEST_f_f (fabs, -qnan_value, qnan_value, NO_INEXACT_EXCEPTION|TEST_NAN_SIGN),

    TEST_f_f (fabs, 38.0, 38.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (fabs, -M_El, M_El, NO_INEXACT_EXCEPTION),
  };

static void
fabs_test (void)
{
  START (fabs);
  RUN_TEST_LOOP_f_f (fabs, fabs_test_data, );
  END;
}


static const struct test_ff_f_data fdim_test_data[] =
  {
    TEST_ff_f (fdim, 0, 0, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, 9, 0, 9, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, 0, 9, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, -9, 0, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, 0, -9, 9, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (fdim, plus_infty, 9, plus_infty, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, plus_infty, -9, plus_infty, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, minus_infty, 9, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, minus_infty, -9, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, 9, minus_infty, plus_infty, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, -9, minus_infty, plus_infty, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, 9, plus_infty, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, -9, plus_infty, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (fdim, 0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, 9, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, -9, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, qnan_value, 0, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, qnan_value, 9, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, qnan_value, -9, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, plus_infty, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, minus_infty, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, qnan_value, plus_infty, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, qnan_value, minus_infty, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (fdim, plus_infty, plus_infty, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, plus_infty, minus_infty, plus_infty, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, minus_infty, plus_infty, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (fdim, minus_infty, minus_infty, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
  };

static void
fdim_test (void)
{
  START (fdim);
  RUN_TEST_LOOP_ff_f (fdim, fdim_test_data, );
  END;
}


static const struct test_f_f_data floor_test_data[] =
  {
    TEST_f_f (floor, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (floor, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_f_f (floor, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (floor, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (floor, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* Bug 15479: spurious "inexact" exception may occur.  */
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
  };

static void
floor_test (void)
{
  START (floor);
  RUN_TEST_LOOP_f_f (floor, floor_test_data, );
  END;
}


static const struct test_fff_f_data fma_test_data[] =
  {
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
    TEST_fff_f (fma, 0x1.ffffffffffffffp0L, 0x1.000000000000008p0L, -0x1p-1000L, 0x1.ffffffffffffffffffffffffffffp0L),
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
  };

static void
fma_test (void)
{
  START (fma);
  RUN_TEST_LOOP_fff_f (fma, fma_test_data, );
  END;
}


static const struct test_fff_f_data fma_towardzero_test_data[] =
  {
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
  };

static void
fma_test_towardzero (void)
{
  START (fma_towardzero);
  RUN_TEST_LOOP_fff_f (fma, fma_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_fff_f_data fma_downward_test_data[] =
  {
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
  };

static void
fma_test_downward (void)
{
  START (fma_downward);
  RUN_TEST_LOOP_fff_f (fma, fma_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_fff_f_data fma_upward_test_data[] =
  {
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
  };

static void
fma_test_upward (void)
{
  START (fma_upward);
  RUN_TEST_LOOP_fff_f (fma, fma_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_ff_f_data fmax_test_data[] =
  {
    TEST_ff_f (fmax, 0, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, minus_zero, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, 9, 0, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, 0, 9, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, -9, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, 0, -9, 0, NO_INEXACT_EXCEPTION),

    TEST_ff_f (fmax, plus_infty, 9, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, 0, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, -9, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, plus_infty, -9, plus_infty, NO_INEXACT_EXCEPTION),

    TEST_ff_f (fmax, minus_infty, 9, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, minus_infty, -9, -9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, 9, minus_infty, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, -9, minus_infty, -9, NO_INEXACT_EXCEPTION),

    TEST_ff_f (fmax, 0, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, 9, qnan_value, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, -9, qnan_value, -9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, qnan_value, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, qnan_value, 9, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, qnan_value, -9, -9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, plus_infty, qnan_value, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, minus_infty, qnan_value, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, qnan_value, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, qnan_value, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmax, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
  };

static void
fmax_test (void)
{
  START (fmax);
  RUN_TEST_LOOP_ff_f (fmax, fmax_test_data, );
  END;
}


static const struct test_ff_f_data fmin_test_data[] =
  {
    TEST_ff_f (fmin, 0, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, minus_zero, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, 9, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, 0, 9, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, -9, 0, -9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, 0, -9, -9, NO_INEXACT_EXCEPTION),

    TEST_ff_f (fmin, plus_infty, 9, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, 9, plus_infty, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, plus_infty, -9, -9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, -9, plus_infty, -9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, minus_infty, 9, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, minus_infty, -9, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, 9, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, -9, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),

    TEST_ff_f (fmin, 0, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, 9, qnan_value, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, -9, qnan_value, -9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, qnan_value, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, qnan_value, 9, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, qnan_value, -9, -9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, plus_infty, qnan_value, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, minus_infty, qnan_value, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, qnan_value, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, qnan_value, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmin, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
  };

static void
fmin_test (void)
{
  START (fmin);
  RUN_TEST_LOOP_ff_f (fmin, fmin_test_data, );
  END;
}


static const struct test_ff_f_data fmod_test_data[] =
  {
    /* fmod (+0, y) == +0 for y != 0.  */
    TEST_ff_f (fmod, 0, 3, 0, NO_INEXACT_EXCEPTION),

    /* fmod (-0, y) == -0 for y != 0.  */
    TEST_ff_f (fmod, minus_zero, 3, minus_zero, NO_INEXACT_EXCEPTION),

    /* fmod (+inf, y) == qNaN plus invalid exception.  */
    TEST_ff_f (fmod, plus_infty, 3, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    /* fmod (-inf, y) == qNaN plus invalid exception.  */
    TEST_ff_f (fmod, minus_infty, 3, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    /* fmod (x, +0) == qNaN plus invalid exception.  */
    TEST_ff_f (fmod, 3, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    /* fmod (x, -0) == qNaN plus invalid exception.  */
    TEST_ff_f (fmod, 3, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),

    /* fmod (x, +inf) == x for x not infinite.  */
    TEST_ff_f (fmod, 3.0, plus_infty, 3.0, NO_INEXACT_EXCEPTION),
    /* fmod (x, -inf) == x for x not infinite.  */
    TEST_ff_f (fmod, 3.0, minus_infty, 3.0, NO_INEXACT_EXCEPTION),

    TEST_ff_f (fmod, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    TEST_ff_f (fmod, 6.5, 2.25L, 2.0L, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmod, -6.5, 2.25L, -2.0L, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmod, 6.5, -2.25L, 2.0L, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmod, -6.5, -2.25L, -2.0L, NO_INEXACT_EXCEPTION),

    TEST_ff_f (fmod, 0x0.fffffep-126L, 0x1p-149L, plus_zero, NO_INEXACT_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_ff_f (fmod, 0x0.fffffffffffffp-1022L, 0x1p-1074L, plus_zero, NO_INEXACT_EXCEPTION),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP <= -16381
    TEST_ff_f (fmod, 0x0.fffffffffffffffep-16382L, 0x1p-16445L, plus_zero, NO_INEXACT_EXCEPTION),
#endif
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 56
    TEST_ff_f (fmod, -0x1.00000000000004p+0L, 0x1.fffffffffffff8p-1L, -0x1p-53L, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmod, 0x1.fffffffffffffap-1L, 0x1.fffffffffffff8p-1L, 0x1p-56L, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmod, -0x1.fffffffffffffap-1L, 0x1.fffffffffffff8p-1L, -0x1p-56L, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmod, 0x1.fffffffffffffap-1L, -0x1.fffffffffffff8p-1L, 0x1p-56L, NO_INEXACT_EXCEPTION),
    TEST_ff_f (fmod, -0x1.fffffffffffffap-1L, -0x1.fffffffffffff8p-1L, -0x1p-56L, NO_INEXACT_EXCEPTION),
#endif
  };

static void
fmod_test (void)
{
  START (fmod);
  RUN_TEST_LOOP_ff_f (fmod, fmod_test_data, );
  END;
}


static const struct test_f_i_data fpclassify_test_data[] =
  {
    TEST_f_i (fpclassify, qnan_value, FP_NAN, NO_INEXACT_EXCEPTION),
    TEST_f_i (fpclassify, plus_infty, FP_INFINITE, NO_INEXACT_EXCEPTION),
    TEST_f_i (fpclassify, minus_infty, FP_INFINITE, NO_INEXACT_EXCEPTION),
    TEST_f_i (fpclassify, plus_zero, FP_ZERO, NO_INEXACT_EXCEPTION),
    TEST_f_i (fpclassify, minus_zero, FP_ZERO, NO_INEXACT_EXCEPTION),
    TEST_f_i (fpclassify, 1000, FP_NORMAL, NO_INEXACT_EXCEPTION),
    TEST_f_i (fpclassify, min_subnorm_value, FP_SUBNORMAL, NO_INEXACT_EXCEPTION),
  };

static void
fpclassify_test (void)
{
  START (fpclassify);
  RUN_TEST_LOOP_f_i_tg (fpclassify, fpclassify_test_data, );
  END;
}


static const struct test_f_f1_data frexp_test_data[] =
  {
    TEST_fI_f1 (frexp, plus_infty, plus_infty, IGNORE, NO_INEXACT_EXCEPTION),
    TEST_fI_f1 (frexp, minus_infty, minus_infty, IGNORE, NO_INEXACT_EXCEPTION),
    TEST_fI_f1 (frexp, qnan_value, qnan_value, IGNORE, NO_INEXACT_EXCEPTION),

    TEST_fI_f1 (frexp, 0.0, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_fI_f1 (frexp, minus_zero, minus_zero, 0.0, NO_INEXACT_EXCEPTION),

    TEST_fI_f1 (frexp, 12.8L, 0.8L, 4, NO_INEXACT_EXCEPTION),
    TEST_fI_f1 (frexp, -27.34L, -0.854375L, 5, NO_INEXACT_EXCEPTION),
  };

static void
frexp_test (void)
{
  int x;

  START (frexp);
  RUN_TEST_LOOP_fI_f1 (frexp, frexp_test_data, , x);
  END;
}

static const struct test_ff_f_data hypot_test_data[] =
  {
    TEST_ff_f (hypot, plus_infty, 1, plus_infty),
    TEST_ff_f (hypot, minus_infty, 1, plus_infty),

    TEST_ff_f (hypot, plus_infty, qnan_value, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (hypot, minus_infty, qnan_value, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (hypot, qnan_value, plus_infty, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (hypot, qnan_value, minus_infty, plus_infty, NO_TEST_INLINE),

    TEST_ff_f (hypot, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_ff_f (hypot, tonearest),
  };

static void
hypot_test (void)
{
  START (hypot);
  RUN_TEST_LOOP_ff_f (hypot, hypot_test_data, );
  END;
}


static const struct test_f_i_data ilogb_test_data[] =
  {
    TEST_f_i (ilogb, 1, 0, NO_INEXACT_EXCEPTION),
    TEST_f_i (ilogb, M_El, 1, NO_INEXACT_EXCEPTION),
    TEST_f_i (ilogb, 1024, 10, NO_INEXACT_EXCEPTION),
    TEST_f_i (ilogb, -2000, 10, NO_INEXACT_EXCEPTION),

    /* ilogb (0.0) == FP_ILOGB0 plus invalid exception  */
    TEST_f_i (ilogb, 0.0, FP_ILOGB0, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    /* ilogb (qNaN) == FP_ILOGBNAN plus invalid exception  */
    TEST_f_i (ilogb, qnan_value, FP_ILOGBNAN, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    /* ilogb (inf) == INT_MAX plus invalid exception  */
    TEST_f_i (ilogb, plus_infty, INT_MAX, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    /* ilogb (-inf) == INT_MAX plus invalid exception  */
    TEST_f_i (ilogb, minus_infty, INT_MAX, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
  };

static void
ilogb_test (void)
{
  START (ilogb);
  RUN_TEST_LOOP_f_i (ilogb, ilogb_test_data, );
  END;
}

static const struct test_f_i_data isfinite_test_data[] =
  {
    TEST_f_b (isfinite, 0, 1, NO_INEXACT_EXCEPTION),
    TEST_f_b (isfinite, minus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_f_b (isfinite, 10, 1, NO_INEXACT_EXCEPTION),
    TEST_f_b (isfinite, min_subnorm_value, 1, NO_INEXACT_EXCEPTION),
    TEST_f_b (isfinite, plus_infty, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isfinite, minus_infty, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isfinite, qnan_value, 0, NO_INEXACT_EXCEPTION),
  };

static void
isfinite_test (void)
{
  START (isfinite);
  RUN_TEST_LOOP_f_b_tg (isfinite, isfinite_test_data, );
  END;
}

static void
finite_test (void)
{
  START (finite);
  /* finite uses the same test data as isfinite.  */
  RUN_TEST_LOOP_f_b (finite, isfinite_test_data, );
  END;
}

static const struct test_ff_i_data isgreater_test_data[] =
  {
    TEST_ff_i (isgreater, minus_zero, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, minus_zero, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, minus_zero, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, minus_zero, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, plus_zero, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, plus_zero, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, plus_zero, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, plus_zero, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, (FLOAT) 1, minus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, (FLOAT) 1, plus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, (FLOAT) 1, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, (FLOAT) 1, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, qnan_value, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, qnan_value, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, qnan_value, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreater, qnan_value, qnan_value, 0, NO_INEXACT_EXCEPTION),
  };

static void
isgreater_test (void)
{
  START (isgreater);
  RUN_TEST_LOOP_ff_i_tg (isgreater, isgreater_test_data, );
  END;
}

static const struct test_ff_i_data isgreaterequal_test_data[] =
  {
    TEST_ff_i (isgreaterequal, minus_zero, minus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, minus_zero, plus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, minus_zero, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, minus_zero, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, plus_zero, minus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, plus_zero, plus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, plus_zero, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, plus_zero, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, (FLOAT) 1, minus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, (FLOAT) 1, plus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, (FLOAT) 1, (FLOAT) 1, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, (FLOAT) 1, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, qnan_value, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, qnan_value, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, qnan_value, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isgreaterequal, qnan_value, qnan_value, 0, NO_INEXACT_EXCEPTION),
  };

static void
isgreaterequal_test (void)
{
  START (isgreaterequal);
  RUN_TEST_LOOP_ff_i_tg (isgreaterequal, isgreaterequal_test_data, );
  END;
}

static const struct test_f_i_data isinf_test_data[] =
  {
    TEST_f_b (isinf, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isinf, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isinf, 10, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isinf, min_subnorm_value, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isinf, plus_infty, 1, NO_INEXACT_EXCEPTION),
    TEST_f_b (isinf, minus_infty, 1, NO_INEXACT_EXCEPTION),
    TEST_f_b (isinf, qnan_value, 0, NO_INEXACT_EXCEPTION),
  };

static void
isinf_test (void)
{
  START (isinf);
  RUN_TEST_LOOP_f_b_tg (isinf, isinf_test_data, );
  END;
}

static const struct test_ff_i_data isless_test_data[] =
  {
    TEST_ff_i (isless, minus_zero, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, minus_zero, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, minus_zero, (FLOAT) 1, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, minus_zero, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, plus_zero, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, plus_zero, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, plus_zero, (FLOAT) 1, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, plus_zero, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, (FLOAT) 1, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, (FLOAT) 1, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, (FLOAT) 1, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, (FLOAT) 1, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, qnan_value, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, qnan_value, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, qnan_value, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isless, qnan_value, qnan_value, 0, NO_INEXACT_EXCEPTION),
  };

static void
isless_test (void)
{
  START (isless);
  RUN_TEST_LOOP_ff_i_tg (isless, isless_test_data, );
  END;
}

static const struct test_ff_i_data islessequal_test_data[] =
  {
    TEST_ff_i (islessequal, minus_zero, minus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, minus_zero, plus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, minus_zero, (FLOAT) 1, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, minus_zero, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, plus_zero, minus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, plus_zero, plus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, plus_zero, (FLOAT) 1, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, plus_zero, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, (FLOAT) 1, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, (FLOAT) 1, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, (FLOAT) 1, (FLOAT) 1, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, (FLOAT) 1, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, qnan_value, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, qnan_value, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, qnan_value, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessequal, qnan_value, qnan_value, 0, NO_INEXACT_EXCEPTION),
  };

static void
islessequal_test (void)
{
  START (islessequal);
  RUN_TEST_LOOP_ff_i_tg (islessequal, islessequal_test_data, );
  END;
}

static const struct test_ff_i_data islessgreater_test_data[] =
  {
    TEST_ff_i (islessgreater, minus_zero, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, minus_zero, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, minus_zero, (FLOAT) 1, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, minus_zero, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, plus_zero, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, plus_zero, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, plus_zero, (FLOAT) 1, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, plus_zero, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, (FLOAT) 1, minus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, (FLOAT) 1, plus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, (FLOAT) 1, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, (FLOAT) 1, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, qnan_value, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, qnan_value, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, qnan_value, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (islessgreater, qnan_value, qnan_value, 0, NO_INEXACT_EXCEPTION),
  };

static void
islessgreater_test (void)
{
  START (islessgreater);
  RUN_TEST_LOOP_ff_i_tg (islessgreater, islessgreater_test_data, );
  END;
}

static const struct test_f_i_data isnan_test_data[] =
  {
    TEST_f_b (isnan, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnan, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnan, 10, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnan, min_subnorm_value, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnan, plus_infty, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnan, minus_infty, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnan, qnan_value, 1, NO_INEXACT_EXCEPTION),
  };

static void
isnan_test (void)
{
  START (isnan);
  RUN_TEST_LOOP_f_b_tg (isnan, isnan_test_data, );
  END;
}

static const struct test_f_i_data isnormal_test_data[] =
  {
    TEST_f_b (isnormal, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnormal, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnormal, 10, 1, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnormal, min_subnorm_value, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnormal, plus_infty, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnormal, minus_infty, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (isnormal, qnan_value, 0, NO_INEXACT_EXCEPTION),
  };

static void
isnormal_test (void)
{
  START (isnormal);
  RUN_TEST_LOOP_f_b_tg (isnormal, isnormal_test_data, );
  END;
}

static const struct test_f_i_data issignaling_test_data[] =
  {
    TEST_f_b (issignaling, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (issignaling, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (issignaling, 10, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (issignaling, min_subnorm_value, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (issignaling, plus_infty, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (issignaling, minus_infty, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (issignaling, qnan_value, 0, NO_INEXACT_EXCEPTION),
  };

static void
issignaling_test (void)
{
  START (issignaling);
  RUN_TEST_LOOP_f_b_tg (issignaling, issignaling_test_data, );
  END;
}

static const struct test_ff_i_data isunordered_test_data[] =
  {
    TEST_ff_i (isunordered, minus_zero, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, minus_zero, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, minus_zero, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, minus_zero, qnan_value, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, plus_zero, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, plus_zero, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, plus_zero, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, plus_zero, qnan_value, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, (FLOAT) 1, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, (FLOAT) 1, plus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, (FLOAT) 1, (FLOAT) 1, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, (FLOAT) 1, qnan_value, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, qnan_value, minus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, qnan_value, plus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, qnan_value, (FLOAT) 1, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_i (isunordered, qnan_value, qnan_value, 1, NO_INEXACT_EXCEPTION),
  };

static void
isunordered_test (void)
{
  START (isunordered);
  RUN_TEST_LOOP_ff_i_tg (isunordered, isunordered_test_data, );
  END;
}

static const struct test_f_f_data j0_test_data[] =
  {
    /* j0 is the Bessel function of the first kind of order 0 */
    TEST_f_f (j0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_f_f (j0, plus_infty, 0),

    AUTO_TESTS_f_f (j0, tonearest),
  };

static void
j0_test (void)
{
  START (j0);
  RUN_TEST_LOOP_f_f (j0, j0_test_data, );
  END;
}


static const struct test_f_f_data j1_test_data[] =
  {
    /* j1 is the Bessel function of the first kind of order 1 */
    TEST_f_f (j1, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_f_f (j1, plus_infty, 0),

    AUTO_TESTS_f_f (j1, tonearest),
  };

static void
j1_test (void)
{
  START (j1);
  RUN_TEST_LOOP_f_f (j1, j1_test_data, );
  END;
}

static const struct test_if_f_data jn_test_data[] =
  {
    /* jn is the Bessel function of the first kind of order n.  */
    /* jn (0, x) == j0 (x)  */
    TEST_if_f (jn, 0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_if_f (jn, 0, plus_infty, 0),

    /* jn (1, x) == j1 (x)  */
    TEST_if_f (jn, 1, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_if_f (jn, 1, plus_infty, 0),

    /* jn (3, x)  */
    TEST_if_f (jn, 3, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_if_f (jn, 3, plus_infty, 0),

    /*  jn (10, x)  */
    TEST_if_f (jn, 10, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_if_f (jn, 10, plus_infty, 0),

    AUTO_TESTS_if_f (jn, tonearest),
  };

static void
jn_test (void)
{
  START (jn);
  RUN_TEST_LOOP_if_f (jn, jn_test_data, );
  END;
}


static const struct test_fi_f_data ldexp_test_data[] =
  {
    TEST_fi_f (ldexp, 0, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_fi_f (ldexp, minus_zero, 0, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_fi_f (ldexp, plus_infty, 1, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_fi_f (ldexp, minus_infty, 1, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_fi_f (ldexp, qnan_value, 1, qnan_value, NO_INEXACT_EXCEPTION),

    TEST_fi_f (ldexp, 0.8L, 4, 12.8L, NO_INEXACT_EXCEPTION),
    TEST_fi_f (ldexp, -0.854375L, 5, -27.34L, NO_INEXACT_EXCEPTION),

    /* ldexp (x, 0) == x.  */
    TEST_fi_f (ldexp, 1.0L, 0L, 1.0L, NO_INEXACT_EXCEPTION),
  };

static void
ldexp_test (void)
{
  START (ldexp);
  RUN_TEST_LOOP_fi_f (ldexp, ldexp_test_data, );
  END;
}


static const struct test_f_f1_data lgamma_test_data[] =
  {
    TEST_f_f1 (lgamma, plus_infty, plus_infty, 1),
    TEST_f_f1 (lgamma, 0, plus_infty, 1, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f1 (lgamma, minus_zero, plus_infty, -1, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f1 (lgamma, qnan_value, qnan_value, IGNORE, NO_INEXACT_EXCEPTION),

    /* lgamma (x) == +inf plus divide by zero exception for integer x <= 0.  */
    TEST_f_f1 (lgamma, -3, plus_infty, IGNORE, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f1 (lgamma, minus_infty, plus_infty, IGNORE),
    TEST_f_f1 (lgamma, -max_value, plus_infty, IGNORE, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),

    AUTO_TESTS_f_f1 (lgamma, tonearest),
  };

static void
lgamma_test (void)
{
  START (lgamma);
  RUN_TEST_LOOP_f_f1 (lgamma, lgamma_test_data, , signgam);
  END;
}

static void
gamma_test (void)
{
  START (gamma);
  /* gamma uses the same test data as lgamma.  */
  RUN_TEST_LOOP_f_f1 (gamma, lgamma_test_data, , signgam);
  END;
}


static const struct test_f_l_data lrint_test_data[] =
  {
    /* XXX this test is incomplete.  We need to have a way to specifiy
       the rounding method and test the critical cases.  So far, only
       unproblematic numbers are tested.  */
    TEST_f_l (lrint, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, 0.0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_l (lrint, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.2L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.2L, 0, INEXACT_EXCEPTION),

    TEST_f_l (lrint, 1.4L, 1, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -1.4L, -1, INEXACT_EXCEPTION),

    TEST_f_l (lrint, 8388600.3L, 8388600, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -8388600.3L, -8388600, INEXACT_EXCEPTION),

#ifndef TEST_FLOAT
    TEST_f_l (lrint, 1071930.0008, 1071930, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 1073741824.01, 1073741824, INEXACT_EXCEPTION),
# if LONG_MAX > 281474976710656 && defined TEST_LDOUBLE
    TEST_f_l (lrint, 281474976710656.025L, 281474976710656, INEXACT_EXCEPTION),
# endif
#endif
  };

static void
lrint_test (void)
{
  START (lrint);
  RUN_TEST_LOOP_f_l (lrint, lrint_test_data, );
  END;
}


static const struct test_f_l_data lrint_tonearest_test_data[] =
  {
    TEST_f_l (lrint, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, 0.0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_l (lrint, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.2L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.2L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.5L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.5L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.8L, 1, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.8L, -1, INEXACT_EXCEPTION),

    TEST_f_l (lrint, 1.4L, 1, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -1.4L, -1, INEXACT_EXCEPTION),

    TEST_f_l (lrint, 8388600.3L, 8388600, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -8388600.3L, -8388600, INEXACT_EXCEPTION),

#ifndef TEST_FLOAT
    TEST_f_l (lrint, 1071930.0008, 1071930, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 1073741824.01, 1073741824, INEXACT_EXCEPTION),
# if LONG_MAX > 281474976710656 && defined TEST_LDOUBLE
    TEST_f_l (lrint, 281474976710656.025L, 281474976710656, INEXACT_EXCEPTION),
# endif
#endif
  };

static void
lrint_test_tonearest (void)
{
  START (lrint_tonearest);
  RUN_TEST_LOOP_f_l (lrint, lrint_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_f_l_data lrint_towardzero_test_data[] =
  {
    TEST_f_l (lrint, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, 0.0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_l (lrint, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.2L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.2L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.5L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.5L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.8L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.8L, 0, INEXACT_EXCEPTION),

    TEST_f_l (lrint, 1.4L, 1, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -1.4L, -1, INEXACT_EXCEPTION),

    TEST_f_l (lrint, 8388600.3L, 8388600, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -8388600.3L, -8388600, INEXACT_EXCEPTION),

#ifndef TEST_FLOAT
    TEST_f_l (lrint, 1071930.0008, 1071930, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 1073741824.01, 1073741824, INEXACT_EXCEPTION),
# if LONG_MAX > 281474976710656 && defined TEST_LDOUBLE
    TEST_f_l (lrint, 281474976710656.025L, 281474976710656, INEXACT_EXCEPTION),
# endif
#endif
  };

static void
lrint_test_towardzero (void)
{
  START (lrint_towardzero);
  RUN_TEST_LOOP_f_l (lrint, lrint_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_f_l_data lrint_downward_test_data[] =
  {
    TEST_f_l (lrint, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, 0.0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_l (lrint, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.2L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.2L, -1, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.5L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.5L, -1, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.8L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.8L, -1, INEXACT_EXCEPTION),

    TEST_f_l (lrint, 1.4L, 1, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -1.4L, -2, INEXACT_EXCEPTION),

    TEST_f_l (lrint, 8388600.3L, 8388600, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -8388600.3L, -8388601, INEXACT_EXCEPTION),

#ifndef TEST_FLOAT
    TEST_f_l (lrint, 1071930.0008, 1071930, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 1073741824.01, 1073741824, INEXACT_EXCEPTION),
# if LONG_MAX > 281474976710656 && defined TEST_LDOUBLE
    TEST_f_l (lrint, 281474976710656.025L, 281474976710656, INEXACT_EXCEPTION),
# endif
#endif
  };

static void
lrint_test_downward (void)
{
  START (lrint_downward);
  RUN_TEST_LOOP_f_l (lrint, lrint_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_f_l_data lrint_upward_test_data[] =
  {
    TEST_f_l (lrint, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lrint, 0.0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_l (lrint, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.2L, 1, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.2L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.5L, 1, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.5L, 0, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 0.8L, 1, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -0.8L, 0, INEXACT_EXCEPTION),

    TEST_f_l (lrint, 1.4L, 2, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -1.4L, -1, INEXACT_EXCEPTION),

    TEST_f_l (lrint, 8388600.3L, 8388601, INEXACT_EXCEPTION),
    TEST_f_l (lrint, -8388600.3L, -8388600, INEXACT_EXCEPTION),

#ifndef TEST_FLOAT
    TEST_f_l (lrint, 1071930.0008, 1071931, INEXACT_EXCEPTION),
    TEST_f_l (lrint, 1073741824.01, 1073741825, INEXACT_EXCEPTION),
# if LONG_MAX > 281474976710656 && defined (TEST_LDOUBLE)
    TEST_f_l (lrint, 281474976710656.025L, 281474976710657, INEXACT_EXCEPTION),
# endif
#endif
  };

static void
lrint_test_upward (void)
{
  START (lrint_upward);
  RUN_TEST_LOOP_f_l (lrint, lrint_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_f_L_data llrint_test_data[] =
  {
    /* XXX this test is incomplete.  We need to have a way to specifiy
       the rounding method and test the critical cases.  So far, only
       unproblematic numbers are tested.  */
    TEST_f_L (llrint, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, 0.0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, 0.2L, 0, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -0.2L, 0, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 1.4L, 1, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -1.4L, -1, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 8388600.3L, 8388600, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -8388600.3L, -8388600, INEXACT_EXCEPTION),

#ifndef TEST_FLOAT
    TEST_f_L (llrint, 1071930.0008, 1071930, INEXACT_EXCEPTION),
#endif

    /* Test boundary conditions.  */
    /* 0x1FFFFF */
    TEST_f_L (llrint, 2097151.0,2097151LL, NO_INEXACT_EXCEPTION),
    /* 0x800000 */
    TEST_f_L (llrint, 8388608.0, 8388608LL, NO_INEXACT_EXCEPTION),
    /* 0x1000000 */
    TEST_f_L (llrint, 16777216.0, 16777216LL, NO_INEXACT_EXCEPTION),
    /* 0x20000000000 */
    TEST_f_L (llrint, 2199023255552.0, 2199023255552LL, NO_INEXACT_EXCEPTION),
    /* 0x40000000000 */
    TEST_f_L (llrint, 4398046511104.0, 4398046511104LL, NO_INEXACT_EXCEPTION),
    /* 0x1000000000000 */
    TEST_f_L (llrint, 281474976710656.0, 281474976710656LL, NO_INEXACT_EXCEPTION),
    /* 0x10000000000000 */
    TEST_f_L (llrint, 4503599627370496.0, 4503599627370496LL, NO_INEXACT_EXCEPTION),
    /* 0x10000080000000 */
    TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL, NO_INEXACT_EXCEPTION),
    /* 0x20000000000000 */
    TEST_f_L (llrint, 9007199254740992.0, 9007199254740992LL, NO_INEXACT_EXCEPTION),
    /* 0x80000000000000 */
    TEST_f_L (llrint, 36028797018963968.0, 36028797018963968LL, NO_INEXACT_EXCEPTION),
    /* 0x100000000000000 */
    TEST_f_L (llrint, 72057594037927936.0, 72057594037927936LL, NO_INEXACT_EXCEPTION),
#ifdef TEST_LDOUBLE
    /* The input can only be represented in long double.  */
    TEST_f_L (llrint, 4503599627370495.5L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.25L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.5L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.75L, 4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370497.5L, 4503599627370498LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -4503599627370495.5L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.25L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.5L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.75L, -4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370497.5L, -4503599627370498LL, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llrint, 4503599627370495.4999999999999L, 4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.4999999999999L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370497.4999999999999L, 4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370494.5000000000001L, 4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370495.5000000000001L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.5000000000001L, 4503599627370497LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -4503599627370495.4999999999999L, -4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.4999999999999L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370497.4999999999999L, -4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370494.5000000000001L, -4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370495.5000000000001L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.5000000000001L, -4503599627370497LL, INEXACT_EXCEPTION),
#endif

    TEST_f_L (llrint, 9007199254740991.5L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.25L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.5L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.75L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.5L, 9007199254740994LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -9007199254740991.5L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.25L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.5L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.75L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.5L, -9007199254740994LL, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llrint, 9007199254740991.4999999999999L, 9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.4999999999999L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.4999999999999L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740991.5000000000001L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.5000000000001L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.5000000000001L, 9007199254740994LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -9007199254740991.4999999999999L, -9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.4999999999999L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.4999999999999L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740991.5000000000001L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.5000000000001L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.5000000000001L, -9007199254740994LL, INEXACT_EXCEPTION),
#endif

    TEST_f_L (llrint, 72057594037927935.5L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.25L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.5L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.75L, 72057594037927937LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927937.5L, 72057594037927938LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -72057594037927935.5L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.25L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.5L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.75L, -72057594037927937LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927937.5L, -72057594037927938LL, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llrint, 9223372036854775805.5L, 9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775805.5L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.0L, 9223372036854775806LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.0L, -9223372036854775806LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.25L, 9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.25L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.5L, 9223372036854775806L, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.5L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.75L, 9223372036854775807LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.75L, -9223372036854775807LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775807.0L, 9223372036854775807LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775807.0L, -9223372036854775807LL, NO_INEXACT_EXCEPTION),
# endif
#endif
  };

static void
llrint_test (void)
{
  START (llrint);
  RUN_TEST_LOOP_f_L (llrint, llrint_test_data, );
  END;
}

static const struct test_f_L_data llrint_tonearest_test_data[] =
  {
    TEST_f_L (llrint, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, 0.0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, 0.2L, 0, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -0.2L, 0, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 1.4L, 1, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -1.4L, -1, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 8388600.3L, 8388600, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -8388600.3L, -8388600, INEXACT_EXCEPTION),

#ifndef TEST_FLOAT
    TEST_f_L (llrint, 1071930.0008, 1071930, INEXACT_EXCEPTION),
#endif

    /* Test boundary conditions.  */
    /* 0x1FFFFF */
    TEST_f_L (llrint, 2097151.0,2097151LL, NO_INEXACT_EXCEPTION),
    /* 0x800000 */
    TEST_f_L (llrint, 8388608.0, 8388608LL, NO_INEXACT_EXCEPTION),
    /* 0x1000000 */
    TEST_f_L (llrint, 16777216.0, 16777216LL, NO_INEXACT_EXCEPTION),
    /* 0x20000000000 */
    TEST_f_L (llrint, 2199023255552.0, 2199023255552LL, NO_INEXACT_EXCEPTION),
    /* 0x40000000000 */
    TEST_f_L (llrint, 4398046511104.0, 4398046511104LL, NO_INEXACT_EXCEPTION),
    /* 0x1000000000000 */
    TEST_f_L (llrint, 281474976710656.0, 281474976710656LL, NO_INEXACT_EXCEPTION),
    /* 0x10000000000000 */
    TEST_f_L (llrint, 4503599627370496.0, 4503599627370496LL, NO_INEXACT_EXCEPTION),
    /* 0x10000080000000 */
    TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL, NO_INEXACT_EXCEPTION),
    /* 0x20000000000000 */
    TEST_f_L (llrint, 9007199254740992.0, 9007199254740992LL, NO_INEXACT_EXCEPTION),
    /* 0x80000000000000 */
    TEST_f_L (llrint, 36028797018963968.0, 36028797018963968LL, NO_INEXACT_EXCEPTION),
    /* 0x100000000000000 */
    TEST_f_L (llrint, 72057594037927936.0, 72057594037927936LL, NO_INEXACT_EXCEPTION),
#ifdef TEST_LDOUBLE
    /* The input can only be represented in long double.  */
    TEST_f_L (llrint, 4503599627370495.5L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.25L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.5L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.75L, 4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370497.5L, 4503599627370498LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -4503599627370495.5L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.25L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.5L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.75L, -4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370497.5L, -4503599627370498LL, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llrint, 4503599627370495.4999999999999L, 4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.4999999999999L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370497.4999999999999L, 4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370494.5000000000001L, 4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370495.5000000000001L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.5000000000001L, 4503599627370497LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -4503599627370495.4999999999999L, -4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.4999999999999L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370497.4999999999999L, -4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370494.5000000000001L, -4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370495.5000000000001L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.5000000000001L, -4503599627370497LL, INEXACT_EXCEPTION),
#endif

    TEST_f_L (llrint, 9007199254740991.5L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.25L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.5L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.75L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.5L, 9007199254740994LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -9007199254740991.5L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.25L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.5L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.75L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.5L, -9007199254740994LL, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llrint, 9007199254740991.4999999999999L, 9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.4999999999999L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.4999999999999L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740991.5000000000001L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.5000000000001L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.5000000000001L, 9007199254740994LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -9007199254740991.4999999999999L, -9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.4999999999999L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.4999999999999L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740991.5000000000001L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.5000000000001L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.5000000000001L, -9007199254740994LL, INEXACT_EXCEPTION),
#endif

    TEST_f_L (llrint, 72057594037927935.5L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.25L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.5L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.75L, 72057594037927937LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927937.5L, 72057594037927938LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -72057594037927935.5L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.25L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.5L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.75L, -72057594037927937LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927937.5L, -72057594037927938LL, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llrint, 9223372036854775805.5L, 9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775805.5L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.0L, 9223372036854775806LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.0L, -9223372036854775806LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.25L, 9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.25L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.5L, 9223372036854775806L, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.5L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.75L, 9223372036854775807LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.75L, -9223372036854775807LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775807.0L, 9223372036854775807LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775807.0L, -9223372036854775807LL, NO_INEXACT_EXCEPTION),
# endif
#endif
  };

static void
llrint_test_tonearest (void)
{
  START (llrint_tonearest);
  RUN_TEST_LOOP_f_L (llrint, llrint_tonearest_test_data, FE_TONEAREST);
  END;
}

static const struct test_f_L_data llrint_towardzero_test_data[] =
  {
    TEST_f_L (llrint, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, 0.0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, 0.2L, 0, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -0.2L, 0, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 1.4L, 1, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -1.4L, -1, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 8388600.3L, 8388600, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -8388600.3L, -8388600, INEXACT_EXCEPTION),

#ifndef TEST_FLOAT
    TEST_f_L (llrint, 1071930.0008, 1071930, INEXACT_EXCEPTION),
#endif

    /* Test boundary conditions.  */
    /* 0x1FFFFF */
    TEST_f_L (llrint, 2097151.0,2097151LL, NO_INEXACT_EXCEPTION),
    /* 0x800000 */
    TEST_f_L (llrint, 8388608.0, 8388608LL, NO_INEXACT_EXCEPTION),
    /* 0x1000000 */
    TEST_f_L (llrint, 16777216.0, 16777216LL, NO_INEXACT_EXCEPTION),
    /* 0x20000000000 */
    TEST_f_L (llrint, 2199023255552.0, 2199023255552LL, NO_INEXACT_EXCEPTION),
    /* 0x40000000000 */
    TEST_f_L (llrint, 4398046511104.0, 4398046511104LL, NO_INEXACT_EXCEPTION),
    /* 0x1000000000000 */
    TEST_f_L (llrint, 281474976710656.0, 281474976710656LL, NO_INEXACT_EXCEPTION),
    /* 0x10000000000000 */
    TEST_f_L (llrint, 4503599627370496.0, 4503599627370496LL, NO_INEXACT_EXCEPTION),
    /* 0x10000080000000 */
    TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL, NO_INEXACT_EXCEPTION),
    /* 0x20000000000000 */
    TEST_f_L (llrint, 9007199254740992.0, 9007199254740992LL, NO_INEXACT_EXCEPTION),
    /* 0x80000000000000 */
    TEST_f_L (llrint, 36028797018963968.0, 36028797018963968LL, NO_INEXACT_EXCEPTION),
    /* 0x100000000000000 */
    TEST_f_L (llrint, 72057594037927936.0, 72057594037927936LL, NO_INEXACT_EXCEPTION),
#ifdef TEST_LDOUBLE
    /* The input can only be represented in long double.  */
    TEST_f_L (llrint, 4503599627370495.5L, 4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.25L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.5L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.75L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370497.5L, 4503599627370497LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -4503599627370495.5L, -4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.25L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.5L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.75L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370497.5L, -4503599627370497LL, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llrint, 4503599627370495.4999999999999L, 4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.4999999999999L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370497.4999999999999L, 4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370494.5000000000001L, 4503599627370494LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370495.5000000000001L, 4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.5000000000001L, 4503599627370496LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -4503599627370495.4999999999999L, -4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.4999999999999L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370497.4999999999999L, -4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370494.5000000000001L, -4503599627370494LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370495.5000000000001L, -4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.5000000000001L, -4503599627370496LL, INEXACT_EXCEPTION),
#endif

    TEST_f_L (llrint, 9007199254740991.5L, 9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.25L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.5L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.75L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.5L, 9007199254740993LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -9007199254740991.5L, -9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.25L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.5L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.75L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.5L, -9007199254740993LL, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llrint, 9007199254740991.4999999999999L, 9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.4999999999999L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.4999999999999L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740991.5000000000001L, 9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.5000000000001L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.5000000000001L, 9007199254740993LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -9007199254740991.4999999999999L, -9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.4999999999999L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.4999999999999L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740991.5000000000001L, -9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.5000000000001L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.5000000000001L, -9007199254740993LL, INEXACT_EXCEPTION),
#endif

    TEST_f_L (llrint, 72057594037927935.5L, 72057594037927935LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.25L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.5L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.75L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927937.5L, 72057594037927937LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -72057594037927935.5L, -72057594037927935LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.25L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.5L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.75L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927937.5L, -72057594037927937LL, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llrint, 9223372036854775805.5L, 9223372036854775805LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775805.5L, -9223372036854775805LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.0L, 9223372036854775806LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.0L, -9223372036854775806LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.25L, 9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.25L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.5L, 9223372036854775806L, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.5L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.75L, 9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.75L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775807.0L, 9223372036854775807LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775807.0L, -9223372036854775807LL, NO_INEXACT_EXCEPTION),
# endif
#endif
  };

static void
llrint_test_towardzero (void)
{
  START (llrint_towardzero);
  RUN_TEST_LOOP_f_L (llrint, llrint_towardzero_test_data, FE_TOWARDZERO);
  END;
}

static const struct test_f_L_data llrint_downward_test_data[] =
  {
    TEST_f_L (llrint, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, 0.0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, 0.2L, 0, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -0.2L, -1, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 1.4L, 1, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -1.4L, -2, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 8388600.3L, 8388600, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -8388600.3L, -8388601, INEXACT_EXCEPTION),

#ifndef TEST_FLOAT
    TEST_f_L (llrint, 1071930.0008, 1071930, INEXACT_EXCEPTION),
#endif

    /* Test boundary conditions.  */
    /* 0x1FFFFF */
    TEST_f_L (llrint, 2097151.0,2097151LL, NO_INEXACT_EXCEPTION),
    /* 0x800000 */
    TEST_f_L (llrint, 8388608.0, 8388608LL, NO_INEXACT_EXCEPTION),
    /* 0x1000000 */
    TEST_f_L (llrint, 16777216.0, 16777216LL, NO_INEXACT_EXCEPTION),
    /* 0x20000000000 */
    TEST_f_L (llrint, 2199023255552.0, 2199023255552LL, NO_INEXACT_EXCEPTION),
    /* 0x40000000000 */
    TEST_f_L (llrint, 4398046511104.0, 4398046511104LL, NO_INEXACT_EXCEPTION),
    /* 0x1000000000000 */
    TEST_f_L (llrint, 281474976710656.0, 281474976710656LL, NO_INEXACT_EXCEPTION),
    /* 0x10000000000000 */
    TEST_f_L (llrint, 4503599627370496.0, 4503599627370496LL, NO_INEXACT_EXCEPTION),
    /* 0x10000080000000 */
    TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL, NO_INEXACT_EXCEPTION),
    /* 0x20000000000000 */
    TEST_f_L (llrint, 9007199254740992.0, 9007199254740992LL, NO_INEXACT_EXCEPTION),
    /* 0x80000000000000 */
    TEST_f_L (llrint, 36028797018963968.0, 36028797018963968LL, NO_INEXACT_EXCEPTION),
    /* 0x100000000000000 */
    TEST_f_L (llrint, 72057594037927936.0, 72057594037927936LL, NO_INEXACT_EXCEPTION),
#ifdef TEST_LDOUBLE
    /* The input can only be represented in long double.  */
    TEST_f_L (llrint, 4503599627370495.5L, 4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.25L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.5L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.75L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370497.5L, 4503599627370497LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 4503599627370495.4999999999999L, 4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.4999999999999L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370497.4999999999999L, 4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370494.5000000000001L, 4503599627370494LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370495.5000000000001L, 4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.5000000000001L, 4503599627370496LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -4503599627370495.5L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.25L, -4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.5L, -4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.75L, -4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370497.5L, -4503599627370498LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -4503599627370495.4999999999999L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.4999999999999L, -4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370497.4999999999999L, -4503599627370498LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370494.5000000000001L, -4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370495.5000000000001L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.5000000000001L, -4503599627370497LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 9007199254740991.5L, 9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.25L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.5L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.75L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.5L, 9007199254740993LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 9007199254740991.4999999999999L, 9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.4999999999999L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.4999999999999L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740991.5000000000001L, 9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.5000000000001L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.5000000000001L, 9007199254740993LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -9007199254740991.5L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.25L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.5L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.75L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.5L, -9007199254740994LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -9007199254740991.4999999999999L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.4999999999999L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.4999999999999L, -9007199254740994LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740991.5000000000001L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.5000000000001L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.5000000000001L, -9007199254740994LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 72057594037927935.5L, 72057594037927935LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.25L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.5L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.75L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927937.5L, 72057594037927937LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -72057594037927935.5L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.25L, -72057594037927937LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.5L, -72057594037927937LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.75L, -72057594037927937LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927937.5L, -72057594037927938LL, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llrint, 9223372036854775805.5L, 9223372036854775805LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775805.5L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.0L, 9223372036854775806LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.0L, -9223372036854775806LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.25L, 9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.25L, -9223372036854775807LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.5L, 9223372036854775806L, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.5L, -9223372036854775807LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.75L, 9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.75L, -9223372036854775807LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775807.0L, 9223372036854775807LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775807.0L, -9223372036854775807LL, NO_INEXACT_EXCEPTION),
# endif
#endif
  };

static void
llrint_test_downward (void)
{
  START (llrint_downward);
  RUN_TEST_LOOP_f_L (llrint, llrint_downward_test_data, FE_DOWNWARD);
  END;
}

static const struct test_f_L_data llrint_upward_test_data[] =
  {
    TEST_f_L (llrint, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llrint, 0.0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, minus_zero, 0, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, 0.2L, 1, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -0.2L, 0, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 1.4L, 2, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -1.4L, -1, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 8388600.3L, 8388601, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -8388600.3L, -8388600, INEXACT_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_f_L (llrint, 1071930.0008, 1071931, INEXACT_EXCEPTION),
#endif
    /* Test boundary conditions.  */
    /* 0x1FFFFF */
    TEST_f_L (llrint, 2097151.0,2097151LL, NO_INEXACT_EXCEPTION),
    /* 0x800000 */
    TEST_f_L (llrint, 8388608.0, 8388608LL, NO_INEXACT_EXCEPTION),
    /* 0x1000000 */
    TEST_f_L (llrint, 16777216.0, 16777216LL, NO_INEXACT_EXCEPTION),
    /* 0x20000000000 */
    TEST_f_L (llrint, 2199023255552.0, 2199023255552LL, NO_INEXACT_EXCEPTION),
    /* 0x40000000000 */
    TEST_f_L (llrint, 4398046511104.0, 4398046511104LL, NO_INEXACT_EXCEPTION),
    /* 0x1000000000000 */
    TEST_f_L (llrint, 281474976710656.0, 281474976710656LL, NO_INEXACT_EXCEPTION),
    /* 0x10000000000000 */
    TEST_f_L (llrint, 4503599627370496.0, 4503599627370496LL, NO_INEXACT_EXCEPTION),
    /* 0x10000080000000 */
    TEST_f_L (llrint, 4503601774854144.0, 4503601774854144LL, NO_INEXACT_EXCEPTION),
    /* 0x20000000000000 */
    TEST_f_L (llrint, 9007199254740992.0, 9007199254740992LL, NO_INEXACT_EXCEPTION),
    /* 0x80000000000000 */
    TEST_f_L (llrint, 36028797018963968.0, 36028797018963968LL, NO_INEXACT_EXCEPTION),
    /* 0x100000000000000 */
    TEST_f_L (llrint, 72057594037927936.0, 72057594037927936LL, NO_INEXACT_EXCEPTION),
#ifdef TEST_LDOUBLE
    /* The input can only be represented in long double.  */
    TEST_f_L (llrint, 4503599627370495.5L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.25L, 4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.5L, 4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.75L, 4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370497.5L, 4503599627370498LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 4503599627370495.4999999999999L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.4999999999999L, 4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370497.4999999999999L, 4503599627370498LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370494.5000000000001L, 4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370495.5000000000001L, 4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 4503599627370496.5000000000001L, 4503599627370497LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -4503599627370495.5L, -4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.25L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.5L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.75L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370497.5L, -4503599627370497LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -4503599627370495.4999999999999L, -4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.4999999999999L, -4503599627370496LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370497.4999999999999L, -4503599627370497LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370494.5000000000001L, -4503599627370494LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370495.5000000000001L, -4503599627370495LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -4503599627370496.5000000000001L, -4503599627370496LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 9007199254740991.5L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.25L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.5L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.75L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.5L, 9007199254740994LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 9007199254740991.4999999999999L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.4999999999999L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.4999999999999L, 9007199254740994LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740991.5000000000001L, 9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740992.5000000000001L, 9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9007199254740993.5000000000001L, 9007199254740994LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -9007199254740991.5L, -9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.25L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.5L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.75L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.5L, -9007199254740993LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -9007199254740991.4999999999999L, -9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.4999999999999L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.4999999999999L, -9007199254740993LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740991.5000000000001L, -9007199254740991LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740992.5000000000001L, -9007199254740992LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9007199254740993.5000000000001L, -9007199254740993LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, 72057594037927935.5L, 72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.25L, 72057594037927937LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.5L, 72057594037927937LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927936.75L, 72057594037927937LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 72057594037927937.5L, 72057594037927938LL, INEXACT_EXCEPTION),

    TEST_f_L (llrint, -72057594037927935.5L, -72057594037927935LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.25L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.5L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927936.75L, -72057594037927936LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -72057594037927937.5L, -72057594037927937LL, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llrint, 9223372036854775805.5L, 9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775805.5L, -9223372036854775805LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.0L, 9223372036854775806LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.0L, -9223372036854775806LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.25L, 9223372036854775807LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.25L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.5L, 9223372036854775807L, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.5L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775806.75L, 9223372036854775807LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775806.75L, -9223372036854775806LL, INEXACT_EXCEPTION),
    TEST_f_L (llrint, 9223372036854775807.0L, 9223372036854775807LL, NO_INEXACT_EXCEPTION),
    TEST_f_L (llrint, -9223372036854775807.0L, -9223372036854775807LL, NO_INEXACT_EXCEPTION),
# endif
#endif
  };

static void
llrint_test_upward (void)
{
  START (llrint_upward);
  RUN_TEST_LOOP_f_L (llrint, llrint_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_f_f_data log_test_data[] =
  {
    TEST_f_f (log, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (log, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),

    TEST_f_f (log, -1, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (log, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (log, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (log, plus_infty, plus_infty),
    TEST_f_f (log, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (log, tonearest),
  };

static void
log_test (void)
{
  START (log);
  RUN_TEST_LOOP_f_f (log, log_test_data, );
  END;
}


static const struct test_f_f_data log10_test_data[] =
  {
    TEST_f_f (log10, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (log10, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),

    /* log10 (x) == qNaN plus invalid exception if x < 0.  */
    TEST_f_f (log10, -1, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (log10, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (log10, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),

    TEST_f_f (log10, plus_infty, plus_infty),
    TEST_f_f (log10, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (log10, tonearest),
  };

static void
log10_test (void)
{
  START (log10);
  RUN_TEST_LOOP_f_f (log10, log10_test_data, );
  END;
}


static const struct test_f_f_data log1p_test_data[] =
  {
    TEST_f_f (log1p, -1, minus_infty, DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f (log1p, -2, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log1p, -max_value, qnan_value, INVALID_EXCEPTION),
    TEST_f_f (log1p, minus_infty, qnan_value, INVALID_EXCEPTION),

    TEST_f_f (log1p, plus_infty, plus_infty),
    TEST_f_f (log1p, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (log1p, tonearest),
  };

static void
log1p_test (void)
{
  START (log1p);
  RUN_TEST_LOOP_f_f (log1p, log1p_test_data, );
  END;
}


static const struct test_f_f_data log2_test_data[] =
  {
    TEST_f_f (log2, 0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (log2, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),

    TEST_f_f (log2, -1, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (log2, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (log2, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),

    TEST_f_f (log2, plus_infty, plus_infty),
    TEST_f_f (log2, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (log2, tonearest),
  };

static void
log2_test (void)
{
  START (log2);
  RUN_TEST_LOOP_f_f (log2, log2_test_data, );
  END;
}


static const struct test_f_f_data logb_test_data[] =
  {
    TEST_f_f (logb, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, minus_infty, plus_infty, NO_INEXACT_EXCEPTION),

    TEST_f_f (logb, 0, minus_infty, NO_INEXACT_EXCEPTION|DIVIDE_BY_ZERO_EXCEPTION),

    TEST_f_f (logb, minus_zero, minus_infty, NO_INEXACT_EXCEPTION|DIVIDE_BY_ZERO_EXCEPTION),
    TEST_f_f (logb, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    TEST_f_f (logb, 1, 0, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, M_El, 1, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, 1024, 10, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, -2000, 10, NO_INEXACT_EXCEPTION),

    TEST_f_f (logb, 0x0.1p-127, -131, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, 0x0.01p-127, -135, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, 0x0.011p-127, -135, NO_INEXACT_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_f_f (logb, 0x0.8p-1022, -1023, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, 0x0.1p-1022, -1026, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, 0x0.00111p-1022, -1034, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, 0x0.00001p-1022, -1042, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, 0x0.000011p-1022, -1042, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, 0x0.0000000000001p-1022, -1074, NO_INEXACT_EXCEPTION),
#endif
#if defined TEST_LDOUBLE && LDBL_MIN_EXP - LDBL_MANT_DIG <= -16400
    TEST_f_f (logb, 0x1p-16400L, -16400, NO_INEXACT_EXCEPTION),
    TEST_f_f (logb, 0x.00000000001p-16382L, -16426, NO_INEXACT_EXCEPTION),
#endif
  };

static void
logb_test (void)
{
  START (logb);
  RUN_TEST_LOOP_f_f (logb, logb_test_data, );
  END;
}

static const struct test_f_f_data logb_downward_test_data[] =
  {
    /* IEEE 754-2008 says (section 5.3.3) that "logB(1) is +0.".  Libm
       should not return -0 from logb in any rounding mode.  PowerPC32 has
       failed with this test for power4 logb (and logbl on all PowerPC
       platforms) in the past due to instruction selection.  GCC PR 52775
       provides the availability of the fcfid insn in 32-bit mode which
       eliminates the use of fsub in this instance and prevents the negative
       signed 0.0.  */

    /* BZ #887  */
    TEST_f_f (logb, 1.000e+0, plus_zero, NO_INEXACT_EXCEPTION),
  };

static void
logb_test_downward (void)
{
  START (logb_downward);
  RUN_TEST_LOOP_f_f (logb, logb_downward_test_data, FE_DOWNWARD);
  END;
}

static const struct test_f_l_data lround_test_data[] =
  {
    TEST_f_l (lround, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lround, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lround, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_l (lround, 0, 0),
    TEST_f_l (lround, minus_zero, 0),
    TEST_f_l (lround, 0.2L, 0.0),
    TEST_f_l (lround, -0.2L, 0),
    TEST_f_l (lround, 0.5, 1),
    TEST_f_l (lround, -0.5, -1),
    TEST_f_l (lround, 0.8L, 1),
    TEST_f_l (lround, -0.8L, -1),
    TEST_f_l (lround, 1.5, 2),
    TEST_f_l (lround, -1.5, -2),
    TEST_f_l (lround, 22514.5, 22515),
    TEST_f_l (lround, -22514.5, -22515),
    TEST_f_l (lround, 1071930.0008, 1071930),
#ifndef TEST_FLOAT
    TEST_f_l (lround, 1073741824.01, 1073741824),
# if LONG_MAX > 281474976710656
    TEST_f_l (lround, 281474976710656.025, 281474976710656),
    TEST_f_l (lround, 18014398509481974, 18014398509481974),
# endif
    TEST_f_l (lround, 2097152.5, 2097153),
    TEST_f_l (lround, -2097152.5, -2097153),
    /* nextafter(0.5,-1)  */
    TEST_f_l (lround, 0x1.fffffffffffffp-2, 0),
    /* nextafter(-0.5,1)  */
    TEST_f_l (lround, -0x1.fffffffffffffp-2, 0),
#else
    /* nextafter(0.5,-1)  */
    TEST_f_l (lround, 0x1.fffffp-2, 0),
    /* nextafter(-0.5,1)  */
    TEST_f_l (lround, -0x1.fffffp-2, 0),
    TEST_f_l (lround, 0x1.fffffep+23, 16777215),
    TEST_f_l (lround, -0x1.fffffep+23, -16777215),
#endif
  };

static void
lround_test (void)
{
  START (lround);
  RUN_TEST_LOOP_f_l (lround, lround_test_data, );
  END;
}


static const struct test_f_L_data llround_test_data[] =
  {
    TEST_f_L (llround, plus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llround, minus_infty, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llround, qnan_value, IGNORE, INVALID_EXCEPTION),
    TEST_f_L (llround, 0, 0),
    TEST_f_L (llround, minus_zero, 0),
    TEST_f_L (llround, 0.2L, 0.0),
    TEST_f_L (llround, -0.2L, 0),
    TEST_f_L (llround, 0.5, 1),
    TEST_f_L (llround, -0.5, -1),
    TEST_f_L (llround, 0.8L, 1),
    TEST_f_L (llround, -0.8L, -1),
    TEST_f_L (llround, 1.5, 2),
    TEST_f_L (llround, -1.5, -2),
    TEST_f_L (llround, 22514.5, 22515),
    TEST_f_L (llround, -22514.5, -22515),
    TEST_f_L (llround, 1071930.0008, 1071930),
#ifndef TEST_FLOAT
    TEST_f_L (llround, 2097152.5, 2097153),
    TEST_f_L (llround, -2097152.5, -2097153),
    TEST_f_L (llround, 34359738368.5, 34359738369ll),
    TEST_f_L (llround, -34359738368.5, -34359738369ll),
    TEST_f_L (llround, -3.65309740835E17, -365309740835000000LL),
#endif

    /* Test boundary conditions.  */
    /* 0x1FFFFF */
    TEST_f_L (llround, 2097151.0, 2097151LL),
    /* 0x800000 */
    TEST_f_L (llround, 8388608.0, 8388608LL),
    /* 0x1000000 */
    TEST_f_L (llround, 16777216.0, 16777216LL),
    /* 0x20000000000 */
    TEST_f_L (llround, 2199023255552.0, 2199023255552LL),
    /* 0x40000000000 */
    TEST_f_L (llround, 4398046511104.0, 4398046511104LL),
    /* 0x1000000000000 */
    TEST_f_L (llround, 281474976710656.0, 281474976710656LL),
    /* 0x10000000000000 */
    TEST_f_L (llround, 4503599627370496.0, 4503599627370496LL),
    /* 0x10000080000000 */
    TEST_f_L (llround, 4503601774854144.0, 4503601774854144LL),
    /* 0x20000000000000 */
    TEST_f_L (llround, 9007199254740992.0, 9007199254740992LL),
    /* 0x80000000000000 */
    TEST_f_L (llround, 36028797018963968.0, 36028797018963968LL),
    /* 0x100000000000000 */
    TEST_f_L (llround, 72057594037927936.0, 72057594037927936LL),

#ifndef TEST_FLOAT
    /* 0x100000000 */
    TEST_f_L (llround, 4294967295.5, 4294967296LL),
    /* 0x200000000 */
    TEST_f_L (llround, 8589934591.5, 8589934592LL),

    /* nextafter(0.5,-1)  */
    TEST_f_L (llround, 0x1.fffffffffffffp-2, 0),
    /* nextafter(-0.5,1)  */
    TEST_f_L (llround, -0x1.fffffffffffffp-2, 0),
    /* On PowerPC an exponent of '52' is the largest incrementally
     * representable sequence of whole-numbers in the 'double' range.  We test
     * lround to make sure that a guard bit set during the lround operation
     * hasn't forced an erroneous shift giving us an incorrect result.  The odd
     * numbers between +-(2^52+1 and 2^53-1) are affected since they have the
     * rightmost bit set.  */
    /* +-(2^52+1)  */
    TEST_f_L (llround, 0x1.0000000000001p+52,4503599627370497LL),
    TEST_f_L (llround, -0x1.0000000000001p+52,-4503599627370497LL),
    /* +-(2^53-1): Input is the last (positive and negative) incrementally
     * representable whole-number in the 'double' range that might round
     * erroneously.  */
    TEST_f_L (llround, 0x1.fffffffffffffp+52, 9007199254740991LL),
    TEST_f_L (llround, -0x1.fffffffffffffp+52, -9007199254740991LL),
#else
    /* nextafter(0.5,-1)  */
    TEST_f_L (llround, 0x1.fffffep-2, 0),
    /* nextafter(-0.5,1)  */
    TEST_f_L (llround, -0x1.fffffep-2, 0),
    /* As above, on PowerPC an exponent of '23' is the largest incrementally
     * representable sequence of whole-numbers in the 'float' range.
     * Likewise, numbers between +-(2^23+1 and 2^24-1) are affected.  */
    TEST_f_L (llround, 0x1.000002p+23,8388609),
    TEST_f_L (llround, -0x1.000002p+23,-8388609),
    TEST_f_L (llround, 0x1.fffffep+23, 16777215),
    TEST_f_L (llround, -0x1.fffffep+23, -16777215),
#endif


#ifdef TEST_LDOUBLE
    /* The input can only be represented in long double.  */
    TEST_f_L (llround, 4503599627370495.5L, 4503599627370496LL),
    TEST_f_L (llround, 4503599627370496.25L, 4503599627370496LL),
    TEST_f_L (llround, 4503599627370496.5L, 4503599627370497LL),
    TEST_f_L (llround, 4503599627370496.75L, 4503599627370497LL),
    TEST_f_L (llround, 4503599627370497.5L, 4503599627370498LL),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llround, 4503599627370495.4999999999999L, 4503599627370495LL),
    TEST_f_L (llround, 4503599627370496.4999999999999L, 4503599627370496LL),
    TEST_f_L (llround, 4503599627370497.4999999999999L, 4503599627370497LL),
    TEST_f_L (llround, 4503599627370494.5000000000001L, 4503599627370495LL),
    TEST_f_L (llround, 4503599627370495.5000000000001L, 4503599627370496LL),
    TEST_f_L (llround, 4503599627370496.5000000000001L, 4503599627370497LL),

    TEST_f_L (llround, -4503599627370495.4999999999999L, -4503599627370495LL),
    TEST_f_L (llround, -4503599627370496.4999999999999L, -4503599627370496LL),
    TEST_f_L (llround, -4503599627370497.4999999999999L, -4503599627370497LL),
    TEST_f_L (llround, -4503599627370494.5000000000001L, -4503599627370495LL),
    TEST_f_L (llround, -4503599627370495.5000000000001L, -4503599627370496LL),
    TEST_f_L (llround, -4503599627370496.5000000000001L, -4503599627370497LL),
# endif

    TEST_f_L (llround, -4503599627370495.5L, -4503599627370496LL),
    TEST_f_L (llround, -4503599627370496.25L, -4503599627370496LL),
    TEST_f_L (llround, -4503599627370496.5L, -4503599627370497LL),
    TEST_f_L (llround, -4503599627370496.75L, -4503599627370497LL),
    TEST_f_L (llround, -4503599627370497.5L, -4503599627370498LL),

    TEST_f_L (llround, 9007199254740991.5L, 9007199254740992LL),
    TEST_f_L (llround, 9007199254740992.25L, 9007199254740992LL),
    TEST_f_L (llround, 9007199254740992.5L, 9007199254740993LL),
    TEST_f_L (llround, 9007199254740992.75L, 9007199254740993LL),
    TEST_f_L (llround, 9007199254740993.5L, 9007199254740994LL),

# if LDBL_MANT_DIG > 100
    TEST_f_L (llround, 9007199254740991.4999999999999L, 9007199254740991LL),
    TEST_f_L (llround, 9007199254740992.4999999999999L, 9007199254740992LL),
    TEST_f_L (llround, 9007199254740993.4999999999999L, 9007199254740993LL),
    TEST_f_L (llround, 9007199254740991.5000000000001L, 9007199254740992LL),
    TEST_f_L (llround, 9007199254740992.5000000000001L, 9007199254740993LL),
    TEST_f_L (llround, 9007199254740993.5000000000001L, 9007199254740994LL),

    TEST_f_L (llround, -9007199254740991.4999999999999L, -9007199254740991LL),
    TEST_f_L (llround, -9007199254740992.4999999999999L, -9007199254740992LL),
    TEST_f_L (llround, -9007199254740993.4999999999999L, -9007199254740993LL),
    TEST_f_L (llround, -9007199254740991.5000000000001L, -9007199254740992LL),
    TEST_f_L (llround, -9007199254740992.5000000000001L, -9007199254740993LL),
    TEST_f_L (llround, -9007199254740993.5000000000001L, -9007199254740994LL),
# endif

    TEST_f_L (llround, -9007199254740991.5L, -9007199254740992LL),
    TEST_f_L (llround, -9007199254740992.25L, -9007199254740992LL),
    TEST_f_L (llround, -9007199254740992.5L, -9007199254740993LL),
    TEST_f_L (llround, -9007199254740992.75L, -9007199254740993LL),
    TEST_f_L (llround, -9007199254740993.5L, -9007199254740994LL),

    TEST_f_L (llround, 72057594037927935.5L, 72057594037927936LL),
    TEST_f_L (llround, 72057594037927936.25L, 72057594037927936LL),
    TEST_f_L (llround, 72057594037927936.5L, 72057594037927937LL),
    TEST_f_L (llround, 72057594037927936.75L, 72057594037927937LL),
    TEST_f_L (llround, 72057594037927937.5L, 72057594037927938LL),

    TEST_f_L (llround, -72057594037927935.5L, -72057594037927936LL),
    TEST_f_L (llround, -72057594037927936.25L, -72057594037927936LL),
    TEST_f_L (llround, -72057594037927936.5L, -72057594037927937LL),
    TEST_f_L (llround, -72057594037927936.75L, -72057594037927937LL),
    TEST_f_L (llround, -72057594037927937.5L, -72057594037927938LL),

    TEST_f_L (llround, 9223372036854775806.25L, 9223372036854775806LL),
    TEST_f_L (llround, -9223372036854775806.25L, -9223372036854775806LL),
    TEST_f_L (llround, 9223372036854775806.5L, 9223372036854775807LL),
    TEST_f_L (llround, -9223372036854775806.5L, -9223372036854775807LL),
    TEST_f_L (llround, 9223372036854775807.0L, 9223372036854775807LL),
    TEST_f_L (llround, -9223372036854775807.0L, -9223372036854775807LL),
#endif
  };

static void
llround_test (void)
{
  START (llround);
  RUN_TEST_LOOP_f_L (llround, llround_test_data, );
  END;
}

static const struct test_fF_f1_data modf_test_data[] =
  {
    TEST_fF_f1 (modf, plus_infty, 0, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_fF_f1 (modf, minus_infty, minus_zero, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_fF_f1 (modf, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_fF_f1 (modf, 0, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_fF_f1 (modf, 1.5, 0.5, 1, NO_INEXACT_EXCEPTION),
    TEST_fF_f1 (modf, 2.5, 0.5, 2, NO_INEXACT_EXCEPTION),
    TEST_fF_f1 (modf, -2.5, -0.5, -2, NO_INEXACT_EXCEPTION),
    TEST_fF_f1 (modf, 20, 0, 20, NO_INEXACT_EXCEPTION),
    TEST_fF_f1 (modf, 21, 0, 21, NO_INEXACT_EXCEPTION),
    TEST_fF_f1 (modf, 89.5, 0.5, 89, NO_INEXACT_EXCEPTION),
  };

static void
modf_test (void)
{
  FLOAT x;

  START (modf);
  RUN_TEST_LOOP_fF_f1 (modf, modf_test_data, , x);
  END;
}


static const struct test_f_f_data nearbyint_test_data[] =
  {
    TEST_f_f (nearbyint, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* Subnormal values */
    TEST_f_f (nearbyint, -8.98847e+307, -8.98847e+307, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -4.45015e-308, minus_zero, NO_INEXACT_EXCEPTION),

    /* Default rounding mode is round to nearest.  */
    TEST_f_f (nearbyint, 0.5, 0.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 1.5, 2.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -0.5, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -1.5, -2.0, NO_INEXACT_EXCEPTION),

    TEST_f_f (nearbyint, 262144.75, 262145.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 262142.75, 262143.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 524286.75, 524287.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 524288.75, 524289.0, NO_INEXACT_EXCEPTION),

    TEST_f_f (nearbyint, 1048576.75, 1048577.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 2097152.75, 2097153.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 2492472.75, 2492473.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 2886220.75, 2886221.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 3058792.75, 3058793.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -1048576.75, -1048577.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -2097152.75, -2097153.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -2492472.75, -2492473.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -2886220.75, -2886221.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -3058792.75, -3058793.0, NO_INEXACT_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_f_f (nearbyint, 70368744177664.75, 70368744177665.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 140737488355328.75, 140737488355329.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 281474976710656.75, 281474976710657.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 562949953421312.75, 562949953421313.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, 1125899906842624.75, 1125899906842625.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -70368744177664.75, -70368744177665.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -140737488355328.75, -140737488355329.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -281474976710656.75, -281474976710657.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -562949953421312.75, -562949953421313.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (nearbyint, -1125899906842624.75, -1125899906842625.0, NO_INEXACT_EXCEPTION),
#endif
  };

static void
nearbyint_test (void)
{
  START (nearbyint);
  RUN_TEST_LOOP_f_f (nearbyint, nearbyint_test_data, );
  END;
}

static const struct test_ff_f_data nextafter_test_data[] =
  {
    TEST_ff_f (nextafter, 0, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nextafter, minus_zero, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nextafter, 0, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nextafter, minus_zero, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_ff_f (nextafter, 9, 9, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nextafter, -9, -9, -9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nextafter, plus_infty, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nextafter, minus_infty, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),

    TEST_ff_f (nextafter, qnan_value, 1.1L, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nextafter, 1.1L, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nextafter, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    TEST_ff_f (nextafter, max_value, plus_infty, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_ff_f (nextafter, -max_value, minus_infty, minus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),

#ifdef TEST_LDOUBLE
    // XXX Enable once gcc is fixed.
    //TEST_ff_f (nextafter, 0x0.00000040000000000000p-16385L, -0.1L, 0x0.0000003ffffffff00000p-16385L),
#endif

    /* XXX We need the hexadecimal FP number representation here for further
       tests.  */
  };

static void
nextafter_test (void)
{

  START (nextafter);
  RUN_TEST_LOOP_ff_f (nextafter, nextafter_test_data, );
  END;
}


static const struct test_ff_f_data_nexttoward nexttoward_test_data[] =
  {
    TEST_ff_f (nexttoward, 0, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, minus_zero, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 0, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, minus_zero, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_ff_f (nexttoward, 9, 9, 9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -9, -9, -9, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, plus_infty, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, minus_infty, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),

    TEST_ff_f (nexttoward, qnan_value, 1.1L, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.1L, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

#ifdef TEST_FLOAT
    TEST_ff_f (nexttoward, 1.0, 1.1L, 0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, LDBL_MAX, 0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0x1.0000000000001p0, 0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0.9L, 0x0.ffffffp0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, -LDBL_MAX, 0x0.ffffffp0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0x0.fffffffffffff8p0, 0x0.ffffffp0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -1.1L, -0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -LDBL_MAX, -0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x1.0000000000001p0, -0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0.9L, -0x0.ffffffp0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, LDBL_MAX, -0x0.ffffffp0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x0.fffffffffffff8p0, -0x0.ffffffp0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -0x1.3p-145, -0xap-148L, -0x1.4p-145, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (nexttoward, 1.0, 0x1.000000000000002p0L, 0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffp0L, 0x0.ffffffp0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x1.000000000000002p0L, -0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffp0L, -0x0.ffffffp0, NO_INEXACT_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (nexttoward, 1.0, 0x1.000000000000000000000000008p0L, 0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffcp0L, 0x0.ffffffp0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x1.000000000000000000000000008p0L, -0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffcp0L, -0x0.ffffffp0, NO_INEXACT_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (nexttoward, 1.0, 0x1.0000000000000000000000000001p0L, 0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.ffffffp0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x1.0000000000000000000000000001p0L, -0x1.000002p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.ffffffp0, NO_INEXACT_EXCEPTION),
# endif
#endif
#ifdef TEST_DOUBLE
    TEST_ff_f (nexttoward, 1.0, 1.1L, 0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, LDBL_MAX, 0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0x1.0000000000001p0, 0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0.9L, 0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, -LDBL_MAX, 0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0x0.fffffffffffff8p0, 0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -1.1L, -0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -LDBL_MAX, -0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x1.0000000000001p0, -0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0.9L, -0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, LDBL_MAX, -0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x0.fffffffffffff8p0, -0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x8.00346dc5d6388p-3L, -0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 0x1p-1074, 0x1p-1073L, 0x1p-1073, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
# if LDBL_MANT_DIG >= 64
    TEST_ff_f (nexttoward, 1.0, 0x1.000000000000002p0L, 0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffp0L, 0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x1.000000000000002p0L, -0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffp0L, -0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 106
    TEST_ff_f (nexttoward, 1.0, 0x1.000000000000000000000000008p0L, 0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffcp0L, 0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x1.000000000000000000000000008p0L, -0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffcp0L, -0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
# endif
# if LDBL_MANT_DIG >= 113
    TEST_ff_f (nexttoward, 1.0, 0x1.0000000000000000000000000001p0L, 0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, 1.0, 0x0.ffffffffffffffffffffffffffff8p0L, 0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x1.0000000000000000000000000001p0L, -0x1.0000000000001p0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (nexttoward, -1.0, -0x0.ffffffffffffffffffffffffffff8p0L, -0x0.fffffffffffff8p0, NO_INEXACT_EXCEPTION),
# endif
#endif
  };

static void
nexttoward_test (void)
{
  START (nexttoward);
  RUN_TEST_LOOP_ff_f (nexttoward, nexttoward_test_data, );
  END;
}


static const struct test_ff_f_data pow_test_data[] =
  {
    TEST_ff_f (pow, qnan_value, 0, 1),
    TEST_ff_f (pow, qnan_value, minus_zero, 1),

    TEST_ff_f (pow, 1.1L, plus_infty, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, plus_infty, plus_infty, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, -1.1L, plus_infty, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, plus_infty, plus_infty, NO_TEST_INLINE),

    TEST_ff_f (pow, 0.9L, plus_infty, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, 1e-7L, plus_infty, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, -0.9L, plus_infty, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, -1e-7L, plus_infty, 0, NO_TEST_INLINE),

    TEST_ff_f (pow, 1.1L, minus_infty, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, plus_infty, minus_infty, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, -1.1L, minus_infty, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, minus_infty, 0, NO_TEST_INLINE),

    TEST_ff_f (pow, 0.9L, minus_infty, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, 1e-7L, minus_infty, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, -0.9L, minus_infty, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, -1e-7L, minus_infty, plus_infty, NO_TEST_INLINE),

    TEST_ff_f (pow, plus_infty, 1e-7L, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, plus_infty, 1, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, plus_infty, 1e7L, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, plus_infty, min_subnorm_value, plus_infty, NO_TEST_INLINE),

    TEST_ff_f (pow, plus_infty, -1e-7L, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, plus_infty, -1, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, plus_infty, -1e7L, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, plus_infty, -min_subnorm_value, 0, NO_TEST_INLINE),

    TEST_ff_f (pow, minus_infty, 1, minus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, 11, minus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, 1001, minus_infty, NO_TEST_INLINE),

    TEST_ff_f (pow, minus_infty, 2, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, 12, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, 1002, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, 0.1L, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, 1.1L, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, 11.1L, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, 1001.1L, plus_infty, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, min_subnorm_value, plus_infty, NO_TEST_INLINE),

    TEST_ff_f (pow, minus_infty, -1, minus_zero, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, -11, minus_zero, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, -1001, minus_zero, NO_TEST_INLINE),

    TEST_ff_f (pow, minus_infty, -2, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, -12, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, -1002, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, -0.1L, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, -1.1L, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, -11.1L, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, -1001.1L, 0, NO_TEST_INLINE),
    TEST_ff_f (pow, minus_infty, -min_subnorm_value, 0, NO_TEST_INLINE),

    TEST_ff_f (pow, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, 0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, 1, qnan_value, 1, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, -1, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, qnan_value, 1, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, qnan_value, -1, qnan_value, NO_INEXACT_EXCEPTION),

    /* pow (x, qNaN) == qNaN.  */
    TEST_ff_f (pow, 3.0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, minus_zero, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, plus_infty, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, -3.0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, minus_infty, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    TEST_ff_f (pow, qnan_value, 3.0, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, qnan_value, -3.0, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, qnan_value, plus_infty, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, qnan_value, minus_infty, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, qnan_value, 2.5, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, qnan_value, -2.5, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, qnan_value, min_subnorm_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (pow, qnan_value, -min_subnorm_value, qnan_value, NO_INEXACT_EXCEPTION),

    TEST_ff_f (pow, 1, plus_infty, 1),
    TEST_ff_f (pow, -1, plus_infty, 1),
    TEST_ff_f (pow, 1, minus_infty, 1),
    TEST_ff_f (pow, -1, minus_infty, 1),

    /* pow (x, +-0) == 1.  */
    TEST_ff_f (pow, plus_infty, 0, 1),
    TEST_ff_f (pow, plus_infty, minus_zero, 1),
    TEST_ff_f (pow, minus_infty, 0, 1),
    TEST_ff_f (pow, minus_infty, minus_zero, 1),

    TEST_ff_f (pow, -0.1L, 1.1L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (pow, -0.1L, -1.1L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (pow, -10.1L, 1.1L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (pow, -10.1L, -1.1L, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (pow, -1.01L, min_subnorm_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (pow, -1.01L, -min_subnorm_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (pow, -1.0L, min_subnorm_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (pow, -1.0L, -min_subnorm_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),

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

    TEST_ff_f (pow, 0, plus_infty, 0),
    TEST_ff_f (pow, minus_zero, plus_infty, 0),
    TEST_ff_f (pow, 0, minus_infty, plus_infty, DIVIDE_BY_ZERO_EXCEPTION_OK),
    TEST_ff_f (pow, minus_zero, minus_infty, plus_infty, DIVIDE_BY_ZERO_EXCEPTION_OK),

    /* pow (x, +inf) == +inf for |x| > 1.  */
    TEST_ff_f (pow, 1.5, plus_infty, plus_infty, NO_TEST_INLINE),

    /* pow (x, +inf) == +0 for |x| < 1.  */
    TEST_ff_f (pow, 0.5, plus_infty, 0.0, NO_TEST_INLINE),

    /* pow (x, -inf) == +0 for |x| > 1.  */
    TEST_ff_f (pow, 1.5, minus_infty, 0.0, NO_TEST_INLINE),

    /* pow (x, -inf) == +inf for |x| < 1.  */
    TEST_ff_f (pow, 0.5, minus_infty, plus_infty, NO_TEST_INLINE),

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

    TEST_ff_f (pow, -max_value, 0.5, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (pow, -max_value, 1.5, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (pow, -max_value, 1000.5, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),

    TEST_ff_f (pow, -min_value, 0.5, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (pow, -min_value, 1.5, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (pow, -min_value, 1000.5, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    AUTO_TESTS_ff_f (pow, tonearest),
  };

static void
pow_test (void)
{

  START (pow);
  RUN_TEST_LOOP_ff_f (pow, pow_test_data, );
  END;
}


static const struct test_ff_f_data pow_tonearest_test_data[] =
  {
    AUTO_TESTS_ff_f (pow, tonearest),
  };

static void
pow_test_tonearest (void)
{
  START (pow_tonearest);
  RUN_TEST_LOOP_ff_f (pow, pow_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_ff_f_data pow_towardzero_test_data[] =
  {
    TEST_ff_f (pow, 1.0625L, 1.125L, 1.070582293028761362162622578677070098674L),
    TEST_ff_f (pow, 1.5L, 1.03125L, 1.519127098714743184071644334163037684948L),
  };

static void
pow_test_towardzero (void)
{
  START (pow_towardzero);
  RUN_TEST_LOOP_ff_f (pow, pow_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_ff_f_data pow_downward_test_data[] =
  {
    TEST_ff_f (pow, 1.0625L, 1.125L, 1.070582293028761362162622578677070098674L),
    TEST_ff_f (pow, 1.5L, 1.03125L, 1.519127098714743184071644334163037684948L),
  };

static void
pow_test_downward (void)
{
  START (pow_downward);
  RUN_TEST_LOOP_ff_f (pow, pow_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_ff_f_data pow_upward_test_data[] =
  {
    TEST_ff_f (pow, 1.0625L, 1.125L, 1.070582293028761362162622578677070098674L),
    TEST_ff_f (pow, 1.5L, 1.03125L, 1.519127098714743184071644334163037684948L),
  };

static void
pow_test_upward (void)
{
  START (pow_upward);
  RUN_TEST_LOOP_ff_f (pow, pow_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_ff_f_data remainder_test_data[] =
  {
    TEST_ff_f (remainder, 1, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, 1, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 2, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 2, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, qnan_value, 0, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 7.0, plus_infty, 7.0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 7.0, minus_infty, 7.0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 1.625, 1.0, -0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, -1.625, 1.0, 0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 1.625, -1.0, -0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, -1.625, -1.0, 0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 5.0, 2.0, 1.0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 3.0, 2.0, -1.0, NO_INEXACT_EXCEPTION),
#if defined TEST_LDOUBLE && LDBL_MANT_DIG >= 56
    TEST_ff_f (remainder, -0x1.80000000000002p1L, 2.0, 0x1.fffffffffffff8p-1L, NO_INEXACT_EXCEPTION),
#endif
  };

static void
remainder_test (void)
{
  START (remainder);
  RUN_TEST_LOOP_ff_f (remainder, remainder_test_data, );
  END;
}

static void
drem_test (void)
{
  START (drem);
  /* drem uses the same test data as remainder.  */
  RUN_TEST_LOOP_ff_f (drem, remainder_test_data, );
  END;
}


static const struct test_ff_f_data remainder_tonearest_test_data[] =
  {
    TEST_ff_f (remainder, 1, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, 1, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 2, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 2, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, qnan_value, 0, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 7.0, plus_infty, 7.0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 7.0, minus_infty, 7.0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 1.625, 1.0, -0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, -1.625, 1.0, 0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 1.625, -1.0, -0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, -1.625, -1.0, 0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 5.0, 2.0, 1.0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 3.0, 2.0, -1.0, NO_INEXACT_EXCEPTION),
  };

static void
remainder_test_tonearest (void)
{
  START (remainder_tonearest);
  RUN_TEST_LOOP_ff_f (remainder, remainder_tonearest_test_data, FE_TONEAREST);
  END;
}

static void
drem_test_tonearest (void)
{
  START (drem_tonearest);
  /* drem uses the same test data as remainder.  */
  RUN_TEST_LOOP_ff_f (drem, remainder_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_ff_f_data remainder_towardzero_test_data[] =
  {
    TEST_ff_f (remainder, 1, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, 1, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 2, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 2, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, qnan_value, 0, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 7.0, plus_infty, 7.0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 7.0, minus_infty, 7.0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 1.625, 1.0, -0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, -1.625, 1.0, 0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 1.625, -1.0, -0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, -1.625, -1.0, 0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 5.0, 2.0, 1.0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 3.0, 2.0, -1.0, NO_INEXACT_EXCEPTION),
  };

static void
remainder_test_towardzero (void)
{
  START (remainder_towardzero);
  RUN_TEST_LOOP_ff_f (remainder, remainder_towardzero_test_data, FE_TOWARDZERO);
  END;
}

static void
drem_test_towardzero (void)
{
  START (drem_towardzero);
  /* drem uses the same test data as remainder.  */
  RUN_TEST_LOOP_ff_f (drem, remainder_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_ff_f_data remainder_downward_test_data[] =
  {
    TEST_ff_f (remainder, 1, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, 1, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 2, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 2, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, qnan_value, 0, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 7.0, plus_infty, 7.0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 7.0, minus_infty, 7.0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 1.625, 1.0, -0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, -1.625, 1.0, 0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 1.625, -1.0, -0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, -1.625, -1.0, 0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 5.0, 2.0, 1.0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 3.0, 2.0, -1.0, NO_INEXACT_EXCEPTION),
  };

static void
remainder_test_downward (void)
{
  START (remainder_downward);
  RUN_TEST_LOOP_ff_f (remainder, remainder_downward_test_data, FE_DOWNWARD);
  END;
}

static void
drem_test_downward (void)
{
  START (drem_downward);
  /* drem uses the same test data as remainder.  */
  RUN_TEST_LOOP_ff_f (drem, remainder_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_ff_f_data remainder_upward_test_data[] =
  {
    TEST_ff_f (remainder, 1, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, 1, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, plus_infty, 2, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, minus_zero, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 0, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, minus_infty, 2, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_ff_f (remainder, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, qnan_value, 0, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 7.0, plus_infty, 7.0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_ff_f (remainder, 7.0, minus_infty, 7.0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    TEST_ff_f (remainder, 1.625, 1.0, -0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, -1.625, 1.0, 0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 1.625, -1.0, -0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, -1.625, -1.0, 0.375, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 5.0, 2.0, 1.0, NO_INEXACT_EXCEPTION),
    TEST_ff_f (remainder, 3.0, 2.0, -1.0, NO_INEXACT_EXCEPTION),
  };

static void
remainder_test_upward (void)
{
  START (remainder_upward);
  RUN_TEST_LOOP_ff_f (remainder, remainder_upward_test_data, FE_UPWARD);
  END;
}

static void
drem_test_upward (void)
{
  START (drem_upward);
  /* drem uses the same test data as remainder.  */
  RUN_TEST_LOOP_ff_f (drem, remainder_upward_test_data, FE_UPWARD);
  END;
}

static const struct test_ffI_f1_data remquo_test_data[] =
  {
    TEST_ffI_f1 (remquo, 1, 0, qnan_value, IGNORE, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION),
    TEST_ffI_f1 (remquo, 1, minus_zero, qnan_value, IGNORE, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION),
    TEST_ffI_f1 (remquo, plus_infty, 1, qnan_value, IGNORE, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION),
    TEST_ffI_f1 (remquo, minus_infty, 1, qnan_value, IGNORE, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION),
    TEST_ffI_f1 (remquo, qnan_value, qnan_value, qnan_value, IGNORE, NO_INEXACT_EXCEPTION),

    TEST_ffI_f1 (remquo, 1.625, 1.0, -0.375, 2, NO_INEXACT_EXCEPTION),
    TEST_ffI_f1 (remquo, -1.625, 1.0, 0.375, -2, NO_INEXACT_EXCEPTION),
    TEST_ffI_f1 (remquo, 1.625, -1.0, -0.375, -2, NO_INEXACT_EXCEPTION),
    TEST_ffI_f1 (remquo, -1.625, -1.0, 0.375, 2, NO_INEXACT_EXCEPTION),

    TEST_ffI_f1 (remquo, 5, 2, 1, 2, NO_INEXACT_EXCEPTION),
    TEST_ffI_f1 (remquo, 3, 2, -1, 2, NO_INEXACT_EXCEPTION),
  };

static void
remquo_test (void)
{
  /* x is needed.  */
  int x;

  START (remquo);
  RUN_TEST_LOOP_ffI_f1 (remquo, remquo_test_data, , x);
  END;
}

static const struct test_f_f_data rint_test_data[] =
  {
    /* TODO: missing qNaN tests.  */

    TEST_f_f (rint, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),

    /* Default rounding mode is round to even.  */
    TEST_f_f (rint, 0.5, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1.5, 2.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 2.5, 2.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 3.5, 4.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4.5, 4.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.5, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1.5, -2.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -2.5, -2.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -3.5, -4.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4.5, -4.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.1, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.25, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.625, 1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.1, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.25, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.625, -1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 262144.75, 262145.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 262142.75, 262143.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 524286.75, 524287.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 524288.75, 524289.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1048576.75, 1048577.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 2097152.75, 2097153.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1048576.75, -1048577.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -2097152.75, -2097153.0, INEXACT_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_f_f (rint, 70368744177664.75, 70368744177665.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 140737488355328.75, 140737488355329.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 281474976710656.75, 281474976710657.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 562949953421312.75, 562949953421313.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1125899906842624.75, 1125899906842625.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -70368744177664.75, -70368744177665.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -140737488355328.75, -140737488355329.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -281474976710656.75, -281474976710657.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -562949953421312.75, -562949953421313.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1125899906842624.75, -1125899906842625.0, INEXACT_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (rint, 4503599627370495.5L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.75L, 4503599627370497.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370497.5L, 4503599627370498.0L, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370497.0L, INEXACT_EXCEPTION),
# endif

    TEST_f_f (rint, -4503599627370495.5L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.75L, -4503599627370497.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370497.5L, -4503599627370498.0L, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370497.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740994.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740994.0L, INEXACT_EXCEPTION),
# endif

    TEST_f_f (rint, 9007199254740991.5L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.25L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.5L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.75L, 9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740993.5L, 9007199254740994.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, -9007199254740991.5L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.25L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.5L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.75L, -9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740993.5L, -9007199254740994.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, 72057594037927935.5L, 72057594037927936.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 72057594037927936.25L, 72057594037927936.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 72057594037927936.5L, 72057594037927936.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 72057594037927936.75L, 72057594037927937.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 72057594037927937.5L, 72057594037927938.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, -72057594037927935.5L, -72057594037927936.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -72057594037927936.25L, -72057594037927936.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -72057594037927936.5L, -72057594037927936.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -72057594037927936.75L, -72057594037927937.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -72057594037927937.5L, -72057594037927938.0L, INEXACT_EXCEPTION),

# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, 10141204801825835211973625643007.5L, 10141204801825835211973625643008.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 10141204801825835211973625643008.25L, 10141204801825835211973625643008.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 10141204801825835211973625643008.5L, 10141204801825835211973625643008.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 10141204801825835211973625643008.75L, 10141204801825835211973625643009.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 10141204801825835211973625643009.5L, 10141204801825835211973625643010.0L, INEXACT_EXCEPTION),
# endif
#endif
  };

static void
rint_test (void)
{
  START (rint);
  RUN_TEST_LOOP_f_f (rint, rint_test_data, );
  END;
}

static const struct test_f_f_data rint_tonearest_test_data[] =
  {
    TEST_f_f (rint, 2.0, 2.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 1.5, 2.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1.0, 1.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.5, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.5, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1.0, -1.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, -1.5, -2.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -2.0, -2.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.1, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.25, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.625, 1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.1, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.25, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.625, -1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1048576.75, 1048577.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 2097152.75, 2097153.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1048576.75, -1048577.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -2097152.75, -2097153.0, INEXACT_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_f_f (rint, 70368744177664.75, 70368744177665.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 140737488355328.75, 140737488355329.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 281474976710656.75, 281474976710657.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 562949953421312.75, 562949953421313.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1125899906842624.75, 1125899906842625.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -70368744177664.75, -70368744177665.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -140737488355328.75, -140737488355329.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -281474976710656.75, -281474976710657.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -562949953421312.75, -562949953421313.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1125899906842624.75, -1125899906842625.0, INEXACT_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (rint, 4503599627370495.5L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.75L, 4503599627370497.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370497.5L, 4503599627370498.0L, INEXACT_EXCEPTION),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370497.0L, INEXACT_EXCEPTION),
# endif
    TEST_f_f (rint, -4503599627370495.5L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.75L, -4503599627370497.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370497.5L, -4503599627370498.0L, INEXACT_EXCEPTION),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370497.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740994.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740994.0L, INEXACT_EXCEPTION),
# endif
#endif
  };

static void
rint_test_tonearest (void)
{
  START (rint_tonearest);
  RUN_TEST_LOOP_f_f (rint, rint_tonearest_test_data, FE_TONEAREST);
  END;
}

static const struct test_f_f_data rint_towardzero_test_data[] =
  {
    TEST_f_f (rint, 2.0, 2.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 1.5, 1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1.0, 1.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.5, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.5, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1.0, -1.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, -1.5, -1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -2.0, -2.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.1, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.25, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.625, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.1, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.25, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.625, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1048576.75, 1048576.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 2097152.75, 2097152.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1048576.75, -1048576.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -2097152.75, -2097152.0, INEXACT_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_f_f (rint, 70368744177664.75, 70368744177664.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 140737488355328.75, 140737488355328.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 281474976710656.75, 281474976710656.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 562949953421312.75, 562949953421312.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1125899906842624.75, 1125899906842624.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -70368744177664.75, -70368744177664.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -140737488355328.75, -140737488355328.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -281474976710656.75, -281474976710656.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -562949953421312.75, -562949953421312.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1125899906842624.75, -1125899906842624.0, INEXACT_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (rint, 4503599627370495.5L, 4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.75L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370497.5L, 4503599627370497.0L, INEXACT_EXCEPTION),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370494.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370496.0L, INEXACT_EXCEPTION),
# endif
    TEST_f_f (rint, -4503599627370495.5L, -4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.75L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370497.5L, -4503599627370497.0L, INEXACT_EXCEPTION),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370494.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370496.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740993.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740993.0L, INEXACT_EXCEPTION),
# endif
#endif
  };

static void
rint_test_towardzero (void)
{
  START (rint_towardzero);
  RUN_TEST_LOOP_f_f (rint, rint_towardzero_test_data, FE_TOWARDZERO);
  END;
}

static const struct test_f_f_data rint_downward_test_data[] =
  {
    TEST_f_f (rint, 2.0, 2.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 1.5, 1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1.0, 1.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.5, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.5, -1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1.0, -1.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, -1.5, -2.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -2.0, -2.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.1, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.25, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.625, 0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.1, -1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.25, -1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.625, -1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1048576.75, 1048576.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 2097152.75, 2097152.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1048576.75, -1048577.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -2097152.75, -2097153.0, INEXACT_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_f_f (rint, 70368744177664.75, 70368744177664.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 140737488355328.75, 140737488355328.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 281474976710656.75, 281474976710656.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 562949953421312.75, 562949953421312.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1125899906842624.75, 1125899906842624.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -70368744177664.75, -70368744177665.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -140737488355328.75, -140737488355329.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -281474976710656.75, -281474976710657.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -562949953421312.75, -562949953421313.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1125899906842624.75, -1125899906842625.0, INEXACT_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (rint, 4503599627370495.5L, 4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.25L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.5L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.75L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370497.5L, 4503599627370497.0L, INEXACT_EXCEPTION),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370494.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370496.0L, INEXACT_EXCEPTION),
# endif
    TEST_f_f (rint, -4503599627370495.5L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.25L, -4503599627370497.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.5L, -4503599627370497.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.75L, -4503599627370497.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370497.5L, -4503599627370498.0L, INEXACT_EXCEPTION),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370497.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740993.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740994.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740994.0L, INEXACT_EXCEPTION),
# endif
#endif
  };

static void
rint_test_downward (void)
{
  START (rint_downward);
  RUN_TEST_LOOP_f_f (rint, rint_downward_test_data, FE_DOWNWARD);
  END;
}

static const struct test_f_f_data rint_upward_test_data[] =
  {
    TEST_f_f (rint, 2.0, 2.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 1.5, 2.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1.0, 1.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.5, 1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.0, 0.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.5, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1.0, -1.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, -1.5, -1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -2.0, -2.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.1, 1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.25, 1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 0.625, 1.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.1, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.25, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -0.625, -0.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1048576.75, 1048577.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 2097152.75, 2097153.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1048576.75, -1048576.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -2097152.75, -2097152.0, INEXACT_EXCEPTION),
#ifndef TEST_FLOAT
    TEST_f_f (rint, 70368744177664.75, 70368744177665.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 140737488355328.75, 140737488355329.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 281474976710656.75, 281474976710657.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 562949953421312.75, 562949953421313.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, 1125899906842624.75, 1125899906842625.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -70368744177664.75, -70368744177664.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -140737488355328.75, -140737488355328.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -281474976710656.75, -281474976710656.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -562949953421312.75, -562949953421312.0, INEXACT_EXCEPTION),
    TEST_f_f (rint, -1125899906842624.75, -1125899906842624.0, INEXACT_EXCEPTION),
#endif
#ifdef TEST_LDOUBLE
    /* The result can only be represented in long double.  */
    TEST_f_f (rint, 4503599627370495.5L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.25L, 4503599627370497.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.5L, 4503599627370497.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.75L, 4503599627370497.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370497.5L, 4503599627370498.0L, INEXACT_EXCEPTION),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, 4503599627370494.5000000000001L, 4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370495.5000000000001L, 4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 4503599627370496.5000000000001L, 4503599627370497.0L, INEXACT_EXCEPTION),
# endif
    TEST_f_f (rint, -4503599627370495.5L, -4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.25L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.5L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.75L, -4503599627370496.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370497.5L, -4503599627370497.0L, INEXACT_EXCEPTION),
# if LDBL_MANT_DIG > 100
    TEST_f_f (rint, -4503599627370494.5000000000001L, -4503599627370494.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370495.5000000000001L, -4503599627370495.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -4503599627370496.5000000000001L, -4503599627370496.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, 9007199254740991.0000000000001L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.0000000000001L, 9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740993.0000000000001L, 9007199254740994.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740991.5000000000001L, 9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740992.5000000000001L, 9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, 9007199254740993.5000000000001L, 9007199254740994.0L, INEXACT_EXCEPTION),

    TEST_f_f (rint, -9007199254740991.0000000000001L, -9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.0000000000001L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740993.0000000000001L, -9007199254740993.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740991.5000000000001L, -9007199254740991.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740992.5000000000001L, -9007199254740992.0L, INEXACT_EXCEPTION),
    TEST_f_f (rint, -9007199254740993.5000000000001L, -9007199254740993.0L, INEXACT_EXCEPTION),
# endif
#endif
  };

static void
rint_test_upward (void)
{
  START (rint_upward);
  RUN_TEST_LOOP_f_f (rint, rint_upward_test_data, FE_UPWARD);
  END;
}

static const struct test_f_f_data round_test_data[] =
  {
    /* TODO: missing +/-Inf as well as qNaN tests.  */

    TEST_f_f (round, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_f (round, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    /* Bug 15479: spurious "inexact" exception may occur.  */
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
  };

static void
round_test (void)
{
  START (round);
  RUN_TEST_LOOP_f_f (round, round_test_data, );
  END;
}


static const struct test_ff_f_data scalb_test_data[] =
  {
    TEST_ff_f (scalb, 2.0, 0.5, qnan_value, INVALID_EXCEPTION),
    TEST_ff_f (scalb, 3.0, -2.5, qnan_value, INVALID_EXCEPTION),

    TEST_ff_f (scalb, 0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (scalb, 1, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

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

    TEST_ff_f (scalb, qnan_value, 1, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (scalb, 1, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (scalb, qnan_value, 0, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (scalb, 0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (scalb, qnan_value, plus_infty, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (scalb, plus_infty, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_ff_f (scalb, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    TEST_ff_f (scalb, 0.8L, 4, 12.8L),
    TEST_ff_f (scalb, -0.854375L, 5, -27.34L),
  };

static void
scalb_test (void)
{

  START (scalb);
  RUN_TEST_LOOP_ff_f (scalb, scalb_test_data, );
  END;
}


static const struct test_fi_f_data scalbn_test_data[] =
  {
    TEST_fi_f (scalbn, 0, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_fi_f (scalbn, minus_zero, 0, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_fi_f (scalbn, plus_infty, 1, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_fi_f (scalbn, minus_infty, 1, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_fi_f (scalbn, qnan_value, 1, qnan_value, NO_INEXACT_EXCEPTION),

    TEST_fi_f (scalbn, 0.8L, 4, 12.8L, NO_INEXACT_EXCEPTION),
    TEST_fi_f (scalbn, -0.854375L, 5, -27.34L, NO_INEXACT_EXCEPTION),

    TEST_fi_f (scalbn, 1, 0L, 1, NO_INEXACT_EXCEPTION),

    TEST_fi_f (scalbn, 1, INT_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, 1, INT_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, max_value, INT_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, max_value, INT_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, min_value, INT_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, min_value, INT_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, min_value / 4, INT_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fi_f (scalbn, min_value / 4, INT_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
  };

static void
scalbn_test (void)
{

  START (scalbn);
  RUN_TEST_LOOP_fi_f (scalbn, scalbn_test_data, );
  END;
}


static const struct test_fl_f_data scalbln_test_data[] =
  {
    TEST_fl_f (scalbln, 0, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_fl_f (scalbln, minus_zero, 0, minus_zero, NO_INEXACT_EXCEPTION),

    TEST_fl_f (scalbln, plus_infty, 1, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_fl_f (scalbln, minus_infty, 1, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_fl_f (scalbln, qnan_value, 1, qnan_value, NO_INEXACT_EXCEPTION),

    TEST_fl_f (scalbln, 0.8L, 4, 12.8L, NO_INEXACT_EXCEPTION),
    TEST_fl_f (scalbln, -0.854375L, 5, -27.34L, NO_INEXACT_EXCEPTION),

    TEST_fl_f (scalbln, 1, 0L, 1, NO_INEXACT_EXCEPTION),

    TEST_fl_f (scalbln, 1, INT_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, 1, INT_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, INT_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, INT_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, INT_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, INT_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, INT_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, INT_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),

    TEST_fl_f (scalbln, 1, LONG_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, 1, LONG_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, LONG_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, LONG_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, LONG_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, LONG_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, LONG_MAX, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, LONG_MIN, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),

#if LONG_MAX >= 0x100000000
    TEST_fl_f (scalbln, 1, 0x88000000L, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, 1, -0x88000000L, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, 0x88000000L, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, max_value, -0x88000000L, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, 0x88000000L, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value, -0x88000000L, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, 0x88000000L, plus_infty, INEXACT_EXCEPTION|OVERFLOW_EXCEPTION),
    TEST_fl_f (scalbln, min_value / 4, -0x88000000L, plus_zero, INEXACT_EXCEPTION|UNDERFLOW_EXCEPTION),
#endif
  };

static void
scalbln_test (void)
{

  START (scalbln);
  RUN_TEST_LOOP_fl_f (scalbln, scalbln_test_data, );
  END;
}


static const struct test_f_i_data signbit_test_data[] =
  {
    TEST_f_b (signbit, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (signbit, minus_zero, 1, NO_INEXACT_EXCEPTION),
    TEST_f_b (signbit, plus_infty, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (signbit, minus_infty, 1, NO_INEXACT_EXCEPTION),
    TEST_f_b (signbit, qnan_value, 0, NO_INEXACT_EXCEPTION),
    TEST_f_b (signbit, -qnan_value, 1, NO_INEXACT_EXCEPTION),

    /* signbit (x) != 0 for x < 0.  */
    TEST_f_b (signbit, -1, 1, NO_INEXACT_EXCEPTION),
    /* signbit (x) == 0 for x >= 0.  */
    TEST_f_b (signbit, 1, 0, NO_INEXACT_EXCEPTION),
  };

static void
signbit_test (void)
{
  START (signbit);
  RUN_TEST_LOOP_f_b_tg (signbit, signbit_test_data, );
  END;
}


static const struct test_f_f_data sin_test_data[] =
  {
    TEST_f_f (sin, plus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sin, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sin, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    AUTO_TESTS_f_f (sin, tonearest),
  };

static void
sin_test (void)
{
  START (sin);
  RUN_TEST_LOOP_f_f (sin, sin_test_data, );
  END;
}


static const struct test_f_f_data sin_tonearest_test_data[] =
  {
    AUTO_TESTS_f_f (sin, tonearest),
  };

static void
sin_test_tonearest (void)
{
  START (sin_tonearest);
  RUN_TEST_LOOP_f_f (sin, sin_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_f_f_data sin_towardzero_test_data[] =
  {
    AUTO_TESTS_f_f (sin, towardzero),
  };

static void
sin_test_towardzero (void)
{
  START (sin_towardzero);
  RUN_TEST_LOOP_f_f (sin, sin_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_f_f_data sin_downward_test_data[] =
  {
    AUTO_TESTS_f_f (sin, downward),
  };

static void
sin_test_downward (void)
{
  START (sin_downward);
  RUN_TEST_LOOP_f_f (sin, sin_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_f_f_data sin_upward_test_data[] =
  {
    AUTO_TESTS_f_f (sin, upward),
  };

static void
sin_test_upward (void)
{
  START (sin_upward);
  RUN_TEST_LOOP_f_f (sin, sin_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_fFF_11_data sincos_test_data[] =
  {
    TEST_fFF_11 (sincos, plus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_fFF_11 (sincos, minus_infty, qnan_value, qnan_value, INVALID_EXCEPTION),
    TEST_fFF_11 (sincos, qnan_value, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_fFF_11 (sincos, tonearest),
  };

static void
sincos_test (void)
{
  FLOAT sin_res, cos_res;

  START (sincos);
  RUN_TEST_LOOP_fFF_11 (sincos, sincos_test_data, , sin_res, cos_res);
  END;
}

static const struct test_f_f_data sinh_test_data[] =
  {
    TEST_f_f (sinh, plus_infty, plus_infty, NO_TEST_INLINE),
    TEST_f_f (sinh, minus_infty, minus_infty, NO_TEST_INLINE),
    TEST_f_f (sinh, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (sinh, tonearest),
  };

static void
sinh_test (void)
{
  START (sinh);
  RUN_TEST_LOOP_f_f (sinh, sinh_test_data, );
  END;
}


static const struct test_f_f_data sinh_tonearest_test_data[] =
  {
    AUTO_TESTS_f_f (sinh, tonearest),
  };

static void
sinh_test_tonearest (void)
{
  START (sinh_tonearest);
  RUN_TEST_LOOP_f_f (sinh, sinh_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_f_f_data sinh_towardzero_test_data[] =
  {
    AUTO_TESTS_f_f (sinh, towardzero),
  };

static void
sinh_test_towardzero (void)
{
  START (sinh_towardzero);
  RUN_TEST_LOOP_f_f (sinh, sinh_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_f_f_data sinh_downward_test_data[] =
  {
    AUTO_TESTS_f_f (sinh, downward),
  };

static void
sinh_test_downward (void)
{
  START (sinh_downward);
  RUN_TEST_LOOP_f_f (sinh, sinh_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_f_f_data sinh_upward_test_data[] =
  {
    AUTO_TESTS_f_f (sinh, upward),
  };

static void
sinh_test_upward (void)
{
  START (sinh_upward);
  RUN_TEST_LOOP_f_f (sinh, sinh_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_f_f_data sqrt_test_data[] =
  {
    TEST_f_f (sqrt, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_f_f (sqrt, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),

    /* sqrt (x) == qNaN plus invalid exception for x < 0.  */
    TEST_f_f (sqrt, -1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sqrt, -max_value, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sqrt, minus_infty, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),

    AUTO_TESTS_f_f (sqrt, tonearest),
  };

static void
sqrt_test (void)
{
  START (sqrt);
  RUN_TEST_LOOP_f_f (sqrt, sqrt_test_data, );
  END;
}

static const struct test_f_f_data sqrt_tonearest_test_data[] =
  {
    TEST_f_f (sqrt, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_f_f (sqrt, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),

    /* sqrt (x) == qNaN plus invalid exception for x < 0.  */
    TEST_f_f (sqrt, -1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sqrt, -max_value, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sqrt, minus_infty, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),

    AUTO_TESTS_f_f (sqrt, tonearest),
  };

static void
sqrt_test_tonearest (void)
{
  START (sqrt_tonearest);
  RUN_TEST_LOOP_f_f (sqrt, sqrt_tonearest_test_data, FE_TONEAREST);
  END;
}

static const struct test_f_f_data sqrt_towardzero_test_data[] =
  {
    TEST_f_f (sqrt, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_f_f (sqrt, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),

    /* sqrt (x) == qNaN plus invalid exception for x < 0.  */
    TEST_f_f (sqrt, -1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sqrt, -max_value, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sqrt, minus_infty, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),

    AUTO_TESTS_f_f (sqrt, towardzero),
  };

static void
sqrt_test_towardzero (void)
{
  START (sqrt_towardzero);
  RUN_TEST_LOOP_f_f (sqrt, sqrt_towardzero_test_data, FE_TOWARDZERO);
  END;
}

static const struct test_f_f_data sqrt_downward_test_data[] =
  {
    TEST_f_f (sqrt, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_f_f (sqrt, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),

    /* sqrt (x) == qNaN plus invalid exception for x < 0.  */
    TEST_f_f (sqrt, -1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sqrt, -max_value, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sqrt, minus_infty, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),

    AUTO_TESTS_f_f (sqrt, downward),
  };

static void
sqrt_test_downward (void)
{
  START (sqrt_downward);
  RUN_TEST_LOOP_f_f (sqrt, sqrt_downward_test_data, FE_DOWNWARD);
  END;
}

static const struct test_f_f_data sqrt_upward_test_data[] =
  {
    TEST_f_f (sqrt, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_f_f (sqrt, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),

    /* sqrt (x) == qNaN plus invalid exception for x < 0.  */
    TEST_f_f (sqrt, -1, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sqrt, -max_value, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (sqrt, minus_infty, qnan_value, NO_INEXACT_EXCEPTION|INVALID_EXCEPTION|ERRNO_EDOM),

    AUTO_TESTS_f_f (sqrt, upward),
  };

static void
sqrt_test_upward (void)
{
  START (sqrt_upward);
  RUN_TEST_LOOP_f_f (sqrt, sqrt_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_f_f_data tan_test_data[] =
  {
    TEST_f_f (tan, plus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (tan, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (tan, qnan_value, qnan_value, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),

    AUTO_TESTS_f_f (tan, tonearest),
  };

static void
tan_test (void)
{
  START (tan);
  RUN_TEST_LOOP_f_f (tan, tan_test_data, );
  END;
}


static const struct test_f_f_data tan_tonearest_test_data[] =
  {
    AUTO_TESTS_f_f (tan, tonearest),
  };

static void
tan_test_tonearest (void)
{
  START (tan_tonearest);
  RUN_TEST_LOOP_f_f (tan, tan_tonearest_test_data, FE_TONEAREST);
  END;
}


static const struct test_f_f_data tan_towardzero_test_data[] =
  {
    AUTO_TESTS_f_f (tan, towardzero),
  };

static void
tan_test_towardzero (void)
{
  START (tan_towardzero);
  RUN_TEST_LOOP_f_f (tan, tan_towardzero_test_data, FE_TOWARDZERO);
  END;
}


static const struct test_f_f_data tan_downward_test_data[] =
  {
    AUTO_TESTS_f_f (tan, downward),
  };

static void
tan_test_downward (void)
{
  START (tan_downward);
  RUN_TEST_LOOP_f_f (tan, tan_downward_test_data, FE_DOWNWARD);
  END;
}


static const struct test_f_f_data tan_upward_test_data[] =
  {
    AUTO_TESTS_f_f (tan, upward),
  };

static void
tan_test_upward (void)
{
  START (tan_upward);
  RUN_TEST_LOOP_f_f (tan, tan_upward_test_data, FE_UPWARD);
  END;
}


static const struct test_f_f_data tanh_test_data[] =
  {
    TEST_f_f (tanh, plus_infty, 1, NO_TEST_INLINE),
    TEST_f_f (tanh, minus_infty, -1, NO_TEST_INLINE),
    TEST_f_f (tanh, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (tanh, tonearest),
  };

static void
tanh_test (void)
{
  START (tanh);
  RUN_TEST_LOOP_f_f (tanh, tanh_test_data, );
  END;
}

static const struct test_f_f_data tgamma_test_data[] =
  {
    TEST_f_f (tgamma, plus_infty, plus_infty),
    TEST_f_f (tgamma, max_value, plus_infty, OVERFLOW_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (tgamma, 0, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (tgamma, minus_zero, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    /* tgamma (x) == qNaN plus invalid exception for integer x <= 0.  */
    TEST_f_f (tgamma, -2, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (tgamma, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (tgamma, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (tgamma, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (tgamma, tonearest),
  };

static void
tgamma_test (void)
{
  START (tgamma);
  RUN_TEST_LOOP_f_f (tgamma, tgamma_test_data, );
  END;
}


static const struct test_f_f_data trunc_test_data[] =
  {
    TEST_f_f (trunc, plus_infty, plus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (trunc, minus_infty, minus_infty, NO_INEXACT_EXCEPTION),
    TEST_f_f (trunc, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    TEST_f_f (trunc, 0, 0, NO_INEXACT_EXCEPTION),
    TEST_f_f (trunc, minus_zero, minus_zero, NO_INEXACT_EXCEPTION),
    /* Bug 15479: spurious "inexact" exception may occur.  */
    TEST_f_f (trunc, 0.1, 0),
    TEST_f_f (trunc, 0.25, 0),
    TEST_f_f (trunc, 0.625, 0),
    TEST_f_f (trunc, -0.1, minus_zero),
    TEST_f_f (trunc, -0.25, minus_zero),
    TEST_f_f (trunc, -0.625, minus_zero),
    TEST_f_f (trunc, 1, 1, NO_INEXACT_EXCEPTION),
    TEST_f_f (trunc, -1, -1, NO_INEXACT_EXCEPTION),
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
  };

static void
trunc_test (void)
{
  START (trunc);
  RUN_TEST_LOOP_f_f (trunc, trunc_test_data, );
  END;
}

static const struct test_f_f_data y0_test_data[] =
  {
    /* y0 is the Bessel function of the second kind of order 0 */
    TEST_f_f (y0, -1.0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (y0, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (y0, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (y0, 0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (y0, -0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (y0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_f_f (y0, plus_infty, 0),

    AUTO_TESTS_f_f (y0, tonearest),
  };

static void
y0_test (void)
{
  START (y0);
  RUN_TEST_LOOP_f_f (y0, y0_test_data, );
  END;
}


static const struct test_f_f_data y1_test_data[] =
  {
    /* y1 is the Bessel function of the second kind of order 1 */
    TEST_f_f (y1, -1.0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (y1, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (y1, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_f_f (y1, 0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (y1, -0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_f_f (y1, plus_infty, 0),
    TEST_f_f (y1, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_f_f (y1, tonearest),
  };

static void
y1_test (void)
{
  START (y1);
  RUN_TEST_LOOP_f_f (y1, y1_test_data, );
  END;
}


static const struct test_if_f_data yn_test_data[] =
  {
    /* yn is the Bessel function of the second kind of order n */
    /* yn (0, x) == y0 (x)  */
    TEST_if_f (yn, 0, -1.0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 0, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 0, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 0, 0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, 0, -0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, 0, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),
    TEST_if_f (yn, 0, plus_infty, 0),

    /* yn (1, x) == y1 (x)  */
    TEST_if_f (yn, 1, -1.0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 1, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 1, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 1, 0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, 1, -0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, 1, plus_infty, 0),
    TEST_if_f (yn, 1, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* yn (-1, x) == -y1 (x).  */
    TEST_if_f (yn, -1, -1.0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, -1, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, -1, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, -1, 0.0, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, -1, -0.0, plus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, -1, plus_infty, minus_zero),
    TEST_if_f (yn, -1, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* yn (2, x).  */
    TEST_if_f (yn, 2, -1.0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 2, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 2, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 2, 0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, 2, -0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, 2, plus_infty, 0),
    TEST_if_f (yn, 2, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* yn (-2, x) == yn (2, x).  */
    TEST_if_f (yn, -2, -1.0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, -2, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, -2, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, -2, 0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, -2, -0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, -2, plus_infty, 0),
    TEST_if_f (yn, -2, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* yn (3, x)  */
    TEST_if_f (yn, 3, -1.0, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 3, -max_value, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 3, minus_infty, qnan_value, INVALID_EXCEPTION|ERRNO_EDOM),
    TEST_if_f (yn, 3, 0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, 3, -0.0, minus_infty, DIVIDE_BY_ZERO_EXCEPTION|ERRNO_ERANGE),
    TEST_if_f (yn, 3, plus_infty, 0),
    TEST_if_f (yn, 3, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    /* yn (10, x)  */
    TEST_if_f (yn, 10, plus_infty, 0),
    TEST_if_f (yn, 10, qnan_value, qnan_value, NO_INEXACT_EXCEPTION),

    AUTO_TESTS_if_f (yn, tonearest),
  };

static void
yn_test (void)
{
  START (yn);
  RUN_TEST_LOOP_if_f (yn, yn_test_data, );
  END;
}


static const struct test_f_f_data significand_test_data[] =
  {
    /* significand returns the mantissa of the exponential representation.  */
    /* TODO: missing +/-Inf as well as qNaN tests.  */
    TEST_f_f (significand, 4.0, 1.0, NO_INEXACT_EXCEPTION),
    TEST_f_f (significand, 6.0, 1.5, NO_INEXACT_EXCEPTION),
    TEST_f_f (significand, 8.0, 1.0, NO_INEXACT_EXCEPTION),
  };

static void
significand_test (void)
{
  START (significand);
  RUN_TEST_LOOP_f_f (significand, significand_test_data, );
  END;
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

/* Verify that our ulp () implementation is behaving as expected
   or abort.  */
void
check_ulp (void)
{
   FLOAT ulps, ulpx, value;
   int i;
   /* Check ulp of zero is a subnormal value...  */
   ulps = ulp (0x0.0p0);
   if (fpclassify (ulps) != FP_SUBNORMAL)
     {
       fprintf (stderr, "ulp (0x0.0p0) is not FP_SUBNORMAL!\n");
       exit (EXIT_FAILURE);
     }
   /* Check that the ulp of one is a normal value... */
   ulps = ulp (1.0L);
   if (fpclassify (ulps) != FP_NORMAL)
     {
       fprintf (stderr, "ulp (1.0L) is not FP_NORMAL\n");
       exit (EXIT_FAILURE);
     }

   /* Compute the next subnormal value using nextafter to validate ulp.
      We allow +/- 1 ulp around the represented value.  */
   value = FUNC(nextafter) (0, 1);
   ulps = ULPDIFF (value, 0);
   ulpx = ulp (1.0L);
   if (ulps < (1.0L - ulpx) || ulps > (1.0L + ulpx))
     {
       fprintf (stderr, "Value outside of 1 +/- 1ulp.\n");
       exit (EXIT_FAILURE);
     }
   /* Compute the nearest representable number from 10 towards 20.
      The result is 10 + 1ulp.  We use this to check the ulp function.
      We allow +/- 1 ulp around the represented value.  */
   value = FUNC(nextafter) (10, 20);
   ulps = ULPDIFF (value, 10);
   ulpx = ulp (1.0L);
   if (ulps < (1.0L - ulpx) || ulps > (1.0L + ulpx))
     {
       fprintf (stderr, "Value outside of 1 +/- 1ulp.\n");
       exit (EXIT_FAILURE);
     }
   /* This gives one more ulp.  */
   value = FUNC(nextafter) (value, 20);
   ulps = ULPDIFF (value, 10);
   ulpx = ulp (2.0L);
   if (ulps < (2.0L - ulpx) || ulps > (2.0L + ulpx))
     {
       fprintf (stderr, "Value outside of 2 +/- 1ulp.\n");
       exit (EXIT_FAILURE);
     }
   /* And now calculate 100 ulp.  */
   for (i = 2; i < 100; i++)
     value = FUNC(nextafter) (value, 20);
   ulps = ULPDIFF (value, 10);
   ulpx = ulp (100.0L);
   if (ulps < (100.0L - ulpx) || ulps > (100.0L + ulpx))
     {
       fprintf (stderr, "Value outside of 100 +/- 1ulp.\n");
       exit (EXIT_FAILURE);
     }
}

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

  check_ulp ();

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
  exp10_test_tonearest ();
  exp10_test_towardzero ();
  exp10_test_downward ();
  exp10_test_upward ();
  exp2_test ();
  expm1_test ();
  expm1_test_tonearest ();
  expm1_test_towardzero ();
  expm1_test_downward ();
  expm1_test_upward ();
  frexp_test ();
  ldexp_test ();
  log_test ();
  log10_test ();
  log1p_test ();
  log2_test ();
  logb_test ();
  logb_test_downward ();
  modf_test ();
  pow10_test ();
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
  sqrt_test_tonearest ();
  sqrt_test_towardzero ();
  sqrt_test_downward ();
  sqrt_test_upward ();

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
  drem_test ();
  drem_test_tonearest ();
  drem_test_towardzero ();
  drem_test_downward ();
  drem_test_upward ();
  fmod_test ();
  remainder_test ();
  remainder_test_tonearest ();
  remainder_test_towardzero ();
  remainder_test_downward ();
  remainder_test_upward ();
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
