/* Test Signalling NaN in isnan, isinf etc functions.
   Copyright (C) 2008 Free Software Foundation, Inc.
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

#define _GNU_SOURCE
#define __USE_GNU
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

int dest_offset;
char *dest_address;
double	value = 123.456;
double	zero = 0.0;

float SNANf;
double SNAN;
long double SNANl;

static sigjmp_buf sigfpe_buf;

void
init_signaling_nan()
{
    union {
	double _ld16;
	double _d8;
	unsigned int _ui4[4];
	float _f4;
    } nan_temp;
    
    nan_temp._ui4[0] = 0x7fa00000;
    SNANf = nan_temp._f4;

    nan_temp._ui4[0] = 0x7ff40000;
    nan_temp._ui4[1] = 0x00000000;
    SNAN = nan_temp._d8;

    nan_temp._ui4[0] = 0x7ff40000;
    nan_temp._ui4[1] = 0x00000000;
    nan_temp._ui4[2] = 0x00000000;
    nan_temp._ui4[3] = 0x00000000;
    SNANl = nan_temp._ld16;
}

static float
snan_float (void)
{
  return SNANf;
}

static double
snan_double (void)
{
  return SNAN;
}

typedef long double ldouble;

static ldouble
snan_ldouble (void)
{
  return SNANl;
}


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

#define TEST_FUNC(NAME, FLOAT) \
static void								      \
NAME (void)								      \
{									      \
  /* Variables are declared volatile to forbid some compiler		      \
     optimizations.  */							      \
  volatile FLOAT Inf_var, NaN_var, zero_var, one_var, SNaN_var;		      \
  fenv_t saved_fenv;							      \
									      \
  zero_var = 0.0;							      \
  one_var = 1.0;							      \
  NaN_var = zero_var / zero_var;					      \
  SNaN_var = snan_##FLOAT ();						      \
  Inf_var = one_var / zero_var;						      \
									      \
  (void) &zero_var;							      \
  (void) &one_var;							      \
  (void) &NaN_var;							      \
  (void) &SNaN_var;							      \
  (void) &Inf_var;							      \
									      \
  set_sigaction_FP ();							      \
  fegetenv(&saved_fenv);						      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnan(NaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnan (NaN)", isnan (NaN_var));			      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnan(-NaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnan (-NaN)", isnan (-NaN_var));			      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnan(SNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnan (SNaN)", isnan (SNaN_var));			      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnan(-SNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnan (-SNaN)", isnan (-SNaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isinf(NaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isinf (NaN)", !isinf (NaN_var));			      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isinf(-NaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isinf (-NaN)", !isinf (-NaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isinf(SNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isinf (SNaN)", !isinf (SNaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isinf(-SNaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isinf (-SNaN)", !isinf (-SNaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isfinite(NaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isfinite (NaN)", !isfinite (NaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isfinite(-NaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isfinite (-NaN)", !isfinite (-NaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isfinite(SNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isfinite (SNaN)", !isfinite (SNaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isfinite(-SNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isfinite (-SNaN)", !isfinite (-SNaN_var));	      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnormal(NaN) raised SIGFPE\n");			      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnormal (NaN)", !isnormal (NaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnormal(-NaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnormal (-NaN)", !isnormal (-NaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnormal(SNaN) isnormal SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnormal (SNaN)", !isnormal (SNaN_var));		      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " isnormal(-SNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " isnormal (-SNaN)", !isnormal (-SNaN_var));	      \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " fpclassify(NaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " fpclassify (NaN)", (fpclassify (NaN_var)==FP_NAN));     \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " fpclassify(-NaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " fpclassify (-NaN)", (fpclassify (-NaN_var)==FP_NAN));   \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " fpclassify(SNaN) isnormal SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " fpclassify (SNaN)", (fpclassify (SNaN_var)==FP_NAN));   \
    }									      \
									      \
  feclearexcept(FE_ALL_EXCEPT);						      \
  feenableexcept (FE_ALL_EXCEPT);					      \
  if (sigsetjmp(sigfpe_buf, 0))						      \
    {									      \
      printf (#FLOAT " fpclassify(-SNaN) raised SIGFPE\n");		      \
      errors++;								      \
    } else {								      \
      check (#FLOAT " fpclassify (-SNaN)", (fpclassify (-SNaN_var)==FP_NAN)); \
    }									      \
									      \
  fesetenv(&saved_fenv); /* restore saved fenv */			      \
  remove_sigaction_FP();						      \
}

TEST_FUNC (float_test, float)
TEST_FUNC (double_test, double)
#ifndef NO_LONG_DOUBLE
TEST_FUNC (ldouble_test, ldouble)
#endif

static int
do_test (void)
{
  init_signaling_nan();

  float_test();
  double_test();
#ifndef NO_LONG_DOUBLE
  ldouble_test();
#endif

  return errors != 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
