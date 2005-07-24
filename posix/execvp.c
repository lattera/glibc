/* Copyright (C) 1991,92,1995-99,2002,2004,2005 Free Software Foundation, Inc.
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

#include <unistd.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <paths.h>


/* The file is accessible but it is not an executable file.  Invoke
   the shell to interpret it as a script.  */
static char **
internal_function
allocate_scripts_argv (const char *file, char *const argv[])
{
  /* Count the arguments.  */
  int argc = 0;
  while (argv[argc++])
    ;

  /* Construct an argument list for the shell.  */
  char **new_argv = (char **) malloc ((argc + 1) * sizeof (char *));
  if (new_argv != NULL)
    {
      new_argv[0] = (char *) _PATH_BSHELL;
      new_argv[1] = (char *) file;
      while (argc > 1)
	{
	  new_argv[argc] = argv[argc - 1];
	  --argc;
	}
    }

  return new_argv;
}


/* Execute FILE, searching in the `PATH' environment variable if it contains
   no slashes, with arguments ARGV and environment from `environ'.  */
int
execvp (file, argv)
     const char *file;
     char *const argv[];
{
  if (*file == '\0')
    {
      /* We check the simple case first. */
      __set_errno (ENOENT);
      return -1;
    }

  char **script_argv = NULL;

  if (strchr (file, '/') != NULL)
    {
      /* Don't search when it contains a slash.  */
      __execve (file, argv, __environ);

      if (errno == ENOEXEC)
	{
	  script_argv = allocate_scripts_argv (file, argv);
	  if (script_argv != NULL)
	    {
	      __execve (script_argv[0], script_argv, __environ);

	      free (script_argv);
	    }
	}
    }
  else
    {
      char *path = getenv ("PATH");
      char *path_malloc = NULL;
      if (path == NULL)
	{
	  /* There is no `PATH' in the environment.
	     The default search path is the current directory
	     followed by the path `confstr' returns for `_CS_PATH'.  */
	  size_t len = confstr (_CS_PATH, (char *) NULL, 0);
	  path = (char *) malloc (1 + len);
	  if (path == NULL)
	    return -1;
	  path[0] = ':';
	  (void) confstr (_CS_PATH, path + 1, len);
	  path_malloc = path;
	}

      size_t len = strlen (file) + 1;
      size_t pathlen = strlen (path);
      char *name = malloc (pathlen + len + 1);
      if (name == NULL)
	{
	  free (path_malloc);
	  return -1;
	}
      /* Copy the file name at the top.  */
      name = (char *) memcpy (name + pathlen + 1, file, len);
      /* And add the slash.  */
      *--name = '/';

      bool got_eacces = false;
      char *p = path;
      do
	{
	  char *startp;

	  path = p;
	  p = __strchrnul (path, ':');

	  if (p == path)
	    /* Two adjacent colons, or a colon at the beginning or the end
	       of `PATH' means to search the current directory.  */
	    startp = name + 1;
	  else
	    startp = (char *) memcpy (name - (p - path), path, p - path);

	  /* Try to execute this name.  If it works, execve will not return. */
	  __execve (startp, argv, __environ);

	  if (errno == ENOEXEC)
	    {
	      if (script_argv == NULL)
		{
		  script_argv = allocate_scripts_argv (startp, argv);
		  if (script_argv == NULL)
		    {
		      /* A possible EACCES error is not as important as
			 the ENOMEM.  */
		      got_eacces = false;
		      break;
		    }
		}

	      __execve (script_argv[0], script_argv, __environ);
	    }

	  switch (errno)
	    {
	    case EACCES:
	      /* Record the we got a `Permission denied' error.  If we end
		 up finding no executable we can use, we want to diagnose
		 that we did find one but were denied access.  */
	      got_eacces = true;
	    case ENOENT:
	    case ESTALE:
	    case ENOTDIR:
	      /* Those errors indicate the file is missing or not executable
		 by us, in which case we want to just try the next path
		 directory.  */
	    case ENODEV:
	    case ETIMEDOUT:
	      /* Some strange filesystems like AFS return even
		 stranger error numbers.  They cannot reasonably mean
		 anything else so ignore those, too.  */
	      break;

	    default:
	      /* Some other error means we found an executable file, but
		 something went wrong executing it; return the error to our
		 caller.  */
	      return -1;
	    }
	}
      while (*p++ != '\0');

      /* We tried every element and none of them worked.  */
      if (got_eacces)
	/* At least one failure was due to permissions, so report that
           error.  */
	__set_errno (EACCES);

      free (script_argv);
      free (name - pathlen);
      free (path_malloc);
    }

  /* Return the error from the last attempt (probably ENOENT).  */
  return -1;
}
libc_hidden_def (execvp)
