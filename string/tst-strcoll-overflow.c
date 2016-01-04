/* Copyright (C) 2013-2016 Free Software Foundation, Inc.
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

#include <locale.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

/* Verify that strcoll does not crash for large strings for which it cannot
   cache weight lookup results.  The size is large enough to cause integer
   overflows on 32-bit as well as buffer overflows on 64-bit.  The test should
   work reasonably reliably when overcommit is disabled, but it obviously
   depends on how much memory the system has.  There's a limitation to this
   test in that it does not run to completion.  Actually collating such a
   large string can take days and we can't have xcheck running that long.  For
   that reason, we run the test for about 5 minutes and then assume that
   everything is fine if there are no crashes.  */
#define SIZE 0x40000000ul

int
do_test (void)
{
  if (setlocale (LC_COLLATE, "en_GB.UTF-8") == NULL)
    {
      puts ("setlocale failed, cannot test for overflow");
      return 0;
    }

  char *p = malloc (SIZE);

  if (p == NULL)
    {
      puts ("could not allocate memory");
      return 1;
    }

  memset (p, 'x', SIZE - 1);
  p[SIZE - 1] = 0;
  printf ("%d\n", strcoll (p, p));
  return 0;
}

#define TIMEOUT 300
#define EXPECTED_SIGNAL SIGALRM
#define EXPECTED_STATUS 0
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
