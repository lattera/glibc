/* Test strtod functions work with all ASCII letters in NAN(...) in
   Turkish locales (bug 19266).
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <locale.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <wchar.h>

#define STR_(X) #X
#define STR(X) STR_(X)
#define FNPFXS STR (FNPFX)
#define CONCAT_(X, Y) X ## Y
#define CONCAT(X, Y) CONCAT_ (X, Y)
#define FNX(FN) CONCAT (FNPFX, FN)

#define TEST(LOC, STR, FN, TYPE)					\
  do									\
    {									\
      CHAR *ep;								\
      TYPE val = FNX (FN) (STR, &ep);					\
      if (isnan (val) && *ep == 0)					\
	printf ("PASS: %s: " FNPFXS #FN " (" SFMT ")\n", LOC, STR);	\
      else								\
	{								\
	  printf ("FAIL: %s: " FNPFXS #FN " (" SFMT ")\n", LOC, STR);	\
	  result = 1;							\
	}								\
    }									\
  while (0)

static int
test_one_locale (const char *loc)
{
  if (setlocale (LC_ALL, loc) == NULL)
    {
      printf ("setlocale (LC_ALL, \"%s\") failed\n", loc);
      return 1;
    }
  int result = 0;
  for (int i = 10; i < 36; i++)
    {
      CHAR s[7];
      s[0] = L_('N');
      s[1] = L_('A');
      s[2] = L_('N');
      s[3] = L_('(');
      s[4] = L_('A') + i - 10;
      s[5] = L_(')');
      s[6] = 0;
      TEST (loc, s, f, float);
      TEST (loc, s, d, double);
      TEST (loc, s, ld, long double);
      s[4] = L_('a') + i - 10;
      TEST (loc, s, f, float);
      TEST (loc, s, d, double);
      TEST (loc, s, ld, long double);
    }
  return result;
}

static int
do_test (void)
{
  int result = 0;
  result |= test_one_locale ("C");
  result |= test_one_locale ("tr_TR.UTF-8");
  result |= test_one_locale ("tr_TR.ISO-8859-9");
  return result;
}

#define TEST_FUNCTION do_test ()
#include "../test-skeleton.c"
