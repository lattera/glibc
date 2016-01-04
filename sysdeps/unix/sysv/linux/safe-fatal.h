/* Crash the process immediately, without possibility of deadlock.  Linux.
   Copyright (C) 2014-2016 Free Software Foundation, Inc.
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

#ifndef _SAFE_FATAL_H
#define _SAFE_FATAL_H   1

#include <sysdep.h>
#include <unistd.h>

static inline void
__safe_fatal (void)
{
  INTERNAL_SYSCALL_DECL (err);
  pid_t self = INTERNAL_SYSCALL (getpid, err, 0);
  INTERNAL_SYSCALL (kill, err, 2, self, SIGKILL);
}

#endif  /* safe-fatal.h */
