/* Return the canonical absolute name of a given file.
   Copyright (C) 1996 Free Software Foundation, Inc.
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>

/* Return the canonical absolute name of file NAME.  A canonical name
   does not contain any `.', `..' components nor any repeated path
   separators ('/') or symlinks.  All path components must exist.  If
   RESOLVED is null, the result is malloc'd; otherwise, if the
   canonical name is PATH_MAX chars or more, returns null with `errno'
   set to ENAMETOOLONG; if the name fits in fewer than PATH_MAX chars,
   returns the name in RESOLVED.  If the name cannot be resolved and
   RESOLVED is non-NULL, it contains the path of the first component
   that cannot be resolved.  If the path can be resolved, RESOLVED
   holds the same value as the value returned.  */

static char *
canonicalize (const char *name, char *resolved)
{
  char *rpath, *dest, *extra_buf = NULL;
  const char *start, *end, *rpath_limit;
  long int path_max;
  int num_links = 0;

#ifdef PATH_MAX
  path_max = PATH_MAX;
#else
  path_max = pathconf (name, _PC_PATH_MAX);
  if (path_max <= 0)
    path_max = 1024;
#endif

  rpath = resolved ? __alloca (path_max) : malloc (path_max);
  rpath_limit = rpath + path_max;

  if (name[0] != '/')
    {
      if (!getcwd (rpath, path_max))
	goto error;
      dest = strchr (rpath, '\0');
    }
  else
    {
      rpath[0] = '/';
      dest = rpath + 1;
    }

  for (start = end = name; *start; start = end)
    {
      struct stat st;
      int n;

      /* skip sequence of multiple path-separators: */
      while (*start == '/') ++start;

      /* find end of path component: */
      for (end = start; *end && *end != '/'; ++end);

      if (end - start == 0)
	break;
      else if (strncmp (start, ".", end - start) == 0)
	/* nothing */;
      else if (strncmp (start, "..", end - start) == 0) {
	/* back up to previous component, ignore if at root already: */
	if (dest > rpath + 1)
	  while ((--dest)[-1] != '/');
      } else
	{
	  size_t new_size;

	  if (dest[-1] != '/')
	    *dest++ = '/';

	  if (dest + (end - start) >= rpath_limit)
	    {
	      if (resolved)
		{
		  __set_errno (ENAMETOOLONG);
		  goto error;
		}
	      new_size = rpath_limit - rpath;
	      if (end - start + 1 > path_max)
		new_size += end - start + 1;
	      else
		new_size += path_max;
	      rpath = realloc (rpath, new_size);
	      rpath_limit = rpath + new_size;
	      if (!rpath)
		return NULL;
	    }

	  memcpy (dest, start, end - start);
	  dest += end - start;
	  *dest = '\0';

	  if (__lstat (rpath, &st) < 0)
	    goto error;

	  if (S_ISLNK (st.st_mode))
	    {
	      char *buf = __alloca (path_max);

	      if (++num_links > MAXSYMLINKS)
		{
		  __set_errno (ELOOP);
		  goto error;
		}

	      n = readlink (rpath, buf, path_max);
	      if (n < 0)
		goto error;
	      buf[n] = '\0';

	      if (!extra_buf)
		extra_buf = __alloca (path_max);

	      if ((long int) (n + strlen (end)) >= path_max)
		{
		  __set_errno (ENAMETOOLONG);
		  goto error;
		}

	      /* careful here, end may be a pointer into extra_buf... */
	      strcat (buf, end);
	      strcpy (extra_buf, buf);
	      name = end = extra_buf;

	      if (buf[0] == '/')
		dest = rpath + 1;	/* it's an absolute symlink */
	      else
		/* back up to previous component, ignore if at root already: */
		if (dest > rpath + 1)
		  while ((--dest)[-1] != '/');
	    }
	  else
	    num_links = 0;
	}
    }
  if (dest > rpath + 1 && dest[-1] == '/')
    --dest;
  *dest = '\0';

  return resolved ? strcpy (resolved, rpath) : rpath;

error:
  if (resolved)
    strcpy (resolved, rpath);
  else
    free (rpath);
  return NULL;
}
weak_alias (canonicalize, realpath)


char *
canonicalize_file_name (const char *name)
{
  return canonicalize (name, NULL);
}
