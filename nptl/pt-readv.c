/* Copyright (C) 2002 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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
#include <sysdep.h>
#include <sys/uio.h>
#include "pthreadP.h"


/* Not all versions of the kernel support extremely large numbers
   of records.  */
#ifndef UIO_FASTIOV
/* 1024 is what the kernels with NPTL support use.  */
# define UIO_FASTIOV	1024
#endif


ssize_t
readv (fd, vector, count)
     int fd;
     const struct iovec *vector;
     int count;
{
  int oldtype = CANCEL_ASYNC ();

  ssize_t result;
#ifdef INTERNAL_SYSCALL
  result = INTERNAL_SYSCALL (readv, 3, fd, vector, count);
  if (__builtin_expect (INTERNAL_SYSCALL_ERROR_P (result), 0))
    {
      if (count <= UIO_FASTIOV)
	{
	  __set_errno (INTERNAL_SYSCALL_ERRNO (result));
	  result = -1;
	}
      else
	result = __libc_readv (fd, vector, count);
    }
#else
# if defined INLINE_SYSCALL
  result = INLINE_SYSCALL (readv, 3, fd, vector, count);
  if (result < 0 && errno == EINVAL
      && __builtin_expect (count > UIO_FASTIOV, 0))
# endif
    result = __libc_readv (fd, vector, count);
#endif

  CANCEL_RESET (oldtype);

  return result;
}
