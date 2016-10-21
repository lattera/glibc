/* Test for the C++ implementation of iszero.
   Copyright (C) 2016 Free Software Foundation, Inc.
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

#define _GNU_SOURCE 1
#include <math.h>
#include <stdio.h>

#include <limits>

static bool errors;

static void
check (int actual, int expected, const char *actual_expr, int line)
{
  if (actual != expected)
    {
      errors = true;
      printf ("%s:%d: error: %s\n", __FILE__, line, actual_expr);
      printf ("%s:%d:   expected: %d\n", __FILE__, line, expected);
      printf ("%s:%d:   actual: %d\n", __FILE__, line, actual);
    }
}

#define CHECK(actual, expected) \
  check ((actual), (expected), #actual, __LINE__)

template <class T>
static void
check_type ()
{
  typedef std::numeric_limits<T> limits;
  CHECK (iszero (T{}), 1);
  CHECK (iszero (T{0}), 1);
  CHECK (iszero (T{-0.0}), 1);
  CHECK (iszero (T{1}), 0);
  CHECK (iszero (T{-1}), 0);
  CHECK (iszero (limits::min ()), 0);
  CHECK (iszero (-limits::min ()), 0);
  CHECK (iszero (limits::max ()), 0);
  CHECK (iszero (-limits::max ()), 0);
  if (limits::has_infinity)
    {
      CHECK (iszero (limits::infinity ()), 0);
      CHECK (iszero (-limits::infinity ()), 0);
    }
  CHECK (iszero (limits::epsilon ()), 0);
  CHECK (iszero (-limits::epsilon ()), 0);
  if (limits::has_quiet_NaN)
    CHECK (iszero (limits::quiet_NaN ()), 0);
  if (limits::has_signaling_NaN)
    CHECK (iszero (limits::signaling_NaN ()), 0);
  if (limits::has_signaling_NaN)
    CHECK (iszero (limits::signaling_NaN ()), 0);
  CHECK (iszero (limits::denorm_min ()),
         std::numeric_limits<T>::has_denorm == std::denorm_absent);
  CHECK (iszero (-limits::denorm_min ()),
         std::numeric_limits<T>::has_denorm == std::denorm_absent);
}

static int
do_test (void)
{
  check_type<float> ();
  check_type<double> ();
#ifndef NO_LONG_DOUBLE
  check_type<long double> ();
#endif
  return errors;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
