/* Copyright (C) 1997-2012 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <errno.h>
#include <unistd.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <kernel-features.h>

ssize_t
__libc_pread (fd, buf, count, offset)
     int fd;
     void *buf;
     size_t count;
     off_t offset;
{
  ssize_t result;

  if (SINGLE_THREAD_P)
    {
      /* On PPC32 64bit values are aligned in odd/even register pairs.  */
      result = INLINE_SYSCALL (pread, 6, fd, CHECK_N (buf, count), count,
			       0, offset >> 31, offset);

      return result;
    }

  int oldtype = LIBC_CANCEL_ASYNC ();

  /* On PPC32 64bit values are aligned in odd/even register pairs.  */
  result = INLINE_SYSCALL (pread, 6, fd, CHECK_N (buf, count), count,
			       0, offset >> 31, offset);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}

strong_alias (__libc_pread, __pread)
weak_alias (__libc_pread, pread)
