/* Copyright (C) 1991 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

struct ltest
  {
    CONST char *str;		/* Convert this.  */
    double expect;		/* To get this.  */
    char left;			/* With this left over.  */
    int err;			/* And this in errno.  */
  };
static CONST struct ltest tests[] =
  {
    { "12.345", 12.345, '\0', 0 },
    { "12.345e19", 12.345e19, '\0', 0 },
    { "-.1e+9", -.1e+9, '\0', 0 },
    { ".125", .125, '\0', 0 },
    { "1e20", 1e20, '\0', 0 },
    { NULL, 0, '\0', 0 }
  };

static void EXFUN(expand, (char *dst, int c));

int
DEFUN_VOID(main)
{
  register CONST struct ltest *lt;
  char *ep;
  int status = 0;

  for (lt = tests; lt->str != NULL; ++lt)
    {
      double d;

      errno = 0;
      d = strtod(lt->str, &ep);
      printf("strtod(\"%s\") test %u",
	     lt->str, (unsigned int) (lt - tests));
      if (d == lt->expect && *ep == lt->left && errno == lt->err)
	puts("\tOK");
      else
	{
	  puts("\tBAD");
	  if (d != lt->expect)
	    printf("  returns %.60g, expected %.60g\n", d, lt->expect);
	  if (lt->left != *ep)
	    {
	      char exp1[5], exp2[5];
	      expand(exp1, *ep);
	      expand(exp2, lt->left);
	      printf("  leaves '%s', expected '%s'\n", exp1, exp2);
	    }
	  if (errno != lt->err)
	    printf("  errno %d (%s)  instead of %d (%s)\n",
		   errno, strerror(errno), lt->err, strerror(lt->err));
	  status = 1;
	}
    }

  exit(status ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void
DEFUN(expand, (dst, c), register char *dst AND register int c)
{
  if (isprint(c))
    {
      dst[0] = c;
      dst[1] = '\0';
    }
  else
    (void) sprintf(dst, "%#.3o", (unsigned int) c);
}
