/* O_*, F_*, FD_* bit values for Linux.
   Copyright (C) 1995, 1996, 1997 Free Software Foundation, Inc.
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

#ifndef	_FCNTLBITS_H
#define	_FCNTLBITS_H	1

#include <sys/types.h>


/* In GNU, read and write are bits (unlike BSD).  */
#ifdef __USE_GNU
#define	O_READ		O_RDONLY /* Open for reading.  */
#define O_WRITE		O_WRONLY /* Open for writing.  */
#endif
/* open/fcntl - O_SYNC is only implemented on blocks devices and on files
   located on an ext2 file system */
#define O_ACCMODE	  0003
#define O_RDONLY	    00
#define O_WRONLY	    01
#define O_RDWR		    02
#define O_CREAT		  0100	/* not fcntl */
#define O_EXCL		  0200	/* not fcntl */
#define O_NOCTTY	  0400	/* not fcntl */
#define O_TRUNC		 01000	/* not fcntl */
#define O_APPEND	 02000
#define O_NONBLOCK	 04000
#define O_NDELAY	O_NONBLOCK
#define O_SYNC		010000
#define O_FSYNC		O_SYNC
#define O_ASYNC		020000

#define F_DUPFD		0	/* dup */
#define F_GETFD		1	/* get f_flags */
#define F_SETFD		2	/* set f_flags */
#define F_GETFL		3	/* more flags (cloexec) */
#define F_SETFL		4
#define F_GETLK		5
#define F_SETLK		6
#define F_SETLKW	7

#define F_SETOWN	8	/*  for sockets. */
#define F_GETOWN	9	/*  for sockets. */

/* for F_[GET|SET]FL */
#define FD_CLOEXEC	1	/* actually anything with low bit set goes */

/* for posix fcntl() and lockf() */
#define F_RDLCK		0
#define F_WRLCK		1
#define F_UNLCK		2

/* for old implementation of bsd flock () */
#define F_EXLCK		4	/* or 3 */
#define F_SHLCK		8	/* or 4 */

/* operations for bsd flock(), also used by the kernel implementation */
#define LOCK_SH		1	/* shared lock */
#define LOCK_EX		2	/* exclusive lock */
#define LOCK_NB		4	/* or'd with one of the above to prevent
				   blocking */
#define LOCK_UN		8	/* remove lock */

struct flock
  {
    short int l_type;
    short int l_whence;
    __off_t l_start;
    __off_t l_len;
    __pid_t l_pid;
  };


/* Define some more compatibility macros to be backward compatible with
   BSD systems which did not managed to hide these kernel macros.  */
#ifdef	__USE_BSD
#define	FAPPEND		O_APPEND
#define	FFSYNC		O_FSYNC
#define	FASYNC		O_ASYNC
#define	FNONBLOCK	O_NONBLOCK
#define	FNDELAY		O_NDELAY
#endif /* Use BSD.  */

#endif	/* fcntlbits.h */
