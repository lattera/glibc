/* O_*, F_*, FD_* bit values for Linux.
   Copyright (C) 1995-1999, 2000, 2004 Free Software Foundation, Inc.
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


#include <sys/types.h>

/* open/fcntl.  */
#define O_ACCMODE	  0003
#define O_RDONLY	    00
#define O_WRONLY	    01
#define O_RDWR		    02
#define O_NONBLOCK	    04
#define O_NDELAY    O_NONBLOCK
#define O_APPEND	   010
#define O_SYNC		   020
#define O_FSYNC		O_SYNC
#define O_CREAT		  0400	/* not fcntl */
#define O_TRUNC		 01000	/* not fcntl */
#define O_EXCL		 02000	/* not fcntl */
#define O_NOCTTY	 04000	/* not fcntl */
#define O_ASYNC	       0400000

#ifdef __USE_LARGEFILE64
# define O_LARGEFILE 0400000000
#endif

/* For now Linux has synchronisity options for data and read operations.
   We define the symbols here but let them do the same as O_SYNC since
   this is a superset.  */
#if defined __USE_POSIX199309 || defined __USE_UNIX98
# define O_DSYNC     020000000	/* Synchronize data.  */
# define O_RSYNC     010000000	/* Synchronize read operations.  */
#endif

/* Values for the second argument to `fcntl'.  */
#define F_DUPFD		0	/* Duplicate file descriptor.  */
#define F_GETFD		1	/* Get file descriptor flags.  */
#define F_SETFD		2	/* Set file descriptor flags.  */
#define F_GETFL		3	/* Get file status flags.  */
#define F_SETFL		4	/* Set file status flags.  */
#ifndef __USE_FILE_OFFSET64
# define F_GETLK	5	/* Get record locking info.  */
# define F_SETLK	6	/* Set record locking info (non-blocking).  */
# define F_SETLKW	7	/* Set record locking info (blocking).  */
#else
# define F_GETLK       11	/* Get record locking info.  */
# define F_SETLK       12	/* Set record locking info (non-blocking).  */
# define F_SETLKW      13	/* Set record locking info (blocking).  */
#endif

#ifdef __USE_LARGEFILE64
# define F_GETLK64      11	/* Get record locking info.  */
# define F_SETLK64      12	/* Set record locking info (non-blocking).  */
# define F_SETLKW64     13	/* Set record locking info (blocking).  */
#endif

#if defined __USE_BSD || defined __USE_UNIX98
# define F_SETOWN	8	/* Get owner of socket (receiver of SIGIO).  */
# define F_GETOWN	9	/* Set owner of socket (receiver of SIGIO).  */
#endif

/* For F_[GET|SET]FD.  */
#define FD_CLOEXEC	1	/* actually anything with low bit set goes */

/* For posix fcntl() and `l_type' field of a `struct flock' for lockf().  */
#define F_RDLCK		1	/* Read lock.  */
#define F_WRLCK		2	/* Write lock.  */
#define F_UNLCK		3	/* Remove lock.  */

#ifdef __USE_BSD
/* Operations for bsd flock(), also used by the kernel implementation */
# define LOCK_SH	1	/* shared lock */
# define LOCK_EX	2	/* exclusive lock */
# define LOCK_NB	4	/* or'd with one of the above to prevent
				   blocking */
# define LOCK_UN	8	/* remove lock */
#endif

struct flock
  {
    short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.  */
    short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
#ifndef __USE_FILE_OFFSET64
    __off_t l_start;	/* Offset where the lock begins.  */
    __off_t l_len;	/* Size of the locked area; zero means until EOF.  */
#endif
    unsigned int l_sysid;
    __pid_t l_pid;	/* Process holding the lock.  */
    int l_vfs;
#ifdef __USE_FILE_OFFSET64
    __off64_t l_start;	/* Offset where the lock begins.  */
    __off64_t l_len;	/* Size of the locked area; zero means until EOF.  */
#endif
  };

#ifdef __USE_LARGEFILE64
struct flock64
  {
    short int l_type;	/* Type of lock: F_RDLCK, F_WRLCK, or F_UNLCK.  */
    short int l_whence;	/* Where `l_start' is relative to (like `lseek').  */
    unsigned int l_sysid;
    __pid_t l_pid;	/* Process holding the lock.  */
    int l_vfs;
    __off64_t l_start;	/* Offset where the lock begins.  */
    __off64_t l_len;	/* Size of the locked area; zero means until EOF.  */
  };
#endif

/* Define some more compatibility macros to be backward compatible with
   BSD systems which did not managed to hide these kernel macros.  */
#ifdef	__USE_BSD
# define FAPPEND	O_APPEND
# define FFSYNC		O_FSYNC
# define FASYNC		O_ASYNC
# define FNONBLOCK	O_NONBLOCK
# define FNDELAY	O_NDELAY
#endif /* Use BSD.  */
