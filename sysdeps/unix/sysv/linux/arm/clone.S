/* Copyright (C) 1996-2015 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Pat Beirne <patb@corelcomputer.com>

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#include <tcb-offsets.h>
#define _ERRNO_H	1
#include <bits/errno.h>

#define CLONE_VM      0x00000100
#define CLONE_THREAD  0x00010000

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg,
	     pid_t *ptid, struct user_desc *tls, pid_t *ctid); */

        .text
ENTRY(__clone)
	@ sanity check args
	cmp	r0, #0
	ite	ne
	cmpne	r1, #0
	moveq	r0, #-EINVAL
	beq	PLTJMP(syscall_error)

	@ insert the args onto the new stack
	str	r3, [r1, #-4]!
	str	r0, [r1, #-4]!

	@ do the system call
	@ get flags
	mov	r0, r2
	mov	ip, r2
	@ new sp is already in r1
	push	{r4, r7}
	cfi_adjust_cfa_offset (8)
	cfi_rel_offset (r4, 0)
	cfi_rel_offset (r7, 4)
	ldr	r2, [sp, #8]
	ldr	r3, [sp, #12]
	ldr	r4, [sp, #16]
	ldr	r7, =SYS_ify(clone)
	swi	0x0
	cfi_endproc
	cmp	r0, #0
	beq	1f
	pop	{r4, r7}
	blt	PLTJMP(C_SYMBOL_NAME(__syscall_error))
	RETINSTR(, lr)

	cfi_startproc
PSEUDO_END (__clone)

1:
	.fnstart
	.cantunwind
	tst	ip, #CLONE_THREAD
	bne	3f
	GET_TLS (lr)
	mov	r1, r0
	tst	ip, #CLONE_VM
	ldr	r7, =SYS_ify(getpid)
	ite	ne
	movne	r0, #-1
	swieq	0x0
	NEGOFF_ADJ_BASE (r1, TID_OFFSET)
	str	r0, NEGOFF_OFF1 (r1, TID_OFFSET)
	str	r0, NEGOFF_OFF2 (r1, PID_OFFSET, TID_OFFSET)
3:
	@ pick the function arg and call address off the stack and execute
	ldr	r0, [sp, #4]
	ldr 	ip, [sp], #8
	BLX (ip)

	@ and we are done, passing the return value through r0
	b	PLTJMP(HIDDEN_JUMPTARGET(_exit))

	.fnend

weak_alias (__clone, clone)
