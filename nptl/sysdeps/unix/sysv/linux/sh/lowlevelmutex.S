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
#include <pthread-errnos.h>
#include "lowlevel-atomic.h"

	.text

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.globl	__lll_mutex_lock_wait
	.type	__lll_mutex_lock_wait,@function
	.hidden	__lll_mutex_lock_wait
	.align	5
__lll_mutex_lock_wait:
	mov	r4, r6
	mov	r5, r4
	mov	#0, r7		/* No timeout.  */
	mov	#FUTEX_WAIT, r5
1:
	add	#1, r6		/* account for the preceeded xadd.  */
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov	#1, r3
	XADD (r3, @r4, r6)
	tst	r6, r6
	bf	1b
	mov	#2, r1
	mov.l	r1, @r4
	ret
	 mov	#0, r0
	.size	__lll_mutex_lock_wait,.-__lll_mutex_lock_wait


#ifdef NOT_IN_libc
	.globl	__lll_mutex_timedlock_wait
	.type	__lll_mutex_timedlock_wait,@function
	.hidden	__lll_mutex_timedlock_wait
	.align	5
__lll_mutex_timedlock_wait:
	/* Check for a valid timeout value.  */
	mov.l	@(4,r6), r1
	mov.l	.L1g, r0
	cmp/hs	r0, r1
	bt	3f

	mov.l	r10, @-r15
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	mov	r5, r8
	mov	r6, r9
	mov	r4, r10
	add	#1, r10

	/* Stack frame for the timespec and timeval structs.  */
	add	#-8, r15

1:
	/* Get current time.  */
	mov	r15, r4
	mov	#0, r5
	mov	#SYS_gettimeofday, r3
	trapa	#0x12
	SYSCALL_INST_PAD

	/* Compute relative timeout.  */
	mov.l	@(4,r15), r0
	mov.w	.L1k, r1
	dmulu.l	r0, r1		/* Micro seconds to nano seconds.  */
	mov.l	@r9, r2
	mov.l	@(4,r9), r3
	mov.l	@r15, r0
	sts	macl, r1
	sub	r0, r2
	clrt
	subc	r1, r3
	bf	4f
	mov.l	.L1g, r1
	add	r1, r3
	add	#-1, r2
4:
	cmp/pz	r2
	bf	5f		/* Time is already up.  */

	mov.l	r2, @r15	/* Store relative timeout.  */
	mov.l	r3, @(4,r15)

	mov	r8, r4
	mov	#FUTEX_WAIT, r5
	mov	r10, r6
	mov	r15, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov	#1, r3
	XADD (r3, @r8, r10)
	tst	r10, r10
	bf	7f

	mov	#2, r1
	mov.l	r1, @r8
	mov	#0, r0
6:
	add	#8, r15
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	rts
	 mov.l	@r15+, r10
7:
	/* Check whether the time expired.  */
	mov	#-ETIMEDOUT, r1
	cmp/eq	r0, r1
	bt	5f
	bra	1b
	 nop
3:
	rts
	 mov	#EINVAL, r0
5:
	bra	6b
	 mov	#ETIMEDOUT, r0

.L1k:
	.word	1000
	.align	2
.L1g:
	.long	1000000000

	.size	__lll_mutex_timedlock_wait,.-__lll_mutex_timedlock_wait
#endif


	.globl	__lll_mutex_unlock_wake
	.type	__lll_mutex_unlock_wake,@function
	.hidden	__lll_mutex_unlock_wake
	.align	5
__lll_mutex_unlock_wake:
	mov	#FUTEX_WAKE, r5
	mov	#1, r6		/* Wake one thread.  */
	mov	#0, r7
	mov.l	r7, @r4		/* Stores 0.  */
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	rts
	 nop
	.size	__lll_mutex_unlock_wake,.-__lll_mutex_unlock_wake
