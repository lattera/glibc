/* Copyright 2001 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Alexandre Oliva <aoliva@redhat.com>.
   Based on ../i386/clone.S.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public License as
   published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

/* clone() is even more special than fork() as it mucks with stacks
   and invokes a function in the right context after its all over.  */

#include <sysdep.h>
#define _ERRNO_H	1
#include <bits/errno.h>
#include <asm-syntax.h>
#include <bp-sym.h>

/* int clone(int (*fn)(void *arg), void *child_stack, int flags, void *arg); */

        .text
ENTRY (BP_SYM (__clone))
	/* Sanity check arguments.  */
	cmp	0,d0	/* no NULL function pointers */
	beq	L(error_inval)
	cmp	0,d1	/* no NULL stack pointers */
	beq	L(error_inval)

	/* Allocate room for a function call in the new stack, and
	   store fn and arg in it.  They will be read back in
	   thread_start.  */
	mov	d1,a0
	sub	12,a0
	mov	d0,(a0)
	mov	(16,sp),d1
	mov	d1,(4,a0)

	/* Do the system call */
	mov	a0,d1
	mov	(12,sp),a0
	mov	SYS_ify(clone),d0
	syscall	0

	cmp	0,d0
	beq	thread_start
	blt	L(to_SYSCALL_ERROR_LABEL)

L(pseudo_end):
	ret

L(error_inval):
	mov	-EINVAL,d0
L(to_SYSCALL_ERROR_LABEL):
	jmp	SYSCALL_ERROR_LABEL

thread_start:
	mov	0,a3	/* terminate the stack frame */
	mov	(4,sp),d0
	mov	(sp),a0
	calls	(a0)
#ifdef PIC
L(here):
	mov	pc,a2
	add	_GLOBAL_OFFSET_TABLE_-(L(here) - .),a2
#endif
	call	JUMPTARGET (_exit),[],0

PSEUDO_END (BP_SYM (__clone))

weak_alias (BP_SYM (__clone), BP_SYM (clone))
