/* Test lgamma functions set signgam for -ffinite-math-only (bug 19211).
   Copyright (C) 2015-2018 Free Software Foundation, Inc.
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

#include <math.h>
#include <stdio.h>

#define RUN_TESTS(FUNC, TYPE)					\
  do								\
    {								\
      volatile TYPE a, b, c __attribute__ ((unused));		\
      a = 0.5;							\
      b = -0.5;							\
      signgam = 123;						\
      c = FUNC (a);						\
      if (signgam == 1)						\
	puts ("PASS: " #FUNC " (0.5) setting signgam");		\
      else							\
	{							\
	  puts ("FAIL: " #FUNC " (0.5) setting signgam");	\
	  result = 1;						\
	}							\
      signgam = 123;						\
      c = FUNC (b);						\
      if (signgam == -1)					\
	puts ("PASS: " #FUNC " (-0.5) setting signgam");	\
      else							\
	{							\
	  puts ("FAIL: " #FUNC " (-0.5) setting signgam");	\
	  result = 1;						\
	}							\
    }								\
  while (0)

static int
do_test (void)
{
  int result = 0;
  RUN_TESTS (lgammaf, float);
  RUN_TESTS (gammaf, float);
  RUN_TESTS (lgamma, double);
  RUN_TESTS (gamma, double);
  RUN_TESTS (lgammal, long double);
  RUN_TESTS (gammal, long double);
  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
