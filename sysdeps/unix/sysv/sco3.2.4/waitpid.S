/* Copyright (C) 1993, 1994 Free Software Foundation, Inc.
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

ENTRY (__waitpid)
	/* The `waitpid' system call is distinguished from plain
	   `wait' by setting lots of bits in the processor flags.  */
	pushfl			/* Push the flags word.  */
	popl %eax		/* Pop it into the accumulator.  */
	orl $0x8c4, %eax	/* Set lots of bits.  */
	pushl %eax		/* Push the new flags word.  */
	popfl			/* Pop it into the flags.  */
	DO_CALL (wait, 2)
	movl 8(%esp), scratch	/* Put status pointer in scratch register.  */
	testl scratch, scratch	/* Is it non-nil?  */
	je null
	movl r1, (scratch)	/* Yes; store the status there.  */
null:	ret
