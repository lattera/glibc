/* Copyright (C) 1996, 1997 Free Software Foundation, Inc.
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

/*
 * Never include this file directly; use <limits.h> instead.
 */

/* Additional definitions from X/Open Portability Guide, Issue 4, Version 2
   System Interfaces and Headers, 4.16 <limits.h>

   Please note only the values which are not greater than the minimum
   stated in the standard document are listed.  The `sysconf' functions
   should be used to obtain the actual value.  */

#ifndef _XOPEN_LIM_H
#define _XOPEN_LIM_H	1

#define __need_FOPEN_MAX
#include <bits/stdio_lim.h>

/* We do not provide fixed values for

   ARG_MAX	Maximum length of argument to the `exec' function
		including environment data.

   ATEXIT_MAX	Maximum number of functions that may be registered
		with `atexit'.

   CHILD_MAX	Maximum number of simultaneous processes per real
		user ID.

   OPEN_MAX	Maximum number of files that one process can have open
		at anyone time.

   PAGESIZE
   PAGE_SIZE	Size of bytes of a page.

   PASS_MAX	Maximum number of significant bytes in a password.
*/


/* Maximum number of `iovec' structures that one process has available
   for use with `readv' or writev'.  */
#define IOV_MAX		_XOPEN_IOV_MAX

/* The number of streams that one process can have open at one time.  */
#define STREAM_MAX	FOPEN_MAX

/* Maximum number of bytes supported for the name of a time zone.  */
#define TZNAME_MAX	_POSIX_TZNAME_MAX


/* Maximum number of `iovec' structures that one process has available
   for use with `readv' or writev'.  */
#define	_XOPEN_IOV_MAX	_POSIX_UIO_MAXIOV


/* Maximum value of `digit' in calls to the `printf' and `scanf'
   functions.  We have no limit, so return a reasonable value.  */
#define NL_ARGMAX	_POSIX_ARG_MAX

/* Maximum number of bytes in a `LANG' name.  We have no limit.  */
#define NL_LANGMAX	_POSIX2_LINE_MAX

/* Maximum message number.  We have no limit.  */
#define NL_MSGMAX	INT_MAX

/* Maximum number of bytes in N-to-1 collation mapping.  We have no
   limit.  */
#define NL_NMAX		INT_MAX

/* Maximum set number.  We have no limit.  */
#define NL_SETMAX	INT_MAX

/* Maximum number of bytes in a message.  We have no limit.  */
#define NL_TEXTMAX	INT_MAX

/* Default process priority.  */
#define NZERO		20

#endif /* bits/xopen_lim.h */
