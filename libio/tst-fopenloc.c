/* Test for ,ccs= handling in fopen.
   Copyright (C) 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 2001.

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

#include <errno.h>
#include <locale.h>
#include <mcheck.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wchar.h>


static const char inputfile[] = "../iconvdata/testdata/ISO-8859-1";


int
main (void)
{
  FILE *fp;

  mtrace ();

  setlocale (LC_ALL, "de_DE.UTF-8");

  fp = fopen (inputfile, "r,ccs=ISO-8859-1");
  if (fp == NULL)
    {
      printf ("cannot open \"%s\": %s\n", inputfile, strerror (errno));
      exit (1);
    }

  while (! feof_unlocked (fp))
    {
      wchar_t buf[200];

      if (fgetws_unlocked (buf, sizeof (buf) / sizeof (buf[0]), fp) == NULL)
	break;

      fputws (buf, stdout);
    }

  fclose (fp);

  return 0;
}
