/* Copyright (C) 1991, 92, 93, 94, 95, 96 Free Software Foundation, Inc.
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

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

#include "dirstream.h"

/* Open a directory stream on NAME.  */
DIR *
opendir (const char *name)
{
  DIR *dirp;
  struct stat statbuf;
  int fd;

  if (name[0] == '\0')
    {
      /* POSIX.1-1990 says an empty name gets ENOENT;
	 but `open' might like it fine.  */
      errno = ENOENT;
      return NULL;
    }

  fd = __open (name, O_RDONLY);
  if (fd < 0)
    return NULL;

  if (fcntl (fd, F_SETFD, FD_CLOEXEC) < 0)
    goto lose;

  if (fstat (fd, &statbuf) < 0)
    goto lose;
  if (! S_ISDIR (statbuf.st_mode))
    {
      errno = ENOTDIR;
      goto lose;
    }

  dirp = (DIR *) calloc (1, sizeof (DIR)); /* Zero-fill.  */
  if (dirp == NULL)
  lose:
    {
      int save = errno;
      (void) __close (fd);
      errno = save;
      return NULL;
    }

#ifdef _STATBUF_ST_BLKSIZE
  if (statbuf.st_blksize < sizeof (struct dirent))
    dirp->allocation = sizeof (struct dirent);
  else
    dirp->allocation = statbuf.st_blksize;
#else
  dirp->allocation = (BUFSIZ < sizeof (struct dirent) ?
		      sizeof (struct dirent) : BUFSIZ);
#endif
  dirp->data = (char *) malloc (dirp->allocation);
  if (dirp->data == NULL)
    {
      int save = errno;
      free (dirp);
      (void) __close (fd);
      errno = save;
      return NULL;
    }

  dirp->fd = fd;
  return dirp;
}
