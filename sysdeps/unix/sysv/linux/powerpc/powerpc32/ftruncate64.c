/* Copyright (C) 1997-2015 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <sysdep.h>
#include <sys/syscall.h>

/* Truncate the file referenced by FD to LENGTH bytes.  */
int
__ftruncate64 (fd, length)
     int fd;
     off64_t length;
{
  /* On PPC32 64bit values are aligned in odd/even register pairs.  */
  int result = INLINE_SYSCALL (ftruncate64, 4, fd, 0,
			       (long) (length >> 32),
			       (long) length);

  return result;
}
weak_alias (__ftruncate64, ftruncate64)
