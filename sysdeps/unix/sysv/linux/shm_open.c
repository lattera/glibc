/* Copyright (C) 2000, 2001, 2002, 2003, 2004 Free Software Foundation, Inc.
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
#include <fcntl.h>
#include <mntent.h>
#include <paths.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/statfs.h>
#include <bits/libc-lock.h>
#include "linux_fsinfo.h"


/* Mount point of the shared memory filesystem.  */
static struct
{
  char *dir;
  size_t dirlen;
} mountpoint;

/* This is the default directory.  */
static const char defaultdir[] = "/dev/shm/";

/* Protect the `mountpoint' variable above.  */
__libc_once_define (static, once);


/* Determine where the shmfs is mounted (if at all).  */
static void
where_is_shmfs (void)
{
  char buf[512];
  struct statfs f;
  struct mntent resmem;
  struct mntent *mp;
  FILE *fp;

  /* The canonical place is /dev/shm.  This is at least what the
     documentation tells everybody to do.  */
  if (__statfs (defaultdir, &f) == 0 && f.f_type == SHMFS_SUPER_MAGIC)
    {
      /* It is in the normal place.  */
      mountpoint.dir = (char *) defaultdir;
      mountpoint.dirlen = sizeof (defaultdir) - 1;

      return;
    }

  /* OK, do it the hard way.  Look through the /proc/mounts file and if
     this does not exist through /etc/fstab to find the mount point.  */
  fp = __setmntent ("/proc/mounts", "r");
  if (__builtin_expect (fp == NULL, 0))
    {
      fp = __setmntent (_PATH_MNTTAB, "r");
      if (__builtin_expect (fp == NULL, 0))
	/* There is nothing we can do.  Blind guesses are not helpful.  */
	return;
    }

  /* Now read the entries.  */
  while ((mp = __getmntent_r (fp, &resmem, buf, sizeof buf)) != NULL)
    /* The original name is "shm" but this got changed in early Linux
       2.4.x to "tmpfs".  */
    if (strcmp (mp->mnt_type, "tmpfs") == 0
	|| strcmp (mp->mnt_type, "shm") == 0)
      {
	/* Found it.  There might be more than one place where the
           filesystem is mounted but one is enough for us.  */
	size_t namelen;

	/* First make sure this really is the correct entry.  At least
	   some versions of the kernel give wrong information because
	   of the implicit mount of the shmfs for SysV IPC.  */
	if (__statfs (mp->mnt_dir, &f) != 0 || f.f_type != SHMFS_SUPER_MAGIC)
	  continue;

	namelen = strlen (mp->mnt_dir);

	if (namelen == 0)
	  /* Hum, maybe some crippled entry.  Keep on searching.  */
	  continue;

	mountpoint.dir = (char *) malloc (namelen + 2);
	if (mountpoint.dir != NULL)
	  {
	    char *cp = __mempcpy (mountpoint.dir, mp->mnt_dir, namelen);
	    if (cp[-1] != '/')
	      *cp++ = '/';
	    *cp = '\0';
	    mountpoint.dirlen = cp - mountpoint.dir;
	  }

	break;
      }

  /* Close the stream.  */
  __endmntent (fp);
}


/* Open shared memory object.  This implementation assumes the shmfs
   implementation introduced in the late 2.3.x kernel series to be
   available.  Normally the filesystem will be mounted at /dev/shm but
   we fall back on searching for the actual mount point should opening
   such a file fail.  */
int
shm_open (const char *name, int oflag, mode_t mode)
{
  size_t namelen;
  char *fname;
  int fd;

  /* Determine where the shmfs is mounted.  */
  __libc_once (once, where_is_shmfs);

  /* If we don't know the mount points there is nothing we can do.  Ever.  */
  if (mountpoint.dir == NULL)
    {
      __set_errno (ENOSYS);
      return -1;
    }

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
  fname = (char *) alloca (mountpoint.dirlen + namelen + 1);
  __mempcpy (__mempcpy (fname, mountpoint.dir, mountpoint.dirlen),
	     name, namelen + 1);

  /* And get the file descriptor.
     XXX Maybe we should test each descriptor whether it really is for a
     file on the shmfs.  If this is what should be done the whole function
     should be revamped since we can determine whether shmfs is available
     while trying to open the file, all in one turn.  */
  fd = open (fname, oflag | O_NOFOLLOW, mode);
  if (fd != -1)
    {
      /* We got a descriptor.  Now set the FD_CLOEXEC bit.  */
      int flags = fcntl (fd, F_GETFD, 0);

      if (__builtin_expect (flags, 0) >= 0)
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
  else if (__builtin_expect (errno == EISDIR, 0))
    /* It might be better to fold this error with EINVAL since
       directory names are just another example for unsuitable shared
       object names and the standard does not mention EISDIR.  */
    __set_errno (EINVAL);

  return fd;
}


/* Unlink a shared memory object.  */
int
shm_unlink (const char *name)
{
  size_t namelen;
  char *fname;

  /* Determine where the shmfs is mounted.  */
  __libc_once (once, where_is_shmfs);

  if (mountpoint.dir == NULL)
    {
      /* We cannot find the shmfs.  If `name' is really a shared
	 memory object it must have been created by another process
	 and we have no idea where that process found the mountpoint.  */
      __set_errno (ENOENT);
      return -1;
    }

  /* Construct the filename.  */
  while (name[0] == '/')
    ++name;

  if (name[0] == '\0')
    {
      /* The name "/" is not supported.  */
      __set_errno (ENOENT);
      return -1;
    }

  namelen = strlen (name);
  fname = (char *) alloca (mountpoint.dirlen + namelen + 1);
  __mempcpy (__mempcpy (fname, mountpoint.dir, mountpoint.dirlen),
	     name, namelen + 1);

  /* And remove the file.  */
  int ret = unlink (fname);
  if (ret < 0 && errno == EPERM)
    __set_errno (EACCES);
  return ret;
}


/* Make sure the table is freed if we want to free everything before
   exiting.  */
libc_freeres_fn (freeit)
{
  if (mountpoint.dir != defaultdir)
    free (mountpoint.dir);
}
