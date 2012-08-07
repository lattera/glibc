/* Copyright (C) 1991,92,93,94,95,96,97,98,99,2000,02,10
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <errno.h>
#include <limits.h>
#include <stddef.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/types.h>
#include <assert.h>

#include <dirstream.h>

#ifndef __READDIR_R
# define __READDIR_R __readdir_r
# define __GETDENTS __getdents
# define DIRENT_TYPE struct dirent
# define __READDIR_R_ALIAS
#endif

/* Read a directory entry from DIRP.  */
int
__READDIR_R (DIR *dirp, DIRENT_TYPE *entry, DIRENT_TYPE **result)
{
  DIRENT_TYPE *dp;
  size_t reclen;
  const int saved_errno = errno;

  __libc_lock_lock (dirp->lock);

  do
    {
      if (dirp->offset >= dirp->size)
	{
	  /* We've emptied out our buffer.  Refill it.  */

	  size_t maxread;
	  ssize_t bytes;

#ifndef _DIRENT_HAVE_D_RECLEN
	  /* Fixed-size struct; must read one at a time (see below).  */
	  maxread = sizeof *dp;
#else
	  maxread = dirp->allocation;
#endif

	  bytes = __GETDENTS (dirp->fd, dirp->data, maxread);
	  if (bytes <= 0)
	    {
	      /* On some systems getdents fails with ENOENT when the
		 open directory has been rmdir'd already.  POSIX.1
		 requires that we treat this condition like normal EOF.  */
	      if (bytes < 0 && errno == ENOENT)
		{
		  bytes = 0;
		  __set_errno (saved_errno);
		}

	      dp = NULL;
	      /* Reclen != 0 signals that an error occurred.  */
	      reclen = bytes != 0;
	      break;
	    }
	  dirp->size = (size_t) bytes;

	  /* Reset the offset into the buffer.  */
	  dirp->offset = 0;
	}

      dp = (DIRENT_TYPE *) &dirp->data[dirp->offset];

#ifdef _DIRENT_HAVE_D_RECLEN
      reclen = dp->d_reclen;
#else
      /* The only version of `struct dirent*' that lacks `d_reclen'
	 is fixed-size.  */
      assert (sizeof dp->d_name > 1);
      reclen = sizeof *dp;
      /* The name is not terminated if it is the largest possible size.
	 Clobber the following byte to ensure proper null termination.  We
	 read just one entry at a time above so we know that byte will not
	 be used later.  */
      dp->d_name[sizeof dp->d_name] = '\0';
#endif

      dirp->offset += reclen;

#ifdef _DIRENT_HAVE_D_OFF
      dirp->filepos = dp->d_off;
#else
      dirp->filepos += reclen;
#endif

      /* Skip deleted files.  */
    }
  while (dp->d_ino == 0);

  if (dp != NULL)
    {
#ifdef GETDENTS_64BIT_ALIGNED
      /* The d_reclen value might include padding which is not part of
	 the DIRENT_TYPE data structure.  */
      reclen = MIN (reclen,
		    offsetof (DIRENT_TYPE, d_name) + sizeof (dp->d_name));
#endif
      *result = memcpy (entry, dp, reclen);
#ifdef GETDENTS_64BIT_ALIGNED
      entry->d_reclen = reclen;
#endif
    }
  else
    *result = NULL;

  __libc_lock_unlock (dirp->lock);

  return dp != NULL ? 0 : reclen ? errno : 0;
}

#ifdef __READDIR_R_ALIAS
weak_alias (__readdir_r, readdir_r)
#endif
