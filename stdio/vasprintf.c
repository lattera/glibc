/* Copyright (C) 1991, 1992, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <stddef.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/* Enlarge STREAM's buffer.  */
static void
enlarge_buffer (FILE *stream, int c)
{
  ptrdiff_t bufp_offset = stream->__bufp - stream->__buffer;
  char *newbuf;

  stream->__bufsize += 100;
  newbuf = (char *) realloc ((void *) stream->__buffer, stream->__bufsize);
  if (newbuf == NULL)
    {
      free ((void *) stream->__buffer);
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
vasprintf (char **string_ptr,
	   const char *format,
	   va_list args)
{
  FILE f;
  int done;

  memset ((void *) &f, 0, sizeof (f));
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
