/* Test for fenv inline implementations.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

/* To make sure the fenv inline function are used.  */
#undef __NO_MATH_INLINES

#include <fenv.h>
#include <stdio.h>
#include <math-tests.h>

/*
  Since not all architectures might define all exceptions, we define
  a private set and map accordingly.
*/
#define NO_EXC 0
#define INEXACT_EXC 0x1
#define DIVBYZERO_EXC 0x2
#define UNDERFLOW_EXC 0x04
#define OVERFLOW_EXC 0x08
#define INVALID_EXC 0x10
#define ALL_EXC \
        (INEXACT_EXC | DIVBYZERO_EXC | UNDERFLOW_EXC | OVERFLOW_EXC | \
         INVALID_EXC)
static int count_errors;

#if FE_ALL_EXCEPT
static void
test_single_exception_fp_int (int exception,
			      int exc_flag,
			      int fe_flag,
			      const char *flag_name)
{
  if (exception & exc_flag)
    {
      if (fetestexcept (fe_flag))
        printf ("  Pass: Exception \"%s\" is set\n", flag_name);
      else
        {
          printf ("  Fail: Exception \"%s\" is not set\n", flag_name);
          ++count_errors;
        }
    }
  else
    {
      if (fetestexcept (fe_flag))
        {
          printf ("  Fail: Exception \"%s\" is set\n", flag_name);
          ++count_errors;
        }
      else
        printf ("  Pass: Exception \"%s\" is not set\n", flag_name);
    }
}
/* Test whether a given exception was raised.  */
static void
test_single_exception_fp_double (int exception,
				 int exc_flag,
				 double fe_flag,
				 const char *flag_name)
{
  if (exception & exc_flag)
    {
      if (fetestexcept (fe_flag))
        printf ("  Pass: Exception \"%s\" is set\n", flag_name);
      else
        {
          printf ("  Fail: Exception \"%s\" is not set\n", flag_name);
          ++count_errors;
        }
    }
  else
    {
      if (fetestexcept (fe_flag))
        {
          printf ("  Fail: Exception \"%s\" is set\n", flag_name);
          ++count_errors;
        }
      else
        printf ("  Pass: Exception \"%s\" is not set\n", flag_name);
    }
}
#endif

static void
test_exceptions (const char *test_name, int exception)
{
  printf ("Test: %s\n", test_name);
#ifdef FE_DIVBYZERO
  test_single_exception_fp_double (exception, DIVBYZERO_EXC, FE_DIVBYZERO,
				   "DIVBYZERO");
#endif
#ifdef FE_INVALID
  test_single_exception_fp_double (exception, INVALID_EXC, FE_INVALID,
				   "INVALID");
#endif
#ifdef FE_INEXACT
  test_single_exception_fp_double (exception, INEXACT_EXC, FE_INEXACT,
				   "INEXACT");
#endif
#ifdef FE_UNDERFLOW
  test_single_exception_fp_double (exception, UNDERFLOW_EXC, FE_UNDERFLOW,
				   "UNDERFLOW");
#endif
#ifdef FE_OVERFLOW
  test_single_exception_fp_double (exception, OVERFLOW_EXC, FE_OVERFLOW,
				   "OVERFLOW");
#endif
}

static void
test_exceptionflag (void)
{
  printf ("Test: fegetexceptionflag (FE_ALL_EXCEPT)\n");
#if FE_ALL_EXCEPT
  fexcept_t excepts;

  feclearexcept (FE_ALL_EXCEPT);

  feraiseexcept (FE_INVALID);
  fegetexceptflag (&excepts, FE_ALL_EXCEPT);

  feclearexcept (FE_ALL_EXCEPT);
  feraiseexcept (FE_OVERFLOW | FE_INEXACT);

  fesetexceptflag (&excepts, FE_ALL_EXCEPT);

  test_single_exception_fp_int (INVALID_EXC, INVALID_EXC, FE_INVALID,
				"INVALID (int)");
  test_single_exception_fp_int (INVALID_EXC, OVERFLOW_EXC, FE_OVERFLOW,
				"OVERFLOW (int)");
  test_single_exception_fp_int (INVALID_EXC, INEXACT_EXC, FE_INEXACT,
				"INEXACT (int)");

  /* Same test, but using double as argument  */
  feclearexcept (FE_ALL_EXCEPT);

  feraiseexcept (FE_INVALID);
  fegetexceptflag (&excepts, (double)FE_ALL_EXCEPT);

  feclearexcept (FE_ALL_EXCEPT);
  feraiseexcept (FE_OVERFLOW | FE_INEXACT);

  fesetexceptflag (&excepts, (double)FE_ALL_EXCEPT);

  test_single_exception_fp_double (INVALID_EXC, INVALID_EXC, FE_INVALID,
				   "INVALID (double)");
  test_single_exception_fp_double (INVALID_EXC, OVERFLOW_EXC, FE_OVERFLOW,
				   "OVERFLOW (double)");
  test_single_exception_fp_double (INVALID_EXC, INEXACT_EXC, FE_INEXACT,
				   "INEXACT (double)");
#endif
}

static void
test_fesetround (void)
{
#if defined FE_TONEAREST && defined FE_TOWARDZERO
  int res1;
  int res2;

  printf ("Tests for fesetround\n");

  /* The fesetround should not itself cause the test to fail, however it
     should either succeed for both 'int' and 'double' argument, or fail
     for both.  */
  res1 = fesetround ((int) FE_TOWARDZERO);
  res2 = fesetround ((double) FE_TOWARDZERO);
  if (res1 != res2)
    {
      printf ("fesetround (FE_TOWARDZERO) failed: %d, %d\n", res1, res2);
      ++count_errors;
    }

  res1 = fesetround ((int) FE_TONEAREST);
  res2 = fesetround ((double) FE_TONEAREST);
  if (res1 != res2)
    {
      printf ("fesetround (FE_TONEAREST) failed: %d, %d\n", res1, res2);
      ++count_errors;
    }
#endif
}

#if FE_ALL_EXCEPT
/* Tests for feenableexcept/fedisableexcept.  */
static void
feenable_test (const char *flag_name, fexcept_t fe_exc)
{
  int fe_exci = fe_exc;
  double fe_excd = fe_exc;
  int excepts;

  /* First disable all exceptions.  */
  if (fedisableexcept (FE_ALL_EXCEPT) == -1)
    {
      printf ("Test: fedisableexcept (FE_ALL_EXCEPT) failed\n");
      ++count_errors;
      /* If this fails, the other tests don't make sense.  */
      return;
    }

  /* Test for inline macros using integer argument.  */
  excepts = feenableexcept (fe_exci);
  if (!EXCEPTION_ENABLE_SUPPORTED (fe_exci) && excepts == -1)
    {
      printf ("Test: not testing feenableexcept, it isn't implemented.\n");
      return;
    }
  if (excepts == -1)
    {
      printf ("Test: feenableexcept (%s) failed\n", flag_name);
      ++count_errors;
      return;
    }
  if (excepts != 0)
    {
      printf ("Test: feenableexcept (%s) failed, return should be 0, is %x\n",
              flag_name, excepts);
      ++count_errors;
    }

  /* And now disable the exception again.  */
  excepts = fedisableexcept (fe_exc);
  if (excepts == -1)
    {
      printf ("Test: fedisableexcept (%s) failed\n", flag_name);
      ++count_errors;
      return;
    }
  if (excepts != fe_exc)
    {
      printf ("Test: fedisableexcept (%s) failed, return should be 0x%x, is 0x%x\n",
              flag_name, (unsigned int)fe_exc, excepts);
      ++count_errors;
    }

  /* Test for inline macros using double argument.  */
  excepts = feenableexcept (fe_excd);
  if (!EXCEPTION_ENABLE_SUPPORTED (fe_excd) && excepts == -1)
    {
      printf ("Test: not testing feenableexcept, it isn't implemented.\n");
      return;
    }
  if (excepts == -1)
    {
      printf ("Test: feenableexcept (%s) failed\n", flag_name);
      ++count_errors;
      return;
    }
  if (excepts != 0)
    {
      printf ("Test: feenableexcept (%s) failed, return should be 0, is %x\n",
              flag_name, excepts);
      ++count_errors;
    }

  /* And now disable the exception again.  */
  excepts = fedisableexcept (fe_exc);
  if (excepts == -1)
    {
      printf ("Test: fedisableexcept (%s) failed\n", flag_name);
      ++count_errors;
      return;
    }
  if (excepts != fe_exc)
    {
      printf ("Test: fedisableexcept (%s) failed, return should be 0x%x, is 0x%x\n",
              flag_name, (unsigned int)fe_exc, excepts);
      ++count_errors;
    }
}
#endif

static void
test_feenabledisable (void)
{
  printf ("Tests for feenableexcepts/fedisableexcept\n");

  /* We might have some exceptions still set.  */
  feclearexcept (FE_ALL_EXCEPT);

#ifdef FE_DIVBYZERO
  feenable_test ("FE_DIVBYZERO", FE_DIVBYZERO);
#endif
#ifdef FE_INVALID
  feenable_test ("FE_INVALID", FE_INVALID);
#endif
#ifdef FE_INEXACT
  feenable_test ("FE_INEXACT", FE_INEXACT);
#endif
#ifdef FE_UNDERFLOW
  feenable_test ("FE_UNDERFLOW", FE_UNDERFLOW);
#endif
#ifdef FE_OVERFLOW
  feenable_test ("FE_OVERFLOW", FE_OVERFLOW);
#endif
  fesetenv (FE_DFL_ENV);
}

static int
do_test (void)
{
  /* clear all exceptions and test if all are cleared  */
  feclearexcept (FE_ALL_EXCEPT);
  test_exceptions ("feclearexcept (FE_ALL_EXCEPT) clears all exceptions",
                   NO_EXC);

  /* raise all exceptions and test if all are raised  */
  feraiseexcept (FE_ALL_EXCEPT);
  test_exceptions ("feraiseexcept (FE_ALL_EXCEPT) raises all exceptions",
                   ALL_EXC);

  /* Same test, but using double as argument  */
  feclearexcept ((double)FE_ALL_EXCEPT);
  test_exceptions ("feclearexcept ((double)FE_ALL_EXCEPT) clears all exceptions",
                   NO_EXC);

  feraiseexcept ((double)FE_ALL_EXCEPT);
  test_exceptions ("feraiseexcept ((double)FE_ALL_EXCEPT) raises all exceptions",
                   ALL_EXC);

  test_exceptionflag ();

  test_fesetround ();

  test_feenabledisable ();

  return count_errors;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
