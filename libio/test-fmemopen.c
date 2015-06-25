/* Test for fmemopen implementation.
   Copyright (C) 2000-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Hanno Mueller, kontakt@hanno.de, 2000.

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

static char buffer[] = "foobar";

#include <stdio.h>
#include <string.h>
#include <errno.h>

static int
do_test (void)
{
  int ch;
  FILE *stream;
  int ret = 0;

  stream = fmemopen (buffer, strlen (buffer), "r+");

  while ((ch = fgetc (stream)) != EOF)
    printf ("Got %c\n", ch);

  fputc ('1', stream);
  if (fflush (stream) != EOF || errno != ENOSPC)
    {
      printf ("fflush didn't fail with ENOSPC\n");
      ret = 1;
    }

  fclose (stream);

  return ret;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
