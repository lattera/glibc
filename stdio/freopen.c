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
#include <errno.h>
#include <stdio.h>


/* Defined in fopen.c.  */
extern int __getmode __P ((const char *, __io_mode *));

/* Defined in sysd-stdio.c.  */
extern int __stdio_reopen __P ((const char *filename, __io_mode mode,
				PTR *cookieptr, __io_close_fn closefn));

/* Replace STREAM, opening it on FILENAME.  */
FILE *
DEFUN(freopen, (filename, mode, stream),
      CONST char *filename AND CONST char *mode AND register FILE *stream)
{
  __io_mode m;
  PTR cookie;

  if (!__getmode (mode, &m))
    {
      (void) fclose (stream);
      errno = EINVAL;
      return NULL;
    }

  if (stream->__mode.__write)
    /* Flush the stream.  */
    (void) fflush (stream);

  /* Open the file, attempting to preserve the old cookie value.  */
  cookie = stream->__cookie;
  if (__stdio_reopen (filename, m, &cookie,
		      stream->__seen ?
		      stream->__io_funcs.__close :
		      __stdio_close))
    {
      int save = errno;
      (void) fclose (stream);
      errno = save;
      return NULL;
    }

  /* Close the stream, first disabling its cookie close function because
     __stdio_reopen has already dealt with closing the old cookie.  */
  stream->__seen = 1;		/* It might have no functions yet.  */
  stream->__io_funcs.__close = NULL;
  (void) fclose (stream);

  stream->__magic = _IOMAGIC;
  stream->__cookie = cookie;
  stream->__mode = m;

  return stream;
}
