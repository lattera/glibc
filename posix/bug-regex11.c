/* Test for newline handling in regex.
   Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2002.

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

int
main (void)
{
  regex_t re;
  regmatch_t rm[2];
  int n;

  mtrace ();

  n = regcomp (&re, "[^~]*~", 0);
  if (n != 0)
    {
      char buf[500];
      regerror (n, &re, buf, sizeof (buf));
      printf ("regcomp failed: %s\n", buf);
      exit (1);
    }

  if (regexec (&re, "\nx~y", 2, rm, 0))
    {
      puts ("regexec failed");
      exit (2);
    }
  if (rm[0].rm_so != 0 || rm[0].rm_eo != 3)
    {
      printf ("regexec match failure: %d %d\n",
	      rm[0].rm_so, rm[0].rm_eo);
      exit (3);
    }

  regfree (&re);

  return 0;
}
