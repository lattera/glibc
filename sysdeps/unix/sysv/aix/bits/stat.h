/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

struct stat
  {
    __dev_t st_dev;			/* Device.  */
    __ino_t st_ino;			/* File serial number.	*/
    __mode_t st_mode;			/* File mode.  */
    __nlink_t st_nlink;			/* Link count.  */
    unsigned short int st_flag;		/* Flag word.  */
    __uid_t st_uid;			/* User ID of the file's owner.	*/
    __gid_t st_gid;			/* Group ID of the file's group.*/
    __dev_t st_rdev;			/* Device number, if device.  */
#ifndef __USE_FILE_OFFSET64
    __off_t st_size;			/* Size of file, in bytes.  */
#else
    int st_ssize;			/* Size of file, in bytes.  */
#endif
    __time_t st_atime;			/* Time of last access.  */
    unsigned long int __unused1;
    __time_t st_mtime;			/* Time of last modification.  */
    unsigned long int __unused2;
    __time_t st_ctime;			/* Time of last status change.  */
    unsigned long int __unused3;
    __blksize_t st_blksize;		/* Optimal block size for I/O.  */
    __blkcnt_t st_blocks;		/* Number 512-byte blocks allocated. */
    int st_vfstype;			/* Type of the filesystem.  */
    unsigned int st_vfs;		/* Vfs number.  */
    unsigned int st_type;		/* Vnode type.  */
    unsigned int st_gen;		/* Inode generation number.  */

#define _STATBUF_RESERVED_SPACE 9
    unsigned int st_reserved[_STATBUF_RESERVED_SPACE];

#ifdef __USE_FILE_OFFSET64
    unsigned int st_padto_ll;
    __off64_t st_size;			/* 64 bit file size in bytes.  */
#endif
  };

#ifdef __USE_LARGEFILE64
struct stat64
  {
    __dev_t st_dev;			/* Device.  */
    __ino_t st_ino;			/* File serial number.	*/
    __mode_t st_mode;			/* File mode.  */
    __nlink_t st_nlink;			/* Link count.  */
    unsigned short int st_flag;		/* Flag word.  */
    __uid_t st_uid;			/* User ID of the file's owner.	*/
    __gid_t st_gid;			/* Group ID of the file's group.*/
    __dev_t st_rdev;			/* Device number, if device.  */
    int st_ssize;			/* Size of file, in bytes.  */
    __time_t st_atime;			/* Time of last access.  */
    unsigned long int __unused1;
    __time_t st_mtime;			/* Time of last modification.  */
    unsigned long int __unused2;
    __time_t st_ctime;			/* Time of last status change.  */
    unsigned long int __unused3;
    __blksize_t st_blksize;		/* Optimal block size for I/O.  */
    __blkcnt_t st_blocks;		/* Number 512-byte blocks allocated. */
    int st_vfstype;			/* Type of the filesystem.  */
    unsigned int st_vfs;		/* Vfs number.  */
    unsigned int st_type;		/* Vnode type.  */
    unsigned int st_gen;		/* Inode generation number.  */
    unsigned int st_reserved[_STATBUF_RESERVED_SPACE];
    unsigned int st_padto_ll;
    __off64_t st_size;			/* 64 bit file size in bytes.  */
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
