/* Test for exception handling functions of libm */

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
#include <string.h>

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
test_exceptions (const char *test_name, short int exception)
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
set_single_exc (const char *test_name, int fe_exc, fexcept_t exception)
{
  char str[200];

  strcpy (str, test_name);
  strcat (str, ": set flag, with rest not set");
  feclearexcept (FE_ALL_EXCEPT);
  feraiseexcept (exception);
  test_exceptions (str, fe_exc);

  strcpy (str, test_name);
  strcat (str, ": clear flag, rest also unset");
  feclearexcept (exception);
  test_exceptions (str, NO_EXC);

  strcpy (str, test_name);
  strcat (str, ": set flag, with rest set");
  feraiseexcept (FE_ALL_EXCEPT ^ exception);
  feraiseexcept (exception);
  test_exceptions (str, ALL_EXC);

  strcpy (str, test_name);
  strcat (str, ": clear flag, leave rest set");
  feclearexcept (exception);
  test_exceptions (str, ALL_EXC ^ fe_exc);
}

static void
fe_tests (void)
{
  /* clear all exceptions and test if all are cleared */
  feclearexcept (FE_ALL_EXCEPT);
  test_exceptions ("feclearexcept (FE_ALL_EXCEPT) clears all exceptions",
                   NO_EXC);

  /* raise all exceptions and test if all are raised */
  feraiseexcept (FE_ALL_EXCEPT);
  test_exceptions ("feraiseexcept (FE_ALL_EXCEPT) raises all exceptions",
                   ALL_EXC);
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

int
main (void)
{
  fe_tests ();
  /* _LIB_VERSION = _SVID;*/

  if (count_errors)
    {
      printf ("\n%d errors occured.\n", count_errors);
      exit (1);
    }
  printf ("\n All tests passed successfully.\n");
  exit (0);
}
