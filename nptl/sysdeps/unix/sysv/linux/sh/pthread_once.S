/* Copyright (C) 2003 Free Software Foundation, Inc.
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
#include "lowlevel-atomic.h"

#define SYS_futex	240
#define FUTEX_WAIT	0
#define FUTEX_WAKE	1

	.comm	__fork_generation, 4, 4

	.text
	.globl	__pthread_once
	.type	__pthread_once,@function
	.align	5
__pthread_once:
	mov.l	@r4, r0
	tst	#2, r0
	bt	1f
	rts
	 mov	#0, r0

1:
	mov.l	r12, @-r15
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	mov	r5, r8

	/* Not yet initialized or initialization in progress.
	   Get the fork generation counter now.  */
6:
	mov.l	@r4, r1
#ifdef PIC
	mova	.Lgot, r0
	mov.l	.Lgot, r12
	add	r0, r12
#endif

5:
	mov	r1, r0

	tst	#2, r0
	bf	4f

	and	#3, r0
	mov.l	.Lfgen, r2
#ifdef PIC
	add	r12, r2
#endif
	mov.l	@r2, r3
	or	r3, r0	
	or	#1, r0
	mov	r0, r3
	mov	r1, r5

	CMPXCHG (r5, @r4, r3, r2)
	bf	5b

	/* Check whether another thread already runs the initializer.  */
	mov	r2, r0
	tst	#1, r0
	bt	3f	/* No -> do it.  */

	/* Check whether the initializer execution was interrupted
	   by a fork.  */
	xor	r3, r0
	mov	#-4, r1	/* -4 = 0xfffffffc */
	tst	r1, r0
	bf	3f	/* Different for generation -> run initializer.  */

	/* Somebody else got here first.  Wait.  */
	mov	#FUTEX_WAIT, r5
	mov	r3, r6
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	bra	6b
	 nop

	.align	2
#ifdef PIC
.Lgot:
	.long	_GLOBAL_OFFSET_TABLE_
.Lfgen:	
	.long	__fork_generation@GOTOFF
#else
.Lfgen:	
	.long	__fork_generation
#endif

3:
	/* Call the initializer function after setting up the
	   cancellation handler.  */
	/* Allocate a _pthread_cleanup_buffer on stack.  */
	add	#-16, r15

	/* Push the cleanup handler.  */
	mov	r4, r9
	mov	r15, r4
	mov.l	.Lconce, r5
#ifdef PIC
	add	r12, r5
#endif
	mov.l	.Lcpush, r1
	bsrf	r1
	 mov	r9, r6
.Lcpush0:
	jsr	@r8
	 nop

	/* Pop the cleanup handler.  */
	mov	r15, r4
	mov.l	.Lcpop, r1
	bsrf	r1
	 mov	#0, r5
.Lcpop0:
	add	#16, r15

	/* Sucessful run of the initializer.  Signal that we are done.  */
	INC (@r9, r2)
	/* Wake up all other threads.  */
	mov	r9, r4
	mov	#FUTEX_WAKE, r5
	mov	#-1, r6
	shlr	r6		/* r6 = 0x7fffffff */
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

4:
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	mov.l	@r15+, r12
	rts
	 mov	#0, r0

	.align	2
.Lconce:
#ifdef PIC
	.long	clear_once_control@GOTOFF
#else
	.long	clear_once_control
#endif
.Lcpush:
	.long	__pthread_cleanup_push - .Lcpush0	/* Note: no @PLT.  */
.Lcpop:
	.long	__pthread_cleanup_pop - .Lcpop0		/* Note: no @PLT.  */

	.size	__pthread_once,.-__pthread_once

	.globl	__pthread_once_internal
__pthread_once_internal = __pthread_once

	.globl	pthread_once
pthread_once = __pthread_once


	.type	clear_once_control,@function
	.align	5
clear_once_control:
	mov	#0, r0
	mov.l	r0, @r4

	mov	#FUTEX_WAKE, r5
	mov	#-1, r6
	shlr	r6		/* r6 = 0x7fffffff */
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	rts
	 nop
	.size	clear_once_control,.-clear_once_control
