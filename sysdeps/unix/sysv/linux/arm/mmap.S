/* Copyright (C) 1998, 2000 Free Software Foundation, Inc.
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

#include <sysdep.h>

	.text

ENTRY (__mmap)

	/* Because we can only get five args through the syscall interface, and
	   mmap() takes six, we need to build a parameter block and pass its
	   address instead.  The 386 port does a similar trick.  */

	/* This code previously moved sp into ip and stored the args using
	   stmdb ip!, {a1-a4}.  It did not modify sp, so the stack never had
	   to be restored after the syscall completed.  It saved an
	   instruction and meant no stack cleanup work was required.

	   This will not work in the case of a mmap call being interrupted
	   by a signal.  If the signal handler uses any stack the arguments
	   to mmap will be trashed.  The results of a restart of mmap are
	   then unpredictable. */

	/* store args on the stack */
	stmdb	sp!, {a1-a4}

	/* do the syscall */
	mov	a1, sp
	swi	SYS_ify (mmap)

	/* pop args off the stack. */
	add	sp, sp, #16

	cmn	r0, $4096
	RETINSTR(movcc, pc, lr)
	b	PLTJMP(syscall_error);

PSEUDO_END (__mmap)

weak_alias (__mmap, mmap)
