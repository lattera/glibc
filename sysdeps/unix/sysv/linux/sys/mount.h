/* Header file for mounting/unmount Linux filesystems.
   Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

/* This is taken from /usr/include/linux/fs.h.  */

#ifndef _SYS_MOUNT_H

#define _SYS_MOUNT_H	1
#include <features.h>

#include <sys/ioctl.h>

__BEGIN_DECLS

#define BLOCK_SIZE	1024
#define BLOCK_SIZE_BITS	10


/* These are the fs-independent mount-flags: up to 16 flags are
   supported  */
#define MS_RDONLY	1	/* Mount read-only.  */
#define MS_NOSUID	2	/* Ignore suid and sgid bits.  */
#define MS_NODEV	4	/* Disallow access to device special files.  */
#define MS_NOEXEC	8	/* Disallow program execution.  */
#define MS_SYNCHRONOUS	16	/* Writes are synced at once.  */
#define MS_REMOUNT	32	/* Alter flags of a mounted FS.  */
#define MS_MANDLOCK	64	/* Allow mandatory locks on an FS.  */
#define S_WRITE		128	/* Write on file/directory/symlink.  */
#define S_APPEND	256	/* Append-only file.  */
#define S_IMMUTABLE	512	/* Immutable file.  */
#define MS_NOATIME	1024	/* Do not update access times.  */


/* Flags that can be altered by MS_REMOUNT  */
#define MS_RMT_MASK (MS_RDONLY | MS_MANDLOCK)


/* Magic mount flag number. Has to be or-ed to the flag values.  */

#define MS_MGC_VAL 0xc0ed0000	/* Magic flag number to indicate "new" flags */
#define MS_MGC_MSK 0xffff0000	/* Magic flag number mask */


/* Note that read-only etc flags are inode-specific: setting some
   file-system flags just means all the inodes inherit those flags by
   default. It might be possible to override it selectively if you
   really wanted to with some ioctl() that is not currently
   implemented.

   Exception: MS_RDONLY is always applied to the entire file system.  */
#define IS_RDONLY(inode) \
     (((inode)->i_sb) && ((inode)->i_sb->s_flags & MS_RDONLY))
#define DO_UPDATE_ATIME(inode) \
     (!((inode)->i_flags & MS_NOATIME) && !IS_RDONLY (inode))
#define IS_NOSUID(inode) ((inode)->i_flags & MS_NOSUID)
#define IS_NODEV(inode) ((inode)->i_flags & MS_NODEV)
#define IS_NOEXEC(inode) ((inode)->i_flags & MS_NOEXEC)
#define IS_SYNC(inode) ((inode)->i_flags & MS_SYNCHRONOUS)
#define IS_MANDLOCK(inode) ((inode)->i_flags & MS_MANDLOCK)

#define IS_WRITABLE(inode) ((inode)->i_flags & S_WRITE)
#define IS_APPEND(inode) ((inode)->i_flags & S_APPEND)
#define IS_IMMUTABLE(inode) ((inode)->i_flags & S_IMMUTABLE)


/* The read-only stuff doesn't really belong here, but any other place
   is probably as bad and I don't want to create yet another include
   file.  */

#define BLKROSET   _IO(0x12, 93) /* Set device read-only (0 = read-write).  */
#define BLKROGET   _IO(0x12, 94) /* Get read-only status (0 = read_write).  */
#define BLKRRPART  _IO(0x12, 95) /* Re-read partition table.  */
#define BLKGETSIZE _IO(0x12, 96) /* Return device size.  */
#define BLKFLSBUF  _IO(0x12, 97) /* Flush buffer cache.  */
#define BLKRASET   _IO(0x12, 98) /* Set read ahead for block device.  */
#define BLKRAGET   _IO(0x12, 99) /* Get current read ahead setting.  */


/* Mount a filesystem.  */
extern int mount __P ((__const char *__special_file, __const char *__dir,
		       __const char *__fstype, unsigned long int __rwflag,
		       __const void *__data));

/* Unmount a filesystem.  */
extern int umount __P ((__const char *__special_file));

__END_DECLS

#endif /* _SYS_MOUNT_H */
