/* Copyright (C) 1989, 1991, 1993, 1996, 1997 Free Software Foundation, Inc.
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

#include <alloca.h>
#include <errno.h>
#include <grp.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>


/* Initialize the group set for the current user
   by reading the group database and using all groups
   of which USER is a member.  Also include GROUP.  */
int
initgroups (user, group)
     const char *user;
     gid_t group;
{
#if defined NGROUPS_MAX && NGROUPS_MAX == 0

  /* No extra groups allowed.  */
  return 0;

#else

  struct group grpbuf, *g;
  size_t buflen = sysconf (_SC_GETPW_R_SIZE_MAX);
  char *tmpbuf;
  size_t n;
  size_t ngroups;
  gid_t *groups;
  int status;
#ifdef NGROUPS_MAX
# define limit NGROUPS_MAX

  ngroups = limit;
#else
  long int limit = sysconf (_SC_NGROUPS_MAX);

  if (limit > 0)
    ngroups = limit;
  else
    /* No fixed limit on groups.  Pick a starting buffer size.  */
    ngroups = 16;
#endif

  groups = __alloca (ngroups * sizeof *groups);
  tmpbuf = __alloca (buflen);

  setgrent ();

  n = 0;
  groups[n++] = group;

  do
    {
      while ((status = __getgrent_r (&grpbuf, tmpbuf, buflen, &g)) != 0
	     && errno == ERANGE)
	{
	  buflen *= 2;
	  tmpbuf = __alloca (buflen);
	}

      if (status == 0 && g->gr_gid != group)
	{
	  char **m;

	  for (m = g->gr_mem; *m != NULL; ++m)
	    if (strcmp (*m, user) == 0)
	      {
		/* Matches user.  Insert this group.  */
		if (n == ngroups && limit <= 0)
		  {
		    /* Need a bigger buffer.  */
		    gid_t *newgrp;
		    newgrp = __alloca (ngroups * 2 * sizeof *groups);
		    groups = memcpy (newgrp, groups, ngroups * sizeof *groups);
		    ngroups *= 2;
		  }

		groups[n++] = g->gr_gid;

		if (n == limit)
		  /* Can't take any more groups; stop searching.  */
		  goto done;

		break;
	      }
	}
    }
  while (status == 0);

done:
  endgrent ();

  return setgroups (n, groups);
#endif
}
