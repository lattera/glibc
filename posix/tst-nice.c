/* Copyright (C) 2003 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <stdio.h>
#include <unistd.h>


/* Test that nice() does not incorrectly return 0.  */
static int
do_test (void)
{
  int ret, expected;
  const int incr = 10;

  /* Discover current nice value.  */
  errno = 0;
  ret = nice (0);
  if (ret == -1 && errno != 0)
    {
      printf ("break: nice(%d) return: %d, errno: %d\n", 0, ret, errno);
      return 1;
    }
  expected = ret + incr;

  /* Nice ourselves up.  */
  errno = 0;
  ret = nice (incr);
  if (ret == -1 && errno != 0)
    {
      printf ("break: nice(%d) return: %d, errno: %d\n", incr, ret, errno);
      return 1;
    }

  /* Check for return value being zero when it shouldn't.  Cannot simply
     check for expected value since nice values are capped at 2^n-1.  */
  if (ret == 0 && ret != expected)
    {
      printf ("fail: retval (%d) of nice(%d) != %d\n", ret, incr, expected);
      return 1;
    }

  printf ("pass: nice(%d) return: %d\n", incr, ret);

  return 0;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
