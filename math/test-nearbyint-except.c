/* Test nearbyint functions do not clear exceptions (bug 15491).
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

#include <fenv.h>
#include <math.h>
#include <stdio.h>

#include <math-tests.h>

#ifndef FE_INVALID
# define FE_INVALID 0
#endif

#define TEST_FUNC(NAME, FLOAT, SUFFIX)					\
static int								\
NAME (void)								\
{									\
  int result = 0;							\
  volatile FLOAT a, b __attribute__ ((unused));				\
  a = 1.0;								\
  /* nearbyint must not clear already-raised exceptions.  */		\
  feraiseexcept (FE_ALL_EXCEPT);					\
  b = nearbyint ## SUFFIX (a);						\
  if (fetestexcept (FE_ALL_EXCEPT) == FE_ALL_EXCEPT)			\
    puts ("PASS: " #FLOAT);						\
  else									\
    {									\
      puts ("FAIL: " #FLOAT);						\
      result = 1;							\
    }									\
  /* But it mustn't lose exceptions from sNaN arguments.  */		\
  if (SNAN_TESTS (FLOAT) && EXCEPTION_TESTS (FLOAT))			\
    {									\
      static volatile FLOAT snan = __builtin_nans ## SUFFIX ("");	\
      volatile FLOAT c __attribute__ ((unused));			\
      feclearexcept (FE_ALL_EXCEPT);					\
      c = nearbyint ## SUFFIX (snan);					\
      if (fetestexcept (FE_INVALID) == FE_INVALID)			\
	puts ("PASS: " #FLOAT " sNaN");					\
      else								\
	{								\
	  puts ("FAIL: " #FLOAT " sNaN");				\
	  result = 1;							\
	}								\
    }									\
  return result;							\
}

TEST_FUNC (float_test, float, f)
TEST_FUNC (double_test, double, )
#ifndef NO_LONG_DOUBLE
TEST_FUNC (ldouble_test, long double, l)
#endif

static int
do_test (void)
{
  int result = float_test ();
  result |= double_test ();
#ifndef NO_LONG_DOUBLE
  result |= ldouble_test ();
#endif
  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
