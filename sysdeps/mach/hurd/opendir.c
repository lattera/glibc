/* Copyright (C) 1993,94,95,96,97,98,2001,2003 Free Software Foundation, Inc.
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

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <hurd.h>
#include <hurd/fd.h>
#include "dirstream.h"


/* Open a directory stream on NAME.  */
DIR *
__opendir (const char *name)
{
  DIR *dirp;
  int fd;
  struct hurd_fd *d;

  if (name[0] == '\0')
    {
      /* POSIX.1-1990 says an empty name gets ENOENT;
	 but `open' might like it fine.  */
      __set_errno (ENOENT);
      return NULL;
    }

  {
    /* Append trailing slash to directory name to force ENOTDIR
       if it's not a directory.

       We open using the O_NONBLOCK flag so that a nondirectory with
       blocking behavior (FIFO or device) gets ENOTDIR immediately
       rather than waiting for the special file's open wakeup predicate.  */

    size_t len = strlen (name);
    if (name[len - 1] == '/')
      fd = __open (name, O_RDONLY | O_NONBLOCK);
    else
      {
	char n[len + 2];
	memcpy (n, name, len);
	n[len] = '/';
	n[len + 1] = '\0';
	fd = __open (n, O_RDONLY | O_NONBLOCK);
      }
  }
  if (fd < 0)
    return NULL;

  dirp = (DIR *) malloc (sizeof (DIR));
  if (dirp == NULL)
    {
      __close (fd);
      return NULL;
    }

  /* Extract the pointer to the descriptor structure.  */
  __mutex_lock (&_hurd_dtable_lock);
  d = dirp->__fd = _hurd_dtable[fd];
  __mutex_unlock (&_hurd_dtable_lock);

  /* Set the descriptor to close on exec. */
  __spin_lock (&d->port.lock);
  d->flags |= FD_CLOEXEC;
  __spin_unlock (&d->port.lock);

  dirp->__data = dirp->__ptr = NULL;
  dirp->__entry_data = dirp->__entry_ptr = 0;
  dirp->__allocation = 0;
  dirp->__size = 0;

  __libc_lock_init (dirp->__lock);

  return dirp;
}
weak_alias (__opendir, opendir)
