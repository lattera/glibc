/* Definition of `struct stat' used in the kernel.
   Copyright (C) 1997, 2000 Free Software Foundation, Inc.
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
    unsigned short st_nlink;
    unsigned int st_uid;
    unsigned int st_gid;
    unsigned int st_rdev;
    unsigned long int st_size;
    unsigned long int st_blksize;
    unsigned long int st_blocks;
    unsigned long int st_atime;
    unsigned long int __unused1;
#define _HAVE___UNUSED1
    unsigned long int st_mtime;
    unsigned long int __unused2;
#define _HAVE___UNUSED2
    unsigned long int st_ctime;
    unsigned long int __unused3;
#define _HAVE___UNUSED3
    unsigned long int __unused4;
#define _HAVE___UNUSED4
    unsigned long int __unused5;
#define _HAVE___UNUSED5
  };

#define _HAVE_STAT___UNUSED1
#define _HAVE_STAT___UNUSED2
#define _HAVE_STAT___UNUSED3
#define _HAVE_STAT___UNUSED4
#define _HAVE_STAT___UNUSED5
#define _HAVE_STAT___PAD1
#define _HAVE_STAT___PAD2
#define _HAVE_STAT64___UNUSED1
#define _HAVE_STAT64___UNUSED2
#define _HAVE_STAT64___UNUSED3
#define _HAVE_STAT64___UNUSED4
#define _HAVE_STAT64___UNUSED5
#define _HAVE_STAT64___PAD2
