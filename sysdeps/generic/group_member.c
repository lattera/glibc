/* `group_member' -- test if process is in a given group.
Copyright (C) 1995 Free Software Foundation, Inc.
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

#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>

#ifndef NGROUPS_MAX
#define NGROUPS_MAX	16	/* First guess.  */
#endif

int
__group_member (gid)
     gid_t gid;
{
  int n, size;
  gid_t *groups;

  size = NGROUPS_MAX;
  do
    {
      groups = __alloca (size * sizeof *groups);
      n = __getgroups (size, groups);
      size *= 2;
    } while (n == size / 2);

  while (n > 0)
    if (groups[n--] == gid)
      return 1;

  return 0;
}
weak_alias (__group_member, group_member)
