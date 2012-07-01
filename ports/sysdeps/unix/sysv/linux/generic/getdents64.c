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

#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>
#include <bits/wordsize.h>

#include <sysdep.h>
#include <sys/syscall.h>

/* The kernel struct linux_dirent64 matches the 'struct getdents64' type.  */
ssize_t
__getdents64 (int fd, char *buf, size_t nbytes)
{
  return INLINE_SYSCALL (getdents64, 3, fd, buf, nbytes);
}

#if __WORDSIZE == 64
strong_alias (__getdents64, __getdents)
#endif
