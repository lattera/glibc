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
#include <errno.h>
#include <stdio.h>
#include <string.h>


/* Write the string S to STREAM.  */
int
DEFUN(fputs, (s, stream), CONST char *s AND FILE *stream)
{
  register CONST size_t len = strlen (s);
  if (len == 1)
    return putc (*s, stream) == EOF ? EOF : 0;
  if (fwrite ((PTR) s, 1, len, stream) != len)
    return EOF;
  return 0;
}
