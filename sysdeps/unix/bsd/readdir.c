/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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
#include "direct.h"


/* Read a directory entry from DIRP.  */
struct dirent *
DEFUN(readdir, (dirp), DIR *dirp)
{
  struct dirent *dp;

  if (dirp == NULL || dirp->__data == NULL)
    {
      errno = EINVAL;
      return NULL;
    }

  do
    {
      if (dirp->__offset >= dirp->__size)
	{
	  /* We've emptied out our buffer.  Refill it.  */

	  ssize_t bytes = __getdirentries (dirp->__fd, dirp->__data,
					   dirp->__allocation, &dirp->__pos);
	  if (bytes <= 0)
	    return NULL;
	  dirp->__size = (size_t) bytes;

	  /* Reset the offset into the buffer.  */
	  dirp->__offset = 0;
	}

      dp = (struct dirent *) &dirp->__data[dirp->__offset];
      dirp->__offset += dp->d_reclen;

#ifndef HAVE_D_TYPE
      dp->d_namlen = ((struct direct *) dp)->d_namlen;
      dp->d_type = DT_UNKNOWN;
#endif

      /* Loop to ignore deleted files.  */
    } while (dp->d_fileno == 0);

  return dp;
}
