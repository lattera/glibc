/* Define POSIX options for GNU/Hurd.
   Copyright (C) 1998,2000,2001,2002 Free Software Foundation, Inc.
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

#ifndef _UNISTD_H
#error "Never include this file directly; use <unistd.h> instead."
#endif

#ifndef	_BITS_POSIX_OPT_H
#define	_BITS_POSIX_OPT_H	1


/* Job control is supported.  */
#define	_POSIX_JOB_CONTROL	1

/* Processes have a saved set-user-ID and a saved set-group-ID.  */
#define	_POSIX_SAVED_IDS	1

#if 0				/* XXX implement aio_* */
/* Asynchronous I/O is supported.  */
#define _POSIX_ASYNCHRONOUS_IO	1
/* Alternative name for Unix98.  */
#define _LFS_ASYNCHRONOUS_IO	_POSIX_ASYNCHRONOUS_IO
#endif

/* Synchronizing file data is supported, but msync is missing.  */
#undef _POSIX_SYNCHRONIZED_IO

/* The fsync function is present.  */
#define	_POSIX_FSYNC	200112L

/* Mapping of files to memory is supported.  */
#define	_POSIX_MAPPED_FILES	200112L

/* Locking of ranges of memory is supported.  */
#define	_POSIX_MEMLOCK_RANGE	200112L

/* Setting of memory protections is supported.  */
#define	_POSIX_MEMORY_PROTECTION	200112L

/* POSIX.4 shared memory objects are supported (using regular files).  */
#define _POSIX_SHARED_MEMORY_OBJECTS	_POSIX_MAPPED_FILES

/* Elements of the `c_cc' member of `struct termios' structure
   can be disabled by using the value _POSIX_VDISABLE.  */
#define _POSIX_VDISABLE			((unsigned char) -1)


/* Different Hurd filesystems might do these differently.
   You must query the particular file with `pathconf' or `fpathconf'.  */
#undef _POSIX_CHOWN_RESTRICTED	/* Only root can change owner of file?  */
#undef _POSIX_NO_TRUNC		/* Overlong file names get error?  */
#undef _POSIX_SYNC_IO		/* File supports O_SYNC et al?  */

/* GNU libc provides regular expression handling.  */
#define _POSIX_REGEXP	1

/* We have a POSIX shell.  */
#define _POSIX_SHELL	1

/* The `spawn' function family is supported.  */
#define _POSIX_SPAWN	200112L

#endif /* bits/posix_opt.h */
