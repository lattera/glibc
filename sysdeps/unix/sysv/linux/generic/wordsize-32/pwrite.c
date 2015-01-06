/* Copyright (C) 2011-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.
   Based on work contributed by Ulrich Drepper <drepper@cygnus.com>, 1997.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <assert.h>
#include <errno.h>
#include <endian.h>
#include <unistd.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>

static ssize_t
do_pwrite (int fd, const void *buf, size_t count, off_t offset)
{
  assert (sizeof (offset) == 4);
  return INLINE_SYSCALL (pwrite64, __ALIGNMENT_COUNT (5, 6), fd,
                         buf, count, __ALIGNMENT_ARG
                         __LONG_LONG_PAIR (offset >> 31, offset));
}


ssize_t
__libc_pwrite (int fd, const void *buf, size_t count, off_t offset)
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
