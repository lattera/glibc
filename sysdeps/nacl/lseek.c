/* lseek -- Move the file position of a file descriptor.  NaCl version.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <nacl-interfaces.h>


/* Seek to OFFSET on FD, starting from WHENCE.  */
off_t
__libc_lseek (int fd, off_t offset, int whence)
{
  off_t result;
  int error = __nacl_irt_fdio.seek (fd, offset, whence, &result);
  if (error)
    {
      __set_errno (error);
      return -1;
    }
  return result;
}
libc_hidden_def (__lseek)
weak_alias (__libc_lseek, __lseek)
weak_alias (__libc_lseek, lseek)

/* Since off64_t is the same as off_t, lseek64 is just an alias.  */
weak_alias (__libc_lseek, __libc_lseek64)
weak_alias (__libc_lseek, __lseek64)
weak_alias (__libc_lseek, lseek64)
