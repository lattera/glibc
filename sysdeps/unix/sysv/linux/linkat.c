/* Copyright (C) 2005-2014 Free Software Foundation, Inc.
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
#include <stdio.h>
#include <sysdep.h>
#include <unistd.h>


/* Consider moving to syscalls.list.  */

/* Make a link to FROM named TO but relative paths in TO and FROM are
   interpreted relative to FROMFD and TOFD respectively.  */
int
linkat (fromfd, from, tofd, to, flags)
     int fromfd;
     const char *from;
     int tofd;
     const char *to;
     int flags;
{
  return INLINE_SYSCALL (linkat, 5, fromfd, from, tofd, to, flags);
}
