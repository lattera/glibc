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
#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* Enlarge STREAM's buffer.  */
static void
DEFUN(enlarge_buffer, (stream, c),
      register FILE *stream AND int c)
{
  ptrdiff_t bufp_offset = stream->__bufp - stream->__buffer;
  char *newbuf;

  stream->__bufsize += 100;
  newbuf = (char *) realloc ((PTR) stream->__buffer, stream->__bufsize);
  if (newbuf == NULL)
    {
      free ((PTR) stream->__buffer);
      stream->__buffer = stream->__bufp
	= stream->__put_limit = stream->__get_limit = NULL;
      stream->__error = 1;
    }
  else
    {
      stream->__buffer = newbuf;
      stream->__bufp = stream->__buffer + bufp_offset;
      stream->__get_limit = stream->__put_limit;
      stream->__put_limit = stream->__buffer + stream->__bufsize;
      if (c != EOF)
	*stream->__bufp++ = (unsigned char) c;
    }
}

/* Write formatted output from FORMAT to a string which is
   allocated with malloc and stored in *STRING_PTR.  */
int
DEFUN(vasprintf, (string_ptr, format, args),
      char **string_ptr AND CONST char *format AND va_list args)
{
  FILE f;
  int done;

  memset ((PTR) &f, 0, sizeof (f));
  f.__magic = _IOMAGIC;
  f.__bufsize = 100;
  f.__buffer = (char *) malloc (f.__bufsize);
  if (f.__buffer == NULL)
    return -1;
  f.__bufp = f.__buffer;
  f.__put_limit = f.__buffer + f.__bufsize;
  f.__mode.__write = 1;
  f.__room_funcs.__output = enlarge_buffer;
  f.__seen = 1;

  done = vfprintf (&f, format, args);
  if (done < 0)
    return done;

  *string_ptr = realloc (f.__buffer, (f.__bufp - f.__buffer) + 1);
  if (*string_ptr == NULL)
    *string_ptr = f.__buffer;
  (*string_ptr)[f.__bufp - f.__buffer] = '\0';
  return done;
}
