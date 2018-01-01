/* Test freopen with mmap stdio.
   Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

#include <support/check.h>
#include <support/temp_file.h>

static int fd;
static char *name;

static void
do_prepare (int argc, char *argv[])
{
  fd = create_temp_file ("tst-freopen.", &name);
  TEST_VERIFY_EXIT (fd != -1);
}

#define PREPARE do_prepare

/* Basic tests for freopen.  */
static void
do_test_basic (void)
{
  const char * const test = "Let's test freopen.\n";
  char temp[strlen (test) + 1];

  FILE *f = fdopen (fd, "w");
  if (f == NULL)
    FAIL_EXIT1 ("fdopen: %m");

  fputs (test, f);
  fclose (f);

  f = fopen (name, "r");
  if (f == NULL)
    FAIL_EXIT1 ("fopen: %m");

  if (fread (temp, 1, strlen (test), f) != strlen (test))
    FAIL_EXIT1 ("fread: %m");
  temp [strlen (test)] = '\0';

  if (strcmp (test, temp))
    FAIL_EXIT1 ("read different string than was written: (%s, %s)",
	        test, temp);

  f = freopen (name, "r+", f);
  if (f == NULL)
    FAIL_EXIT1 ("freopen: %m");

  if (fseek (f, 0, SEEK_SET) != 0)
    FAIL_EXIT1 ("fseek: %m");

  if (fread (temp, 1, strlen (test), f) != strlen (test))
    FAIL_EXIT1 ("fread: %m");
  temp [strlen (test)] = '\0';

  if (strcmp (test, temp))
    FAIL_EXIT1 ("read different string than was written: (%s, %s)",
	        test, temp);

  fclose (f);
}

/* Test for BZ#21398, where it tries to freopen stdio after the close
   of its file descriptor.  */
static void
do_test_bz21398 (void)
{
  (void) close (STDIN_FILENO);

  FILE *f = freopen (name, "r", stdin);
  if (f == NULL)
    FAIL_EXIT1 ("freopen: %m");

  TEST_VERIFY_EXIT (ferror (f) == 0);

  char buf[128];
  char *ret = fgets (buf, sizeof (buf), stdin);
  TEST_VERIFY_EXIT (ret != NULL);
  TEST_VERIFY_EXIT (ferror (f) == 0);
}

static int
do_test (void)
{
  do_test_basic ();
  do_test_bz21398 ();

  return 0;
}

#include <support/test-driver.c>
