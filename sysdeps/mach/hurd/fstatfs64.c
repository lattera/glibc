/* Copyright (C) 2001,02 Free Software Foundation, Inc.
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

#include <sys/statfs.h>
#include <hurd.h>
#include <hurd/fd.h>

#include "statfsconv.c"

/* Return information about the filesystem on which FD resides.  */
int
__fstatfs64 (int fd, struct statfs64 *buf)
{
  error_t err;

  if (err = HURD_DPORT_USE (fd, __file_statfs (port, buf)))
    return __hurd_dfail (fd, err);

  return 0;
}
weak_alias (__fstatfs64, fstatfs64)
