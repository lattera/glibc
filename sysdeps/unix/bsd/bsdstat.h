/* Copyright (C) 1991, 1997 Free Software Foundation, Inc.
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

#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

/* This will make it not define major, minor, makedev, and S_IF*.  */
#undef	__USE_BSD
#undef	__USE_MISC
#include <sys/stat.h>

#undef	stat
#undef	fstat

#undef	S_IRWXU
#undef	S_IRUSR
#undef	S_IWUSR
#undef	S_IXUSR
#undef	S_IRWXG
#undef	S_IRGRP
#undef	S_IWGRP
#undef	S_IXGRP
#undef	S_IRWXO
#undef	S_IROTH
#undef	S_IWOTH
#undef	S_IXOTH
#undef	S_ISBLK
#undef	S_ISCHR
#undef	S_ISDIR
#undef	S_ISFIFO
#undef	S_ISREG
#undef	S_ISUID
#undef	S_ISGID
#define	stat	system_stat
#define	fstat	system_fstat
#define	KERNEL			/* Try to avoid misc decls.  */
#include "/usr/include/sys/stat.h"
#undef	KERNEL
#undef	stat
#undef	fstat

#define	member_same(statbufp, sysbufp, member) \
  (offsetof(struct __stat, member) == offsetof(struct system_stat, member) && \
   sizeof((statbufp)->member) == sizeof((sysbufp)->member))
#define need_stat_mapping(statbufp, sysbufp)				      \
  (!(member_same(statbufp, sysbufp, st_dev) &&				      \
     member_same(statbufp, sysbufp, st_ino) &&				      \
     member_same(statbufp, sysbufp, st_mode) &&				      \
     member_same(statbufp, sysbufp, st_nlink) &&			      \
     member_same(statbufp, sysbufp, st_uid) &&				      \
     member_same(statbufp, sysbufp, st_gid) &&				      \
     member_same(statbufp, sysbufp, st_rdev) &&				      \
     member_same(statbufp, sysbufp, st_size) &&				      \
     member_same(statbufp, sysbufp, st_atime) &&			      \
     member_same(statbufp, sysbufp, st_mtime) &&			      \
     member_same(statbufp, sysbufp, st_ctime) &&			      \
     member_same(statbufp, sysbufp, st_blksize) &&			      \
     member_same(statbufp, sysbufp, st_blocks)))

/* Map a system `struct stat' to our `struct stat'.  */
#ifdef	__GNUC__
inline
#endif
static int
mapstat (sysbuf, statbuf)
     const struct system_stat *sysbuf;
     struct __stat *buf;
{
  if (buf == NULL)
    {
      errno = EINVAL;
      return -1;
    }

  if (!need_stat_mapping(buf, sysbuf))
    /* Hopefully this will be optimized out.  */
    *buf = *(struct __stat *) sysbuf;
  else
    {
      buf->st_dev = (dev_t) sysbuf->st_dev;
      buf->st_ino = (ino_t) sysbuf->st_ino;
      buf->st_mode = (mode_t) sysbuf->st_mode;
      buf->st_nlink = (nlink_t) sysbuf->st_nlink;
      buf->st_uid = (uid_t) sysbuf->st_uid;
      buf->st_gid = (gid_t) sysbuf->st_gid;
      buf->st_rdev = (dev_t) sysbuf->st_rdev;
      buf->st_size = (size_t) sysbuf->st_size;
      buf->st_atime = (time_t) sysbuf->st_atime;
      buf->st_mtime = (time_t) sysbuf->st_mtime;
      buf->st_ctime = (time_t) sysbuf->st_ctime;
      buf->st_blksize = (size_t) sysbuf->st_blksize;
      buf->st_blocks = (size_t) sysbuf->st_blocks;
    }

  return 0;
}
