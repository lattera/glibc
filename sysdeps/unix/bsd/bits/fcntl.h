/* O_*, F_*, FD_* bit values for 4.3 BSD.
   Copyright (C) 1991, 1992, 1997, 2004 Free Software Foundation, Inc.
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

#ifndef	_FCNTL_H
# error "Never use <bits/fcntl.h> directly; include <fcntl.h> instead."
#endif


/* File access modes for `open' and `fcntl'.  */
#define	O_RDONLY	0	/* Open read-only.  */
#define	O_WRONLY	1	/* Open write-only.  */
#define	O_RDWR		2	/* Open read/write.  */


/* Bits OR'd into the second argument to open.  */
#define	O_CREAT		0x0200	/* Create file if it doesn't exist.  */
#define	O_EXCL		0x0800	/* Fail if file already exists.  */
#define	O_TRUNC		0x0400	/* Truncate file to zero length.  */
/* Apparently not assigning a controlling terminal is the default
   behavior in BSD, so no bit is required to request that behavior.  */
#define	O_NOCTTY	0	/* Don't assign a controlling terminal.  */
#if defined __USE_BSD || defined __USE_SVID
# define O_ASYNC		0x0040	/* Send SIGIO to owner when data is ready.  */
# define O_FSYNC		0x2000	/* Synchronous writes.  */
# define O_SYNC		O_FSYNC
#endif

/* File status flags for `open' and `fcntl'.  */
#define	O_APPEND	0x0008	/* Writes append to the file.  */
#define	O_NONBLOCK	0x0004	/* Non-blocking I/O.  */

#ifdef __USE_BSD
/* BSD before 4.4 doesn't support POSIX.1 O_NONBLOCK,
   but O_NDELAY is close.  */
# define O_NDELAY	O_NONBLOCK
#endif

#ifdef __USE_BSD
/* Bits in the file status flags returned by F_GETFL.
   These are all the O_* flags, plus FREAD and FWRITE, which are
   independent bits set by which of O_RDONLY, O_WRONLY, and O_RDWR, was
   given to `open'.  */
# define FREAD		1
# define FWRITE		2

/* Traditional BSD names the O_* bits.  */
# define FASYNC		O_ASYNC
# define FCREAT		O_CREAT
# define FEXCL		O_EXCL
# define FTRUNC		O_TRUNC
# define FNOCTTY	O_NOCTTY
# define FFSYNC		O_FSYNC
# define FSYNC		O_SYNC
# define FAPPEND	O_APPEND
# define FNONBLOCK	O_NONBLOCK
# define FNDELAY	O_NDELAY
#endif

/* Mask for file access modes.  This is system-dependent in case
   some system ever wants to define some other flavor of access.  */
#define	O_ACCMODE	(O_RDONLY|O_WRONLY|O_RDWR)

/* XXX missing */
#define O_LARGEFILE	0

/* Values for the second argument to `fcntl'.  */
#define	F_DUPFD	  	0	/* Duplicate file descriptor.  */
#define	F_GETFD		1	/* Get file descriptor flags.  */
#define	F_SETFD		2	/* Set file descriptor flags.  */
#define	F_GETFL		3	/* Get file status flags.  */
#define	F_SETFL		4	/* Set file status flags.  */
#if defined __USE_BSD || defined __USE_UNIX98
# define F_GETOWN	5	/* Get owner (receiver of SIGIO).  */
# define F_SETOWN	6	/* Set owner (receiver of SIGIO).  */
#endif
#define	F_GETLK		7	/* Get record locking info.  */
#define	F_SETLK		8	/* Set record locking info (non-blocking).  */
#define	F_SETLKW	9	/* Set record locking info (blocking).  */

/* XXX missing */
#define F_GETLK64	7	/* Get record locking info.  */
#define F_SETLK64	8	/* Set record locking info (non-blocking).  */
#define F_SETLKW64	9	/* Set record locking info (blocking).  */

/* File descriptor flags used with F_GETFD and F_SETFD.  */
#define	FD_CLOEXEC	1	/* Close on exec.  */


#include <bits/types.h>

/* The structure describing an advisory lock.  This is the type of the third
   argument to `fcntl' for the F_GETLK, F_SETLK, and F_SETLKW requests.  */
struct flock
  {
    short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.  */
    short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
#ifndef __USE_FILE_OFFSET64
    __off_t l_start;	/* Offset where the lock begins.  */
    __off_t l_len;	/* Size of the locked area; zero means until EOF.  */
#else
    __off64_t l_start;	/* Offset where the lock begins.  */
    __off64_t l_len;	/* Size of the locked area; zero means until EOF.  */
#endif
    short int l_pid;	/* Process holding the lock.  */
    short int l_xxx;	/* Reserved for future use.  */
  };

#ifdef __USE_LARGEFILE64
struct flock64
  {
    short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.  */
    short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
    __off64_t l_start;	/* Offset where the lock begins.  */
    __off64_t l_len;	/* Size of the locked area; zero means until EOF.  */
    short int l_pid;	/* Process holding the lock.  */
    short int l_xxx;	/* Reserved for future use.  */
  };
#endif

/* Values for the `l_type' field of a `struct flock'.  */
#define	F_RDLCK	1	/* Read lock.  */
#define	F_WRLCK	2	/* Write lock.  */
#define	F_UNLCK	3	/* Remove lock.  */
