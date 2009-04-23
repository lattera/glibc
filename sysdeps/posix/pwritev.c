/* Copyright (C) 2009 Free Software Foundation, Inc.
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
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <stdbool.h>
#include <sys/param.h>
#if __WORDSIZE == 64 && !defined PWRITEV
/* Hide the pwritev64 declaration.  */
# define pwritev64 __redirect_pwritev64
#endif
#include <sys/uio.h>
#include <bits/wordsize.h>

#ifndef PWRITEV
# define PWRITEV pwritev
# define PWRITE __pwrite
# define OFF_T off_t
#endif


static void
ifree (char **ptrp)
{
  free (*ptrp);
}


/* Write data pointed by the buffers described by IOVEC, which is a
   vector of COUNT 'struct iovec's, to file descriptor FD at the given
   position OFFSET without change the file pointer.  The data is
   written in the order specified.  Operates just like 'write' (see
   <unistd.h>) except that the data are taken from IOVEC instead of a
   contiguous buffer.  */
ssize_t
PWRITEV (int fd, const struct iovec *vector, int count, OFF_T offset)
{
  /* Find the total number of bytes to be read.  */
  size_t bytes = 0;
  for (int i = 0; i < count; ++i)
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
  char *buffer;
  char *malloced_buffer __attribute__ ((__cleanup__ (ifree))) = NULL;
  if (__libc_use_alloca (bytes))
    buffer = (char *) __alloca (bytes);
  else
    {
      malloced_buffer = buffer = (char *) malloc (bytes);
      if (buffer == NULL)
	return -1;
    }

  /* Copy the data from BUFFER into the memory specified by VECTOR.  */
  char *ptr = buffer;
  for (int i = 0; i < count; ++i)
    ptr = __mempcpy ((void *) ptr, (void *) vector[i].iov_base,
		     vector[i].iov_len);

  /* Write the data.  */
  return PWRITE (fd, buffer, bytes, offset);
}
#if __WORDSIZE == 64 && defined pwritev64
# undef pwritev64
strong_alias (pwritev, pwritev64)
#endif
