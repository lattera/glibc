/* Copyright (C) 1998-2003, 2004 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

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
#include <mntent.h>
#include <paths.h>
#include <stdbool.h>
#include <stdio_ext.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>
#include "linux_fsinfo.h"


#ifndef STATFS
# define STATFS statfs
# define STATVFS statvfs
# define INTERNAL_STATVFS __internal_statvfs


int
__statvfs_getflags (const char *name, int fstype, struct stat64 *st)
{
  if (st == NULL)
    return 0;

  const char *fsname = NULL;
  const char *fsname2 = NULL;

  /* Map the filesystem type we got from the statfs call to a string.  */
  switch (fstype)
    {
    case EXT2_SUPER_MAGIC:
      fsname = "ext3";
      fsname2 = "ext2";
      break;
    case DEVPTS_SUPER_MAGIC:
      fsname= "devpts";
      break;
    case SHMFS_SUPER_MAGIC:
      fsname = "tmpfs";
      break;
    case PROC_SUPER_MAGIC:
      fsname = "proc";
      break;
    case USBDEVFS_SUPER_MAGIC:
      fsname = "usbdevfs";
      break;
    case AUTOFS_SUPER_MAGIC:
      fsname = "autofs";
      break;
    case NFS_SUPER_MAGIC:
      fsname = "nfs";
      break;
    }

  FILE *mtab = __setmntent ("/proc/mounts", "r");
  if (mtab == NULL)
    mtab = __setmntent (_PATH_MOUNTED, "r");

  int result = 0;
  if (mtab != NULL)
    {
      bool success = false;
      struct mntent mntbuf;
      char tmpbuf[1024];

      /* No locking needed.  */
      (void) __fsetlocking (mtab, FSETLOCKING_BYCALLER);

    again:
      while (__getmntent_r (mtab, &mntbuf, tmpbuf, sizeof (tmpbuf)))
	{
	  /* In a first round we look for a given mount point, if
	     we have a name.  */
	  if (name != NULL && strcmp (name, mntbuf.mnt_dir) != 0)
	    continue;
	  /* We need to look at the entry only if the filesystem
	     name matches.  If we have a filesystem name.  */
	  else if (fsname != NULL
		   && strcmp (fsname, mntbuf.mnt_type) != 0
		   && (fsname2 == NULL
		       || strcmp (fsname2, mntbuf.mnt_type) != 0))
	    continue;

	  /* Find out about the device the current entry is for.  */
	  struct stat64 fsst;
	  if (stat64 (mntbuf.mnt_dir, &fsst) >= 0
	      && st->st_dev == fsst.st_dev)
	    {
	      /* Bingo, we found the entry for the device FD is on.
		 Now interpret the option string.  */
	      char *cp = mntbuf.mnt_opts;
	      char *opt;

	      while ((opt = strsep (&cp, ",")) != NULL)
		if (strcmp (opt, "ro") == 0)
		  result |= ST_RDONLY;
		else if (strcmp (opt, "nosuid") == 0)
		  result |= ST_NOSUID;
		else if (strcmp (opt, "noexec") == 0)
		  result |= ST_NOEXEC;
		else if (strcmp (opt, "nodev") == 0)
		  result |= ST_NODEV;
		else if (strcmp (opt, "sync") == 0)
		  result |= ST_SYNCHRONOUS;
		else if (strcmp (opt, "mand") == 0)
		  result |= ST_MANDLOCK;
		else if (strcmp (opt, "noatime") == 0)
		  result |= ST_NOATIME;
		else if (strcmp (opt, "nodiratime") == 0)
		  result |= ST_NODIRATIME;

	      /* We can stop looking for more entries.  */
	      success = true;
	      break;
	    }
	}
      /* Maybe the kernel names for the filesystems changed or the
	 statvfs call got a name which was not the mount point.  Check
	 again, this time without checking for name matches first.  */
      if (! success && (name != NULL || fsname != NULL))
	{
	  if (name != NULL)
	    /* Try without a mount point name.  */
	    name = NULL;
	  else
	    {
	      /* Try without a filesystem name.  */
	      assert (fsname != NULL);
	      fsname = fsname2 = NULL;
	    }

	  /* It is not strictly allowed to use rewind here.  But
	     this code is part of the implementation so it is
	     acceptable.  */
	  rewind (mtab);

	  goto again;
	}

      /* Close the file.  */
      __endmntent (mtab);
    }

  return result;
}
#else
extern int __statvfs_getflags (const char *name, int fstype,
			       struct stat64 *st);
#endif


void
INTERNAL_STATVFS (const char *name, struct STATVFS *buf,
		  struct STATFS *fsbuf, struct stat64 *st)
{
  /* Now fill in the fields we have information for.  */
  buf->f_bsize = fsbuf->f_bsize;
  /* Linux has the f_frsize size only in later version of the kernel.
     If the value is not filled in use f_bsize.  */
  buf->f_frsize = fsbuf->f_frsize ?: fsbuf->f_bsize;
  buf->f_blocks = fsbuf->f_blocks;
  buf->f_bfree = fsbuf->f_bfree;
  buf->f_bavail = fsbuf->f_bavail;
  buf->f_files = fsbuf->f_files;
  buf->f_ffree = fsbuf->f_ffree;
  if (sizeof (buf->f_fsid) == sizeof (fsbuf->f_fsid))
    buf->f_fsid = (fsbuf->f_fsid.__val[0]
		   | ((unsigned long int) fsbuf->f_fsid.__val[1]
		      << (8 * (sizeof (buf->f_fsid)
			       - sizeof (fsbuf->f_fsid.__val[0])))));
  else
    /* We cannot help here.  The statvfs element is not large enough to
       contain both words of the statfs f_fsid field.  */
    buf->f_fsid = fsbuf->f_fsid.__val[0];
#ifdef _STATVFSBUF_F_UNUSED
  buf->__f_unused = 0;
#endif
  buf->f_namemax = fsbuf->f_namelen;
  memset (buf->__f_spare, '\0', sizeof (buf->__f_spare));

  /* What remains to do is to fill the fields f_favail and f_flag.  */

  /* XXX I have no idea how to compute f_favail.  Any idea???  */
  buf->f_favail = buf->f_ffree;

  /* Determining the flags is tricky.  We have to read /proc/mounts or
     the /etc/mtab file and search for the entry which matches the given
     file.  The way we can test for matching filesystem is using the
     device number.  */
  buf->f_flag = __statvfs_getflags (name, fsbuf->f_type, st);
}
