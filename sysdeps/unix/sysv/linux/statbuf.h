/* Copyright (C) 1992, 1995 Free Software Foundation, Inc.
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
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_STATBUF_H
#define	_STATBUF_H

struct stat
  {
    short int st_dev;			/* Device.  */
    unsigned short __pad1;
    unsigned long int st_ino;		/* File serial number.	*/
    unsigned short int st_mode;		/* File mode.  */
    unsigned short int st_nlink;	/* Link count.  */
    unsigned short int st_uid;		/* User ID of the file's owner.	*/
    unsigned short int st_gid;		/* Group ID of the file's group.*/
    unsigned short int st_rdev;		/* Device number, if device.  */
    unsigned short int __pad2;
    long int st_size;			/* Size of file, in bytes.  */
    unsigned long int st_blksize;	/* Optimal block size for I/O.  */
#define	_STATBUF_ST_BLKSIZE		/* Tell code we have this member.  */

    unsigned long int st_blocks;	/* Number of 512-byte blocks allocated.  */
    long int st_atime;			/* Time of last access.  */
    unsigned long int __unused1;
    long int st_mtime;			/* Time of last modification.  */
    unsigned long int __unused2;
    long int st_ctime;			/* Time of last status change.  */
    unsigned long int __unused3;
    unsigned long int __unused4;
    unsigned long int __unused5;
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

/* Protection bits.  */

#define	__S_ISUID	04000	/* Set user ID on execution.  */
#define	__S_ISGID	02000	/* Set group ID on execution.  */
#define	__S_ISVTX	01000	/* Save swapped text after use (sticky).  */
#define	__S_IREAD	0400	/* Read by owner.  */
#define	__S_IWRITE	0200	/* Write by owner.  */
#define	__S_IEXEC	0100	/* Execute by owner.  */

#endif	/* statbuf.h */
