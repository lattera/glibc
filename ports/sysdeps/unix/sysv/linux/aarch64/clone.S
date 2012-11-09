/* Copyright (C) 1996-2012  Free Software Foundation, Inc.

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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>

#define CLONE_VM_BIT      8
#define CLONE_VM          (1 << CLONE_VM_BIT)

#define CLONE_THREAD_BIT  16
#define CLONE_THREAD      (1 << CLONE_THREAD_BIT)

/* int clone(int (*fn)(void *arg),            x0
	     void *child_stack,               x1
	     int flags,                       x2
	     void *arg,                       x3
	     pid_t *ptid,                     x4
	     struct user_desc *tls,           x5
             pid_t *ctid);                    x6
 */
        .text
ENTRY(__clone)
	/* Sanity check args.  */
	cbz	x0, 1f
	cbz	x1, 1f
	/* Insert the args onto the new stack.  */
	stp	x0, x3, [x1, #-16]!	/* Fn, arg.  */

	/* Do the system call.  */
	mov	x0, x2                  /* flags  */

	/* New sp is already in x1.  */
	mov	x2, x4			/* ptid  */
	mov	x3, x5			/* tls  */
	mov	x4, x6			/* ctid  */

#ifdef RESET_PID
	/* We rely on the kernel preserving the argument regsiters across a
	   each system call so that we can inspect the flags against after
	   the clone call.  */
	mov	x5, x0
#endif

	mov	x8, #SYS_ify(clone)
	/* X0:flags, x1:newsp, x2:parenttidptr, x3:newtls, x4:childtid.  */
	svc	0x0
	cmp	x0, #0
	beq	2f
	blt	C_SYMBOL_NAME(__syscall_error)
	RET
1:	mov	x0, #-EINVAL
	b	syscall_error

2:
#ifdef RESET_PID
	tbnz	x5, #CLONE_THREAD_BIT, 3f
	mov	x0, #-1
	tbnz	x5, #CLONE_VM_BIT, 2f
	mov	x8, #SYS_ify(getpid)
	svc	0x0
2:
	mrs	x1, tpidr_el0
	sub	x1, x1, #PTHREAD_SIZEOF
	str	w0, [x1, #PTHREAD_PID_OFFSET]
	str	w0, [x1, #PTHREAD_TID_OFFSET]

3:
#endif
	/* Pick the function arg and call address from the stack and
	   execute.  */
	ldp	x1, x0, [sp], #16
	blr	x1

	/* We are done, pass the return value through x0.  */
	b	HIDDEN_JUMPTARGET(_exit)

PSEUDO_END (__clone)

weak_alias (__clone, clone)
