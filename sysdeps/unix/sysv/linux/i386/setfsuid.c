/* Copyright (C) 1998-2012 Free Software Foundation, Inc.
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
#include <sys/types.h>
#include <sys/fsuid.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include <linux/posix_types.h>

/* Consider moving to syscalls.list.  */

int
setfsuid (uid_t uid)
{
  INTERNAL_SYSCALL_DECL (err);
  return INTERNAL_SYSCALL (setfsuid32, err, 1, uid);
}
