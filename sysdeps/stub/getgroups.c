/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <ansidecl.h>
#include <errno.h>
#include <stddef.h>
#include <unistd.h>
#include <sys/types.h>
#include <limits.h>


/* If SIZE is zero, return the number of supplementary groups
   the calling process is in.  Otherwise, fill in the group IDs
   of its supplementary groups in LIST and return the number written.  */
int
DEFUN(__getgroups, (size, list), int size AND gid_t *list)
{
#if defined (NGROUPS_MAX) && NGROUPS_MAX == 0
  /* The system has no supplementary groups.  */
  return 0;
#endif

  errno = ENOSYS;
  return -1;
}

#if defined (HAVE_GNU_LD) && !(defined (NGROUPS_MAX) && NGROUPS_MAX == 0)
stub_warning (getgroups);
#endif

weak_alias (__getgroups, getgroups)
