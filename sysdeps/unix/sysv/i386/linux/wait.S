/* Copyright (C) 1991, 1992, 1995 Free Software Foundation, Inc.
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

ENTRY (__wait)
	movl 0(%esp), %eax	/* Fetch the return address.  */
	movl $-1, 0(%esp)	/* First arg is -1.  */
				/* Second arg is our arg at 4(%esp).  */
	pushl $0		/* Third arg is 0.  */
	pushl %eax		/* Push the return address.  */

	/* Jump to waitpid; it will return to our caller.  */
	jmp ___waitpid

weak_alias (__wait, wait)
