/* Copyright (C) 1991, 1992, 1996, 1997, 2002 Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/param.h>
#include <sys/uio.h>
#include <errno.h>

/* Write data pointed by the buffers described by VECTOR, which
   is a vector of COUNT `struct iovec's, to file descriptor FD.
   The data is written in the order specified.
   Operates just like `write' (see <unistd.h>) except that the data
   are taken from VECTOR instead of a contiguous buffer.  */
ssize_t
__libc_writev (int fd, const struct iovec *vector, int count)
{
  char *buffer;
  register char *bp;
  size_t bytes, to_copy;
  ssize_t bytes_written;
  int i;
  bool use_malloc = false;

  /* Find the total number of bytes to be written.  */
  bytes = 0;
  for (i = 0; i < count; ++i)
    {
      /* Check for ssize_t overflow.  */
      if (SSIZE_MAX - bytes < vector[i].iov_len)
	{
	  __set_errno (EINVAL);
	  return -1;
	}
      bytes += vector[i].iov_len;
    }

  /* Allocate a temporary buffer to hold the data.  We should normally
     use alloca since it's faster and does not require synchronization
     with other threads.  But we cannot if the amount of memory
     required is too large.  */
  if (__libc_use_alloca (bytes))
    buffer = (char *) __alloca (bytes);
  else
    {
      buffer = (char *) malloc (bytes);
      if (buffer == NULL)
	/* XXX I don't know whether it is acceptable to try writing
	   the data in chunks.  Probably not so we just fail here.  */
	return -1;

      use_malloc = true;
    }

  /* Copy the data into BUFFER.  */
  to_copy = bytes;
  bp = buffer;
  for (i = 0; i < count; ++i)
    {
      size_t copy = MIN (vector[i].iov_len, to_copy);

      bp = __mempcpy ((void *) bp, (void *) vector[i].iov_base, copy);

      to_copy -= copy;
      if (to_copy == 0)
	break;
    }

  bytes_written = __write (fd, buffer, bytes);

  if (use_malloc)
    free (buffer);

  return bytes_written;
}
#ifndef __libc_writev
strong_alias (__libc_writev, __writev)
weak_alias (__libc_writev, writev)
#endif
