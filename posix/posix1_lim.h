/* Copyright (C) 1991, 1992, 1993 Free Software Foundation, Inc.
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

/*
 *	POSIX Standard: 2.9.2 Minimum Values	Added to <limits.h>
 */

#ifndef	_POSIX1_LIMITS_H

#define	_POSIX1_LIMITS_H	1


/* These are the standard-mandated minimum values.  */

/* Maximum length of arguments to `execve', including environment.  */
#define	_POSIX_ARG_MAX		4096

/* Maximum simultaneous processes per real user ID.  */
#define	_POSIX_CHILD_MAX	6

/* Maximum link count of a file.  */
#define	_POSIX_LINK_MAX		8

/* Number of bytes in a terminal canonical input queue.  */
#define	_POSIX_MAX_CANON	255

/* Number of bytes for which space will be
   available in a terminal input queue.  */
#define	_POSIX_MAX_INPUT	255

/* Number of simultaneous supplementary group IDs per process.  */
#define	_POSIX_NGROUPS_MAX	0

/* Number of files one process can have open at once.  */
#define	_POSIX_OPEN_MAX		16

/* Number of bytes in a filename.  */
#define	_POSIX_NAME_MAX		14

/* Number of bytes in a pathname.  */
#define	_POSIX_PATH_MAX		255

/* Number of bytes than can be written atomically to a pipe.  */
#define	_POSIX_PIPE_BUF		512

/* Largest value of a `ssize_t'.  */
#define	_POSIX_SSIZE_MAX	32767

/* Number of streams a process can have open at once.  */
#define	_POSIX_STREAM_MAX	8

/* Maximum length of a timezone name (element of `tzname').  */
#define	_POSIX_TZNAME_MAX	3


/* Get the implementation-specific values for the above.  */
#include <local_lim.h>


#ifndef	SSIZE_MAX
#define	SSIZE_MAX	INT_MAX
#endif


/* This value is a guaranteed minimum maximum.
   The current maximum can be got from `sysconf'.  */

#ifndef	NGROUPS_MAX
#define	NGROUPS_MAX	_POSIX_NGROUPS_MAX
#endif

#endif	/* posix1_limits.h  */
