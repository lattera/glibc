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
#include <stdio.h>
#include <obstack.h>
#include <stdarg.h>
#include <string.h>

/* Output-room function for obstack streams.  */

static void
DEFUN(grow, (stream, c), FILE *stream AND int c)
{
  struct obstack *const obstack = (struct obstack *) stream->__cookie;

  /* Move the end of the object back to include only the portion
     of the buffer which the user has already written into.  */ 
  obstack_blank_fast (obstack, - (stream->__put_limit - stream->__bufp));

  if (stream->__target > obstack_object_size (obstack))
    {
      /* Our target (where the buffer maps to) is always zero except when
	 the user just did a SEEK_END fseek.  If he sought within the
	 buffer, we need do nothing and will zero the target below.  If he
	 sought past the end of the object, grow and zero-fill the object
	 up to the target address.  */

      obstack_blank (obstack,
		     stream->__target - obstack_object_size (obstack));
      /* fseek has just flushed us, so the put limit points
	 to the end of the written data.  */
      bzero (stream->__put_limit,
	     stream->__target - stream->__bufsize);
    }

  if (c != EOF)
    obstack_1grow (obstack, (unsigned char) c);

  /* The stream buffer always maps exactly to the object on the top
     of the obstack.  The start of the buffer is the start of the object.
     The put limit points just past the end of the object.  On fflush, the
     obstack is sync'd so the end of the object points just past the last
     character written to the stream.  */

  stream->__target = stream->__offset = 0;
  stream->__buffer = obstack_base (obstack);
  stream->__bufsize = obstack_room (obstack);
  stream->__bufp = obstack_next_free (obstack);
  stream->__get_limit = stream->__bufp;

  if (c == EOF)
    /* This is fflush.  Make the stream buffer, the object,
       and the characters actually written all match.  */
    stream->__put_limit = stream->__get_limit;
  else
    {
      /* Extend the buffer (and the object) to include
	 the rest of the obstack chunk (which is unitialized).
	 Data past bufp is undefined.  */
      stream->__put_limit = stream->__buffer + stream->__bufsize;
      obstack_blank_fast (obstack, stream->__put_limit - stream->__bufp);
    }
}

/* Seek function for obstack streams.
   There is no external state to munge.  */

static int
DEFUN(seek, (cookie, pos, whence),
      PTR cookie AND fpos_t *pos AND int whence)
{
  switch (whence)
    {
    case SEEK_SET:
    case SEEK_CUR:
      return 0;

    case SEEK_END:
      /* Return the position relative to the end of the object.
	 fseek has just flushed us, so the obstack is consistent.  */
      *pos += obstack_object_size ((struct obstack *) cookie);
      return 0;

    default:
      __libc_fatal ("obstream::seek called with bogus WHENCE\n");
      return -1;
    }
}

/* Input room function for obstack streams.
   Only what has been written to the stream can be read back.  */

static int
DEFUN(input, (stream), FILE *stream)
{
  /* Re-sync with the obstack, growing the object if necessary.  */
  grow (stream, EOF);

  if (stream->__bufp < stream->__get_limit)
    return (unsigned char) *stream->__bufp++;

  stream->__eof = 1;
  return EOF;
}

/* Initialize STREAM to talk to OBSTACK.  */

static void
DEFUN(init_obstream, (stream, obstack),
      FILE *stream AND struct obstack *obstack)
{
  stream->__mode.__write = 1;
  stream->__mode.__read = 1;

  /* Input can read only what has been written.  */
  stream->__room_funcs.__input = input;

  /* Do nothing for close.  */
  stream->__io_funcs.__close = NULL;

  /* When the buffer is full, grow the obstack.  */
  stream->__room_funcs.__output = grow;

  /* Seek within the object, and extend it.  */
  stream->__io_funcs.__seek = seek;
  stream->__target = stream->__offset = 0;

  stream->__seen = 1;

  /* Don't deallocate that buffer!  */
  stream->__userbuf = 1;

  /* We don't have to initialize the buffer.
     The first read attempt will call grow, which will do all the work.  */
}

FILE *
open_obstack_stream (obstack)
     struct obstack *obstack;
{
  register FILE *stream;

  stream = __newstream ();
  if (stream == NULL)
    return NULL;

  init_obstream (stream, obstack);
  return stream;
}

int
DEFUN(obstack_vprintf, (obstack, format, args),
      struct obstack *obstack AND const char *format AND va_list args)
{
  FILE f;
  bzero (&f, sizeof (f));
  init_obstream (&f, obstack);
  return vfprintf (&f, format, args);
}

int
DEFUN(obstack_printf, (obstack, format),
      struct obstack *obstack AND const char *format DOTS)
{
  int result;
  va_list ap;
  va_start (ap, format);
  result = obstack_vprintf (obstack, format, ap);
  va_end (ap);
  return result;
}
