/* Read block from given position in file without changing file pointer.
   Hurd version.
   Copyright (C) 2001-2016 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <hurd/fd.h>

ssize_t
__libc_pread64 (int fd, void *buf, size_t nbytes, off64_t offset)
{
  error_t err;
  if (offset < 0)
    err = EINVAL;
  else
    err = HURD_FD_USE (fd, _hurd_fd_read (descriptor, buf, &nbytes, offset));
  return err ? __hurd_dfail (fd, err) : nbytes;
}

#ifndef __libc_pread64
weak_alias (__libc_pread64, __pread64)
weak_alias (__libc_pread64, pread64)
#endif
