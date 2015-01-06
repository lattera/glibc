/* Copyright (C) 1997-2015 Free Software Foundation, Inc.
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

#include <errno.h>
#include <endian.h>
#include <unistd.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>

#ifdef __NR_pwrite64		/* Newer kernels renamed but it's the same.  */
# ifdef __NR_pwrite
#  error "__NR_pwrite and __NR_pwrite64 both defined???"
# endif
# define __NR_pwrite __NR_pwrite64
#endif


static ssize_t
do_pwrite64 (int fd, const void *buf, size_t count, off64_t offset)
{
  ssize_t result;

  result = INLINE_SYSCALL (pwrite, 5, fd, buf, count,
			   __LONG_LONG_PAIR ((off_t) (offset >> 32),
					     (off_t) (offset & 0xffffffff)));

  return result;
}


ssize_t
__libc_pwrite64 (int fd, const void *buf, size_t count, off64_t offset)
{
  if (SINGLE_THREAD_P)
    return do_pwrite64 (fd, buf, count, offset);

  int oldtype = LIBC_CANCEL_ASYNC ();

  ssize_t result = do_pwrite64 (fd, buf, count, offset);

  LIBC_CANCEL_RESET (oldtype);

  return result;
}

weak_alias (__libc_pwrite64, __pwrite64)
libc_hidden_weak (__pwrite64)
weak_alias (__libc_pwrite64, pwrite64)
