/* Copyright (C) 1991, 1993, 1994 Free Software Foundation, Inc.
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

/* Return the system file descriptor associated with STREAM.  */
int
DEFUN(fileno, (stream), FILE *stream)
{
  extern void __stdio_check_funcs __P ((FILE *));

  if (! __validfp (stream))
    {
      errno = EINVAL;
      return -1;
    }

  __stdio_check_funcs (stream);

  if (stream->__io_funcs.__fileno == NULL)
    {
#ifdef EOPNOTSUPP
      errno = EOPNOTSUPP;
#else
      errno = ENOSYS;
#endif
      return -1;
    }

  return (*stream->__io_funcs.__fileno) (stream->__cookie);
}
