void
__stat_aix_to_linux (const struct stat *aixstat, struct stat *linuxstat)
{
  linuxstat->st_dev = linux_makedev (major (aixstat->st_dev),
				     minor (aixstat->st_dev));
  linuxstat->st_ino = aixstat->st_ino;
  /* The following assumes that the mode values are the same on AIX
     and Linux which is true in the moment.  */
  linuxstat->st_mode = aixstat->st_mode;
  linuxstat->st_nlink = aixstat->st_nlink;
  /* There is no st_flag field in Linux.  */
  linuxstat->st_uid = aixstat->st_uid;
  linuxstat->st_gid = aixstat->st_gid;
  linuxstat->st_rdev = linux_makedev (major (aixstat->st_rdev),
				      minor (aixstat->st_rdev));
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
