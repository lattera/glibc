/* Test signaling NaNs in issignaling, isnan, isinf, and similar functions.
   Copyright (C) 2008-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2005.

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

#define _GNU_SOURCE 1
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <fenv.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

#include <math-tests.h>


int dest_offset;
char *dest_address;
double	value = 123.456;
double	zero = 0.0;

static sigjmp_buf sigfpe_buf;

typedef long double ldouble;


void
myFPsighandler(int signal,
             siginfo_t *info,
             void *context)
{
  siglongjmp(sigfpe_buf, 0);
}

int
set_sigaction_FP(void)
{
    struct sigaction sa;
    /* register RT signal handler via sigaction */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = &myFPsighandler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGFPE, &sa, NULL);

    return 0;
}

int
remove_sigaction_FP(void)
{
    struct sigaction sa;
    /* restore default RT signal handler via sigaction */
    sa.sa_flags = SA_SIGINFO;
    sa.sa_handler = SIG_DFL;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGFPE, &sa, NULL);

    return 0;
}

static int errors = 0;

static void
check (const char *testname, int result)
{
  if (!result) {
    printf ("Failure: %s\n", testname);
    errors++;
  }
}

#define TEST_FUNC(NAME, FLOAT, SUFFIX)					      \
static void								      \
NAME (void)								      \
{									      \
  /* Variables are declared volatile to forbid some compiler		      \
     optimizations.  */							      \
  volatile FLOAT Inf_var, qNaN_var, zero_var, one_var;			      \
  /* A sNaN is only guaranteed to be representable in variables with */	      \
  /* static (or thread-local) storage duration.  */			      \
  static volatile FLOAT sNaN_var = __builtin_nans ## SUFFIX ("");	      \
  static volatile FLOAT minus_sNaN_var = -__builtin_nans ## SUFFIX ("");      \
  fenv_t saved_fenv;							      \
									      \
  zero_var = 0.0;							      \
  one_var = 1.0;							      \
  qNaN_var = __builtin_nan ## SUFFIX ("");				      \
  Inf_var = one_var / zero_var;						      \
									      \
  (void) &zero_var;							      \
  (void) &one_var;							      \
  (void) &qNaN_var;							      \
  (void) &sNaN_var;							      \
  (void) &minus_sNaN_var;						      \
  (void) &Inf_var;							      \
									      \
  set_sigaction_FP ();							      \
  fegetenv(&saved_fenv);						      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " issignaling (qNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " issignaling (qNaN)", !issignaling (qNaN_var));	      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " issignaling (-qNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " issignaling (-qNaN)", !issignaling (-qNaN_var));	      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " issignaling (sNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " issignaling (sNaN)",				      \
	     SNAN_TESTS (FLOAT) ? issignaling (sNaN_var) : 1);		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " issignaling (-sNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " issignaling (-sNaN)",				      \
	     SNAN_TESTS (FLOAT) ? issignaling (minus_sNaN_var) : 1);	      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnan (qNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnan (qNaN)", isnan (qNaN_var));			      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnan (-qNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnan (-qNaN)", isnan (-qNaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnan (sNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnan (sNaN)",					      \
	     SNAN_TESTS (FLOAT) ? isnan (sNaN_var) : 1);		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnan (-sNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnan (-sNaN)",					      \
	     SNAN_TESTS (FLOAT) ? isnan (minus_sNaN_var) : 1);		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isinf (qNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isinf (qNaN)", !isinf (qNaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isinf (-qNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isinf (-qNaN)", !isinf (-qNaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isinf (sNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isinf (sNaN)",					      \
	     SNAN_TESTS (FLOAT) ? !isinf (sNaN_var) : 1);		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isinf (-sNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isinf (-sNaN)",					      \
	     SNAN_TESTS (FLOAT) ? !isinf (minus_sNaN_var) : 1);		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isfinite (qNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isfinite (qNaN)", !isfinite (qNaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isfinite (-qNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isfinite (-qNaN)", !isfinite (-qNaN_var));	      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isfinite (sNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isfinite (sNaN)",					      \
	     SNAN_TESTS (FLOAT) ? !isfinite (sNaN_var) : 1);		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isfinite (-sNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isfinite (-sNaN)",				      \
	     SNAN_TESTS (FLOAT) ? !isfinite (minus_sNaN_var) : 1);	      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnormal (qNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnormal (qNaN)", !isnormal (qNaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnormal (-qNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnormal (-qNaN)", !isnormal (-qNaN_var));	      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnormal (sNaN) isnormal SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnormal (sNaN)",					      \
	     SNAN_TESTS (FLOAT) ? !isnormal (sNaN_var) : 1);		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnormal (-sNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnormal (-sNaN)",				      \
	     SNAN_TESTS (FLOAT) ? !isnormal (minus_sNaN_var) : 1);	      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " fpclassify (qNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " fpclassify (qNaN)", (fpclassify (qNaN_var)==FP_NAN));   \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " fpclassify (-qNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " fpclassify (-qNaN)", (fpclassify (-qNaN_var)==FP_NAN)); \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " fpclassify (sNaN) isnormal SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " fpclassify (sNaN)",				      \
	     SNAN_TESTS (FLOAT) ? fpclassify (sNaN_var) == FP_NAN : 1);	      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " fpclassify (-sNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " fpclassify (-sNaN)",				      \
	     SNAN_TESTS (FLOAT) ? fpclassify (minus_sNaN_var) == FP_NAN : 1); \
    }									      \
									      \
  fesetenv(&saved_fenv); /* restore saved fenv */			      \
  remove_sigaction_FP();						      \
}

TEST_FUNC (float_test, float, f)
TEST_FUNC (double_test, double, )
#ifndef NO_LONG_DOUBLE
TEST_FUNC (ldouble_test, ldouble, l)
#endif

static int
do_test (void)
{
  float_test();
  double_test();
#ifndef NO_LONG_DOUBLE
  ldouble_test();
#endif

  return errors != 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
