/* Test for case handling in regex.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Jakub Jelinek <jakub@redhat.com>, 2001.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sys/types.h>
#include <regex.h>
#include <stdio.h>
#include <stdlib.h>


int
main (void)
{
  regex_t re;
  int n;

  n = regcomp (&re, "[a-bA-B]", REG_ICASE);
  if (n != 0)
    {
      char buf[500];
      regerror (n, &re, buf, sizeof (buf));
      printf ("regcomp failed: %s\n", buf);
      exit (1);
    }

  regfree (&re);

  return 0;
}
