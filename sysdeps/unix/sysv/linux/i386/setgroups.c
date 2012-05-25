/* Copyright (C) 1997-2012 Free Software Foundation, Inc.
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
#include <grp.h>
#include <unistd.h>
#include <sys/types.h>

#include <sysdep.h>
#include <sys/syscall.h>
#include <bp-checks.h>

#include <setxid.h>
#include <linux/posix_types.h>

/* Set the group set for the current user to GROUPS (N of them).  For
   Linux we must convert the array of groups into the format that the
   kernel expects.  */
int
setgroups (size_t n, const gid_t *groups)
{
  return INLINE_SETXID_SYSCALL (setgroups32, 2, n, CHECK_N (groups, n));
}
libc_hidden_def (setgroups)
