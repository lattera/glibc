/* Test to strtod etc for numbers like x000...0000.000e-nn.
   This file is part of the GNU C Library.
   Copyright (C) 2001-2015 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <string.h>


int
main (void)
{
  char buf[300];
  int cnt;
  int result = 0;

  for (cnt = 0; cnt < 200; ++cnt)
    {
      ssize_t n;
      float f;

      n = sprintf (buf, "%d", cnt);
      memset (buf + n, '0', cnt);
      sprintf (buf + n + cnt, ".000e-%d", cnt);
      f = strtof (buf, NULL);

      if (f != (float) cnt)
	{
	  printf ("strtof(\"%s\") failed for cnt == %d (%g instead of %g)\n",
		  buf, cnt, f, (float) cnt);
	  result = 1;
	}
      else
	printf ("strtof() fine for cnt == %d\n", cnt);
    }

  for (cnt = 0; cnt < 200; ++cnt)
    {
      ssize_t n;
      double f;

      n = sprintf (buf, "%d", cnt);
      memset (buf + n, '0', cnt);
      sprintf (buf + n + cnt, ".000e-%d", cnt);
      f = strtod (buf, NULL);

      if (f != (double) cnt)
	{
	  printf ("strtod(\"%s\") failed for cnt == %d (%g instead of %g)\n",
		  buf, cnt, f, (double) cnt);
	  result = 1;
	}
      else
	printf ("strtod() fine for cnt == %d\n", cnt);
    }

  for (cnt = 0; cnt < 200; ++cnt)
    {
      ssize_t n;
      long double f;

      n = sprintf (buf, "%d", cnt);
      memset (buf + n, '0', cnt);
      sprintf (buf + n + cnt, ".000e-%d", cnt);
      f = strtold (buf, NULL);

      if (f != (long double) cnt)
	{
	  printf ("strtold(\"%s\") failed for cnt == %d (%Lg instead of %Lg)\n",
		  buf, cnt, f, (long double) cnt);
	  result = 1;
	}
      else
	printf ("strtold() fine for cnt == %d\n", cnt);
    }

  return result;
}
