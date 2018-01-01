/* Test isinf.
   Copyright (C) 1997-2018 Free Software Foundation, Inc.
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

#include "libm-test-driver.c"

static const struct test_f_i_data isinf_test_data[] =
  {
    TEST_f_b (isinf, 0, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, minus_zero, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, 10, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, min_subnorm_value, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, -min_subnorm_value, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, min_value, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, -min_value, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, max_value, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, -max_value, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, plus_infty, 1, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, minus_infty, 1, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, qnan_value, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, -qnan_value, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, snan_value, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
    TEST_f_b (isinf, -snan_value, 0, NO_INEXACT_EXCEPTION|ERRNO_UNCHANGED),
  };

static void
isinf_test (void)
{
  ALL_RM_TEST (isinf, 1, isinf_test_data, RUN_TEST_LOOP_f_b_tg, END);
}

static void
do_test (void)
{
  isinf_test ();
}

/*
 * Local Variables:
 * mode:c
 * End:
 */
