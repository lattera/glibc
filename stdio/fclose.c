/* Copyright (C) 1991 Free Software Foundation, Inc.
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
#include <string.h>


/* Close a stream.  */
int
DEFUN(fclose, (stream), register FILE *stream)
{
  int status;

  if (stream == NULL)
    {
      /* Close all streams.  */
      register FILE *f;
      for (f = __stdio_head; f != NULL; f = f->__next)
	if (__validfp(f))
	  (void) fclose(f);
      return 0;
    }

  if (!__validfp(stream))
    {
      errno = EINVAL;
      return EOF;
    }
	
  if (stream->__mode.__write &&
      /* Flush the buffer.  */
      __flshfp (stream, EOF) == EOF)
    return EOF;

  /* Free the buffer's storage.  */
  if (stream->__buffer != NULL && !stream->__userbuf)
    free(stream->__buffer);

  /* Close the system file descriptor.  */
  if (stream->__io_funcs.__close != NULL)
    status = (*stream->__io_funcs.__close)(stream->__cookie);
  else if (!stream->__seen && stream->__cookie != NULL)
    status = __stdio_close(stream->__cookie);
  else
    status = 0;

  /* Nuke the stream, making it available for re-use.  */
  __invalidate(stream);

  return status < 0 ? EOF : 0;
}
