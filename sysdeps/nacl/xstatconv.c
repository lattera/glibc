/* Convert between the NaCl ABI's `struct stat' format, and libc's.
   Copyright (C) 2015-2016 Free Software Foundation, Inc.
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

#include <string.h>
#include <sys/stat.h>
#include <xstatconv.h>

internal_function
int
__xstat_conv (int vers, const struct nacl_abi_stat *kbuf, void *ubuf)
{
  /* It's kosher enough just to crash here, but there are some
     existing NaCl tests that like to see EFAULT, and that's what
     making the IRT call with NULL would give.  */
  if (__glibc_unlikely (ubuf == NULL))
    {
      __set_errno (EFAULT);
      return -1;
    }

  switch (vers)
    {
    case _STAT_VER_NACL:
      /* Nothing to do.  The struct is in the form the NaCl ABI expects.  */
      *(struct nacl_abi_stat *) ubuf = *kbuf;
      break;

    case _STAT_VER_LINUX:
      {
        struct stat *buf = ubuf;

        /* Zero-fill the pad/unused fields.  */
        memset (buf, 0, sizeof *buf);

        /* Convert from NaCl IRT ABI `struct stat'.  */
        buf->st_dev = kbuf->nacl_abi_st_dev;
        buf->st_ino = kbuf->nacl_abi_st_ino;
        buf->st_mode = kbuf->nacl_abi_st_mode;
        buf->st_nlink = kbuf->nacl_abi_st_nlink;
        buf->st_uid = kbuf->nacl_abi_st_uid;
        buf->st_gid = kbuf->nacl_abi_st_gid;
        buf->st_rdev = kbuf->nacl_abi_st_rdev;
        buf->st_size = kbuf->nacl_abi_st_size;
        buf->st_blksize = kbuf->nacl_abi_st_blksize;
        buf->st_blocks = kbuf->nacl_abi_st_blocks;
        buf->st_atim.tv_sec = kbuf->nacl_abi_st_atime;
        buf->st_atim.tv_nsec = kbuf->nacl_abi_st_atimensec;
        buf->st_mtim.tv_sec = kbuf->nacl_abi_st_mtime;
        buf->st_mtim.tv_nsec = kbuf->nacl_abi_st_mtimensec;
        buf->st_ctim.tv_sec = kbuf->nacl_abi_st_ctime;
        buf->st_ctim.tv_nsec = kbuf->nacl_abi_st_ctimensec;
      }
      break;

    default:
      __set_errno (EINVAL);
      return -1;
    }

  return 0;
}
