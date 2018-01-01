/* Verify that ftime is sane.
   Copyright (C) 2014-2018 Free Software Foundation, Inc.
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

#include <sys/timeb.h>
#include <stdio.h>

static int
do_test (void)
{
  struct timeb prev, curr = {.time = 0, .millitm = 0};
  int sec = 0;

  while (sec != 3)
    {
      prev = curr;

      if (ftime (&curr))
        {
          printf ("ftime returned an error\n");
          return 1;
        }

      if (curr.time < prev.time)
        {
          printf ("ftime's time flowed backwards\n");
          return 1;
        }

      if (curr.time == prev.time
          && curr.millitm < prev.millitm)
        {
          printf ("ftime's millitm flowed backwards\n");
          return 1;
        }

      if (curr.time > prev.time)
        sec ++;
    }
  return 0;
}

#define TIMEOUT 3
#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
