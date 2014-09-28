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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <endian.h>
#include <unistd.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>

ssize_t
__libc_pread (int fd, void *buf, size_t count, off_t offset)
{
  /* In the ARM EABI, 64-bit values are aligned to even/odd register
     pairs for syscalls.  */
  return SYSCALL_CANCEL (pread64, fd, buf, count, 0,
			 __LONG_LONG_PAIR (offset >> 31, offset));
}

strong_alias (__libc_pread, __pread)
weak_alias (__libc_pread, pread)
