/* Copyright (C) 1991-1996,98,2000-2003,2005,2007,2009,2011
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

#include <assert.h>
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
#include <string.h>

#include <dirstream.h>
#include <not-cancel.h>
#include <kernel-features.h>


/* opendir() must not accidentally open something other than a directory.
   Some OS's have kernel support for that, some don't.  In the worst
   case we have to stat() before the open() AND fstat() after.

   We have to test at runtime for kernel support since libc may have
   been compiled with different headers to the kernel it's running on.
   This test can't be done reliably in the general case.  We'll use
   /dev/null, which if it's not a device lots of stuff will break, as
   a guinea pig.  It may be missing in chroot environments, so we
   make sure to fail safe. */
#ifdef O_DIRECTORY
# ifdef O_DIRECTORY_WORKS
#  define o_directory_works 1
#  define tryopen_o_directory() while (1) /* This must not be called.  */
# else
static int o_directory_works;

static void
tryopen_o_directory (void)
{
  int serrno = errno;
  int x = open_not_cancel_2 ("/dev/null", O_RDONLY|O_NDELAY|O_DIRECTORY);

  if (x >= 0)
    {
      close_not_cancel_no_status (x);
      o_directory_works = -1;
    }
  else if (errno != ENOTDIR)
    o_directory_works = -1;
  else
    o_directory_works = 1;

  __set_errno (serrno);
}
# endif
# define EXTRA_FLAGS O_DIRECTORY
#else
# define EXTRA_FLAGS 0
#endif


DIR *
internal_function
__opendirat (int dfd, const char *name)
{
  struct stat64 statbuf;
  struct stat64 *statp = NULL;

  if (__builtin_expect (name[0], '\1') == '\0')
    {
      /* POSIX.1-1990 says an empty name gets ENOENT;
	 but `open' might like it fine.  */
      __set_errno (ENOENT);
      return NULL;
    }

#ifdef O_DIRECTORY
  /* Test whether O_DIRECTORY works.  */
  if (o_directory_works == 0)
    tryopen_o_directory ();

  /* We can skip the expensive `stat' call if O_DIRECTORY works.  */
  if (o_directory_works < 0)
#endif
    {
      /* We first have to check whether the name is for a directory.  We
	 cannot do this after the open() call since the open/close operation
	 performed on, say, a tape device might have undesirable effects.  */
      if (__builtin_expect (__xstat64 (_STAT_VER, name, &statbuf), 0) < 0)
	return NULL;
      if (__builtin_expect (! S_ISDIR (statbuf.st_mode), 0))
	{
	  __set_errno (ENOTDIR);
	  return NULL;
	 }
    }

  int flags = O_RDONLY|O_NDELAY|EXTRA_FLAGS|O_LARGEFILE;
#ifdef O_CLOEXEC
  flags |= O_CLOEXEC;
#endif
  int fd;
#ifdef IS_IN_rtld
  assert (dfd == AT_FDCWD);
  fd = open_not_cancel_2 (name, flags);
#else
  fd = openat_not_cancel_3 (dfd, name, flags);
#endif
  if (__builtin_expect (fd, 0) < 0)
    return NULL;

#ifdef O_DIRECTORY
  if (o_directory_works <= 0)
#endif
    {
      /* Now make sure this really is a directory and nothing changed since
	 the `stat' call.  */
      if (__builtin_expect (__fxstat64 (_STAT_VER, fd, &statbuf), 0) < 0)
	goto lose;
      if (__builtin_expect (! S_ISDIR (statbuf.st_mode), 0))
	{
	  __set_errno (ENOTDIR);
	lose:
	  close_not_cancel_no_status (fd);
	  return NULL;
	}
      statp = &statbuf;
    }

  return __alloc_dir (fd, true, 0, statp);
}


/* Open a directory stream on NAME.  */
DIR *
__opendir (const char *name)
{
  return __opendirat (AT_FDCWD, name);
}
weak_alias (__opendir, opendir)


#ifdef __ASSUME_O_CLOEXEC
# define check_have_o_cloexec(fd) 1
#else
static int
check_have_o_cloexec (int fd)
{
  if (__have_o_cloexec == 0)
    __have_o_cloexec = (__fcntl (fd, F_GETFD, 0) & FD_CLOEXEC) == 0 ? -1 : 1;
  return __have_o_cloexec > 0;
}
#endif


DIR *
internal_function
__alloc_dir (int fd, bool close_fd, int flags, const struct stat64 *statp)
{
  /* We always have to set the close-on-exit flag if the user provided
     the file descriptor.  Otherwise only if we have no working
     O_CLOEXEC support.  */
#ifdef O_CLOEXEC
  if ((! close_fd && (flags & O_CLOEXEC) == 0)
      || ! check_have_o_cloexec (fd))
#endif
    {
      if (__builtin_expect (__fcntl (fd, F_SETFD, FD_CLOEXEC), 0) < 0)
	goto lose;
    }

  const size_t default_allocation = (4 * BUFSIZ < sizeof (struct dirent64)
				     ? sizeof (struct dirent64) : 4 * BUFSIZ);
  const size_t small_allocation = (BUFSIZ < sizeof (struct dirent64)
				   ? sizeof (struct dirent64) : BUFSIZ);
  size_t allocation = default_allocation;
#ifdef _STATBUF_ST_BLKSIZE
  if (statp != NULL && default_allocation < statp->st_blksize)
    allocation = statp->st_blksize;
#endif

  DIR *dirp = (DIR *) malloc (sizeof (DIR) + allocation);
  if (dirp == NULL)
    {
      allocation = small_allocation;
      dirp = (DIR *) malloc (sizeof (DIR) + allocation);

      if (dirp == NULL)
      lose:
	{
	  if (close_fd)
	    {
	      int save_errno = errno;
	      close_not_cancel_no_status (fd);
	      __set_errno (save_errno);
	    }
	  return NULL;
	}
    }

  dirp->fd = fd;
#ifndef NOT_IN_libc
  __libc_lock_init (dirp->lock);
#endif
  dirp->allocation = allocation;
  dirp->size = 0;
  dirp->offset = 0;
  dirp->filepos = 0;

  return dirp;
}
