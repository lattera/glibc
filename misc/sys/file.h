/* Copyright (C) 1991, 1992 Free Software Foundation, Inc.
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
not, write to the, 1992 Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#ifndef	_SYS_FILE_H

#define	_SYS_FILE_H	1
#include <features.h>

#ifndef	_FCNTL_H
#include <fcntl.h>
#endif

__BEGIN_DECLS


/* Alternate names for values for the WHENCE argument to `lseek'.
   These are the same as SEEK_SET, SEEK_CUR, and SEEK_END, respectively.  */
#define	L_SET	0	/* Seek from beginning of file.  */
#define	L_INCR	1	/* Seek from current position.  */
#define	L_XTND	2	/* Seek from end of file.  */


/* Operations for the `flock' call.  */
#define	LOCK_SH	1    /* Shared lock.  */
#define	LOCK_EX	2    /* Exclusive lock.  */
#define	LOCK_UN	8    /* Unlock.  */

/* Can be OR'd in to one of the above.  */
#define	LOCK_NB	4    /* Don't block when locking.  */


/* Apply or remove an advisory lock, according to OPERATION,
   on the file FD refers to.  */
extern int __flock __P ((int __fd, int __operation));
extern int flock __P ((int __fd, int __operation));


__END_DECLS

#endif /* sys/file.h  */
