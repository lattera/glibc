/* Copyright (C) 1989, 1991, 1993 Free Software Foundation, Inc.
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
#include <unistd.h>
#include <string.h>
#include <grp.h>
#include <limits.h>
#include <sys/types.h>


/* Initialize the group set for the current user
   by reading the group database and using all groups
   of which USER is a member.  Also include GROUP.  */
int
DEFUN(initgroups, (user, group),
      CONST char *user AND gid_t group)
{
#ifdef NGROUPS_MAX
#if NGROUPS_MAX == 0
  return 0;
#else
  static PTR info = NULL;
  register FILE *stream;
  register struct group *g;
  gid_t groups[NGROUPS_MAX];
  register size_t n;

  if (info == NULL)
    {
      info = __grpalloc();
      if (info == NULL)
	return -1;
    }

  stream = __grpopen();
  if (stream == NULL)
    return -1;

  n = 0;
  groups[n++] = group;

  while (n < NGROUPS_MAX && (g = __grpread(stream, info)) != NULL)
    if (g->gr_gid != group)
      {
	register char **m;

	for (m = g->gr_mem; *m != NULL; ++m)
	  if (!strcmp(*m, user))
	    groups[n++] = g->gr_gid;
      }

  return setgroups(n, groups);
#endif
#else
  return 0;
#endif
}
