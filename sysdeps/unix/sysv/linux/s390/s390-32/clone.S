/* Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).

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

/* clone is even more special than fork as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

/* int __clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
               pid_t *parent_tid, void *tls, pid_t *child_tid);  */
/* sys_clone  (void *child_stack, unsigned long flags,
               pid_t *parent_tid, pid_t *child_tid, void *tls);  */

	.text
ENTRY(__clone)
	st	%r6,24(%r15)		/* store %r6 to save area */
	lr	%r0,%r5			/* move *arg out of the way */
	ltr     %r1,%r2			/* check fn and move to %r1 */
	jz      error			/* no NULL function pointers */
	ltr     %r2,%r3			/* check child_stack and move to %r2 */
	jz	error			/* no NULL stack pointers */
	lr	%r3,%r4			/* move flags to %r3 */
	lr	%r4,%r6			/* move parent_tid to %r4 */
	l	%r5,100(%r15)		/* load child_tid from stack */
	l	%r6,96(%r15)		/* load tls from stack */
	svc	SYS_ify(clone)
	ltr	%r2,%r2			/* check return code */
	jz	thread_start
	l	%r6,24(%r15)		/* restore %r6 */
	jm	SYSCALL_ERROR_LABEL
	br	%r14
error:
	lhi	%r2,-EINVAL
	j	SYSCALL_ERROR_LABEL
	
thread_start:
	/* fn is in gpr 1, arg in gpr 0 */
	lr      %r2,%r0         /* set first parameter to void *arg */
	ahi     %r15,-96        /* make room on the stack for the save area */
	xc	0(4,%r15),0(%r15)
	basr    %r14,%r1        /* jump to fn */
#ifdef PIC
	basr    %r12,0
.L0:    lr      %r1,%r12
	al      %r12,.L1-.L0(%r12)
	al      %r1,.L2-.L0(%r1)
	br      %r1             /* branch to _exit -> thread termination */
.L1:	.long   _GLOBAL_OFFSET_TABLE_ - .L0
.L2:	.long   _exit@PLT - .L0
#else
	basr    %r1,0
.L0:	al      %r1,.L1-.L0(0,%r1)
	br      %r1             /* branch to _exit -> thread termination */
.L1:	.long   _exit - .L0
#endif
PSEUDO_END (__clone)
weak_alias (__clone, clone)
