/* Wrapper around clone system call.  64 bit S/390 version.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

/* clone is even more special than fork as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

	.text
ENTRY(__clone)
	/* Sanity check arguments & move registers */
	ltgr	%r1,%r2			/* no NULL function pointers */
	lghi	%r2,-EINVAL
	jz	SYSCALL_ERROR_LABEL
	ltgr	%r3,%r3			/* no NULL stack pointers */
	jz	SYSCALL_ERROR_LABEL
	/* move child_stack and flags, then call SVC */
	lgr	%r2,%r3
	lgr	%r3,%r4
	svc	SYS_ify(clone)
	ltgr	%r2,%r2			/* check return code */
	jm	SYSCALL_ERROR_LABEL
	jz	thread_start
	br	%r14

thread_start:
	/* fn is in gpr 1, arg in gpr 5 */
	lgr	%r2,%r5		/* set first parameter to void *arg */
	sgr	%r11,%r11	/* terminate the stack frame */
	aghi	%r15,-160	/* make room on the stack for the save area */
	basr	%r14,%r1	/* jump to fn */
#ifdef PIC
	larl	%r12,_GLOBAL_OFFSET_TABLE_
#endif
	jg	_exit@PLT	/* branch to _exit -> thread termination */
PSEUDO_END (__clone)
