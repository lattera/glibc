/* O_*, F_*, FD_* bit values for Linux.
   Copyright (C) 1995, 1996, 1997, 1998 Free Software Foundation, Inc.
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

#ifndef	_FCNTL_H
# error "Never use <bits/fcntl.h> directly; include <fcntl.h> instead."
#endif


#include <sys/types.h>


/* open/fcntl - O_SYNC is only implemented on blocks devices and on files
   located on an ext2 file system */
#define O_ACCMODE	  0003
#define O_RDONLY	    00
#define O_WRONLY	    01
#define O_RDWR		    02
#define O_CREAT		 01000	/* not fcntl */
#define O_TRUNC		 02000	/* not fcntl */
#define O_EXCL		 04000	/* not fcntl */
#define O_NOCTTY	010000	/* not fcntl */

#define O_NONBLOCK	 00004
#define O_APPEND	 00010
#define O_NDELAY	O_NONBLOCK
#define O_SYNC		040000
#define O_FSYNC		O_SYNC
#define O_ASYNC		020000	/* fcntl, for BSD compatibility */

/* XXX missing */
#define O_LARGEFILE	0

/* Values for the second argument to `fcntl'.  */
#define F_DUPFD		0	/* Duplicate file descriptor.  */
#define F_GETFD		1	/* Get file descriptor flags.  */
#define F_SETFD		2	/* Set file descriptor flags.  */
#define F_GETFL		3	/* Get file status flags.  */
#define F_SETFL		4	/* Set file status flags.  */
#define F_GETLK		7	/* Get record locking info.  */
#define F_SETLK		8	/* Set record locking info (non-blocking).  */
#define F_SETLKW	9	/* Set record locking info (blocking).  */

/* XXX missing */
#define F_GETLK64	7	/* Get record locking info.  */
#define F_SETLK64	8	/* Set record locking info (non-blocking).  */
#define F_SETLKW64	9	/* Set record locking info (blocking).  */

#ifdef __USE_BSD
# define F_SETOWN	5	/* Get owner of socket (receiver of SIGIO).  */
# define F_GETOWN	6	/* Set owner of socket (receiver of SIGIO).  */
#endif

#ifdef __USE_GNU
# define F_SETSIG	10	/* Set number of signal to be sent.  */
# define F_GETSIG	11	/* Get number of signal to be sent.  */
#endif

/* for F_[GET|SET]FL */
#define FD_CLOEXEC	1	/* actually anything with low bit set goes */

/* For posix fcntl() and `l_type' field of a `struct flock' for lockf() */
#define F_RDLCK		1	/* Read lock.  */
#define F_WRLCK		2	/* Write lock.  */
#define F_UNLCK		8	/* Remove lock.  */

/* for old implementation of bsd flock () */
#define F_EXLCK		16	/* or 3 */
#define F_SHLCK		32	/* or 4 */

/* operations for bsd flock(), also used by the kernel implementation */
#define LOCK_SH		1	/* shared lock */
#define LOCK_EX		2	/* exclusive lock */
#define LOCK_NB		4	/* or'd with one of the above to prevent
				   blocking */
#define LOCK_UN		8	/* remove lock */

/* We don't need to support __USE_FILE_OFFSET64.  */
struct flock
  {
    short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.  */
    short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
    __off_t l_start;	/* Offset where the lock begins.  */
    __off_t l_len;	/* Size of the locked area; zero means until EOF.  */
    __pid_t l_pid;	/* Process holding the lock.  */
  };

#ifdef __USE_LARGEFILE64
struct flock64
  {
    short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.  */
    short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
    __off64_t l_start;	/* Offset where the lock begins.  */
    __off64_t l_len;	/* Size of the locked area; zero means until EOF.  */
    __pid_t l_pid;	/* Process holding the lock.  */
  };
#endif


/* Define some more compatibility macros to be backward compatible with
   BSD systems which did not managed to hide these kernel macros.  */
#ifdef	__USE_BSD
# define FAPPEND		O_APPEND
# define FFSYNC		O_FSYNC
# define FASYNC		O_ASYNC
# define FNONBLOCK	O_NONBLOCK
# define FNDELAY		O_NDELAY
#endif /* Use BSD.  */
