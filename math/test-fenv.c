/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@arthur.rhein-neckar.de> and
   Ulrich Drepper <drepper@cygnus.com>, 1997.

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

/* Tests for ISO C 9X 7.6: Floating-point environment  */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif

#include <complex.h>
#include <math.h>
#include <float.h>
#include <fenv.h>

#include <errno.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/resource.h>

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

/* Test whether a given exception was raised.  */
static void
test_single_exception (short int exception,
                       short int exc_flag,
                       fexcept_t fe_flag,
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
        {
          printf ("  Pass: Exception \"%s\" is not set\n", flag_name);
        }
    }
}

static void
test_exceptions (const char *test_name, short int exception,
		 int ignore_inexact)
{
  printf ("Test: %s\n", test_name);
#ifdef FE_DIVBYZERO
  test_single_exception (exception, DIVBYZERO_EXC, FE_DIVBYZERO,
                         "DIVBYZERO");
#endif
#ifdef FE_INVALID
  test_single_exception (exception, INVALID_EXC, FE_INVALID,
                         "INVALID");
#endif
#ifdef FE_INEXACT
  if (!ignore_inexact)
    test_single_exception (exception, INEXACT_EXC, FE_INEXACT,
			   "INEXACT");
#endif
#ifdef FE_UNDERFLOW
  test_single_exception (exception, UNDERFLOW_EXC, FE_UNDERFLOW,
                         "UNDERFLOW");
#endif
#ifdef FE_OVERFLOW
  test_single_exception (exception, OVERFLOW_EXC, FE_OVERFLOW,
                         "OVERFLOW");
#endif
}

static void
print_rounding (int rounding)
{

  switch (rounding)
    {
#ifdef FE_TONEAREST
    case FE_TONEAREST:
      printf ("TONEAREST");
      break;
#endif
#ifdef FE_UPWARD
    case FE_UPWARD:
      printf ("UPWARD");
      break;
#endif
#ifdef FE_DOWNWARD
    case FE_DOWNWARD:
      printf ("DOWNWARD");
      break;
#endif
#ifdef FE_TOWARDZERO
    case FE_TOWARDZERO:
      printf ("TOWARDZERO");
      break;
#endif
    }
  printf (".\n");
}


static void
test_rounding (const char *test_name, int rounding_mode)
{
  int curr_rounding = fegetround ();

  printf ("Test: %s\n", test_name);
  if (curr_rounding == rounding_mode)
    {
      printf ("  Pass: Rounding mode is ");
      print_rounding (curr_rounding);
    }
  else
    {
      ++count_errors;
      printf ("  Fail: Rounding mode is ");
      print_rounding (curr_rounding);
    }
}


static void
set_single_exc (const char *test_name, int fe_exc, fexcept_t exception)
{
  char str[200];
  /* The standard allows the inexact exception to be set together with the
     underflow and overflow exceptions.  So ignore the inexact flag if the
     others are raised.  */
  int ignore_inexact = (fe_exc & (UNDERFLOW_EXC | OVERFLOW_EXC)) != 0;

  strcpy (str, test_name);
  strcat (str, ": set flag, with rest not set");
  feclearexcept (FE_ALL_EXCEPT);
  feraiseexcept (exception);
  test_exceptions (str, fe_exc, ignore_inexact);

  strcpy (str, test_name);
  strcat (str, ": clear flag, rest also unset");
  feclearexcept (exception);
  test_exceptions (str, NO_EXC, ignore_inexact);

  strcpy (str, test_name);
  strcat (str, ": set flag, with rest set");
  feraiseexcept (FE_ALL_EXCEPT ^ exception);
  feraiseexcept (exception);
  test_exceptions (str, ALL_EXC, 0);

  strcpy (str, test_name);
  strcat (str, ": clear flag, leave rest set");
  feclearexcept (exception);
  test_exceptions (str, ALL_EXC ^ fe_exc, 0);
}

static void
fe_tests (void)
{
  /* clear all exceptions and test if all are cleared */
  feclearexcept (FE_ALL_EXCEPT);
  test_exceptions ("feclearexcept (FE_ALL_EXCEPT) clears all exceptions",
                   NO_EXC, 0);

  /* raise all exceptions and test if all are raised */
  feraiseexcept (FE_ALL_EXCEPT);
  test_exceptions ("feraiseexcept (FE_ALL_EXCEPT) raises all exceptions",
                   ALL_EXC, 0);
  feclearexcept (FE_ALL_EXCEPT);

#ifdef FE_DIVBYZERO
  set_single_exc ("Set/Clear FE_DIVBYZERO", DIVBYZERO_EXC, FE_DIVBYZERO);
#endif
#ifdef FE_INVALID
  set_single_exc ("Set/Clear FE_INVALID", INVALID_EXC, FE_INVALID);
#endif
#ifdef FE_INEXACT
  set_single_exc ("Set/Clear FE_INEXACT", INEXACT_EXC, FE_INEXACT);
#endif
#ifdef FE_UNDERFLOW
  set_single_exc ("Set/Clear FE_UNDERFLOW", UNDERFLOW_EXC, FE_UNDERFLOW);
#endif
#ifdef FE_OVERFLOW
  set_single_exc ("Set/Clear FE_OVERFLOW", OVERFLOW_EXC, FE_OVERFLOW);
#endif
}

/* Test that program aborts with no masked interrupts */
static void
feenv_nomask_test (const char *flag_name, int fe_exc)
{
#if defined FE_NOMASK_ENV
  int status;
  pid_t pid;
  fenv_t saved;

  fegetenv (&saved);
  errno = 0;
  fesetenv (FE_NOMASK_ENV);
  status = errno;
  fesetenv (&saved);
  if (status == ENOSYS)
    {
      printf ("Test: not testing FE_NOMASK_ENV, it isn't implemented.\n");
      return;
    }

  printf ("Test: after fesetenv (FE_NOMASK_ENV) processes will abort\n");
  printf ("      when feraiseexcept (%s) is called.\n", flag_name);
  pid = fork ();
  if (pid == 0)
    {
#ifdef RLIMIT_CORE
      /* Try to avoid dumping core.  */
      struct rlimit core_limit;
      core_limit.rlim_cur = 0;
      core_limit.rlim_max = 0;
      setrlimit (RLIMIT_CORE, &core_limit);
#endif

      fesetenv (FE_NOMASK_ENV);
      feraiseexcept (fe_exc);
      exit (2);
    }
  else if (pid < 0)
    {
      if (errno != ENOSYS)
	{
	  printf ("  Fail: Could not fork.\n");
	  ++count_errors;
	}
      else
	printf ("  `fork' not implemented, test ignored.\n");
    }
  else {
    if (waitpid (pid, &status, 0) != pid)
      {
	printf ("  Fail: waitpid call failed.\n");
	++count_errors;
      }
    else if (WIFSIGNALED (status) && WTERMSIG (status) == SIGFPE)
      printf ("  Pass: Process received SIGFPE.\n");
    else
      {
	printf ("  Fail: Process didn't receive signal and exited with status %d.\n",
		status);
	++count_errors;
      }
  }
#endif
}

/* Test that program doesn't abort with default environment */
static void
feenv_mask_test (const char *flag_name, int fe_exc)
{
  int status;
  pid_t pid;

  printf ("Test: after fesetenv (FE_DFL_ENV) processes will not abort\n");
  printf ("      when feraiseexcept (%s) is called.\n", flag_name);
  pid = fork ();
  if (pid == 0)
    {
#ifdef RLIMIT_CORE
      /* Try to avoid dumping core.  */
      struct rlimit core_limit;
      core_limit.rlim_cur = 0;
      core_limit.rlim_max = 0;
      setrlimit (RLIMIT_CORE, &core_limit);
#endif

      fesetenv (FE_DFL_ENV);
      feraiseexcept (fe_exc);
      exit (2);
    }
  else if (pid < 0)
    {
      if (errno != ENOSYS)
	{
	  printf ("  Fail: Could not fork.\n");
	  ++count_errors;
	}
      else
	printf ("  `fork' not implemented, test ignored.\n");
    }
  else {
    if (waitpid (pid, &status, 0) != pid)
      {
	printf ("  Fail: waitpid call failed.\n");
	++count_errors;
      }
    else if (WIFEXITED (status) && WEXITSTATUS (status) == 2)
      printf ("  Pass: Process exited normally.\n");
    else
      {
	printf ("  Fail: Process exited abnormally with status %d.\n",
		status);
	++count_errors;
      }
  }
}



static void
feenv_tests (void)
{

#ifdef FE_DIVBYZERO
  feenv_nomask_test ("FE_DIVBYZERO", FE_DIVBYZERO);
  feenv_mask_test ("FE_DIVBYZERO", FE_DIVBYZERO);
#endif
#ifdef FE_INVALID
  feenv_nomask_test ("FE_INVALID", FE_INVALID);
  feenv_mask_test ("FE_INVALID", FE_INVALID);
#endif
#ifdef FE_INEXACT
  feenv_nomask_test ("FE_INEXACT", FE_INEXACT);
  feenv_mask_test ("FE_INEXACT", FE_INEXACT);
#endif
#ifdef FE_UNDERFLOW
  feenv_nomask_test ("FE_UNDERFLOW", FE_UNDERFLOW);
  feenv_mask_test ("FE_UNDERFLOW", FE_UNDERFLOW);
#endif
#ifdef FE_OVERFLOW
  feenv_nomask_test ("FE_OVERFLOW", FE_OVERFLOW);
  feenv_mask_test ("FE_OVERFLOW", FE_OVERFLOW);
#endif
  fesetenv (FE_DFL_ENV);
}


/* IEC 559 and ISO C 9X define a default startup environment */
static void
initial_tests (void)
{
  test_exceptions ("Initially all exceptions should be cleared",
                   NO_EXC, 0);
  test_rounding ("Rounding direction should be initalized to nearest",
                 FE_TONEAREST);
}

int
main (void)
{
  initial_tests ();
  fe_tests ();
  feenv_tests ();

  if (count_errors)
    {
      printf ("\n%d errors occured.\n", count_errors);
      exit (1);
    }
  printf ("\n All tests passed successfully.\n");
  exit (0);
}
