/* Copyright (C) 1992 Free Software Foundation, Inc.
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

int
DEFUN_VOID(main)
{
  int i1, i2;
  int j1, j2;

  /* The C standard says that "If rand is called before any calls to
     srand have been made, the same sequence shall be generated as
     when srand is first called with a seed value of 1." */
  i1 = rand();
  i2 = rand();
  srand (1);
  j1 = rand();
  j2 = rand();
  if (j1 == i1 && j2 == i2)
    {
      puts ("Test succeeded.");
      return 0;
    }
  else
    {
      if (j1 != i1)
	printf ("%d != %d\n", j1, i1);
      if (j2 != i2)
	printf ("%d != %d\n", j2, i2);
      puts ("Test FAILED!");
      return 1;
    }
}
