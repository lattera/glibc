/* Copyright (C) 1999, 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#ifndef _SYS_STAT_H
# error "Never include <bits/stat.h> directly; use <sys/stat.h> instead."
#endif

/* Versions of the `struct stat' data structure.  */
#define _STAT_VER_KERNEL	0
#define _STAT_VER_LINUX		1
#define _STAT_VER		_STAT_VER_LINUX

/* Versions of the `xmknod' interface.  */
#define _MKNOD_VER_LINUX	0

struct stat
  {
    __dev_t st_dev;		/* Device.  */
    __ino_t st_ino;		/* File serial number.	*/
    __nlink_t st_nlink;		/* Link count.  */
    __mode_t st_mode;		/* File mode.  */
    __uid_t st_uid;		/* User ID of the file's owner.	*/
    __gid_t st_gid;		/* Group ID of the file's group.*/
    int pad0;
    __dev_t st_rdev;		/* Device number, if device.  */
    __off_t st_size;		/* Size of file, in bytes.  */
    __time_t st_atime;		/* Time of last access.  */
    long int __reserved0;	/* Reserved for atime.nanoseconds.  */
    __time_t st_mtime;		/* Time of last modification.  */
    long int __reserved1;	/* Reserved for mtime.nanoseconds.  */
    __time_t st_ctime;		/* Time of last status change.  */
    long int __reserved2;	/* Reserved for ctime.nanoseconds.  */
    __blksize_t st_blksize;	/* Optimal block size for I/O.  */
    __blkcnt_t st_blocks;	/* Nr. 512-byte blocks allocated.  */
    long int __unused[3];
  };

#ifdef __USE_LARGEFILE64
/* Note stat64 is the same shape as stat.  */
struct stat64
  {
    __dev_t st_dev;		/* Device.  */
    __ino64_t st_ino;		/* File serial number.  */
    __nlink_t st_nlink;		/* Link count.  */
    __mode_t st_mode;		/* File mode.  */
    __uid_t st_uid;		/* User ID of the file's owner.	*/
    __gid_t st_gid;		/* Group ID of the file's group.*/
    int pad0;
    __dev_t st_rdev;		/* Device number, if device.  */
    __off_t st_size;		/* Size of file, in bytes.  */
    __time_t st_atime;		/* Time of last access.  */
    long int __reserved0;	/* Reserved for atime.nanoseconds.  */
    __time_t st_mtime;		/* Time of last modification.  */
    long int __reserved1;	/* Reserved for mtime.nanoseconds.  */
    __time_t st_ctime;		/* Time of last status change.  */
    long int __reserved2;	/* Reserved for ctime.nanoseconds.  */
    __blksize_t st_blksize;	/* Optimal block size for I/O.  */
    __blkcnt64_t st_blocks;	/* Nr. 512-byte blocks allocated.  */
    long int __unused[3];
  };
#endif

/* Tell code we have these members.  */
#define	_STATBUF_ST_BLKSIZE
#define _STATBUF_ST_RDEV

/* Encoding of the file mode.  */

#define	__S_IFMT	0170000	/* These bits determine file type.  */

/* File types.  */
#define	__S_IFDIR	0040000	/* Directory.  */
#define	__S_IFCHR	0020000	/* Character device.  */
#define	__S_IFBLK	0060000	/* Block device.  */
#define	__S_IFREG	0100000	/* Regular file.  */
#define	__S_IFIFO	0010000	/* FIFO.  */
#define	__S_IFLNK	0120000	/* Symbolic link.  */
#define	__S_IFSOCK	0140000	/* Socket.  */

/* POSIX.1b objects.  Note that these macros always evaluate to zero.  But
   they do it by enforcing the correct use of the macros.  */
#define __S_TYPEISMQ(buf)  ((buf)->st_mode - (buf)->st_mode)
#define __S_TYPEISSEM(buf) ((buf)->st_mode - (buf)->st_mode)
#define __S_TYPEISSHM(buf) ((buf)->st_mode - (buf)->st_mode)

/* Protection bits.  */

#define	__S_ISUID	04000	/* Set user ID on execution.  */
#define	__S_ISGID	02000	/* Set group ID on execution.  */
#define	__S_ISVTX	01000	/* Save swapped text after use (sticky).  */
#define	__S_IREAD	0400	/* Read by owner.  */
#define	__S_IWRITE	0200	/* Write by owner.  */
#define	__S_IEXEC	0100	/* Execute by owner.  */
