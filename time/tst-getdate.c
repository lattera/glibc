/* Test for getdate.
   Copyright (C) 2000, 2001, 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@suse.de>, 2000.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static const struct
{
  const char *str;
  const char *tz;
  int err;
  struct tm tm;
} tests [] =
{
  {"21:01:10 1999-1-31", "Universal", 0, {10, 1, 21, 31, 0, 99, 0, 0, 0}},
  {"21:01:10 1999-2-28", "Universal", 0, {10, 1, 21, 28, 1, 99, 0, 0, 0}},
  {"16:30:46 2000-2-29", "Universal", 0, {46, 30,16, 29, 1, 100, 0, 0, 0}},
  {"01-08-2000 05:06:07", "Europe/Berlin", 0, {7, 6, 5, 1, 7, 100, 0, 0, 0}}
};

static void
report_date_error (int err)
{
  switch(err)
    {
    case 1:
      printf ("The environment variable DATEMSK is not defined or null.\n");
      break;
    case 2:
      printf ("The template file denoted by the DATEMSK environment variable cannot be opened.\n");
      break;
    case 3:
      printf ("Information about the template file cannot retrieved.\n");
      break;
    case 4:
      printf ("The template file is not a regular file.\n");
      break;
    case 5:
      printf ("An I/O error occurred while reading the template file.\n");
      break;
    case 6:
      printf ("Not enough memory available to execute the function.\n");
      break;
    case 7:
      printf ("The template file contains no matching template.\n");
      break;
    case 8:
      printf ("The input date is invalid, but would match a template otherwise.\n");
      break;
    default:
      printf("Unknown error code.\n");
      break;
    }
}


int
main (void)
{
  int errors = 0;
  size_t i;
  struct tm *tm;


  for (i = 0; i < sizeof (tests) / sizeof (tests[0]); ++i)
    {
      setenv ("TZ", tests[i].tz, 1);

      tm = getdate (tests[i].str);

      if (getdate_err != tests[i].err)
	{
	  printf ("Failure for getdate (\"%s\"):\n", tests[i].str);
	  printf ("getdate_err should be %d but returned: %d which means:\n",
		  tests[i].err, getdate_err);
	  report_date_error (getdate_err);
	  ++errors;
	}
      else if (tests[i].tm.tm_mon != tm->tm_mon
	       || tests[i].tm.tm_year != tm->tm_year
	       || tests[i].tm.tm_mday != tm->tm_mday
	       || tests[i].tm.tm_hour != tm->tm_hour
	       || tests[i].tm.tm_min != tm->tm_min
	       || tests[i].tm.tm_sec != tm->tm_sec)
	{
	  printf ("Failure for getdate (\"%s\"):\n", tests[i].str);
	  printf ("struct tm is:  %d-%d-%d %d:%d:%d\n",
		  tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
		  tm->tm_hour, tm->tm_min, tm->tm_sec);
	  printf ("but should be: %d-%d-%d %d:%d:%d\n",
		  tests[i].tm.tm_year + 1900, tests[i].tm.tm_mon + 1,
		  tests[i].tm.tm_mday,
		  tests[i].tm.tm_hour, tests[i].tm.tm_min, tests[i].tm.tm_sec);
	  ++errors;
	}
    }

  if (!errors)
    printf ("No errors found.\n");
  return errors != 0;
}
