/* Copyright (C) 1991, 1992, 1994 Free Software Foundation, Inc.
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

/* Return the offset in bytes from the beginning
   of the file of the file position of STREAM.  */
long int
DEFUN(ftell, (stream), FILE *stream)
{
  long int pos;

  if (!__validfp (stream))
    {
      errno = EINVAL;
      return -1L;
    }

  if (__stdio_check_offset (stream) == EOF)
    return -1L;

  /* Start with the file position associated with the beginning
     of our buffer.  */
  pos = stream->__target;

  if (stream->__pushed_back)
    /* ungetc was just called, so our real buffer pointer is squirreled
       away in STREAM->__pushback_bufp, not in STREAM->__bufp as normal.
       Calling ungetc is supposed to decrement the file position.  ANSI
       says the file position is unspecified if you ungetc when the
       position is zero; -1 seems as good as anything to me.  */
    pos += stream->__pushback_bufp - stream->__buffer - 1;
  else
    pos += stream->__bufp - stream->__buffer;

  return pos;
}
