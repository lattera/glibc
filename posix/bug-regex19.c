/* Regular expression tests.
   Copyright (C) 2003 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2003.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sys/types.h>
#include <mcheck.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

static struct
{
  int syntax;
  const char *pattern;
  const char *string;
  int start, res;
} tests[] = {
  /* \xc3\x84		LATIN CAPITAL LETTER A WITH DIAERESIS
     \xc3\x96		LATIN CAPITAL LETTER O WITH DIAERESIS
     \xe2\x80\x94	EM DASH  */
  /* Should not match.  */
  {RE_SYNTAX_POSIX_BASIC, "\\<A", "aOAA", 0, -1},
  {RE_SYNTAX_POSIX_BASIC, "\\<A", "aOAA", 2, -1},
  {RE_SYNTAX_POSIX_BASIC, "A\\>", "aAAO", 1, -1},
  {RE_SYNTAX_POSIX_BASIC, "\\bA", "aOAA", 0, -1},
  {RE_SYNTAX_POSIX_BASIC, "\\bA", "aOAA", 2, -1},
  {RE_SYNTAX_POSIX_BASIC, "A\\b", "aAAO", 1, -1},
  {RE_SYNTAX_POSIX_BASIC, "\\<\xc3\x84", "a\xc3\x96\xc3\x84\xc3\x84", 0, -1},
  {RE_SYNTAX_POSIX_BASIC, "\\<\xc3\x84", "a\xc3\x96\xc3\x84\xc3\x84", 3, -1},
  {RE_SYNTAX_POSIX_BASIC, "\xc3\x84\\>", "a\xc3\x84\xc3\x84\xc3\x96", 1, -1},
#if 0
  /* XXX these 2 tests still fail.  */
  {RE_SYNTAX_POSIX_BASIC, "\\b\xc3\x84", "a\xc3\x96\xc3\x84\xc3\x84", 0, -1},
  {RE_SYNTAX_POSIX_BASIC, "\\b\xc3\x84", "a\xc3\x96\xc3\x84\xc3\x84", 3, -1},
#endif
  {RE_SYNTAX_POSIX_BASIC, "\xc3\x84\\b", "a\xc3\x84\xc3\x84\xc3\x96", 1, -1},
  /* Should match.  */
  {RE_SYNTAX_POSIX_BASIC, "\\<A", "AA", 0, 0},
  {RE_SYNTAX_POSIX_BASIC, "\\<A", "a-AA", 2, 2},
  {RE_SYNTAX_POSIX_BASIC, "A\\>", "aAA-", 1, 2},
  {RE_SYNTAX_POSIX_BASIC, "A\\>", "aAA", 1, 2},
  {RE_SYNTAX_POSIX_BASIC, "\\bA", "AA", 0, 0},
  {RE_SYNTAX_POSIX_BASIC, "\\bA", "a-AA", 2, 2},
  {RE_SYNTAX_POSIX_BASIC, "A\\b", "aAA-", 1, 2},
  {RE_SYNTAX_POSIX_BASIC, "A\\b", "aAA", 1, 2},
  {RE_SYNTAX_POSIX_BASIC, "\\<\xc3\x84", "\xc3\x84\xc3\x84", 0, 0},
  {RE_SYNTAX_POSIX_BASIC, "\\<\xc3\x84", "a\xe2\x80\x94\xc3\x84\xc3\x84", 4, 4},
  {RE_SYNTAX_POSIX_BASIC, "\xc3\x84\\>", "a\xc3\x84\xc3\x84\xe2\x80\x94", 1, 3},
  {RE_SYNTAX_POSIX_BASIC, "\xc3\x84\\>", "a\xc3\x84\xc3\x84", 1, 3},
  {RE_SYNTAX_POSIX_BASIC, "\\b\xc3\x84", "\xc3\x84\xc3\x84", 0, 0},
  {RE_SYNTAX_POSIX_BASIC, "\\b\xc3\x84", "a\xe2\x80\x94\xc3\x84\xc3\x84", 4, 4},
  {RE_SYNTAX_POSIX_BASIC, "\xc3\x84\\b", "a\xc3\x84\xc3\x84\xe2\x80\x94", 1, 3},
  {RE_SYNTAX_POSIX_BASIC, "\xc3\x84\\b", "a\xc3\x84\xc3\x84", 1, 3}
};

int
main (void)
{
  struct re_pattern_buffer regbuf;
  const char *err;
  size_t i;
  int ret = 0;

  mtrace ();

  setlocale (LC_ALL, "de_DE.UTF-8");
  for (i = 0; i < sizeof (tests) / sizeof (tests[0]); ++i)
    {
      int res;
      re_set_syntax (tests[i].syntax);
      memset (&regbuf, '\0', sizeof (regbuf));
      err = re_compile_pattern (tests[i].pattern, strlen (tests[i].pattern),
                                &regbuf);
      if (err != NULL)
	{
	  printf ("re_compile_pattern failed: %s\n", err);
	  ret = 1;
	  continue;
	}

      res = re_search (&regbuf, tests[i].string, strlen (tests[i].string),
		       tests[i].start,
		       strlen (tests[i].string) - tests[i].start, NULL);
      if (res != tests[i].res)
	{
	  printf ("re_search %zd failed: %d\n", i, res);
	  ret = 1;
	  regfree (&regbuf);
	  continue;
	}
      regfree (&regbuf);
    }

  return ret;
}
