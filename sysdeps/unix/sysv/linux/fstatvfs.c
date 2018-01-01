/* Copyright (C) 1998-2018 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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

#include <stddef.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include "internal_statvfs.h"

int
__fstatvfs (int fd, struct statvfs *buf)
{
  struct statfs fsbuf;

  /* Get as much information as possible from the system.  */
  if (__fstatfs (fd, &fsbuf) < 0)
    return -1;

  /* Convert the result.  */
  __internal_statvfs (NULL, buf, &fsbuf, fd);

  /* We signal success if the statfs call succeeded.  */
  return 0;
}
weak_alias (__fstatvfs, fstatvfs)
libc_hidden_weak (fstatvfs)
