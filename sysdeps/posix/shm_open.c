/* shm_open -- open a POSIX shared memory object.  Generic POSIX file version.
   Copyright (C) 2001,02 Free Software Foundation, Inc.
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

#include <unistd.h>

#if ! _POSIX_MAPPED_FILES
#include <sysdeps/generic/shm_open.c>

#else

#include <errno.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <paths.h>

#define SHMDIR	(_PATH_DEV "shm/")

/* Open shared memory object.  */
int
shm_open (const char *name, int oflag, mode_t mode)
{
  size_t namelen;
  char *fname;
  int fd;

  /* Construct the filename.  */
  while (name[0] == '/')
    ++name;

  if (name[0] == '\0')
    {
      /* The name "/" is not supported.  */
      __set_errno (EINVAL);
      return -1;
    }

  namelen = strlen (name);
  fname = (char *) __alloca (sizeof SHMDIR - 1 + namelen + 1);
  __mempcpy (__mempcpy (fname, SHMDIR, sizeof SHMDIR - 1),
	     name, namelen + 1);

  fd = open (name, oflag, mode);
  if (fd != -1)
    {
      /* We got a descriptor.  Now set the FD_CLOEXEC bit.  */
      int flags = fcntl (fd, F_GETFD, 0);

      if (__builtin_expect (flags, 0) != -1)
	{
	  flags |= FD_CLOEXEC;
	  flags = fcntl (fd, F_SETFD, flags);
	}

      if (flags == -1)
	{
	  /* Something went wrong.  We cannot return the descriptor.  */
	  int save_errno = errno;
	  close (fd);
	  fd = -1;
	  __set_errno (save_errno);
	}
    }

  return fd;
}

#endif
