/* Copyright (C) 1997, 1998 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <grp.h>
#include <unistd.h>
#include <sys/types.h>

#include <sysdep.h>
#include <sys/syscall.h>

#include <linux/posix_types.h>

extern int __syscall_setgroups __P ((int, const __kernel_gid_t *));

/* Set the group set for the current user to GROUPS (N of them).  For
   Linux we must convert the array of groups into the format that the
   kernel expects.  */
int
setgroups (n, groups)
     size_t n;
     const gid_t *groups;
{
  if (n < 0 || n > __sysconf (_SC_NGROUPS_MAX))
    {
      __set_errno (EINVAL);
      return -1;
    }
  else
    {
      size_t i;
      __kernel_gid_t kernel_groups[n];

      for (i = 0; i < n; i++)
	{
	  kernel_groups[i] = groups[i];
	  if (groups[i] != (gid_t) ((__kernel_gid_t) groups[i]))
	    {
	      __set_errno (EINVAL);
	      return -1;
	    }
	}

      return INLINE_SYSCALL (setgroups, 2, n, kernel_groups);
    }
}
