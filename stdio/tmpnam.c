/* Copyright (C) 1991, 1993 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <stdio.h>
#include <string.h>


/* Generate a unique filename in P_tmpdir.  */
char *
DEFUN(tmpnam, (s), register char *s)
{
  register char *t = __stdio_gen_tempname((CONST char *) NULL,
					  (CONST char *) NULL, 0,
					  (size_t *) NULL, (FILE **) NULL);

  if (t == NULL)
    return NULL;

  if (s != NULL)
    (void) strcpy(s, t);
  else
    s = t;

  return s;
}
