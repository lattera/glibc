/* Copyright (C) 1994, 1995, 1997, 2004 Free Software Foundation, Inc.
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

#include <sys/syssgi.h>
#include <sys/types.h>
#include <grp.h>

extern int __syssgi (int, ...);

/* Set the group set for the current user to GROUPS (N of them).  */
int
__getgroups (n, groups)
     size_t n;
     gid_t *groups;
{
  return __syssgi (SGI_GETGROUPS, n, groups);
}

weak_alias (__getgroups, getgroups)
