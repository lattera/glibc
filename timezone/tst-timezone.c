/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Andreas Jaeger <aj@arthur.rhein-neckar.de>, 1998.

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

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int failed = 0;

struct test_times
{
  const char *name;
  int daylight;
  int timezone;
};

static const struct test_times tests[] =
{
  { "Europe/Berlin", 1, -3600 },
  { "Universal", 0, 0 },
  { "Australia/Melbourne", 1, -36000 },
  { "America/Sao_Paulo", 1, 10800 },
  { NULL, 0, 0 }
};


void
print_tzvars (void)
{
  printf ("tzname[0]: %s\n", tzname[0]);
  printf ("tzname[1]: %s\n", tzname[1]);
  printf ("daylight: %d\n", daylight);
  printf ("timezone: %ld\n", timezone);
}


void
check_tzvars (const char *name, int dayl, int timez)
{
  if (daylight != dayl)
    {
      printf ("Timezone: %s, daylight is: %d but should be: %d\n",
	      name, daylight, dayl);
      ++failed;
    }
  if (timezone != timez)
    {
      printf ("Timezone: %s, timezone is: %ld but should be: %d\n",
	      name, timezone, timez);
      ++failed;
    }
}


int
main (int argc, char ** argv)
{
  time_t t;
  const struct test_times *pt;
  char buf[BUFSIZ];

  /* This should be: Thu May 14 18:02:16 1998.  */
  t = 895194136;
  printf ("We use this date: %s\n", ctime (&t));

  for (pt = tests; pt->name != NULL; ++pt)
    {
      /* Start with a known state */
      printf ("Checking timezone %s\n", pt->name);
      sprintf (buf, "TZ=%s", pt->name);
      if (putenv (buf))
	{
	  puts ("putenv failed.");
	  failed = 1;
	}
      tzset ();
      print_tzvars ();
      check_tzvars (pt->name, pt->daylight, pt->timezone);

      /* calling localtime shouldn't make a difference */
      localtime (&t);
      print_tzvars ();
      check_tzvars (pt->name, pt->daylight, pt->timezone);
    }

  return failed ? EXIT_FAILURE : EXIT_SUCCESS;
}
