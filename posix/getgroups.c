/* Copyright (C) 1991,1992,1995,1996,1997,2005 Free Software Foundation, Inc.
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
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>


/* If SIZE is zero, return the number of supplementary groups
   the calling process is in.  Otherwise, fill in the group IDs
   of its supplementary groups in LIST and return the number written.  */
int
__getgroups (size, list)
     int size;
     gid_t *list;
{
#if defined (NGROUPS_MAX) && NGROUPS_MAX == 0
  /* The system has no supplementary groups.  */
  return 0;
#endif

  __set_errno (ENOSYS);
  return -1;
}

#if !(defined (NGROUPS_MAX) && NGROUPS_MAX == 0)
stub_warning (getgroups);
#endif

weak_alias (__getgroups, getgroups)
#include <stub-tag.h>
