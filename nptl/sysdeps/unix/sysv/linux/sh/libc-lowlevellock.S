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
#include <tls.h>
#include "lowlevel-atomic.h"

	.text

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

#define ETIMEDOUT               110


	.globl	__lll_lock_wait
	.type	__lll_lock_wait,@function
	.hidden	__lll_lock_wait
	.align	5
__lll_lock_wait:
	mov	r4, r6
	mov	r5, r4
	mov	#0, r7		/* No timeout.  */
	mov	#FUTEX_WAIT, r5
2:
	add	#-1, r6		/* account for the preceeded xadd.  */
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov	#-1, r3
	XADD (r3, @r4, r2)
	tst	r3, r3
	bf/s	2b
	 mov	r2, r6

	mov	#-1, r1
	mov.l	r1, @r4
	rts
	 mov	r2, r0
	.size	__lll_lock_wait,.-__lll_lock_wait


	.type	lll_unlock_wake_cb,@function
	.align	5
lll_unlock_wake_cb:

	.align	2
	mova	1f, r0
	mov	r15, r1
	mov	#-6, r15
0:
	mov.l	@r4, r2
	add	#1, r2
	mov.l	r2, @r4
1:
	mov	r1, r15
	cmp/pl	r2
	bf	2f
	rts
	 nop
	.size	lll_unlock_wake_cb,.-lll_unlock_wake_cb


	.globl	__lll_unlock_wake
	.type	__lll_unlock_wake,@function
	.hidden	__lll_unlock_wake
__lll_unlock_wake:
2:
	mov	#FUTEX_WAKE, r5
	mov	#1, r6		/* Wake one thread.  */
	mov	#0, r7
	mov.l	r6, @r4		/* Stores 1.  */
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	rts
	 nop
	.size	__lll_unlock_wake,.-__lll_unlock_wake


	.globl	__lll_wait_tid
	.type	__lll_wait_tid,@function
	.hidden	__lll_wait_tid
__lll_wait_tid:
	mov.l	@r4, r6
1:
	mov	#FUTEX_WAIT, r5
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov	r0, r1

	mov.l	@r4, r0
	tst	r0, r0
	bf/s	1b
	 mov	r0, r6
	rts
	 nop
	.size	__lll_wait_tid,.-__lll_wait_tid

	
	.globl	__lll_timedwait_tid
	.type	__lll_timedwait_tid,@function
	.hidden	__lll_timedwait_tid
__lll_timedwait_tid:
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	mov	r4, r8
	mov	r5, r9
	add	#-8, r15

2:
	/* Get current time.  */
	mov	r15, r4
	mov	#0, r5
	mov	#SYS_gettimeofday, r3
	trapa	#0x12
	SYSCALL_INST_PAD

	/* Compute relative timeout.  */
	mov.l	@(4,r15), r0
	mov.w	.L1k, r1
	dmulu.l	r0, r1		/* Milli seconds to nano seconds.  */
	mov.l	@r9, r2
	mov.l	@(4,r9), r3
	mov.l	@r15, r0
	sts	macl, r1
	sub	r0, r2
	clrt
	subc	r1, r3
	bf	5f
	mov.l	.L1g, r1
	add	r1, r3
	add	#-1, r2
5:
	cmp/pz	r2
	bf	6f		/* Time is already up.  */

	mov.l	r2, @r15	/* Store relative timeout.  */
	mov.l	r3, @(4,r15)

	mov.l	@r8, r6
	tst	r6, r6
	bt	4f

	mov	r8, r4
	mov	#FUTEX_WAIT, r5
	mov	r15, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov	r0, r1

	mov.l	@r8, r0
	tst	r0, r0
	bf	1f
4:
	mov	#0, r0
3:
	add	#8, r15
	mov.l	@r15+, r8
	rts
	 mov.l	@r15+, r9

1:
	mov	#-ETIMEDOUT, r1
	cmp/eq	r0, r1
	bf	2b
6:
	bra	3b
	 mov	#ETIMEDOUT, r0

.L1k:
	.word	1000
	.align	2
.L1g:
	.long	1000000000

	.size	__lll_timedwait_tid,.-__lll_timedwait_tid


	.globl	__lll_wake_tid
	.type	__lll_wake_tid,@function
	.hidden	__lll_wake_tid
__lll_wake_tid:
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
	.size	__lll_wake_tid,.-__lll_wake_tid
