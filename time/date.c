/* Copyright (C) 1991, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>


/* Prints the date in the form "Day Mon dd hh:mm:ss ZZZ yyyy\n".
   A simple test for localtime and strftime.  */
int
main (argc, argv)
     int argc;
     char **argv;
{
  time_t t = time (NULL);
  struct tm *tp = localtime (&t);
  char good = tp != NULL;

  if (good)
    {
      char buf[BUFSIZ];
      good = strftime (buf, sizeof (buf), "%a %b %d %X %Z %Y", tp);
      if (good)
	puts (buf);
      else
	perror ("strftime");
    }
  else
    perror ("localtime");

  return good ? EXIT_SUCCESS : EXIT_FAILURE;
}
