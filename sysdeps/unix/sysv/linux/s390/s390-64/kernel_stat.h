/* Definition of `struct stat' used in the kernel.  64 bit S/390 version.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

struct kernel_stat
  {
    unsigned int st_dev;
    unsigned int st_ino;
    unsigned int st_mode;
    unsigned int st_nlink;
    unsigned int st_uid;
    unsigned int st_gid;
    unsigned int st_rdev;
    unsigned int __pad1;
    unsigned long int st_size;
    unsigned long int st_atime;
    unsigned long int st_mtime;
    unsigned long int st_ctime;
    unsigned int  st_blksize;
    int st_blocks;
    unsigned long __unused1;
    unsigned long __unused2;
  };

extern int __xstat_conv (int vers, struct kernel_stat *kbuf, void *ubuf);

#define XSTAT_IS_XSTAT64 1
#define _HAVE___UNUSED1
#define _HAVE___UNUSED2

