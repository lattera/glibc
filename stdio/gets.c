/* Copyright (C) 1991, 1994, 1995 Free Software Foundation, Inc.
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
#include <errno.h>
#include <string.h>

link_warning ("the `gets' function is unreliable and should not be used.")

/* Read a newline-terminated string from stdin into S,
   removing the trailing newline.  Return S or NULL.  */
char *
DEFUN(gets, (s), char *s)
{
  register char *p = s;
  register int c;
  FILE *stream = stdin;

  if (!__validfp(stream) || p == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  if (feof(stream) || ferror(stream))
    return NULL;

  while ((c = getchar()) != EOF)
    if (c == '\n')
      break;
    else
      *p++ = c;

  *p = '\0';

  /* Return null if we had an error, or if we got EOF
     before writing any characters.  */

  if (ferror (stream) || (feof (stream) && p == s))
    return NULL;

  return s;
}
