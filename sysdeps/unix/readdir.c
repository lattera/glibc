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

#include <ansidecl.h>
#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>

#include "direct.h"		/* This file defines `struct direct'.  */
#include "dirstream.h"

/* direct.h may have an alternate definition for this.  */
#ifndef D_RECLEN
#define D_RECLEN(dp)	((dp)->d_reclen)
#endif


/* Read a directory entry from DIRP.  */
struct dirent *
DEFUN(readdir, (dirp), DIR *dirp)
{
  if (dirp == NULL || dirp->__data == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  while (1)
    {
      struct direct *dp;

      if (dirp->__offset >= dirp->__size)
	{
	  /* We've emptied out our buffer.  Refill it.  */

	  off_t base;
	  ssize_t bytes = __getdirentries (dirp->__fd, dirp->__data,
					   dirp->__allocation, &base);
	  if (bytes <= 0)
	    return NULL;
	  dirp->__size = (size_t) bytes;

	  /* Reset the offset into the buffer.  */
	  dirp->__offset = 0;
	}

      dp = (struct direct *) &dirp->__data[dirp->__offset];
      dirp->__offset += D_RECLEN (dp);

      if (dp->d_ino != 0)
	{
	  /* Not a deleted file.  */
	  register struct dirent *d = &dirp->__entry;
	  register const char *p;
	  d->d_fileno = (ino_t) dp->d_ino;
	  /* On some systems the name length does not actually mean much.
	     But we always use it as a maximum.  */
	  p = memchr ((PTR) dp->d_name, '\0', D_NAMLEN (dp) + 1);
	  d->d_namlen = (p != NULL) ? p - dp->d_name : D_NAMLEN (dp);
	  memcpy (d->d_name, dp->d_name, d->d_namlen + 1);
	  d->d_type = DT_UNKNOWN;
	  d->d_reclen = &d->d_name[d->d_namlen + 1] - (char *) d;
	  return d;
	}
    }
}
