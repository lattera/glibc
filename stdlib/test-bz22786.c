/* Bug 22786: test for buffer overflow in realpath.
   Copyright (C) 2018 Free Software Foundation, Inc.
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

/* This file must be run from within a directory called "stdlib".  */

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <support/test-driver.h>
#include <libc-diag.h>

static int
do_test (void)
{
  const char dir[] = "bz22786";
  const char lnk[] = "bz22786/symlink";

  rmdir (dir);
  if (mkdir (dir, 0755) != 0 && errno != EEXIST)
    {
      printf ("mkdir %s: %m\n", dir);
      return EXIT_FAILURE;
    }
  if (symlink (".", lnk) != 0 && errno != EEXIST)
    {
      printf ("symlink (%s, %s): %m\n", dir, lnk);
      return EXIT_FAILURE;
    }

  const size_t path_len = (size_t) INT_MAX + 1;

  DIAG_PUSH_NEEDS_COMMENT;
#if __GNUC_PREREQ (7, 0)
  /* GCC 7 warns about too-large allocations; here we need such
     allocation to succeed for the test to work.  */
  DIAG_IGNORE_NEEDS_COMMENT (7, "-Walloc-size-larger-than=");
#endif
  char *path = malloc (path_len);
  DIAG_POP_NEEDS_COMMENT;

  if (path == NULL)
    {
      printf ("malloc (%zu): %m\n", path_len);
      return EXIT_UNSUPPORTED;
    }

  /* Construct very long path = "bz22786/symlink/aaaa....."  */
  char *p = mempcpy (path, lnk, sizeof (lnk) - 1);
  *(p++) = '/';
  memset (p, 'a', path_len - (path - p) - 2);
  p[path_len - (path - p) - 1] = '\0';

  /* This call crashes before the fix for bz22786 on 32-bit platforms.  */
  p = realpath (path, NULL);

  if (p != NULL || errno != ENAMETOOLONG)
    {
      printf ("realpath: %s (%m)", p);
      return EXIT_FAILURE;
    }

  /* Cleanup.  */
  unlink (lnk);
  rmdir (dir);

  return 0;
}

#define TEST_FUNCTION do_test
#include <support/test-driver.c>
