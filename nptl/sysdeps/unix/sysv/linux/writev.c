/* writev supports all Linux kernels >= 2.0.
   Copyright (C) 1997, 1998, 2000, 2002 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <sys/param.h>
#include <sys/uio.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>
#include <nptl/pthreadP.h>
#include <tls.h>

extern ssize_t __syscall_writev (int, const struct iovec *__unbounded, int);


ssize_t
__libc_writev (fd, vector, count)
     int fd;
     const struct iovec *vector;
     int count;
{
#ifndef NOT_IN_libc
  if (__builtin_expect (THREAD_GETMEM (THREAD_SELF,
				       header.data.multiple_threads) == 0, 1))
    return INLINE_SYSCALL (writev, 3, fd, CHECK_N (vector, count), count);

  int oldtype = LIBC_CANCEL_ASYNC ();
#endif

  ssize_t result = INLINE_SYSCALL (writev, 3, fd, CHECK_N (vector, count),
				   count);

#ifndef NOT_IN_libc
  LIBC_CANCEL_RESET (oldtype);
#endif

  return result;
}
strong_alias (__libc_writev, __writev)
weak_alias (__libc_writev, writev)
