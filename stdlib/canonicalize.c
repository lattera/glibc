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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/stat.h>
#include <errno.h>

/* Return the canonical absolute name of file NAME.  The last file name
   component need not exist, and may be a symlink to a nonexistent file.
   If RESOLVED is null, the result is malloc'd; otherwise, if the canonical
   name is PATH_MAX chars or more, returns null with `errno' set to
   ENAMETOOLONG; if the name fits in fewer than PATH_MAX chars, returns the
   name in RESOLVED.  */

static char *
canonicalize (const char *name, char *resolved)
{
  struct stat st;
  const char *p;
  long int path_max;
  char *result, *dir, *end;
  size_t namelen;

  if (! resolved)
    path_max = 0;
  else
    {
#ifdef PATH_MAX
      path_max = PATH_MAX;
#else
      path_max = sysconf (_SC_PATH_MAX);
      if (path_max <= 0)
	path_max = 1024;
#endif
    }

  p = strrchr (name, '/');
  if (!p)
    {
      dir = (char *) ".";
      p = name;
    }
  else
    {
      if (p++ == name)
	dir = (char *) "/";
      else
	{
	  dir = __alloca (p - name);
	  memcpy (dir, name, p - name - 1);
	  dir[p - name] = '\0';
	}
    }

  result = __canonicalize_directory_name_internal (dir, resolved, path_max);
  if (!result)
    return NULL;

  /* Reconstruct the file name in the canonicalized directory.  */
  namelen = strlen (name);
  end = strchr (result, '\0');
  if (resolved)
    {
      /* Make sure the name is not too long.  */
      if (end - result + namelen > path_max)
	{
	  errno = ENAMETOOLONG;
	  return NULL;
	}
    }
  else
    {
      /* The name is dynamically allocated.  Extend it.  */
      char *new = realloc (result, end - result + namelen + 1);
      if (! new)
	{
	  free (result);
	  return NULL;
	}
      end = new + (end - result);
      result = new;
    }
  memcpy (end, name, namelen + 1);

  while (__lstat (result, &st) == 0 && S_ISLNK (st.st_mode))
    {
      /* The file is a symlink.  Read its contents.  */
      ssize_t n;
    read_link_contents:
      n = readlink (result, end,
		    resolved ? result + path_max - end : namelen + 1);
      if (n < 0)
	/* Error reading the link contents.  */
	return NULL;

      if (end[0] == '/')
	{
	  /* Absolute symlink.  */
	  if (resolved ? (end + n < result + path_max) : (n < namelen + 1))
	    {
	      /* It fit in our buffer, so we have the whole thing.  */
	      memcpy (result, end, n);
	      result[n] = '\0';
	    }
	  else if (resolved)
	    {
	      /* It didn't fit in the remainder of the buffer.  Either it
		 fits in the entire buffer, or it doesn't.  Copy back the
		 unresolved name onto the canonical directory and try once
		 more.  */
	      memcpy (end, name, namelen + 1);
	      n = readlink (result, result, path_max);
	      if (n < 0)
		return NULL;
	      if (n == path_max)
		{
		  errno = ENAMETOOLONG;
		  return NULL;
		}
	      result[n] = '\0';
	    }
	  else
	    /* Try again with a bigger buffer.  */
	    goto extend_buffer;

	  /* Check the resolved name for being a symlink too.  */
	  continue;
	}

      if (resolved)
	{
	  if (end + n == result + path_max)
	    {
	      /* The link contents we read fill the buffer completely.
		 There may be even more to read, and there is certainly no
		 space for the null terminator.  */
	      errno = ENAMETOOLONG;
	      return NULL;
	    }
	}
      else if (n == namelen + 1)
      extend_buffer:
	{
	  /* The name buffer is dynamically allocated.  Extend it.  */
	  char *new;

	  /* Copy back the unresolved name onto the canonical directory.  */
	  memcpy (end, name, namelen + 1);

	  /* Make more space for readlink.  */
	  namelen *= 2;
	  new = realloc (result, end - result + namelen + 1);
	  if (! new)
	    {
	      free (result);
	      return NULL;
	    }
	  end = new + (end - result);
	  result = new;

	  goto read_link_contents;
	}

      /* Terminate the string; readlink does not.  */
      end[n] = '\0';
    }

  return result;
}

weak_alias (canonicalize, realpath)

char *
canonicalize_file_name (const char *name)
{
  return canonicalize (name, NULL);
}
