/* Support code for testing libm functions.
   Copyright (C) 1997-2017 Free Software Foundation, Inc.
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

/* Part of testsuite for libm.

   libm-test.inc is processed by a perl script.  The resulting file has to
   be included by a master file that defines:

   Macros:
   FUNC(function): converts general function name (like cos) to
   name with correct suffix (e.g. cosl or cosf)
   FLOAT:	   floating point type to test
   - TEST_MSG:	   informal message to be displayed
   chooses one of the parameters as delta for testing
   equality
   PREFIX A macro which defines the prefix for common macros for the
   type (i.e LDBL, DBL, or FLT).
   LIT A function which appends the correct suffix to a literal.
   TYPE_STR A macro which defines a stringitized name of the type.
   FTOSTR This macro defines a function similar in type to strfromf
   which converts a FLOAT to a string.  */

/* Parameter handling is primitive in the moment:
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

   NaN values: The payload of NaNs is set in inputs for functions
   where it is significant, and is examined in the outputs of some
   functions.

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
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <argp.h>
#include <tininess.h>
#include <math-tests.h>
#include <math-tests-arch.h>
#include <nan-high-order-bit.h>

/* This header defines func_ulps, func_real_ulps and func_imag_ulps
   arrays.  */
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
#define TEST_NAN_PAYLOAD		0x1000
#define NO_TEST_INLINE			0x2000
#define XFAIL_TEST			0x4000
/* Indicate errno settings required or disallowed.  */
#define ERRNO_UNCHANGED			0x8000
#define ERRNO_EDOM			0x10000
#define ERRNO_ERANGE			0x20000
/* Flags generated by gen-libm-test.pl, not entered here manually.  */
#define IGNORE_RESULT			0x40000
#define NON_FINITE			0x80000
#define TEST_SNAN			0x100000
#define NO_TEST_MATHVEC			0x200000

#define TEST_NAN_PAYLOAD_CANONICALIZE	(SNAN_TESTS_PRESERVE_PAYLOAD	\
					 ? TEST_NAN_PAYLOAD		\
					 : 0)

#define __CONCATX(a,b) __CONCAT(a,b)

#define TYPE_MIN __CONCATX (PREFIX, _MIN)
#define TYPE_TRUE_MIN __CONCATX (PREFIX, _TRUE_MIN)
#define TYPE_MAX __CONCATX (PREFIX, _MAX)
#define MIN_EXP __CONCATX (PREFIX, _MIN_EXP)
#define MAX_EXP __CONCATX (PREFIX, _MAX_EXP)
#define MANT_DIG __CONCATX (PREFIX, _MANT_DIG)

/* Maximum character buffer to store a stringitized FLOAT value.  */
#define FSTR_MAX (128)

#if TEST_INLINE
# define ULP_IDX __CONCATX (ULP_I_, PREFIX)
# define QTYPE_STR "i" TYPE_STR
#else
# define ULP_IDX __CONCATX (ULP_, PREFIX)
# define QTYPE_STR TYPE_STR
#endif

/* Format specific test macros.  */
#define TEST_COND_binary32 (MANT_DIG == 24	\
			    && MIN_EXP == -125	\
			    && MAX_EXP == 128)

#define TEST_COND_binary64 (MANT_DIG == 53	\
			    && MIN_EXP == -1021	\
			    && MAX_EXP == 1024)

#define TEST_COND_binary128 (MANT_DIG == 113		\
			     && MIN_EXP == -16381	\
			     && MAX_EXP == 16384)

#define TEST_COND_ibm128 (MANT_DIG == 106)

#define TEST_COND_intel96 (MANT_DIG == 64	\
			   && MIN_EXP == -16381	\
			   && MAX_EXP == 16384)

#define TEST_COND_m68k96 (MANT_DIG == 64	\
			  && MIN_EXP == -16382	\
			  && MAX_EXP == 16384)

/* The condition ibm128-libgcc is used instead of ibm128 to mark tests
   where in principle the glibc code is OK but the tests fail because
   of limitations of the libgcc support for that format (e.g. GCC bug
   59666, in non-default rounding modes).  */
#define TEST_COND_ibm128_libgcc TEST_COND_ibm128

/* Mark a test as expected to fail for ibm128-libgcc.  This is used
   via XFAIL_ROUNDING_IBM128_LIBGCC, which gen-libm-test.pl transforms
   appropriately for each rounding mode.  */
#define XFAIL_IBM128_LIBGCC (TEST_COND_ibm128_libgcc ? XFAIL_TEST : 0)

/* Number of bits in NaN payload.  */
#if TEST_COND_ibm128
# define PAYLOAD_DIG (DBL_MANT_DIG - 2)
#else
# define PAYLOAD_DIG (MANT_DIG - 2)
#endif

/* Values underflowing only for float.  */
#if TEST_COND_binary32
# define UNDERFLOW_EXCEPTION_FLOAT	UNDERFLOW_EXCEPTION
# define UNDERFLOW_EXCEPTION_OK_FLOAT	UNDERFLOW_EXCEPTION_OK
#else
# define UNDERFLOW_EXCEPTION_FLOAT	0
# define UNDERFLOW_EXCEPTION_OK_FLOAT	0
#endif

/* Values underflowing only for double or types with a larger least
   positive normal value.  */
#if TEST_COND_binary32 || TEST_COND_binary64 || TEST_COND_ibm128
# define UNDERFLOW_EXCEPTION_DOUBLE	UNDERFLOW_EXCEPTION
# define UNDERFLOW_EXCEPTION_OK_DOUBLE	UNDERFLOW_EXCEPTION_OK
#else
# define UNDERFLOW_EXCEPTION_DOUBLE	0
# define UNDERFLOW_EXCEPTION_OK_DOUBLE	0
#endif

/* Values underflowing only for IBM long double or types with a larger least
   positive normal value.  */
#if TEST_COND_binary32 || TEST_COND_ibm128
# define UNDERFLOW_EXCEPTION_LDOUBLE_IBM	UNDERFLOW_EXCEPTION
#else
# define UNDERFLOW_EXCEPTION_LDOUBLE_IBM	0
#endif

/* Values underflowing on architectures detecting tininess before
   rounding, but not on those detecting tininess after rounding.  */
#define UNDERFLOW_EXCEPTION_BEFORE_ROUNDING	(TININESS_AFTER_ROUNDING \
						 ? 0			\
						 : UNDERFLOW_EXCEPTION)

#if LONG_MAX == 0x7fffffff
# define TEST_COND_long32	1
# define TEST_COND_long64	0
#else
# define TEST_COND_long32	0
# define TEST_COND_long64	1
#endif
#define TEST_COND_before_rounding	(!TININESS_AFTER_ROUNDING)
#define TEST_COND_after_rounding	TININESS_AFTER_ROUNDING

/* Various constants derived from pi.  We must supply them precalculated for
   accuracy.  They are written as a series of postfix operations to keep
   them concise yet somewhat readable.  */

/* (pi * 3) / 4 */
#define lit_pi_3_m_4_d		LIT (2.356194490192344928846982537459627163)
/* pi * 3 / (4 * ln(10)) */
#define lit_pi_3_m_4_ln10_m_d	LIT (1.023282265381381010614337719073516828)
/* pi / (2 * ln(10)) */
#define lit_pi_2_ln10_m_d	LIT (0.682188176920920673742891812715677885)
/* pi / (4 * ln(10)) */
#define lit_pi_4_ln10_m_d	LIT (0.341094088460460336871445906357838943)
/* pi / ln(10) */
#define lit_pi_ln10_d		LIT (1.364376353841841347485783625431355770)
/* pi / 2 */
#define lit_pi_2_d		LITM (M_PI_2)
/* pi / 4 */
#define lit_pi_4_d		LITM (M_PI_4)
/* pi */
#define lit_pi			LITM (M_PI)

/* Other useful constants.  */

/* e */
#define lit_e			LITM (M_E)

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

#define plus_zero	LIT (0.0)
#define minus_zero	LIT (-0.0)
#define plus_infty	FUNC (__builtin_inf) ()
#define minus_infty	-(FUNC (__builtin_inf) ())
#define qnan_value_pl(S)	FUNC (__builtin_nan) (S)
#define qnan_value	qnan_value_pl ("")
#define snan_value_pl(S)	FUNC (__builtin_nans) (S)
#define snan_value	snan_value_pl ("")
#define max_value	TYPE_MAX
#define min_value	TYPE_MIN
#define min_subnorm_value TYPE_TRUE_MIN

/* For nexttoward tests.  */
#define snan_value_ld	__builtin_nansl ("")

static FLOAT max_error, real_max_error, imag_max_error;

static FLOAT prev_max_error, prev_real_max_error, prev_imag_max_error;

static FLOAT max_valid_error;

/* Sufficient numbers of digits to represent any floating-point value
   unambiguously (for any choice of the number of bits in the first
   hex digit, in the case of TYPE_HEX_DIG).  When used with printf
   formats where the precision counts only digits after the point, 1
   is subtracted from these values. */
#define TYPE_DECIMAL_DIG __CONCATX (PREFIX, _DECIMAL_DIG)
#define TYPE_HEX_DIG ((MANT_DIG + 6) / 4)

/* Converts VALUE (a floating-point number) to string and writes it to DEST.
   PRECISION specifies the number of fractional digits that should be printed.
   CONVERSION is the conversion specifier, such as in printf, e.g. 'f' or 'a'.
   The output is prepended with an empty space if VALUE is non-negative.  */
static void
fmt_ftostr (char *dest, size_t size, int precision, const char *conversion,
	    FLOAT value)
{
  char format[64];
  char *ptr_format;
  int ret;

  /* Generate the format string.  */
  ptr_format = stpcpy (format, "%.");
  ret = sprintf (ptr_format, "%d", precision);
  ptr_format += ret;
  ptr_format = stpcpy (ptr_format, conversion);

  /* Add a space to the beginning of the output string, if the floating-point
     number is non-negative.  This mimics the behavior of the space (' ') flag
     in snprintf, which is not available on strfrom.  */
  if (! signbit (value))
    {
      *dest = ' ';
      dest++;
      size--;
    }

  /* Call the float to string conversion function, e.g.: strfromd.  */
  FTOSTR (dest, size, format, value);
}

/* Compare KEY (a string, with the name of a function) with ULP (a
   pointer to a struct ulp_data structure), returning a value less
   than, equal to or greater than zero for use in bsearch.  */

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
    return entry->max_ulp[ULP_IDX];
}

static void
init_max_error (const char *name, int exact)
{
  max_error = 0;
  real_max_error = 0;
  imag_max_error = 0;
  prev_max_error = find_ulps (name, func_ulps,
			      sizeof (func_ulps) / sizeof (func_ulps[0]));
  prev_real_max_error = find_ulps (name, func_real_ulps,
				   (sizeof (func_real_ulps)
				    / sizeof (func_real_ulps[0])));
  prev_imag_max_error = find_ulps (name, func_imag_ulps,
				   (sizeof (func_imag_ulps)
				    / sizeof (func_imag_ulps[0])));
#if TEST_COND_ibm128
  /* The documented accuracy of IBM long double division is 3ulp (see
     libgcc/config/rs6000/ibm-ldouble-format), so do not require
     better accuracy for libm functions that are exactly defined for
     other formats.  */
  max_valid_error = exact ? 3 : 16;
#else
  max_valid_error = exact ? 0 : 9;
#endif
  prev_max_error = (prev_max_error <= max_valid_error
		    ? prev_max_error
		    : max_valid_error);
  prev_real_max_error = (prev_real_max_error <= max_valid_error
			 ? prev_real_max_error
			 : max_valid_error);
  prev_imag_max_error = (prev_imag_max_error <= max_valid_error
			 ? prev_imag_max_error
			 : max_valid_error);
  feclearexcept (FE_ALL_EXCEPT);
  errno = 0;
}

static void
set_max_error (FLOAT current, FLOAT *curr_max_error)
{
  if (current > *curr_max_error && current <= max_valid_error)
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
    {
      char fstrn[FSTR_MAX], fstrx[FSTR_MAX];
      fmt_ftostr (fstrn, FSTR_MAX, TYPE_DECIMAL_DIG - 1, "e", f);
      fmt_ftostr (fstrx, FSTR_MAX, TYPE_HEX_DIG - 1, "a", f);
      printf ("%s  %s\n", fstrn, fstrx);
    }
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
print_function_ulps (const char *function_name, FLOAT ulp)
{
  if (output_ulps)
    {
      char ustrn[FSTR_MAX];
      FTOSTR (ustrn, FSTR_MAX, "%.0f", FUNC (ceil) (ulp));
      fprintf (ulps_file, "Function: \"%s\":\n", function_name);
      fprintf (ulps_file, QTYPE_STR ": %s\n", ustrn);
    }
}


static void
print_complex_function_ulps (const char *function_name, FLOAT real_ulp,
			     FLOAT imag_ulp)
{
  if (output_ulps)
    {
      char fstrn[FSTR_MAX];
      if (real_ulp != 0.0)
	{
	  FTOSTR (fstrn, FSTR_MAX, "%.0f", FUNC (ceil) (real_ulp));
	  fprintf (ulps_file, "Function: Real part of \"%s\":\n", function_name);
	  fprintf (ulps_file, QTYPE_STR ": %s\n", fstrn);
	}
      if (imag_ulp != 0.0)
	{
	  FTOSTR (fstrn, FSTR_MAX, "%.0f", FUNC (ceil) (imag_ulp));
	  fprintf (ulps_file, "Function: Imaginary part of \"%s\":\n", function_name);
	  fprintf (ulps_file, QTYPE_STR ": %s\n", fstrn);
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
  int ok = 0;

  if (max_error == 0.0 || (max_error <= prev_max_error && !ignore_max_ulp))
    {
      ok = 1;
    }

  if (!ok)
    print_function_ulps (func_name, max_error);


  if (print_screen_max_error (ok))
    {
      char mestr[FSTR_MAX], pmestr[FSTR_MAX];
      FTOSTR (mestr, FSTR_MAX, "%.0f", FUNC (ceil) (max_error));
      FTOSTR (pmestr, FSTR_MAX, "%.0f", FUNC (ceil) (prev_max_error));
      printf ("Maximal error of `%s'\n", func_name);
      printf (" is      : %s ulp\n", mestr);
      printf (" accepted: %s ulp\n", pmestr);
    }

  update_stats (ok);
}


static void
print_complex_max_error (const char *func_name)
{
  int real_ok = 0, imag_ok = 0, ok;

  if (real_max_error == 0
      || (real_max_error <= prev_real_max_error && !ignore_max_ulp))
    {
      real_ok = 1;
    }

  if (imag_max_error == 0
      || (imag_max_error <= prev_imag_max_error && !ignore_max_ulp))
    {
      imag_ok = 1;
    }

  ok = real_ok && imag_ok;

  if (!ok)
    print_complex_function_ulps (func_name,
				 real_ok ? 0 : real_max_error,
				 imag_ok ? 0 : imag_max_error);

  if (print_screen_max_error (ok))
    {
      char rmestr[FSTR_MAX], prmestr[FSTR_MAX];
      char imestr[FSTR_MAX], pimestr[FSTR_MAX];
      FTOSTR (rmestr, FSTR_MAX, "%.0f", FUNC (ceil) (real_max_error));
      FTOSTR (prmestr, FSTR_MAX, "%.0f", FUNC (ceil) (prev_real_max_error));
      FTOSTR (imestr, FSTR_MAX, "%.0f", FUNC (ceil) (imag_max_error));
      FTOSTR (pimestr, FSTR_MAX, "%.0f", FUNC (ceil) (prev_imag_max_error));
      printf ("Maximal error of real part of: %s\n", func_name);
      printf (" is      : %s ulp\n", rmestr);
      printf (" accepted: %s ulp\n", prmestr);
      printf ("Maximal error of imaginary part of: %s\n", func_name);
      printf (" is      : %s ulp\n", imestr);
      printf (" accepted: %s ulp\n", pimestr);
    }

  update_stats (ok);
}


#if FE_ALL_EXCEPT
/* Test whether a given exception was raised.  */
static void
test_single_exception (const char *test_name,
		       int exception,
		       int exc_flag,
		       int fe_flag,
		       const char *flag_name)
{
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
}
#endif

/* Test whether exceptions given by EXCEPTION are raised.  Ignore thereby
   allowed but not required exceptions.
*/
static void
test_exceptions (const char *test_name, int exception)
{
  if (TEST_EXCEPTIONS && EXCEPTION_TESTS (FLOAT))
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
      /* Spurious "underflow" and "inexact" exceptions are always
	 allowed for IBM long double, in line with the underlying
	 arithmetic.  */
#ifdef FE_UNDERFLOW
      if ((exception & UNDERFLOW_EXCEPTION_OK) == 0
	  && !(TEST_COND_ibm128
	       && (exception & UNDERFLOW_EXCEPTION) == 0))
	test_single_exception (test_name, exception, UNDERFLOW_EXCEPTION,
			       FE_UNDERFLOW, "Underflow");
#endif
#ifdef FE_INEXACT
      if ((exception & (INEXACT_EXCEPTION | NO_INEXACT_EXCEPTION)) != 0
	  && !(TEST_COND_ibm128
	       && (exception & NO_INEXACT_EXCEPTION) != 0))
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
  if (TEST_ERRNO)
    {
      ++noErrnoTests;
      if (exceptions & ERRNO_UNCHANGED)
	test_single_errno (test_name, errno_value, 0, "unchanged");
      if (exceptions & ERRNO_EDOM)
	test_single_errno (test_name, errno_value, EDOM, "EDOM");
      if (exceptions & ERRNO_ERANGE)
	test_single_errno (test_name, errno_value, ERANGE, "ERANGE");
    }
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
	   2^-(MANT_DIG - 1) which is too large a value to be useful. Note that we
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
	ulp = FUNC(ldexp) (1.0, FUNC(ilogb) (value) - MANT_DIG + 1);
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
		      FLOAT *curr_max_error, FLOAT max_ulp)
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
  if (issignaling (computed) && issignaling (expected))
    {
      if ((exceptions & TEST_NAN_SIGN) != 0
	  && signbit (computed) != signbit (expected))
	{
	  ok = 0;
	  printf ("signaling NaN has wrong sign.\n");
	}
      else if ((exceptions & TEST_NAN_PAYLOAD) != 0
	       && (FUNC (getpayload) (&computed)
		   != FUNC (getpayload) (&expected)))
	{
	  ok = 0;
	  printf ("signaling NaN has wrong payload.\n");
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
      else if ((exceptions & TEST_NAN_PAYLOAD) != 0
	       && (FUNC (getpayload) (&computed)
		   != FUNC (getpayload) (&expected)))
	{
	  ok = 0;
	  printf ("quiet NaN has wrong payload.\n");
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
      else if (ulps <= max_ulp && !ignore_max_ulp)
	ok = 1;
      else
	ok = 0;
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
	  char dstrn[FSTR_MAX], dstrx[FSTR_MAX];
	  char ustrn[FSTR_MAX], mustrn[FSTR_MAX];
	  fmt_ftostr (dstrn, FSTR_MAX, TYPE_DECIMAL_DIG - 1, "e", diff);
	  fmt_ftostr (dstrx, FSTR_MAX, TYPE_HEX_DIG - 1, "a", diff);
	  fmt_ftostr (ustrn, FSTR_MAX, 4, "f", ulps);
	  fmt_ftostr (mustrn, FSTR_MAX, 4, "f", max_ulp);
	  printf (" difference: %s  %s\n", dstrn, dstrx);
	  printf (" ulp       : %s\n", ustrn);
	  printf (" max.ulp   : %s\n", mustrn);
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
			exceptions, &max_error, prev_max_error);
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
			exception, &real_max_error, prev_real_max_error);
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
			&imag_max_error, prev_imag_max_error);
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


/* Check that computed and expected values are equal (intmax_t values).  */
static void
check_intmax_t (const char *test_name, intmax_t computed,
		intmax_t expected, int exceptions)
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
      printf (" is:         %jd\n", computed);
      printf (" should be:  %jd\n", expected);
    }

  update_stats (ok);
 out:
  fpstack_test (test_name);
  errno = 0;
}


/* Check that computed and expected values are equal (uintmax_t values).  */
static void
check_uintmax_t (const char *test_name, uintmax_t computed,
		 uintmax_t expected, int exceptions)
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
      printf (" is:         %ju\n", computed);
      printf (" should be:  %ju\n", expected);
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
  if (TEST_INLINE && (exceptions & NO_TEST_INLINE))
    return 0;
  if (TEST_FINITE && (exceptions & NON_FINITE) != 0)
    return 0;
  if (!SNAN_TESTS (FLOAT) && (exceptions & TEST_SNAN) != 0)
    return 0;
  if (TEST_MATHVEC && (exceptions & NO_TEST_MATHVEC) != 0)
    return 0;

  return 1;
}

/* Structures for each kind of test.  */
/* Used for both RUN_TEST_LOOP_f_f and RUN_TEST_LOOP_fp_f.  */
struct test_f_f_data
{
  const char *arg_str;
  FLOAT arg;
  struct
  {
    FLOAT expected;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_ff_f_data
{
  const char *arg_str;
  FLOAT arg1, arg2;
  struct
  {
    FLOAT expected;
    int exceptions;
  } rd, rn, rz, ru;
};
/* Strictly speaking, a j type argument is one gen-libm-test.pl will not
   attempt to muck with.  For now, it is only used to prevent it from
   mucking up an explicitly long double argument.  */
struct test_fj_f_data
{
  const char *arg_str;
  FLOAT arg1;
  long double arg2;
  struct
  {
    FLOAT expected;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_fi_f_data
{
  const char *arg_str;
  FLOAT arg1;
  int arg2;
  struct
  {
    FLOAT expected;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_fl_f_data
{
  const char *arg_str;
  FLOAT arg1;
  long int arg2;
  struct
  {
    FLOAT expected;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_if_f_data
{
  const char *arg_str;
  int arg1;
  FLOAT arg2;
  struct
  {
    FLOAT expected;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_fff_f_data
{
  const char *arg_str;
  FLOAT arg1, arg2, arg3;
  struct
  {
    FLOAT expected;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_fiu_M_data
{
  const char *arg_str;
  FLOAT arg1;
  int arg2;
  unsigned int arg3;
  struct
  {
    intmax_t expected;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_fiu_U_data
{
  const char *arg_str;
  FLOAT arg1;
  int arg2;
  unsigned int arg3;
  struct
  {
    uintmax_t expected;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_c_f_data
{
  const char *arg_str;
  FLOAT argr, argc;
  struct
  {
    FLOAT expected;
    int exceptions;
  } rd, rn, rz, ru;
};
/* Used for both RUN_TEST_LOOP_f_f1 and RUN_TEST_LOOP_fI_f1.  */
struct test_f_f1_data
{
  const char *arg_str;
  FLOAT arg;
  struct
  {
    FLOAT expected;
    int exceptions;
    int extra_test;
    int extra_expected;
  } rd, rn, rz, ru;
};
struct test_fF_f1_data
{
  const char *arg_str;
  FLOAT arg;
  struct
  {
    FLOAT expected;
    int exceptions;
    int extra_test;
    FLOAT extra_expected;
  } rd, rn, rz, ru;
};
struct test_ffI_f1_data
{
  const char *arg_str;
  FLOAT arg1, arg2;
  struct
  {
    FLOAT expected;
    int exceptions;
    int extra_test;
    int extra_expected;
  } rd, rn, rz, ru;
};
struct test_c_c_data
{
  const char *arg_str;
  FLOAT argr, argc;
  struct
  {
    FLOAT expr, expc;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_cc_c_data
{
  const char *arg_str;
  FLOAT arg1r, arg1c, arg2r, arg2c;
  struct
  {
    FLOAT expr, expc;
    int exceptions;
  } rd, rn, rz, ru;
};
/* Used for all of RUN_TEST_LOOP_f_i, RUN_TEST_LOOP_f_i_tg,
   RUN_TEST_LOOP_f_b and RUN_TEST_LOOP_f_b_tg.  */
struct test_f_i_data
{
  const char *arg_str;
  FLOAT arg;
  struct
  {
    int expected;
    int exceptions;
  } rd, rn, rz, ru;
};
/* Used for both RUN_TEST_LOOP_ff_b and RUN_TEST_LOOP_ff_i_tg.  */
struct test_ff_i_data
{
  const char *arg_str;
  FLOAT arg1, arg2;
  struct
  {
    int expected;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_f_l_data
{
  const char *arg_str;
  FLOAT arg;
  struct
  {
    long int expected;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_f_L_data
{
  const char *arg_str;
  FLOAT arg;
  struct
  {
    long long int expected;
    int exceptions;
  } rd, rn, rz, ru;
};
struct test_fFF_11_data
{
  const char *arg_str;
  FLOAT arg;
  struct
  {
    int exceptions;
    int extra1_test;
    FLOAT extra1_expected;
    int extra2_test;
    FLOAT extra2_expected;
  } rd, rn, rz, ru;
};
/* Used for both RUN_TEST_LOOP_Ff_b1 and RUN_TEST_LOOP_Ffp_b1.  */
struct test_Ff_b1_data
{
  const char *arg_str;
  FLOAT arg;
  struct
  {
    int expected;
    int exceptions;
    int extra_test;
    FLOAT extra_expected;
  } rd, rn, rz, ru;
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

/* Field name to use for a given rounding mode.  */
#define RM_			rn
#define RM_FE_DOWNWARD		rd
#define RM_FE_TONEAREST		rn
#define RM_FE_TOWARDZERO	rz
#define RM_FE_UPWARD		ru

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
	check_float (test_name,	FUNC_TEST (FUNC_NAME) (ARG),		\
		     EXPECTED, EXCEPTIONS);				\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_f ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		    (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_fp_f(ARG_STR, FUNC_NAME, ARG, EXPECTED,		\
		     EXCEPTIONS)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_float (test_name,	FUNC_TEST (FUNC_NAME) (&(ARG)),		\
		     EXPECTED, EXCEPTIONS);				\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_fp_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_fp_f ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		    (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_2_f(ARG_STR, FUNC_NAME, ARG1, ARG2, EXPECTED,	\
		     EXCEPTIONS)				\
  do								\
    if (enable_test (EXCEPTIONS))				\
      {								\
	COMMON_TEST_SETUP (ARG_STR);				\
	check_float (test_name, FUNC_TEST (FUNC_NAME) (ARG1, ARG2),	\
		     EXPECTED, EXCEPTIONS);			\
	COMMON_TEST_CLEANUP;					\
      }								\
  while (0)
#define RUN_TEST_LOOP_2_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_2_f ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg1,	\
		    (ARRAY)[i].arg2,					\
		    (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		    (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_ff_f RUN_TEST_2_f
#define RUN_TEST_LOOP_ff_f RUN_TEST_LOOP_2_f
#define RUN_TEST_LOOP_fj_f RUN_TEST_LOOP_2_f
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
	check_float (test_name, FUNC_TEST (FUNC_NAME) (ARG1, ARG2, ARG3),	\
		     EXPECTED, EXCEPTIONS);				\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_fff_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_fff_f ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg1,	\
		      (ARRAY)[i].arg2, (ARRAY)[i].arg3,			\
		      (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		      (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_fiu_M(ARG_STR, FUNC_NAME, ARG1, ARG2, ARG3,		\
		       EXPECTED, EXCEPTIONS)				\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_intmax_t (test_name,					\
			FUNC_TEST (FUNC_NAME) (ARG1, ARG2, ARG3),	\
			EXPECTED, EXCEPTIONS);				\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_fiu_M(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_fiu_M ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg1,	\
		      (ARRAY)[i].arg2, (ARRAY)[i].arg3,			\
		      (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		      (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_fiu_U(ARG_STR, FUNC_NAME, ARG1, ARG2, ARG3,		\
		       EXPECTED, EXCEPTIONS)				\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_uintmax_t (test_name,					\
			 FUNC_TEST (FUNC_NAME) (ARG1, ARG2, ARG3),	\
			 EXPECTED, EXCEPTIONS);				\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_fiu_U(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_fiu_U ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg1,	\
		      (ARRAY)[i].arg2, (ARRAY)[i].arg3,			\
		      (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		      (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_c_f(ARG_STR, FUNC_NAME, ARG1, ARG2, EXPECTED,		\
		     EXCEPTIONS)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_float (test_name,						\
		     FUNC_TEST (FUNC_NAME) (BUILD_COMPLEX (ARG1, ARG2)),\
		     EXPECTED, EXCEPTIONS);				\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_c_f(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_c_f ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].argr,	\
		    (ARRAY)[i].argc,					\
		    (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		    (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_f1(ARG_STR, FUNC_NAME, ARG, EXPECTED,		\
		      EXCEPTIONS, EXTRA_VAR, EXTRA_TEST,		\
		      EXTRA_EXPECTED)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	(EXTRA_VAR) = (EXTRA_EXPECTED) == 0 ? 1 : 0;			\
	check_float (test_name, FUNC_TEST (FUNC_NAME) (ARG), EXPECTED,	\
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
		     (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		     (ARRAY)[i].RM_##ROUNDING_MODE.exceptions,		\
		     EXTRA_VAR,						\
		     (ARRAY)[i].RM_##ROUNDING_MODE.extra_test,		\
		     (ARRAY)[i].RM_##ROUNDING_MODE.extra_expected);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_fF_f1(ARG_STR, FUNC_NAME, ARG, EXPECTED,		\
		       EXCEPTIONS, EXTRA_VAR, EXTRA_TEST,		\
		       EXTRA_EXPECTED)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	(EXTRA_VAR) = (EXTRA_EXPECTED) == 0 ? 1 : 0;			\
	check_float (test_name, FUNC_TEST (FUNC_NAME) (ARG, &(EXTRA_VAR)),	\
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
		      (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		      (ARRAY)[i].RM_##ROUNDING_MODE.exceptions,		\
		      EXTRA_VAR,					\
		      (ARRAY)[i].RM_##ROUNDING_MODE.extra_test,		\
		      (ARRAY)[i].RM_##ROUNDING_MODE.extra_expected);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_fI_f1(ARG_STR, FUNC_NAME, ARG, EXPECTED,		\
		       EXCEPTIONS, EXTRA_VAR, EXTRA_TEST,		\
		       EXTRA_EXPECTED)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	(EXTRA_VAR) = (EXTRA_EXPECTED) == 0 ? 1 : 0;			\
	check_float (test_name, FUNC_TEST (FUNC_NAME) (ARG, &(EXTRA_VAR)),	\
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
		      (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		      (ARRAY)[i].RM_##ROUNDING_MODE.exceptions,		\
		      EXTRA_VAR,					\
		      (ARRAY)[i].RM_##ROUNDING_MODE.extra_test,		\
		      (ARRAY)[i].RM_##ROUNDING_MODE.extra_expected);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_ffI_f1_mod8(ARG_STR, FUNC_NAME, ARG1, ARG2, EXPECTED,	\
			     EXCEPTIONS, EXTRA_VAR, EXTRA_TEST,		\
			     EXTRA_EXPECTED)				\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	(EXTRA_VAR) = (EXTRA_EXPECTED) == 0 ? 1 : 0;			\
	check_float (test_name,						\
		     FUNC_TEST (FUNC_NAME) (ARG1, ARG2, &(EXTRA_VAR)),	\
		     EXPECTED, EXCEPTIONS);				\
	EXTRA_OUTPUT_TEST_SETUP (ARG_STR, 1);				\
	if (EXTRA_TEST)							\
	  check_int (extra1_name, (EXTRA_VAR) % 8, EXTRA_EXPECTED, 0);	\
	EXTRA_OUTPUT_TEST_CLEANUP (1);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_ffI_f1_mod8(FUNC_NAME, ARRAY, ROUNDING_MODE,	\
				  EXTRA_VAR)				\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_ffI_f1_mod8 ((ARRAY)[i].arg_str, FUNC_NAME,		\
			    (ARRAY)[i].arg1, (ARRAY)[i].arg2,		\
			    (ARRAY)[i].RM_##ROUNDING_MODE.expected,	\
			    (ARRAY)[i].RM_##ROUNDING_MODE.exceptions,	\
			    EXTRA_VAR,					\
			    (ARRAY)[i].RM_##ROUNDING_MODE.extra_test,	\
			    (ARRAY)[i].RM_##ROUNDING_MODE.extra_expected); \
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_Ff_b1(ARG_STR, FUNC_NAME, ARG, EXPECTED,		\
		       EXCEPTIONS, EXTRA_VAR, EXTRA_TEST,		\
		       EXTRA_EXPECTED)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	(EXTRA_VAR) = (EXTRA_EXPECTED) == 0 ? 1 : 0;			\
	/* Clear any exceptions from comparison involving sNaN		\
	   EXTRA_EXPECTED.  */						\
	feclearexcept (FE_ALL_EXCEPT);					\
	check_bool (test_name, FUNC_TEST (FUNC_NAME) (&(EXTRA_VAR),	\
						      (ARG)),		\
		    EXPECTED, EXCEPTIONS);				\
	EXTRA_OUTPUT_TEST_SETUP (ARG_STR, 1);				\
	if (EXTRA_TEST)							\
	  check_float (extra1_name, EXTRA_VAR, EXTRA_EXPECTED,		\
		       (EXCEPTIONS) & TEST_NAN_PAYLOAD);		\
	EXTRA_OUTPUT_TEST_CLEANUP (1);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_Ff_b1(FUNC_NAME, ARRAY, ROUNDING_MODE,		\
			    EXTRA_VAR)					\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_Ff_b1 ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		      (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		      (ARRAY)[i].RM_##ROUNDING_MODE.exceptions,		\
		      EXTRA_VAR,					\
		      (ARRAY)[i].RM_##ROUNDING_MODE.extra_test,		\
		      (ARRAY)[i].RM_##ROUNDING_MODE.extra_expected);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_Ffp_b1(ARG_STR, FUNC_NAME, ARG, EXPECTED,		\
			EXCEPTIONS, EXTRA_VAR, EXTRA_TEST,		\
			EXTRA_EXPECTED)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	(EXTRA_VAR) = (EXTRA_EXPECTED) == 0 ? 1 : 0;			\
	check_bool (test_name, FUNC_TEST (FUNC_NAME) (&(EXTRA_VAR),	\
						      &(ARG)),		\
		    EXPECTED, EXCEPTIONS);				\
	EXTRA_OUTPUT_TEST_SETUP (ARG_STR, 1);				\
	if (EXTRA_TEST)							\
	  check_float (extra1_name, EXTRA_VAR, EXTRA_EXPECTED,		\
		       (EXCEPTIONS) & TEST_NAN_PAYLOAD);		\
	EXTRA_OUTPUT_TEST_CLEANUP (1);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_Ffp_b1(FUNC_NAME, ARRAY, ROUNDING_MODE,		\
			     EXTRA_VAR)					\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_Ffp_b1 ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		       (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		       (ARRAY)[i].RM_##ROUNDING_MODE.exceptions,	\
		       EXTRA_VAR,					\
		       (ARRAY)[i].RM_##ROUNDING_MODE.extra_test,	\
		       (ARRAY)[i].RM_##ROUNDING_MODE.extra_expected);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_c_c(ARG_STR, FUNC_NAME, ARGR, ARGC, EXPR, EXPC,	\
		     EXCEPTIONS)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_complex (test_name,					\
		       FUNC_TEST (FUNC_NAME) (BUILD_COMPLEX (ARGR, ARGC)),	\
		       BUILD_COMPLEX (EXPR, EXPC), EXCEPTIONS);		\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_c_c(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_c_c ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].argr,	\
		    (ARRAY)[i].argc,					\
		    (ARRAY)[i].RM_##ROUNDING_MODE.expr,			\
		    (ARRAY)[i].RM_##ROUNDING_MODE.expc,			\
		    (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_cc_c(ARG_STR, FUNC_NAME, ARG1R, ARG1C, ARG2R, ARG2C,	\
		      EXPR, EXPC, EXCEPTIONS)				\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_complex (test_name,					\
		       FUNC_TEST (FUNC_NAME) (BUILD_COMPLEX (ARG1R, ARG1C),	\
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
		     (ARRAY)[i].arg2c,					\
		     (ARRAY)[i].RM_##ROUNDING_MODE.expr,		\
		     (ARRAY)[i].RM_##ROUNDING_MODE.expc,		\
		     (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_i(ARG_STR, FUNC_NAME, ARG, EXPECTED, EXCEPTIONS)	\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_int (test_name, FUNC_TEST (FUNC_NAME) (ARG), EXPECTED,	\
		   EXCEPTIONS);						\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_i(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_i ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		    (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);		\
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
		       (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		       (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_ff_b(ARG_STR, FUNC_NAME, ARG1, ARG2, EXPECTED,		\
		      EXCEPTIONS)					\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_bool (test_name, FUNC_TEST (FUNC_NAME) (ARG1, ARG2),	\
		    EXPECTED, EXCEPTIONS);				\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_ff_b(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_ff_b ((ARRAY)[i].arg_str, FUNC_NAME,			\
		     (ARRAY)[i].arg1, (ARRAY)[i].arg2,			\
		     (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		     (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);		\
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
      RUN_TEST_ff_i_tg ((ARRAY)[i].arg_str, FUNC_NAME,			\
			(ARRAY)[i].arg1, (ARRAY)[i].arg2,		\
			(ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
			(ARRAY)[i].RM_##ROUNDING_MODE.exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_b(ARG_STR, FUNC_NAME, ARG, EXPECTED, EXCEPTIONS)	\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_bool (test_name, FUNC_TEST (FUNC_NAME) (ARG), EXPECTED,	\
		    EXCEPTIONS);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_b(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_b ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		    (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);		\
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
		       (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		       (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);	\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_l(ARG_STR, FUNC_NAME, ARG, EXPECTED, EXCEPTIONS)	\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_long (test_name, FUNC_TEST (FUNC_NAME) (ARG), EXPECTED,	\
		    EXCEPTIONS);					\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_l(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_l ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		    (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_f_L(ARG_STR, FUNC_NAME, ARG, EXPECTED, EXCEPTIONS)	\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	check_longlong (test_name, FUNC_TEST (FUNC_NAME) (ARG),		\
			EXPECTED, EXCEPTIONS);				\
	COMMON_TEST_CLEANUP;						\
      }									\
  while (0)
#define RUN_TEST_LOOP_f_L(FUNC_NAME, ARRAY, ROUNDING_MODE)		\
  IF_ROUND_INIT_ ## ROUNDING_MODE					\
    for (size_t i = 0; i < sizeof (ARRAY) / sizeof (ARRAY)[0]; i++)	\
      RUN_TEST_f_L ((ARRAY)[i].arg_str, FUNC_NAME, (ARRAY)[i].arg,	\
		    (ARRAY)[i].RM_##ROUNDING_MODE.expected,		\
		    (ARRAY)[i].RM_##ROUNDING_MODE.exceptions);		\
  ROUND_RESTORE_ ## ROUNDING_MODE
#define RUN_TEST_fFF_11(ARG_STR, FUNC_NAME, ARG, EXCEPTIONS,		\
			EXTRA1_VAR, EXTRA1_TEST,			\
			EXTRA1_EXPECTED, EXTRA2_VAR,			\
			EXTRA2_TEST, EXTRA2_EXPECTED)			\
  do									\
    if (enable_test (EXCEPTIONS))					\
      {									\
	COMMON_TEST_SETUP (ARG_STR);					\
	FUNC_TEST (FUNC_NAME) (ARG, &(EXTRA1_VAR), &(EXTRA2_VAR));	\
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
		       (ARRAY)[i].RM_##ROUNDING_MODE.exceptions,	\
		       EXTRA1_VAR,					\
		       (ARRAY)[i].RM_##ROUNDING_MODE.extra1_test,	\
		       (ARRAY)[i].RM_##ROUNDING_MODE.extra1_expected,	\
		       EXTRA2_VAR,					\
		       (ARRAY)[i].RM_##ROUNDING_MODE.extra2_test,	\
		       (ARRAY)[i].RM_##ROUNDING_MODE.extra2_expected);	\
  ROUND_RESTORE_ ## ROUNDING_MODE

#if !TEST_MATHVEC
# define VEC_SUFF
#endif

#define STR_CONCAT(a, b, c) __STRING (a##b##c)
#define STR_CON3(a, b, c) STR_CONCAT (a, b, c)

/* This generated header defines series of macros started with HAVE_VECTOR_. */
#include "libm-have-vector-test.h"

#define HAVE_VECTOR(func) __CONCAT (HAVE_VECTOR_, func)

/* Start and end the tests for a given function.  */
#define START(FUN, SUFF, EXACT)					\
  CHECK_ARCH_EXT;						\
  if (TEST_MATHVEC && !HAVE_VECTOR (FUNC (FUN))) return;	\
  const char *this_func = STR_CON3 (FUN, SUFF, VEC_SUFF);	\
  init_max_error (this_func, EXACT)
#define END					\
  print_max_error (this_func)
#define END_COMPLEX				\
  print_complex_max_error (this_func)

/* Run tests for a given function in all rounding modes.  */
#define ALL_RM_TEST(FUNC, EXACT, ARRAY, LOOP_MACRO, END_MACRO, ...)	\
  do									\
    {									\
      do								\
	{								\
	  START (FUNC,, EXACT);						\
	  LOOP_MACRO (FUNC, ARRAY, , ## __VA_ARGS__);			\
	  END_MACRO;							\
	}								\
      while (0);							\
      do								\
	{								\
	  START (FUNC, _downward, EXACT);				\
	  LOOP_MACRO (FUNC, ARRAY, FE_DOWNWARD, ## __VA_ARGS__);	\
	  END_MACRO;							\
	}								\
      while (0);							\
      do								\
	{								\
	  START (FUNC, _towardzero, EXACT);				\
	  LOOP_MACRO (FUNC, ARRAY, FE_TOWARDZERO, ## __VA_ARGS__);	\
	  END_MACRO;							\
	}								\
      while (0);							\
      do								\
	{								\
	  START (FUNC, _upward, EXACT);				\
	  LOOP_MACRO (FUNC, ARRAY, FE_UPWARD, ## __VA_ARGS__);		\
	  END_MACRO;							\
	}								\
      while (0);							\
    }									\
  while (0);

/* This is to prevent messages from the SVID libm emulation.  */
int
matherr (struct exception *x __attribute__ ((unused)))
{
  return 1;
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

static void do_test (void);

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

  INIT_ARCH_EXT;

  check_ulp ();

  do_test ();

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
