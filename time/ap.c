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
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <malloc.h>

/* Prints the time in the form "hh:mm ?M", where ? is A or P.
   A simple test for strftime().  */
int
DEFUN(main, (argc, argv), int argc AND char **argv)
{
  char buf[20];
  time_t t;

  mcheck (NULL);

  if (argc != 1)
    fprintf(stderr, "Usage: %s\n", argv[0]);

  t = time((time_t *) NULL);
  if (strftime(buf, sizeof(buf), "%I:%M %p", localtime(&t)) == 0)
    exit(EXIT_FAILURE);

  puts(buf);

  exit(EXIT_SUCCESS);
  return EXIT_SUCCESS;
}
