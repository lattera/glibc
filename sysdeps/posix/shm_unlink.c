/* shm_unlink -- remove a POSIX shared memory object.  Generic POSIX version.
   Copyright (C) 2001,2002,2005 Free Software Foundation, Inc.
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
#include <rt/shm_unlink.c>

#else

#include <errno.h>
#include <sys/mman.h>
#include <string.h>
#include <stdlib.h>
#include <paths.h>

#define SHMDIR	(_PATH_DEV "shm/")

/* Remove shared memory object.  */
int
shm_unlink (const char *name)
{
  size_t namelen;
  char *fname;

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

  return unlink (name);
}

#endif
