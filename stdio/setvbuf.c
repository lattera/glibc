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
#include <stdlib.h>


/* Make STREAM use the buffering method given in MODE.
   If MODE indicates full or line buffering, use BUF,
   a buffer of SIZE bytes; if BUF is NULL, malloc a buffer.  */
int
DEFUN(setvbuf, (stream, buf, mode, size),
      FILE *stream AND char *buf AND int mode AND size_t size)
{
  if (!__validfp(stream))
    {
      errno = EINVAL;
      return EOF;
    }

  /* The ANSI standard says setvbuf can only be called before any I/O is done,
     but we allow it to replace an old buffer, flushing it first.  */
  if (stream->__buffer != NULL)
    {
      (void) fflush(stream);
      /* Free the old buffer if it was malloc'd.  */
      if (!stream->__userbuf)
	free(stream->__buffer);
    }

  stream->__get_limit = stream->__put_limit = NULL;
  stream->__bufp = stream->__buffer = NULL;
  stream->__userbuf = stream->__linebuf = stream->__linebuf_active = 0;

  switch (mode)
    {
    default:
      errno = EINVAL;
      return EOF;
    case _IONBF:	/* Unbuffered.  */
      stream->__buffer = NULL;
      stream->__bufsize = 0;
      stream->__userbuf = 1;
      break;
    case _IOLBF:	/* Line buffered.  */
      stream->__linebuf = 1;
    case _IOFBF:	/* Fully buffered.  */
      if (size == 0)
	{
	  errno = EINVAL;
	  return EOF;
	}
      stream->__bufsize = size;
      if (buf != NULL)
	stream->__userbuf = 1;
      else if ((buf = (char *) malloc(size)) == NULL)
	return EOF;
      stream->__buffer = buf;
      break;
    }

  stream->__bufp = stream->__buffer;
  stream->__get_limit = stream->__buffer;
  /* The next output operation will prime the stream for writing.  */
  stream->__put_limit = stream->__buffer;

  return 0;
}
