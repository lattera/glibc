/* Test freopen with mmap stdio.
   Copyright (C) 2002-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static int
do_test (void)
{
  char name[] = "/tmp/tst-freopen.XXXXXX";
  const char * const test = "Let's test freopen.\n";
  char temp[strlen (test) + 1];
  int fd = mkstemp (name);
  FILE *f;

  if (fd == -1)
    {
      printf ("%u: cannot open temporary file: %m\n", __LINE__);
      exit (1);
    }

  f = fdopen (fd, "w");
  if (f == NULL)
    {
      printf ("%u: cannot fdopen temporary file: %m\n", __LINE__);
      exit (1);
    }

  fputs (test, f);
  fclose (f);

  f = fopen (name, "r");
  if (f == NULL)
    {
      printf ("%u: cannot fopen temporary file: %m\n", __LINE__);
      exit (1);
    }

  if (fread (temp, 1, strlen (test), f) != strlen (test))
    {
      printf ("%u: couldn't read the file back: %m\n", __LINE__);
      exit (1);
    }
  temp [strlen (test)] = '\0';

  if (strcmp (test, temp))
    {
      printf ("%u: read different string than was written:\n%s%s",
	      __LINE__, test, temp);
      exit (1);
    }

  f = freopen (name, "r+", f);
  if (f == NULL)
    {
      printf ("%u: cannot freopen temporary file: %m\n", __LINE__);
      exit (1);
    }

  if (fseek (f, 0, SEEK_SET) != 0)
    {
      printf ("%u: couldn't fseek to start: %m\n", __LINE__);
      exit (1);
    }

  if (fread (temp, 1, strlen (test), f) != strlen (test))
    {
      printf ("%u: couldn't read the file back: %m\n", __LINE__);
      exit (1);
    }
  temp [strlen (test)] = '\0';

  if (strcmp (test, temp))
    {
      printf ("%u: read different string than was written:\n%s%s",
	      __LINE__, test, temp);
      exit (1);
    }

  fclose (f);

  unlink (name);
  exit (0);
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
