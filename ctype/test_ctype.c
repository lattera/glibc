/* Copyright (C) 1991, 1994 Free Software Foundation, Inc.
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
#include <limits.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define ISLOWER(c) ('a' <= (c) && (c) <= 'z')
#define TOUPPER(c) (ISLOWER(c) ? 'A' + ((c) - 'a') : (c))
#define XOR(e,f) (((e) && !(f)) || (!(e) && (f)))

#ifdef	__GNUC__
__inline
#endif
static void
DEFUN(print_char, (c), unsigned char c)
{
  printf("%d/", (int) c);
  if (isgraph(c))
    printf("'%c'", c);
  else
    printf("'\\%.3o'", c);
}

int
DEFUN(main, (argc, argv), int argc AND char **argv)
{
  register unsigned short int c;
  int lose = 0;

  for (c = 0; c <= UCHAR_MAX; ++c)
    {
      print_char (c);

      if (XOR (islower (c), ISLOWER (c)) || toupper (c) != TOUPPER (c))
	{
	  fputs (" BOGUS", stdout);
	  ++lose;
	}

      if (isascii(c))
	fputs(" isascii", stdout);
      if (isalnum(c))
	fputs(" isalnum", stdout);
      if (isalpha(c))
	fputs(" isalpha", stdout);
      if (iscntrl(c))
	fputs(" iscntrl", stdout);
      if (isdigit(c))
	fputs(" isdigit", stdout);
      if (isgraph(c))
	fputs(" isgraph", stdout);
      if (islower(c))
	fputs(" islower", stdout);
      if (isprint(c))
	fputs(" isprint", stdout);
      if (ispunct(c))
	fputs(" ispunct", stdout);
      if (isspace(c))
	fputs(" isspace", stdout);
      if (isupper(c))
	fputs(" isupper", stdout);
      if (isxdigit(c))
	fputs(" isxdigit", stdout);
      if (isblank(c))
	fputs(" isblank", stdout);
      fputs("; lower = ", stdout);
      print_char(tolower(c));
      fputs("; upper = ", stdout);
      print_char(toupper(c));
      putchar('\n');
    }

  exit (lose ? EXIT_FAILURE : EXIT_SUCCESS);
}
