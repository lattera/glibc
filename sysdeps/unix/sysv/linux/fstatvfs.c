/* Copyright (C) 1998 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@cygnus.com>, 1998.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <errno.h>
#include <mntent.h>
#include <paths.h>
#include <string.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/statfs.h>
#include <sys/statvfs.h>

/* These definitions come from the kernel headers.  But we cannot
   include the headers here because of type clashes.  If new
   filesystem types will become available we have to add the
   appropriate definitions here.*/
#define ADFS_SUPER_MAGIC	0xadf5
#define AFFS_SUPER_MAGIC	0xadff
#define CODA_SUPER_MAGIC	0x73757245
#define EXT2_SUPER_MAGIC	0xef53
#define HPFS_SUPER_MAGIC	0xf995e849
#define ISOFS_SUPER_MAGIC	0x9660
#define MINIX_SUPER_MAGIC	0x137f
#define MINIX_SUPER_MAGIC2	0x138F
#define MINIX2_SUPER_MAGIC	0x2468
#define MINIX2_SUPER_MAGIC2	0x2478
#define MSDOS_SUPER_MAGIC	0x4d44
#define NCP_SUPER_MAGIC		0x564c
#define NFS_SUPER_MAGIC		0x6969
#define PROC_SUPER_MAGIC	0x9fa0
#define SMB_SUPER_MAGIC		0x517b
#define XENIX_SUPER_MAGIC	0x012ff7b4
#define SYSV4_SUPER_MAGIC	0x012ff7b5
#define SYSV2_SUPER_MAGIC	0x012ff7b6
#define COH_SUPER_MAGIC		0x012ff7b7


int
fstatvfs (int fd, struct statvfs *buf)
{
  struct statfs fsbuf;
  struct stat st;

  /* Get as much information as possible from the system.  */
  if (__fstatfs (fd, &fsbuf) < 0)
    return -1;

  /* Now fill in the fields we have information for.  */
  buf->f_bsize = fsbuf.f_bsize;
  buf->f_blocks = fsbuf.f_blocks;
  buf->f_bfree = fsbuf.f_bfree;
  buf->f_bavail = fsbuf.f_bavail;
  buf->f_files = fsbuf.f_files;
  buf->f_ffree = fsbuf.f_ffree;
  buf->f_fsid = fsbuf.f_fsid;
  buf->f_namemax = fsbuf.f_namelen;
  memset (buf->f_spare, '\0', 6 * sizeof (int));

  /* What remains to do is to fill the fields f_frsize, f_favail,
     and f_flag.  */
  switch (fsbuf.f_type)
    {
    case EXT2_SUPER_MAGIC:
      /* This is not really correct since the fragment size can vary.  */
      buf->f_frsize = 1024;
      break;

    case ADFS_SUPER_MAGIC:
    case AFFS_SUPER_MAGIC:
    case CODA_SUPER_MAGIC:
    case HPFS_SUPER_MAGIC:
    case ISOFS_SUPER_MAGIC:
    case MINIX_SUPER_MAGIC:
    case MINIX_SUPER_MAGIC2:
    case MINIX2_SUPER_MAGIC:
    case MINIX2_SUPER_MAGIC2:
    case MSDOS_SUPER_MAGIC:
    case NCP_SUPER_MAGIC:
    case NFS_SUPER_MAGIC:
    case PROC_SUPER_MAGIC:
    case SMB_SUPER_MAGIC:
    case XENIX_SUPER_MAGIC:
    case SYSV4_SUPER_MAGIC:
    case SYSV2_SUPER_MAGIC:
    case COH_SUPER_MAGIC:
    default:
      /* I hope it's safe to assume no fragmentation.  */
      buf->f_frsize = buf->f_bsize;
      break;
    }

  /* XXX I have no idea how to compute f_favail.  Any idea???  */
  buf->f_favail = buf->f_ffree;

  /* Determining the flags is tricky.  We have to read /proc/mounts or
     the /etc/mtab file and search for the entry which matches the given
     file.  The way we can test for matching filesystem is using the
     device number.  */
  buf->f_flag = 0;
  if (fstat (fd, &st) >= 0)
    {
      int save_errno = errno;
      struct mntent mntbuf;
      FILE *mtab;

      mtab = __setmntent ("/proc/mounts", "r");
      if (mtab == NULL)
	mtab = __setmntent (_PATH_MOUNTED, "r");

      if (mtab != NULL)
	{
	  char tmpbuf[1024];

	  while (__getmntent_r (mtab, &mntbuf, tmpbuf, sizeof (tmpbuf)))
	    {
	      struct stat fsst;

	      /* Find out about the device the current entry is for.  */
	      if (stat (mntbuf.mnt_dir, &fsst) >= 0
		  && st.st_dev == fsst.st_dev)
		{
		  /* Bingo, we found the entry for the device FD is on.
		     Now interpret the option string.  */
		  char *cp = mntbuf.mnt_opts;
		  char *opt;

		  while ((opt = strsep (&cp, ",")) != NULL)
		    if (strcmp (opt, "ro") == 0)
		      buf->f_flag |= ST_RDONLY;
		    else if (strcmp (opt, "nosuid") == 0)
		      buf->f_flag |= ST_NOSUID;
		    else if (strcmp (opt, "noexec") == 0)
		      buf->f_flag |= ST_NOEXEC;
		    else if (strcmp (opt, "nodev") == 0)
		      buf->f_flag |= ST_NODEV;
		    else if (strcmp (opt, "sync") == 0)
		      buf->f_flag |= ST_SYNCHRONOUS;
		    else if (strcmp (opt, "mand") == 0)
		      buf->f_flag |= ST_MANDLOCK;
		    else if (strcmp (opt, "noatime") == 0)
		      buf->f_flag |= ST_NOATIME;
		    else if (strcmp (opt, "nodiratime") == 0)
		      buf->f_flag |= ST_NODIRATIME;

		  /* We can stop looking for more entries.  */
		  break;
		}
	    }

	  /* Close the file.  */
	  __endmntent (mtab);
	}

      __set_errno (save_errno);
    }

  /* We signal success if the statfs call succeeded.  */
  return 0;
}
