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
#include <string.h>
#include <stdlib.h>
#include <time.h>

char *
DEFUN(strfry, (string), char *string)
{
  static int init = 0;
  size_t len, i;

  if (!init)
    {
      srand (time ((time_t *) NULL));
      init = 1;
    }

  len = strlen (string);
  for (i = 0; i < len; ++i)
    {
      size_t j = rand () % len;
      char c = string[i];
      string[i] = string[j];
      string[j] = c;
    }

  return string;
}
