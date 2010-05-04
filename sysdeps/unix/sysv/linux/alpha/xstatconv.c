/* Convert between the kernel's `struct stat' format, and libc's.
   Copyright (C) 1997, 2003, 2004 Free Software Foundation, Inc.
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
#include <string.h>
#include <sys/stat.h>
#include <kernel_stat.h>
#include <xstatconv.h>
#include <sys/syscall.h>


#ifdef __NR_stat64
# if __ASSUME_STAT64_SYSCALL == 0
int __libc_missing_axp_stat64;
# endif
#endif

int
__xstat_conv (int vers, struct kernel_stat *kbuf, void *ubuf)
{
  switch (vers)
    {
    case _STAT_VER_KERNEL:
      *(struct kernel_stat *) ubuf = *kbuf;
      break;

    case _STAT_VER_GLIBC2:
      {
	struct glibc2_stat *buf = ubuf;

	buf->st_dev = kbuf->st_dev;
	buf->st_ino = kbuf->st_ino;
	buf->st_mode = kbuf->st_mode;
	buf->st_nlink = kbuf->st_nlink;
	buf->st_uid = kbuf->st_uid;
	buf->st_gid = kbuf->st_gid;
	buf->st_rdev = kbuf->st_rdev;
	buf->st_size = kbuf->st_size;
	buf->st_atime = kbuf->st_atime;
	buf->st_mtime = kbuf->st_mtime;
	buf->st_ctime = kbuf->st_ctime;
	buf->st_blksize = kbuf->st_blksize;
	buf->st_blocks = kbuf->st_blocks;
	buf->st_flags = kbuf->st_flags;
	buf->st_gen = kbuf->st_gen;
      }
      break;

    case _STAT_VER_GLIBC2_1:
      {
	struct glibc21_stat *buf = ubuf;

	buf->st_dev = kbuf->st_dev;
	buf->st_ino = kbuf->st_ino;
	buf->st_mode = kbuf->st_mode;
	buf->st_nlink = kbuf->st_nlink;
	buf->st_uid = kbuf->st_uid;
	buf->st_gid = kbuf->st_gid;
	buf->st_rdev = kbuf->st_rdev;
	buf->st_size = kbuf->st_size;
	buf->st_atime = kbuf->st_atime;
	buf->st_mtime = kbuf->st_mtime;
	buf->st_ctime = kbuf->st_ctime;
	buf->st_blocks = kbuf->st_blocks;
	buf->st_blksize = kbuf->st_blksize;
	buf->st_flags = kbuf->st_flags;
	buf->st_gen = kbuf->st_gen;
	buf->__pad3 = 0;
	buf->__unused[0] = 0;
	buf->__unused[1] = 0;
	buf->__unused[2] = 0;
	buf->__unused[3] = 0;
      }
      break;

    case _STAT_VER_GLIBC2_3_4:
      {
	struct stat64 *buf = ubuf;

	buf->st_dev = kbuf->st_dev;
	buf->st_ino = kbuf->st_ino;
	buf->st_rdev = kbuf->st_rdev;
	buf->st_size = kbuf->st_size;
	buf->st_blocks = kbuf->st_blocks;

	buf->st_mode = kbuf->st_mode;
	buf->st_uid = kbuf->st_uid;
	buf->st_gid = kbuf->st_gid;
	buf->st_blksize = kbuf->st_blksize;
	buf->st_nlink = kbuf->st_nlink;
	buf->__pad0 = 0;

	buf->st_atime = kbuf->st_atime;
	buf->st_atimensec = 0;
	buf->st_mtime = kbuf->st_mtime;
	buf->st_mtimensec = 0;
	buf->st_ctime = kbuf->st_ctime;
	buf->st_ctimensec = 0;

	buf->__unused[0] = 0;
	buf->__unused[1] = 0;
	buf->__unused[2] = 0;
      }
      break;

    default:
      __set_errno (EINVAL);
      return -1;
    }

  return 0;
}
