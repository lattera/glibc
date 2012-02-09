/* Copyright (C) 2000 Free Software Foundation, Inc.
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

void
__stat_aix_to_linux (const struct aixstat *aixstat, struct stat *linuxstat)
{
  linuxstat->st_dev = makedev (aix_major (aixstat->st_dev),
			       aix_minor (aixstat->st_dev));
  linuxstat->st_ino = aixstat->st_ino;
  /* The following assumes that the mode values are the same on AIX
     and Linux which is true in the moment.  */
  linuxstat->st_mode = aixstat->st_mode;
  linuxstat->st_nlink = aixstat->st_nlink;
  /* There is no st_flag field in Linux.  */
  linuxstat->st_uid = aixstat->st_uid;
  linuxstat->st_gid = aixstat->st_gid;
  linuxstat->st_rdev = makedev (aix_major (aixstat->st_rdev),
				aix_minor (aixstat->st_rdev));
  linuxstat->st_size = aixstat->st_size;
  linuxstat->st_atime = aixstat->st_atime;
  linuxstat->st_mtime = aixstat->st_mtime;
  linuxstat->st_ctime = aixstat->st_ctime;
  linuxstat->st_blksize = aixstat->st_blksize;
  linuxstat->st_blocks = aixstat->st_blocks;
  /* There is no st_vfstype in Linux.  */
  /* There is no st_vfs in Linux.  */
  /* There is no st_type in Linux.  */
  /* There is no st_gen in Linux.  */

  /* File in the padding values with repeatable values.  */
  linuxstat->__pad1 = 0;
  linuxstat->__pad2 = 0;
  linuxstat->__unused1 = 0;
  linuxstat->__unused2 = 0;
  linuxstat->__unused3 = 0;
  linuxstat->__unused4 = 0;
  linuxstat->__unused5 = 0;
}
