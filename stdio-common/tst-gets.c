/* Tests for gets.
   Copyright (C) 2001, 2011, 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2001.

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
#include <string.h>


int
main (void)
{
  char buf[100];
  int result = 0;

  if (gets (buf) != buf)
    {
      printf ("gets: read error: %m\n");
      result = 1;
    }
  else if (strchr (buf, '\n') != NULL)
    {
      printf ("newline not stripped: \"%s\"\n", buf);
      result = 1;
    }
  else if (strcmp (buf, "foo") != 0)
    {
      printf ("read mismatch: expected \"%s\", got \"%s\"\n", "foo", buf);
      result = 1;
    }

  if (gets (buf) != buf)
    {
      printf ("gets: read error: %m\n");
      result = 1;
    }
  else if (strchr (buf, '\n') != NULL)
    {
      printf ("newline not stripped: \"%s\"\n", buf);
      result = 1;
    }
  else if (strcmp (buf, "bar") != 0)
    {
      printf ("read mismatch: expected \"%s\", got \"%s\"\n", "bar", buf);
      result = 1;
    }

  return result;
}
