/* Wrapper around clone system call.  64 bit S/390 version.
   Copyright (C) 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).
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

/* clone is even more special than fork as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

/* int __clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
               void *tls, pid_t *parent_tid, pid_t *child_tid);  */
/* sys_clone  (void *child_stack, unsigned long flags,
               pid_t *parent_tid, pid_t *child_tid, void *tls);  */

	.text
ENTRY(__clone)
	/* Sanity check arguments & move registers */
	lgr	%r0,%r5			/* move *arg out of the way */
	ltgr	%r1,%r2			/* no NULL function pointers */
	lghi	%r2,-EINVAL
	jgz	SYSCALL_ERROR_LABEL
	ltgr	%r3,%r3			/* no NULL stack pointers */
	jgz	SYSCALL_ERROR_LABEL
	/* set up registers, then call SVC */
	lgr	%r2,%r3
	lgr	%r3,%r4
	lmg	%r4,%r5,160(%r15)
	svc	SYS_ify(clone)
	ltgr	%r2,%r2			/* check return code */
	jgm	SYSCALL_ERROR_LABEL
	jz	thread_start
	br	%r14

thread_start:
	/* fn is in gpr 1, arg in gpr 0 */
	lgr	%r2,%r0		/* set first parameter to void *arg */
	aghi	%r15,-160	/* make room on the stack for the save area */
	xc	0(8,%r15),0(%r15)
	basr	%r14,%r1	/* jump to fn */
#ifdef PIC
	jg	_exit@PLT	/* branch to _exit -> thread termination */
#else
	jg	_exit		/* branch to _exit -> thread termination */
#endif
PSEUDO_END (__clone)
weak_alias (__clone, clone)
