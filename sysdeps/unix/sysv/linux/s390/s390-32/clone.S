/* Copyright (C) 2000, 2001 Free Software Foundation, Inc.
   Contributed by Martin Schwidefsky (schwidefsky@de.ibm.com).

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

/*int __clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg);*/
/* sys_clone(void *child_stack, unsigned long flags) */

	.text
ENTRY(__clone)
	/* Sanity check arguments & move registers */
	ltr     %r1,%r2			/* no NULL function pointers */
	lhi     %r2,-EINVAL
	jz      SYSCALL_ERROR_LABEL
	ltr     %r3,%r3			/* no NULL stack pointers */
	jz      SYSCALL_ERROR_LABEL
	/* move child_stack and flags, then call SVC */
	lr      %r2,%r3
	lr      %r3,%r4
	svc     SYS_ify(clone)
	ltr     %r2,%r2			/* check return code */
	jm      SYSCALL_ERROR_LABEL
	jz      thread_start
	br      %r14

thread_start:
	/* fn is in gpr 1, arg in gpr 5 */
	lr      %r2,%r5         /* set first parameter to void *arg */
	sr      %r11,%r11	/* terminate the stack frame */
	ahi     %r15,-96        /* make room on the stack for the save area */
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
