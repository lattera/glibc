/* Copyright (C) 1994 Free Software Foundation, Inc.
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

#include <sysdep.h>

/* In SVR4 the `stat' call is actually done by the `xstat' system call,
   which takes an additional first argument giving a version number for
   `struct stat'.  Likewise for `fstat' and `lstat' there are `fxstat' and
   `lxstat' system calls.  This macro gives the SVR4 version number that
   corresponds to the definition of `struct stat' in <statbuf.h>.  */
#define	_STAT_VER	2

.globl syscall_error
ENTRY (__fstat)
	popl %eax		/* Pop return address into %eax.  */
	pushl $_STAT_VER	/* Push extra first arg to syscall.  */
	pushl %eax		/* Push back the return address.  */
	DO_CALL (fxstat, 3)	/* Do the syscall.   */
	jb syscall_error	/* Check for error.  */
	ret			/* Return success.  */
