/* Define POSIX options for Linux.
   Copyright (C) 1996, 1997, 1999, 2000 Free Software Foundation, Inc.
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

/*
 * Never include this file directly; use <unistd.h> instead.
 */

#ifndef	_BITS_POSIX_OPT_H
#define	_BITS_POSIX_OPT_H	1

/* Job control is supported.  */
#define	_POSIX_JOB_CONTROL	1

/* Processes have a saved set-user-ID and a saved set-group-ID.  */
#define	_POSIX_SAVED_IDS	1

/* Priority scheduling is supported.  */
#define	_POSIX_PRIORITY_SCHEDULING	1

/* Synchronizing file data is supported.  */
#define	_POSIX_SYNCHRONIZED_IO	1

/* The fsync function is present.  */
#define	_POSIX_FSYNC	1

/* Mapping of files to memory is supported.  */
#define	_POSIX_MAPPED_FILES	1

/* Locking of all memory is supported.  */
#define	_POSIX_MEMLOCK	1

/* Locking of ranges of memory is supported.  */
#define	_POSIX_MEMLOCK_RANGE	1

/* Setting of memory protections is supported.  */
#define	_POSIX_MEMORY_PROTECTION	1

/* Implementation supports `poll' function.  */
#define	_POSIX_POLL	1

/* Implementation supports `select' and `pselect' functions.  */
#define	_POSIX_SELECT	1

/* Only root can change owner of file.  */
#define	_POSIX_CHOWN_RESTRICTED	1

/* `c_cc' member of 'struct termios' structure can be disabled by
   using the value _POSIX_VDISABLE.  */
#define	_POSIX_VDISABLE	'\0'

/* Filenames are not silently truncated.  */
#define	_POSIX_NO_TRUNC	1

/* X/Open realtime support is available.  */
#define _XOPEN_REALTIME	1

/* XPG4.2 shared memory is supported.  */
#define	_XOPEN_SHM	1

/* Real-time signals are supported.  */
#define _POSIX_REALTIME_SIGNALS	1

/* The LFS interface is available, except for the asynchronous I/O.  */
#define _LFS_LARGEFILE		1
#define _LFS64_LARGEFILE	1
#define _LFS64_STDIO		1

/* POSIX timers are available.  */
#define _POSIX_TIMERS	1

/* POSIX shared memory objects are implemented.  */
#define _POSIX_SHARED_MEMORY_OBJECTS	1

/* GNU libc provides regular expression handling.  */
#define _POSIX_REGEXP	1

/* We have a POSIX shell.  */
#define _POSIX_SHELL	1

/* The `spawn' function family is supported.  */
#define _POSIX_SPAWN	200912L

#endif /* bits/posix_opt.h */
