/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>

char *__ttyname = NULL;

/* Return the pathname of the terminal FD is open on, or NULL on errors.
   The returned storage is good only until the next call to this function.  */
char *
DEFUN(ttyname, (fd), int fd)
{
  static CONST char dev[] = "/dev";
  static char *name;
  static size_t namelen = 0;
  struct stat st;
  dev_t mydev;
  ino_t myino;
  DIR *dirstream;
  struct dirent *d;
  int save = errno;

  if (fstat (fd, &st) < 0)
    return NULL;
  mydev = st.st_dev;
  myino = st.st_ino;

  dirstream = opendir (dev);
  if (dirstream == NULL)
    return NULL;

  while ((d = readdir (dirstream)) != NULL)
    if (d->d_fileno == myino)
      {
	if (sizeof (dev) + d->d_namlen + 1 > namelen)
	  {
	    free (name);
	    namelen = 2 * (sizeof (dev) + d->d_namlen + 1); /* Big enough.  */
	    name = malloc (namelen);
	    if (! name)
	      return NULL;
	    (void) memcpy (name, dev, sizeof (dev) - 1);
	    name[sizeof (dev) - 1] = '/';
	  }
	(void) memcpy (&name[sizeof (dev)], d->d_name, d->d_namlen + 1);
	if (stat (name, &st) == 0 && st.st_dev == mydev)
	  {
	    (void) closedir (dirstream);
	    __ttyname = name;
	    errno = save;
	    return name;
	  }
      }

  (void) closedir (dirstream);
  errno = save;
  return NULL;
}
