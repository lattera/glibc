/* fdatasync -- synchronize at least the data part of a file with
   the underlying media. Linux version.

   Copyright (C) 2007-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <unistd.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>

#include <kernel-features.h>

int
__fdatasync (int fd)
{
#ifdef __ASSUME_FDATASYNC
  return SYSCALL_CANCEL (fdatasync, fd);
#elif defined __NR_fdatasync
  static int __have_no_fdatasync;

  if (!__builtin_expect (__have_no_fdatasync, 0))
    {
      int result = SYSCALL_CANCEL (fdatasync, fd);
      if (__builtin_expect (result, 0) != -1 || errno != ENOSYS)
	return result;

      __have_no_fdatasync = 1;
    }
#endif
  return SYSCALL_CANCEL (fsync, fd);
}
weak_alias (__fdatasync, fdatasync)
