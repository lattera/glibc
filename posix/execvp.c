/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <limits.h>
#include <sys/types.h>

#ifndef	HAVE_GNU_LD
#define	__environ	environ
#endif

/* Execute FILE, searching in the `PATH' environment variable if it contains
   no slashes, with arguments ARGV and environment from `environ'.  */
int
DEFUN(execvp, (file, argv), CONST char *file AND char *CONST argv[])
{
  if (strchr (file, '/') == NULL)
    {
      char *path, *p;
      struct stat st;
      size_t len;
      uid_t uid;
      gid_t gid;
      int ngroups;
      gid_t groups[NGROUPS_MAX];
      char *name;

      path = getenv ("PATH");
      if (path == NULL)
	{
	  /* There is no `PATH' in the environment.
	     The default search path is the current directory
	     followed by the path `confstr' returns for `_CS_PATH'.  */
	  len = confstr (_CS_PATH, (char *) NULL, 0);
	  path = (char *) __alloca (1 + len);
	  path[0] = ':';
	  (void) confstr (_CS_PATH, path + 1, len);
	}

      len = strlen (file) + 1;
      name = __alloca (strlen (path) + len);
      uid = geteuid ();
      gid = getegid ();
      ngroups = getgroups (sizeof (groups) / sizeof (groups[0]), groups);
      p = path;
      do
	{
	  path = p;
	  p = strchr (path, ':');
	  if (p == NULL)
	    p = strchr (path, '\0');

	  if (p == path)
	    /* Two adjacent colons, or a colon at the beginning or the end
	       of `PATH' means to search the current directory.  */
	    (void) memcpy (name, file, len);
	  else
	    {
	      /* Construct the pathname to try.  */
	      (void) memcpy (name, path, p - path);
	      name[p - path] = '/';
	      (void) memcpy (&name[(p - path) + 1], file, len);
	    }
	  if (stat (name, &st) == 0 && S_ISREG (st.st_mode))
	    {
	      int bit = S_IXOTH;
	      if (st.st_uid == uid)
		bit = S_IXUSR;
	      else if (st.st_gid == gid)
		bit = S_IXGRP;
	      else
		{
		  register int i;
		  for (i = 0; i < ngroups; ++i)
		    if (st.st_gid == groups[i])
		      {
			bit = S_IXGRP;
			break;
		      }
		}
	      if (st.st_mode & bit)
		{
		  file = name;
		  break;
		}
	    }
	}
      while (*p++ != '\0');
    }

  return __execve (file, argv, __environ);
}
