/* Copyright (C) 1993 Free Software Foundation, Inc.
   Contributed by Brendan Kehoe (brendan@zen.org).

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

#include <gnu/types.h>

/* Structure describing file characteristics.  */
struct stat
  {
    int st_dev;			/* Device.  */
    unsigned int st_ino;	/* File serial number.		*/
    unsigned int st_mode;	/* File mode.  */
    unsigned short st_nlink;	/* Link count.  */
    unsigned int st_uid;	/* User ID of the file's owner.	*/
    unsigned int st_gid;	/* Group ID of the file's group.*/
    int st_rdev;		/* Device number, if device.  */

    long st_size;		/* Size of file, in bytes.  */

    int st_atime;		/* Time of last access.  */
    int st_atime_usec;
    int st_mtime;		/* Time of last modification.  */
    int st_mtime_usec;
    int st_ctime;		/* Time of last status change.  */
    int st_ctime_usec;

    unsigned int st_blksize;	/* Optimal block size for I/O.  */
#define	_STATBUF_ST_BLKSIZE	/* Tell code we have this member.  */

    int st_blocks;		/* Number of 512-byte blocks allocated.  */
    unsigned int st_flags;
    unsigned int st_gen;
  };

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

/* Protection bits.  */

#define	__S_ISUID	04000	/* Set user ID on execution.  */
#define	__S_ISGID	02000	/* Set group ID on execution.  */
#define	__S_ISVTX	01000	/* Save swapped text after use (sticky).  */
#define	__S_IREAD	0400	/* Read by owner.  */
#define	__S_IWRITE	0200	/* Write by owner.  */
#define	__S_IEXEC	0100	/* Execute by owner.  */

#endif	/* statbuf.h */
