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
#include <stdio.h>
#include <stdlib.h>


/* Return a new, zeroed, stream.
   You must set its cookie and io_mode.
   The first operation will give it a buffer unless you do.
   It will also give it the default functions unless you set the `seen' flag.
   Returns NULL if a stream can't be created.  */
FILE *
DEFUN_VOID(__newstream)
{
  register FILE *stream;

  stream = __stdio_head;
  while (__validfp (stream))
    stream = stream->__next;
  if (stream == NULL)
    {
      /* None to reuse.  */
      stream = (FILE *) malloc (sizeof (FILE));
      if (stream == NULL)
	return NULL;
      stream->__next = __stdio_head;
      __stdio_head = stream;
    }

  __invalidate (stream);
  stream->__magic = _IOMAGIC;
  stream->__offset = (fpos_t) -1;
  stream->__target = (fpos_t) -1;

  return stream;
}
