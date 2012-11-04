/* Change access and modification times of open file.  Linux version.
   Copyright (C) 2007 Free Software Foundation, Inc.
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
#include <string.h>
#include <time.h>
#include <sysdep.h>


/* Change the access time of the file associated with FD to TSP[0] and
   the modification time of FILE to TSP[1].

   Starting with 2.6.22 the Linux kernel has the utimensat syscall which
   can be used to implement futimens.  */
int
futimens (int fd, const struct timespec tsp[2])
{
#ifdef __NR_utimensat
  if (fd < 0)
    {
      __set_errno (EBADF);
      return -1;
    }
  return INLINE_SYSCALL (utimensat, 4, fd, NULL, tsp, 0);
#else
  __set_errno (ENOSYS);
  return -1;
#endif
}
#ifndef __NR_utimensat
stub_warning (futimens)
#endif
