/* Copyright (C) 1996-2012 Free Software Foundation, Inc.
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
	cmpne	r1, #0
	moveq	r0, #-EINVAL
	beq	PLTJMP(syscall_error)

	@ insert the args onto the new stack
	str	r3, [r1, #-4]!
	str	r0, [r1, #-4]!

	@ do the system call
	@ get flags
	mov	r0, r2
#ifdef RESET_PID
	mov	ip, r2
#endif
	@ new sp is already in r1
	stmfd	sp!, {r4, r7}
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
	ldmfd	sp!, {r4, r7}
	blt	PLTJMP(C_SYMBOL_NAME(__syscall_error))
	RETINSTR(, lr)

	cfi_startproc
PSEUDO_END (__clone)

1:
	.fnstart
	.cantunwind
#ifdef RESET_PID
	tst	ip, #CLONE_THREAD
	bne	3f
	mov	r0, #0xffff0fff
	mov	lr, pc
	sub	pc, r0, #31
	mov	r1, r0
	tst	ip, #CLONE_VM
	movne	r0, #-1
	ldr	r7, =SYS_ify(getpid)
	swieq	0x0
	str	r0, [r1, #PID_OFFSET]
	str	r0, [r1, #TID_OFFSET]
3:
#endif
	@ pick the function arg and call address off the stack and execute
	ldr	r0, [sp, #4]
#if defined(__ARM_ARCH_4T__) && defined(__THUMB_INTERWORK__)
	ldr 	ip, [sp], #8
	mov	lr, pc
	bx      ip
#else
	mov	lr, pc
	ldr 	pc, [sp], #8
#endif

	@ and we are done, passing the return value through r0
	b	PLTJMP(HIDDEN_JUMPTARGET(_exit))

	.fnend

weak_alias (__clone, clone)
