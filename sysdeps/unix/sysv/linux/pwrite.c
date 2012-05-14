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
#include <endian.h>
#include <unistd.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <kernel-features.h>

#ifdef __NR_pwrite64		/* Newer kernels renamed but it's the same.  */
# ifdef __NR_pwrite
#  error "__NR_pwrite and __NR_pwrite64 both defined???"
# endif
# define __NR_pwrite __NR_pwrite64
#endif


static ssize_t
#ifdef NO_CANCELLATION
inline __attribute ((always_inline))
#endif
do_pwrite (int fd, const void *buf, size_t count, off_t offset)
{
  ssize_t result;

  assert (sizeof (offset) == 4);
  result = INLINE_SYSCALL (pwrite, 5, fd, CHECK_N (buf, count), count,
			   __LONG_LONG_PAIR (offset >> 31, offset));

  return result;
}


ssize_t
__libc_pwrite (fd, buf, count, offset)
     int fd;
     const void *buf;
     size_t count;
     off_t offset;
{
  if (SINGLE_THREAD_P)
    return do_pwrite (fd, buf, count, offset);

  int oldtype = LIBC_CANCEL_ASYNC ();

  ssize_t result = do_pwrite (fd, buf, count, offset);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}

strong_alias (__libc_pwrite, __pwrite)
weak_alias (__libc_pwrite, pwrite)
