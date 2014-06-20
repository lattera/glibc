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
#include <stdio.h>
#include <string.h>
#include <sysdep.h>


/* Consider moving to syscalls.list.  */

/* Rename the file OLD relative to OLDFD to NEW relative to NEWFD.  */
int
renameat (oldfd, old, newfd, new)
     int oldfd;
     const char *old;
     int newfd;
     const char *new;
{
  return INLINE_SYSCALL (renameat, 4, oldfd, old, newfd, new);
}
