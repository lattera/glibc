/* Copyright (C) 2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Tulio Magno Quites Machado Filho <tuliom@linux.vnet.ibm.com>,
   2012.

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

/* Test bugzilla 13691  */

#include <stdio.h>
#include <string.h>
#include <wchar.h>
#include <locale.h>

static int
do_test (void)
{
  const char * in = "A";
  const char *inbuf = in;
  size_t inlen = strchr (in, '\0') - inbuf;

  wchar_t out[5];
  mbstate_t ps;

  const char *locale = "vi_VN.TCVN5712-1";
  if (!setlocale (LC_ALL, locale))
    {
      printf ("Locale not available.\n");
      return 1;
    }

  memset (&ps, '\0', sizeof (ps));
  memset (out, '\0', sizeof (out));

  /* If the bug isn't fixed, it isn't going to return from mbsnrtowcs due to
     an assert().  */
  size_t n = mbsnrtowcs (out, &inbuf, inlen, sizeof(out) - 1, &ps);

  int result = 0;

  if (n != 1)
    {
      printf ("n = %zu, expected 1\n", n);
      result = 1;
    }

  int i;
  printf ("in  = ");
  for (i = 0; i < inlen; i++)
    {
      printf ("0x%X ", in[i]);
    }
  printf ("\n");

  char * outb = (char *) out;
  printf ("out =");
  for (i = 0; i < sizeof (out); i++)
    {
      if (i % 4 == 0)
	{
	  printf (" 0x");
	}
      printf ("%X", outb[i]);
    }
  printf ("\n");

  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
