/* Return the canonical absolute name of a given file inside chroot.
   Copyright (C) 1996, 1997, 1998, 1999, 2000, 2001, 2004
   Free Software Foundation, Inc.
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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <errno.h>
#include <stddef.h>
#include <stdint.h>

#include "ldconfig.h"

#ifndef PATH_MAX
#define PATH_MAX 1024
#endif

/* Return the canonical absolute name of file NAME as if chroot(CHROOT) was
   done first.  A canonical name does not contain any `.', `..' components
   nor any repeated path separators ('/') or symlinks.  All path components
   must exist and NAME must be absolute filename.  The result is malloc'd.
   The returned name includes the CHROOT prefix.  */

char *
chroot_canon (const char *chroot, const char *name)
{
  char *rpath;
  char *dest;
  char *extra_buf = NULL;
  char *rpath_root;
  const char *start;
  const char *end;
  const char *rpath_limit;
  int num_links = 0;
  size_t chroot_len = strlen (chroot);

  if (chroot_len < 1)
    {
      __set_errno (EINVAL);
      return NULL;
    }

  rpath = malloc (chroot_len + PATH_MAX);
  if (rpath == NULL)
    return NULL;

  rpath_limit = rpath + chroot_len + PATH_MAX;

  rpath_root = (char *) mempcpy (rpath, chroot, chroot_len) - 1;
  if (*rpath_root != '/')
    *++rpath_root = '/';
  dest = rpath_root + 1;

  for (start = end = name; *start; start = end)
    {
      struct stat64 st;
      int n;

      /* Skip sequence of multiple path-separators.  */
      while (*start == '/')
	++start;

      /* Find end of path component.  */
      for (end = start; *end && *end != '/'; ++end)
	/* Nothing.  */;

      if (end - start == 0)
	break;
      else if (end - start == 1 && start[0] == '.')
	/* nothing */;
      else if (end - start == 2 && start[0] == '.' && start[1] == '.')
	{
	  /* Back up to previous component, ignore if at root already.  */
	  if (dest > rpath_root + 1)
	    while ((--dest)[-1] != '/');
	}
      else
	{
	  size_t new_size;

	  if (dest[-1] != '/')
	    *dest++ = '/';

	  if (dest + (end - start) >= rpath_limit)
	    {
	      ptrdiff_t dest_offset = dest - rpath;
	      char *new_rpath;

	      new_size = rpath_limit - rpath;
	      if (end - start + 1 > PATH_MAX)
		new_size += end - start + 1;
	      else
		new_size += PATH_MAX;
	      new_rpath = (char *) realloc (rpath, new_size);
	      if (new_rpath == NULL)
		goto error;
	      rpath = new_rpath;
	      rpath_limit = rpath + new_size;

	      dest = rpath + dest_offset;
	    }

	  dest = mempcpy (dest, start, end - start);
	  *dest = '\0';

	  if (lstat64 (rpath, &st) < 0)
	    {
	      if (*end == '\0')
		goto done;
	      goto error;
	    }

	  if (S_ISLNK (st.st_mode))
	    {
	      char *buf = alloca (PATH_MAX);
	      size_t len;

	      if (++num_links > MAXSYMLINKS)
		{
		  __set_errno (ELOOP);
		  goto error;
		}

	      n = readlink (rpath, buf, PATH_MAX);
	      if (n < 0)
		{
		  if (*end == '\0')
		    goto done;
		  goto error;
		}
	      buf[n] = '\0';

	      if (!extra_buf)
		extra_buf = alloca (PATH_MAX);

	      len = strlen (end);
	      if ((long int) (n + len) >= PATH_MAX)
		{
		  __set_errno (ENAMETOOLONG);
		  goto error;
		}

	      /* Careful here, end may be a pointer into extra_buf... */
	      memmove (&extra_buf[n], end, len + 1);
	      name = end = memcpy (extra_buf, buf, n);

	      if (buf[0] == '/')
		dest = rpath_root + 1;	/* It's an absolute symlink */
	      else
		/* Back up to previous component, ignore if at root already: */
		if (dest > rpath_root + 1)
		  while ((--dest)[-1] != '/');
	    }
	}
    }
 done:
  if (dest > rpath_root + 1 && dest[-1] == '/')
    --dest;
  *dest = '\0';

  return rpath;

 error:
  free (rpath);
  return NULL;
}
