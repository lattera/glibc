/* Copyright (C) 1991, 1993, 1996, 1997 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stdio.h>


/* Push the character C back onto the input stream of STREAM.  */
int
ungetc (c, stream)
     int c;
     FILE *stream;
{
  if (!__validfp (stream) || !stream->__mode.__read)
    {
      __set_errno (EINVAL);
      return EOF;
    }

  if (c == EOF)
    return EOF;

  if (stream->__pushed_back)
    /* There is already a char pushed back.  */
    return EOF;

  if ((stream->__linebuf_active || stream->__put_limit > stream->__buffer) &&
      /* This is a read-write stream with something in its buffer.
	 Flush the stream.  */
      __flshfp (stream, EOF) == EOF)
    return EOF;

  stream->__pushback = (unsigned char) c;
  /* Tell __fillbf we've pushed back a char.  */
  stream->__pushed_back = 1;
  stream->__pushback_bufp = stream->__bufp;
  /* Make the next getc call __fillbf.  It will return C.  */
  stream->__bufp = stream->__get_limit;

  /* We just gave it another character to read, so it's not at EOF.  */
  stream->__eof = 0;

  return stream->__pushback;
}
