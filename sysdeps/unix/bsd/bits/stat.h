/* Copyright (C) 1991, 92, 96, 97, 99, 2000 Free Software Foundation, Inc.
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

#ifndef _SYS_STAT_H
# error "Never include <bits/stat.h> directly; use <sys/stat.h> instead."
#endif

#include <bits/types.h>

/* Structure describing file characteristics.  */
struct stat
  {
    /* This is a short instead of dev_t for compatibility with 4.3.  */
    short int st_dev;		/* Device containing the file.	*/
    __ino_t st_ino;		/* File serial number.		*/

    /* This is a short instead of mode_t for compatibility with 4.3.  */
    unsigned short int st_mode;	/* File mode.  */

    __nlink_t st_nlink;		/* Link count.  */

    /* These are shorts instead of uid_t/gid_t for compatibility with 4.3.  */
    unsigned short int st_uid;	/* User ID of the file's owner.	*/
    unsigned short int st_gid;	/* Group ID of the file's group.*/

    /* This is a short instead of dev_t for compatibility with 4.3.  */
    short int st_rdev;		/* Device number, if device.  */

    __off_t st_size;		/* Size of file, in bytes.  */

    __time_t st_atime;		/* Time of last access.  */
    unsigned long int st_atime_usec;
    __time_t st_mtime;		/* Time of last modification.  */
    unsigned long int st_mtime_usec;
    __time_t st_ctime;		/* Time of last status change.  */
    unsigned long int st_ctime_usec;

    __blksize_t st_blksize;	/* Optimal block size for I/O.  */
#define	_STATBUF_ST_BLKSIZE	/* Tell code we have this member.  */

    __blkcnt_t st_blocks;	/* Number of 512-byte blocks allocated.  */

    long int st_spare[2];
  };

/* Encoding of the file mode.  */

#define	__S_IFMT	0170000	/* These bits determine file type.  */

/* File types.  */
#define	__S_IFDIR	0040000	/* Directory.  */
#define	__S_IFCHR	0020000	/* Character device.  */
#define	__S_IFBLK	0060000	/* Block device.  */
#define	__S_IFREG	0100000	/* Regular file.  */
#define	__S_IFLNK	0120000	/* Symbolic link.  */
#define	__S_IFSOCK	0140000	/* Socket.  */
#define	__S_IFIFO	0010000	/* FIFO.  */

/* POSIX.1b objects.  */
#define __S_TYPEISMQ(buf) (0)
#define __S_TYPEISSEM(buf) (0)
#define __S_TYPEISSHM(buf) (0)

/* Protection bits.  */

#define	__S_ISUID	04000	/* Set user ID on execution.  */
#define	__S_ISGID	02000	/* Set group ID on execution.  */
#define	__S_ISVTX	01000	/* Save swapped text after use (sticky).  */
#define	__S_IREAD	0400	/* Read by owner.  */
#define	__S_IWRITE	0200	/* Write by owner.  */
#define	__S_IEXEC	0100	/* Execute by owner.  */
