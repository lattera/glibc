/* Test for UTF-8 regular expression optimizations.
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

#define RE_NO_INTERNAL_PROTOTYPES 1
#include "regex_internal.h"

static struct
{
  int syntax;
  const char *pattern;
  const char *string;
  int res, optimize;
} tests[] = {
  /* \xc3\x84		LATIN CAPITAL LETTER A WITH DIAERESIS
     \xc3\x96		LATIN CAPITAL LETTER O WITH DIAERESIS
     \xc3\xa4		LATIN SMALL LETTER A WITH DIAERESIS
     \xc3\xb6		LATIN SMALL LETTER O WITH DIAERESIS
     \xe2\x80\x94	EM DASH  */
  /* Should be optimized.  */
  {RE_SYNTAX_POSIX_BASIC, "foo", "b\xc3\xa4rfoob\xc3\xa4z", 4, 1},
  {RE_SYNTAX_POSIX_BASIC, "b\xc3\xa4z", "b\xc3\xa4rfoob\xc3\xa4z", 7, 1},
  {RE_SYNTAX_POSIX_BASIC, "b\xc3\xa4*z", "b\xc3\xa4rfoob\xc3\xa4z", 7, 1},
  {RE_SYNTAX_POSIX_BASIC, "b\xc3\xa4*z", "b\xc3\xa4rfoobz", 7, 1},
  {RE_SYNTAX_POSIX_BASIC, "b\xc3\xa4\\+z",
   "b\xc3\xa4rfoob\xc3\xa4\xc3\xa4z", 7, 1},
  {RE_SYNTAX_POSIX_BASIC, "b\xc3\xa4\\?z", "b\xc3\xa4rfoob\xc3\xa4z", 7, 1},
  {RE_SYNTAX_POSIX_BASIC, "b\xc3\xa4\\{1,2\\}z",
   "b\xc3\xa4rfoob\xc3\xa4z", 7, 1},
  {RE_SYNTAX_POSIX_BASIC, "^x\\|xy*z$", "\xc3\xb6xyyz", 2, 1},
  {RE_SYNTAX_POSIX_BASIC, "^x\\\\y\\{6\\}z\\+", "x\\yyyyyyzz\xc3\xb6", 0, 1},
  {RE_SYNTAX_POSIX_BASIC, "^x\\\\y\\{2,36\\}z\\+", "x\\yzz\xc3\xb6", -1, 1},
  {RE_SYNTAX_POSIX_BASIC, "^x\\\\y\\{,3\\}z\\+", "x\\yyyzz\xc3\xb6", 0, 1},
  {RE_SYNTAX_POSIX_BASIC, "^x\\|x\xc3\xa4*z$",
   "\xc3\xb6x\xc3\xa4\xc3\xa4z", 2, 1},
  {RE_SYNTAX_POSIX_BASIC, "^x\\\\\xc3\x84\\{6\\}z\\+",
   "x\\\xc3\x84\xc3\x84\xc3\x84\xc3\x84\xc3\x84\xc3\x84zz\xc3\xb6", 0, 1},
  {RE_SYNTAX_POSIX_BASIC, "^x\\\\\xc3\x84\\{2,36\\}z\\+",
   "x\\\xc3\x84zz\xc3\xb6", -1, 1},
  {RE_SYNTAX_POSIX_BASIC, "^x\\\\\xc3\x84\\{,3\\}z\\+",
   "x\\\xc3\x84\xc3\x84\xc3\x84zz\xc3\xb6", 0, 1},
  {RE_SYNTAX_POSIX_BASIC, "x[C]y", "axCy", 1, 1},
  {RE_SYNTAX_POSIX_BASIC, "x[ABC]y", "axCy", 1, 1},
  {RE_SYNTAX_POSIX_BASIC, "\\`x\\|z\\'", "x\xe2\x80\x94", 0, 1},
  {RE_SYNTAX_POSIX_BASIC, "\\(xy\\)z\\1a\\1", "\xe2\x80\x94xyzxyaxy\xc3\x84", 3, 1},
  {RE_SYNTAX_POSIX_BASIC, "xy\\?z", "\xc3\x84xz\xc3\xb6", 2, 1},
  {RE_SYNTAX_POSIX_BASIC, "\\`\xc3\x84\\|z\\'", "\xc3\x84\xe2\x80\x94", 0, 1},
  {RE_SYNTAX_POSIX_BASIC, "\\(x\xc3\x84\\)z\\1\x61\\1",
   "\xe2\x80\x94x\xc3\x84zx\xc3\x84\x61x\xc3\x84\xc3\x96", 3, 1},
  {RE_SYNTAX_POSIX_BASIC, "x\xc3\x96\\?z", "\xc3\x84xz\xc3\xb6", 2, 1},
  {RE_SYNTAX_POSIX_EXTENDED, "foo", "b\xc3\xa4rfoob\xc3\xa4z", 4, 1},
  {RE_SYNTAX_POSIX_EXTENDED, "^x|xy*z$", "\xc3\xb6xyyz", 2, 1},
  {RE_SYNTAX_POSIX_EXTENDED, "^x\\\\y{6}z+", "x\\yyyyyyzz\xc3\xb6", 0, 1},
  {RE_SYNTAX_POSIX_EXTENDED, "^x\\\\y{2,36}z+", "x\\yzz\xc3\xb6", -1, 1},
  {RE_SYNTAX_POSIX_EXTENDED, "^x\\\\y{,3}z+", "x\\yyyzz\xc3\xb6", 0, 1},
  {RE_SYNTAX_POSIX_EXTENDED, "x[C]y", "axCy", 1, 1},
  {RE_SYNTAX_POSIX_EXTENDED, "x[ABC]y", "axCy", 1, 1},
  {RE_SYNTAX_POSIX_EXTENDED, "\\`x|z\\'", "x\xe2\x80\x94", 0, 1},
  {RE_SYNTAX_POSIX_EXTENDED, "(xy)z\\1a\\1", "\xe2\x80\x94xyzxyaxy\xc3\x84", 3, 1},
  {RE_SYNTAX_POSIX_EXTENDED, "xy?z", "\xc3\x84xz\xc3\xb6", 2, 1},
  /* Should not be optimized.  */
  {RE_SYNTAX_POSIX_BASIC, "x.y", "ax\xe2\x80\x94yz", 1, 0},
  {RE_SYNTAX_POSIX_BASIC, "x[\xc3\x84\xc3\xa4]y", "ax\xc3\xa4y", 1, 0},
  {RE_SYNTAX_POSIX_BASIC, "x[A-Z,]y", "axCy", 1, 0},
  {RE_SYNTAX_POSIX_BASIC, "x[^y]z", "ax\xe2\x80\x94z", 1, 0},
  {RE_SYNTAX_POSIX_BASIC, "x[[:alnum:]]z", "ax\xc3\x96z", 1, 0},
  {RE_SYNTAX_POSIX_BASIC, "x[[=A=]]z", "axAz", 1, 0},
  {RE_SYNTAX_POSIX_BASIC, "x[[=\xc3\x84=]]z", "ax\xc3\x84z", 1, 0},
  {RE_SYNTAX_POSIX_BASIC, "\\<g", "\xe2\x80\x94g", 3, 0},
  {RE_SYNTAX_POSIX_BASIC, "\\bg\\b", "\xe2\x80\x94g", 3, 0},
  {RE_SYNTAX_POSIX_BASIC, "\\Bg\\B", "\xc3\xa4g\xc3\xa4", 2, 0},
  {RE_SYNTAX_POSIX_BASIC, "a\\wz", "a\xc3\x84z", 0, 0},
  {RE_SYNTAX_POSIX_BASIC, "x\\Wz", "\xc3\x96x\xe2\x80\x94z", 2, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "x.y", "ax\xe2\x80\x94yz", 1, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "x[\xc3\x84\xc3\xa4]y", "ax\xc3\xa4y", 1, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "x[A-Z,]y", "axCy", 1, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "x[^y]z", "ax\xe2\x80\x94z", 1, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "x[[:alnum:]]z", "ax\xc3\x96z", 1, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "x[[=A=]]z", "axAz", 1, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "x[[=\xc3\x84=]]z", "ax\xc3\x84z", 1, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "\\<g", "\xe2\x80\x94g", 3, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "\\bg\\b", "\xe2\x80\x94g", 3, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "\\Bg\\B", "\xc3\xa4g\xc3\xa4", 2, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "a\\wz", "a\xc3\x84z", 0, 0},
  {RE_SYNTAX_POSIX_EXTENDED, "x\\Wz", "\xc3\x96x\xe2\x80\x94z", 2, 0},
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
      int res, optimized;

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

      /* Check if re_search will be done as multi-byte or single-byte.  */
      optimized = ((re_dfa_t *) regbuf.buffer)->mb_cur_max == 1;
      if (optimized != tests[i].optimize)
        {
          printf ("pattern %zd %soptimized while it should%s be\n",
		  i, optimized ? "" : "not ", tests[i].optimize ? "" : " not");
	  ret = 1;
        }

      res = re_search (&regbuf, tests[i].string, strlen (tests[i].string), 0,
		       strlen (tests[i].string), NULL);
      if (res != tests[i].res)
	{
	  printf ("re_search %zd failed: %d\n", i, res);
	  ret = 1;
	  regfree (&regbuf);
	  continue;
	}
      regfree (&regbuf);

      re_set_syntax (tests[i].syntax | RE_ICASE);
      memset (&regbuf, '\0', sizeof (regbuf));
      err = re_compile_pattern (tests[i].pattern, strlen (tests[i].pattern),
                                &regbuf);
      if (err != NULL)
	{
	  printf ("re_compile_pattern failed: %s\n", err);
	  ret = 1;
	  continue;
	}

      /* Check if re_search will be done as multi-byte or single-byte.  */
      optimized = ((re_dfa_t *) regbuf.buffer)->mb_cur_max == 1;
      if (optimized)
        {
          printf ("pattern %zd optimized while it should not be when case insensitive\n",
		  i);
	  ret = 1;
        }

      res = re_search (&regbuf, tests[i].string, strlen (tests[i].string), 0,
		       strlen (tests[i].string), NULL);
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
