/* Get file-specific information about a file.  Linux version.
   Copyright (C) 1991,1995,1996,1998-2002,2003 Free Software Foundation, Inc.
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
#include <errno.h>
#include "pathconf.h"
#include "linux_fsinfo.h"

static long int posix_pathconf (const char *file, int name);

/* Define this first, so it can be inlined.  */
#define __pathconf static posix_pathconf
#include <sysdeps/posix/pathconf.c>


/* Get file-specific information about FILE.  */
long int
__pathconf (const char *file, int name)
{
  struct statfs fsbuf;

  switch (name)
    {
    case _PC_LINK_MAX:
      return __statfs_link_max (__statfs (file, &fsbuf), &fsbuf);

    case _PC_FILESIZEBITS:
      return __statfs_filesize_max (__statfs (file, &fsbuf), &fsbuf);

    case _PC_2_SYMLINKS:
      return __statfs_symlinks (__statfs (file, &fsbuf), &fsbuf);

    default:
      return posix_pathconf (file, name);
    }
}


/* Used like: return statfs_link_max (__statfs (name, &buf), &buf); */
long int
__statfs_link_max (int result, const struct statfs *fsbuf)
{
  if (result < 0)
    {
      if (errno == ENOSYS)
	/* Not possible, return the default value.  */
	return LINUX_LINK_MAX;

      /* Some error occured.  */
      return -1;
    }

  switch (fsbuf->f_type)
    {
    case EXT2_SUPER_MAGIC:
      return EXT2_LINK_MAX;

    case MINIX_SUPER_MAGIC:
    case MINIX_SUPER_MAGIC2:
      return MINIX_LINK_MAX;

    case MINIX2_SUPER_MAGIC:
    case MINIX2_SUPER_MAGIC2:
      return MINIX2_LINK_MAX;

    case XENIX_SUPER_MAGIC:
      return XENIX_LINK_MAX;

    case SYSV4_SUPER_MAGIC:
    case SYSV2_SUPER_MAGIC:
      return SYSV_LINK_MAX;

    case COH_SUPER_MAGIC:
      return COH_LINK_MAX;

    case UFS_MAGIC:
    case UFS_CIGAM:
      return UFS_LINK_MAX;

    case REISERFS_SUPER_MAGIC:
      return REISERFS_LINK_MAX;

    case XFS_SUPER_MAGIC:
      return XFS_LINK_MAX;

    default:
      return LINUX_LINK_MAX;
    }
}


/* Used like: return statfs_filesize_max (__statfs (name, &buf), &buf); */
long int
__statfs_filesize_max (int result, const struct statfs *fsbuf)
{
  if (result < 0)
    {
      if (errno == ENOSYS)
	/* Not possible, return the default value.  */
	return 32;

      /* Some error occured.  */
      return -1;
    }

  switch (fsbuf->f_type)
    {
    case EXT2_SUPER_MAGIC:
    case UFS_MAGIC:
    case UFS_CIGAM:
    case REISERFS_SUPER_MAGIC:
    case XFS_SUPER_MAGIC:
    case SMB_SUPER_MAGIC:
    case NTFS_SUPER_MAGIC:
    case UDF_SUPER_MAGIC:
    case JFS_SUPER_MAGIC:
    case VXFS_SUPER_MAGIC:
      return 64;

    case MSDOS_SUPER_MAGIC:
    case JFFS_SUPER_MAGIC:
    case JFFS2_SUPER_MAGIC:
    case NCP_SUPER_MAGIC:
    case ROMFS_SUPER_MAGIC:
      return 32;

    default:
      return 32;
    }
}


/* Used like: return statfs_link_max (__statfs (name, &buf), &buf); */
long int
__statfs_symlinks (int result, const struct statfs *fsbuf)
{
  if (result < 0)
    {
      if (errno == ENOSYS)
	/* Not possible, return the default value.  */
	return 1;

      /* Some error occured.  */
      return -1;
    }

  switch (fsbuf->f_type)
    {
    case ADFS_SUPER_MAGIC:
    case BFS_MAGIC:
    case CRAMFS_MAGIC:
    case DEVPTS_SUPER_MAGIC:
    case EFS_SUPER_MAGIC:
    case EFS_MAGIC:
    case MSDOS_SUPER_MAGIC:
    case NTFS_SUPER_MAGIC:
    case QNX4_SUPER_MAGIC:
    case ROMFS_SUPER_MAGIC:
      /* No symlink support.  */
      return 0;

    default:
      return 1;
    }
}
