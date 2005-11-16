/* Copyright (C) 1997, 1998, 1999, 2000, 2002, 2003, 2005
   Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <endian.h>
#include <unistd.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>
#include <bp-checks.h>

ssize_t
__libc_pwrite (fd, buf, count, offset)
     int fd;
     const void *buf;
     size_t count;
     off_t offset;
{
  ssize_t result;

  if (SINGLE_THREAD_P)
    {
      /* In the ARM EABI, 64-bit values are aligned to even/odd register
	 pairs for syscalls.  */
      result = INLINE_SYSCALL (pwrite64, 6, fd, CHECK_N (buf, count), count, 0,
			       __LONG_LONG_PAIR (offset >> 31, offset));

      return result;
    }

  int oldtype = LIBC_CANCEL_ASYNC ();

  /* In the ARM EABI, 64-bit values are aligned to even/odd register
     pairs for syscalls.  */
  result = INLINE_SYSCALL (pwrite64, 6, fd, CHECK_N (buf, count), count, 0,
			   __LONG_LONG_PAIR (offset >> 31, offset));

  LIBC_CANCEL_RESET (oldtype);

  return result;
}

strong_alias (__libc_pwrite, __pwrite)
weak_alias (__libc_pwrite, pwrite)
