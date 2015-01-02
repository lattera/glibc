/* Copyright (C) 2007-2015 Free Software Foundation, Inc.
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
#include <sysdep-cancel.h>


/* Reserve storage for the data of the file associated with FD.  */
int
fallocate (int fd, int mode, __off_t offset, __off_t len)
{
#ifdef __NR_fallocate
  if (SINGLE_THREAD_P)
    return INLINE_SYSCALL (fallocate, 4, fd, mode, offset, len);

  int result;
  int oldtype = LIBC_CANCEL_ASYNC ();

  result = INLINE_SYSCALL (fallocate, 4, fd, mode, offset, len);

  LIBC_CANCEL_RESET (oldtype);

  return result;
#else
  __set_errno (ENOSYS);
  return -1;
#endif
}
strong_alias (fallocate, fallocate64)
