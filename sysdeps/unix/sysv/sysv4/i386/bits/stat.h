/* Copyright (C) 1993, 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
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

/* Versions of the `struct stat' data structure and
   the bits of the `xmknod' interface.  */
#define _STAT_VER	2
#define _MKNOD_VER	2

/* Structure describing file characteristics.  */
struct stat
  {
    unsigned long itn st_dev;	/* Device.  */
    long int st_filler1[3];
    unsigned long int st_ino;	/* File serial number.		*/
    unsigned long int st_mode;	/* File mode.  */
    unsigned long int st_nlink;	/* Link count.  */
    long int st_uid;		/* User ID of the file's owner.	*/
    long int st_gid;		/* Group ID of the file's group.*/
    unsigned long int st_rdev;	/* Device number, if device.  */
    long int st_filler2[2];

    long int st_size;		/* Size of file, in bytes.  */
    /* SVR4 added this extra long to allow for expansion of off_t.  */
    long int st_filler3;

    long int st_atime;		/* Time of last access.  */
    unsigned long int st_atime_usec;
    long int st_mtime;		/* Time of last modification.  */
    unsigned long int st_mtime_usec;
    long int st_ctime;		/* Time of last status change.  */
    unsigned long int st_ctime_usec;

    __blksize_t st_blksize;	/* Optimal block size for I/O.  */
#define	_STATBUF_ST_BLKSIZE	/* Tell code we have this member.  */

    __blkcnt_t st_blocks;	/* Number of 512-byte blocks allocated.  */
    char st_fstype[16];		/* The type of this filesystem.  */
    int st_aclcnt;
    unsigned long int st_level;
    unsigned long int st_flags;
    unsigned long int st_cmwlevel;
    long int st_filler4[4];
  };

/* Encoding of the file mode.  */

#define	__S_IFMT	0170000	/* These bits determine file type.  */

/* File types.  */
#define	__S_IFDIR	0040000	/* Directory.  */
#define	__S_IFCHR	0020000	/* Character device.  */
#define	__S_IFBLK	0060000	/* Block device.  */
#define	__S_IFREG	0100000	/* Regular file.  */
#define	__S_IFIFO	0010000	/* FIFO.  */

/* These don't actually exist on System V, but having them doesn't hurt.  */
#define	__S_IFLNK	0120000	/* Symbolic link.  */
#define	__S_IFSOCK	0140000	/* Socket.  */

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
