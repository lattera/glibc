/* Copyright (C) 1991, 1992, 1993, 1994 Free Software Foundation, Inc.
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

/*
 *	ANSI Standard: 4.1.3 Errors	<errno.h>
 */

#ifndef	_ERRNO_H

#define	_ERRNO_H	1
#include <features.h>

__BEGIN_DECLS

/* Get the error number constants.  */
#include <errnos.h>

/* Declare the `errno' variable.  */
extern int errno;

#ifdef __USE_GNU
/* The full and simple forms of the name with which the program was
   invoked.  These variables are set up automatically at startup based on
   the value of ARGV[0] (this works only if you use GNU ld).  */
extern char *program_invocation_name, *program_invocation_short_name;
#endif

__END_DECLS

#endif /* errno.h	*/
