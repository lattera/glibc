/* Test for strptime.
   Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <stdio.h>
#include <string.h>
#include <time.h>


static const struct
{
  const char *input;
  const char *format;
  int wday;
  int yday;
} day_tests[] =
{
  { "2000-01-01", "%Y-%m-%d", 6, 0 },
  { "03/03/00", "%D", 5, 62 },
  { "9/9/99", "%x", 4, 251 },
};


int
main (int argc, char *argv[])
{
  struct tm tm;
  int i;
  int result = 0;

  for (i = 0; i < sizeof (day_tests) / sizeof (day_tests[0]); ++i)
    {
      memset (&tm, '\0', sizeof (tm));

      if (*strptime (day_tests[i].input, day_tests[i].format, &tm) != '\0')
	{
	  printf ("not all of `%s' read\n", day_tests[i].input);
	  result = 1;
	}
      if (tm.tm_wday != day_tests[i].wday)
	{
	  printf ("weekday for `%s' incorrect: %d instead of %d\n",
		  day_tests[i].input, tm.tm_wday, day_tests[i].wday);
	  result = 1;
	}
      if (tm.tm_yday != day_tests[i].yday)
	{
	  printf ("yearday for `%s' incorrect: %d instead of %d\n",
		  day_tests[i].input, tm.tm_yday, day_tests[i].yday);
	  result = 1;
	}
    }

  return result;
}
