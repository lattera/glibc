/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
#include <setjmp.h>
#include <stdlib.h>

static jmp_buf env;
static int last_value = -1, lose = 0;

void
DEFUN(jump, (val), int val)
{
  longjmp(env, val);
}

int
DEFUN_VOID(main)
{
  int value;

  value = setjmp(env);
  if (value != last_value + 1)
    {
      fputs("Shouldn't have ", stdout);
      lose = 1;
    }
  last_value = value;
  switch (value)
    {
    case 0:
      puts("Saved environment.");
      jump(0);
    default:
      printf("Jumped to %d.\n", value);
      if (value < 10)
	jump(value + 1);
    }

  if (lose || value != 10)
    puts("Test FAILED!");
  else
    puts("Test succeeded!");
  exit(lose ? EXIT_FAILURE : EXIT_SUCCESS);
}
