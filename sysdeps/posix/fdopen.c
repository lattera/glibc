/* Copyright (C) 1991, 1992, 1993, 1996 Free Software Foundation, Inc.
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
#include <fcntl.h>

/* Defined in fopen.c.  */
extern int __getmode (const char *mode, __io_mode *mptr);

/* Open a new stream on a given system file descriptor.  */
FILE *
__fdopen (fd, mode)
     int fd;
     const char *mode;
{
  register FILE *stream;
  __io_mode m;
  int dflags;

  if (!__getmode (mode, &m))
    return NULL;

  /* Verify the FD is valid and allows the access MODE specifies.  */

  dflags = __fcntl (fd, F_GETFL);
  if (dflags == -1)
    /* FD was invalid; fcntl has already set errno.  */
    return NULL;

  /* Check the access mode.  */
  switch (dflags & O_ACCMODE)
    {
    case O_RDONLY:
      if (!m.__read)
	{
	  __set_errno (EBADF);
	  return NULL;
	}
      break;
    case O_WRONLY:
      if (!m.__write)
	{
	  __set_errno (EBADF);
	  return NULL;
	}
      break;
    }

  stream = __newstream ();
  if (stream == NULL)
    return NULL;

  stream->__cookie = (void *) fd;
  stream->__mode = m;

  return stream;
}

weak_alias (__fdopen, fdopen)
