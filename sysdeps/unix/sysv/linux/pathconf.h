/* Common parts of Linux implementation of pathconf and fpathconf.
   Copyright (C) 1991,95,96,98,99,2000,2001,2002 Free Software Foundation, Inc.
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
#include <sys/statfs.h>
#include "linux_fsinfo.h"

/* Used like: return statfs_link_max (__statfs (name, &buf), &buf); */

static inline long int
statfs_link_max (int result, const struct statfs *fsbuf)
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

static inline long int
statfs_filesize_max (int result, const struct statfs *fsbuf)
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
