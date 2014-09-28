/* Selective file content synch'ing.
   Copyright (C) 2006-2015 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>

#include <sysdep-cancel.h>
#include <sys/syscall.h>


#ifdef __NR_sync_file_range
int
sync_file_range (int fd, __off64_t from, __off64_t to, unsigned int flags)
{
  return SYSCALL_CANCEL (sync_file_range, fd,
			 __LONG_LONG_PAIR ((long) (from >> 32), (long) from),
			 __LONG_LONG_PAIR ((long) (to >> 32), (long) to),
			 flags);
}
#elif defined __NR_sync_file_range2
int
sync_file_range (int fd, __off64_t from, __off64_t to, unsigned int flags)
{
  return SYSCALL_CANCEL (sync_file_range2, fd, flags,
			 __LONG_LONG_PAIR ((long) (from >> 32), (long) from),
			 __LONG_LONG_PAIR ((long) (to >> 32), (long) to));
}
#else
int
sync_file_range (int fd, __off64_t from, __off64_t to, unsigned int flags)
{
  __set_errno (ENOSYS);
  return -1;
}
stub_warning (sync_file_range)
#endif
