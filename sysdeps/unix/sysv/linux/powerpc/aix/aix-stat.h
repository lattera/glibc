/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

struct aixstat
  {
    aixdev_t st_dev;
    aixino_t st_ino;
    aixmode_t st_mode;
    aixnlink_t st_nlink;
    unsigned short int st_flag;
    aixuid_t st_uid;
    aixgid_t st_gid;
    aixdev_t st_rdev;
    aixoff_t st_size;
    aixtime_t st_atime;
    unsigned long int __unused1;
    aixtime_t st_mtime;
    unsigned long int __unused2;
    aixtime_t st_ctime;
    unsigned long int __unused3;
    aixblksize_t st_blksize;
    aixblkcnt_t st_blocks;
    int st_vfstype;
    unsigned int st_vfs;
    unsigned int st_type;
    unsigned int st_gen;

#define _STATBUF_RESERVED_SPACE 9
    unsigned int st_reserved[_STATBUF_RESERVED_SPACE];
  };

struct aixstat64
  {
    aixdev_t st_dev;
    aixino64_t st_ino;
    aixmode_t st_mode;
    aixnlink_t st_nlink;
    unsigned short int st_flag;
    aixuid_t st_uid;
    aixgid_t st_gid;
    aixdev_t st_rdev;
    int st_ssize;
    aixtime_t st_atime;
    unsigned long int __unused1;
    aixtime_t st_mtime;
    unsigned long int __unused2;
    aixtime_t st_ctime;
    unsigned long int __unused3;
    aixblksize_t st_blksize;
    aixblkcnt64_t st_blocks;
    int st_vfstype;
    unsigned int st_vfs;
    unsigned int st_type;
    unsigned int st_gen;
    unsigned int st_reserved[_STATBUF_RESERVED_SPACE];
    unsigned int st_padto_ll;
    aixoff64_t st_size;
  };

#define aix_major(x)	(int) ((unsigned int) (x) >> 16)
#define aix_minor(x)	(int) ((x) & 0xFFFF)
