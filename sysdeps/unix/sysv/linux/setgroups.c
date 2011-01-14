/* Copyright (C) 1997,1998,2000,2002,2004,2006,2011
   Free Software Foundation, Inc.
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

#include <errno.h>
#include <grp.h>
#include <setxid.h>
#include <sysdep.h>


/* Set the group set for the current user to GROUPS (N of them).  For
   Linux we must convert the array of groups into the format that the
   kernel expects.  */
int
setgroups (size_t n, const gid_t *groups)
{
#ifdef __NR_setgroups32
# error "wrong setgroups.c file used"
#endif
  return INLINE_SETXID_SYSCALL (setgroups, 2, n, groups);
}
libc_hidden_def (setgroups)
