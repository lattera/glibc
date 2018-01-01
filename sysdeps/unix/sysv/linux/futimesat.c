/* Copyright (C) 2005-2018 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <utime.h>
#include <sys/time.h>
#include <sysdep.h>


/* Change the access time of FILE relative to FD to TVP[0] and
   the modification time of FILE to TVP[1].  */
int
futimesat (int fd, const char *file, const struct timeval tvp[2])
{
  if (file == NULL)
    return __futimes (fd, tvp);

  /* Avoid implicit array coercion in syscall macros.  */
  return INLINE_SYSCALL (futimesat, 3, fd, file, &tvp[0]);
}
