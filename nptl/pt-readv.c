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


/* Not all versions of the kernel support the large number of records.  */
#ifndef UIO_FASTIOV
# define UIO_FASTIOV	8	/* 8 is a safe number.  */
#endif


ssize_t
readv (fd, vector, count)
     int fd;
     const struct iovec *vector;
     int count;
{
  int oldtype;
  ssize_t result;

  CANCEL_ASYNC (oldtype);

#ifdef INTERNAL_SYSCALL
  result = INTERNAL_SYSCALL (readv, 3, fd, vector, count);
  if (INTERNAL_SYSCALL_ERROR_P (result)
      && __builtin_expect (count > UIO_FASTIOV, 0))
#elif defined INLINE_SYSCALL
  result = INLINE_SYSCALL (readv, 3, fd, vector, count);
  if (result < 0 && errno == EINVAL
      && __builtin_expect (count > UIO_FASTIOV, 0))
#endif
    result = __libc_readv (fd, vector, count);

  CANCEL_RESET (oldtype);

  return result;
}
