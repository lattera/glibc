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

#include <unistd.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

/* Execute FILE, searching in the `PATH' environment variable if it contains
   no slashes, with arguments ARGV and environment from `environ'.  */
int
execvp (file, argv)
     const char *file;
     char *const argv[];
{
  if (strchr (file, '/') != NULL)
    /* Don't search when it contains a slash.  */
    return execv (file, argv);
  else
    {
      char *path, *p, *name;
      size_t len;

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

	  /* Try to execute this name.  If it works, execv will not return.  */
	  execv (name, argv);
	  if (errno != ENOENT && errno != EACCES)
	    /* Those errors indicate the file is missing or not executable
	       by us, in which case we want to just try the next path
	       directory.  Some other error means we found an executable
	       file, but something went wrong executing it; return the
	       error to our caller.  */
	    return -1;
	}
      while (*p++ != '\0');
    }

  /* We tried every element and none of them worked.
     Return the error from the last attempt (probably ENOENT).  */
  return -1;
}
