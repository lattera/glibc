/* Copyright (C) 2000 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <errno.h>
#include <unistd.h>
#include <sys/types.h>

/* Seek to OFFSET on FD, starting from WHENCE.  */
off64_t
__libc_lseek64 (int fd, off64_t offset, int whence)
{
  /* XXX We don't really support large files on the Hurd.  So if
     OFFSET doesn't fit in an `off_t', we'll return `-1' and set
     errno.  EOVERFLOW probably isn't the right error value, but seems
     appropriate here.  */
  if ((off_t) offset != offset)
    {
      __set_errno (EOVERFLOW);
      return -1;
    }

  return __libc_lseek (fd, offset, whence);
}

weak_alias (__libc_lseek64, __lseek64)
weak_alias (__libc_lseek64, lseek64)
