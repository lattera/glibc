/* Copyright (C) 2011 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Chris Metcalf <cmetcalf@tilera.com>, 2011.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <sys/types.h>
#include <unistd.h>

/* Truncate the file FD refers to to LENGTH bytes.  */
int
__ftruncate64 (int fd, off64_t length)
{
  unsigned int low = length & 0xffffffff;
  unsigned int high = length >> 32;
  return INLINE_SYSCALL (ftruncate64, __ALIGNMENT_COUNT (3, 4), fd,
                         __ALIGNMENT_ARG __LONG_LONG_PAIR (high, low));
}
weak_alias (__ftruncate64, ftruncate64)
