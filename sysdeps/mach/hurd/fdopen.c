/* Copyright (C) 1991, 1994, 1995, 1997, 2000 Free Software Foundation, Inc.
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
#include <hurd/fd.h>
#include <fcntl.h>
#include <hurd/io.h>

/* Defined in fopen.c.  */
extern int __getmode (const char *mode, __io_mode *mptr);

/* Open a new stream on a given system file descriptor.  */
FILE *
__fdopen (fd, mode)
     int fd;
     const char *mode;
{
  FILE *stream;
  __io_mode m;
  struct hurd_fd *d;
  error_t err;
  int openmodes;

  if (!__getmode (mode, &m))
    return NULL;

  HURD_CRITICAL_BEGIN;
  d = _hurd_fd_get (fd);
  if (d == NULL)
    err = EBADF;
  else
    err = HURD_FD_PORT_USE (d, __io_get_openmodes (port, &openmodes));
  HURD_CRITICAL_END;

  if (err)
    return __hurd_dfail (fd, err), NULL;

  /* Check the access mode.  */
  if ((m.__read && !(openmodes & O_READ)) ||
      (m.__write && !(openmodes & O_WRITE)))
    {
      errno = EBADF;
      return NULL;
    }

  stream = __newstream ();
  if (stream == NULL)
    return NULL;

  stream->__cookie = d;
  stream->__mode = m;

  return stream;
}

weak_alias (__fdopen, fdopen)
