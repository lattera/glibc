/* Copyright (C) 1994 Free Software Foundation, Inc.
   Contributed by Joel Sherrill (jsherril@redstone-emh2.army.mil),
     On-Line Applications Research Corporation.

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
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int
DEFUN(main, (argc, argv),
      int argc AND char **argv)
{
  int failures = 0;
  int i;
  void try (int value, int expected)
    {
      if (ffs (value) != expected)
	{
	  fprintf (stderr, "%#x expected %d got %d\n",
		   value, expected, ffs (value));
	  ++failures;
	}
    }

  try (0, 0);
  for (i=0 ; i<32 ; i++)
    try (1<<i, i+1);
  try (0x80008000, 16);

  if (failures)
    printf ("Test FAILED!  %d failure%s.\n", failures, &"s"[failures == 1]);
  else
    puts ("Test succeeded.");

  exit (failures);
}
