/* Copyright (C) 2003, 2004 Free Software Foundation, Inc.
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
	mov.l	r8, @-r15
	mov	r4, r6
	mov	r5, r8
	mov	#0, r7		/* No timeout.  */
	mov	#FUTEX_WAIT, r5

	mov	#2, r4
	cmp/eq	r4, r6
	bf	2f

1:
	mov	r8, r4
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

2:
	mov	#2, r4
	XCHG (r4, @r8, r2)
	tst	r2, r2
	bf	1b

	mov.l	@r15+, r8
	ret
	 mov	r2, r0
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
	mov	r4, r10
	mov	r6, r9
	mov	r5, r8

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

	mov	#1, r3
	mov	#2, r4
	CMPXCHG (r3, @r8, r4, r2)
	tst	r2, r2
	bt	8f

	mov	r8, r4
	mov	#FUTEX_WAIT, r5
	mov	r10, r6
	mov	r15, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	mov	r0, r5

8:
	mov	#0, r3
	mov	#2, r4
	CMPXCHG (r3, @r8, r4, r2)
	bf/s	7f
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
	cmp/eq	r5, r1
	bt	5f

	/* Make sure the current holder knows we are going to sleep.  */
	XCHG (r2, @r8, r3)
	tst	r3, r3
	bt/s	6b
	 mov	#0, r0
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


#ifdef NOT_IN_libc
	.globl	lll_unlock_wake_cb
	.type	lll_unlock_wake_cb,@function
	.hidden	lll_unlock_wake_cb
	.align	5
lll_unlock_wake_cb:
	DEC	(@r4, r2)
	tst	r2, r2
	bt	1f

	mov	#FUTEX_WAKE, r5
	mov	#1, r6		/* Wake one thread.  */
	mov	#0, r7
	mov.l	r7, @r4		/* Stores 0.  */
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

1:	
	rts
	 nop
	.size	lll_unlock_wake_cb,.-lll_unlock_wake_cb
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


#ifdef NOT_IN_libc
	.globl	__lll_timedwait_tid
	.type	__lll_timedwait_tid,@function
	.hidden	__lll_timedwait_tid
	.align	5
__lll_timedwait_tid:
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	mov	r4, r8
	mov	r5, r9

	/* Stack frame for the timespec and timeval structs.  */
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
	mov.w	.L1k2, r1
	dmulu.l	r0, r1		/* Micro seconds to nano seconds.  */
	mov.l	@r9, r2
	mov.l	@(4,r9), r3
	mov.l	@r15, r0
	sts	macl, r1
	sub	r0, r2
	clrt
	subc	r1, r3
	bf	5f
	mov.l	.L1g2, r1
	add	r1, r3
	add	#-1, r2
5:
	cmp/pz	r2
	bf	6f		/* Time is already up.  */

	mov.l	r2, @r15	/* Store relative timeout.  */
	mov.l	r3, @(4,r15)

	mov.l	@r8, r2
	tst	r2, r2
	bt	4f

	mov	r8, r4
	mov	#FUTEX_WAIT, r5
	mov	r2, r6
	mov	r15, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov.l	@r8, r2
	tst	r2, r2
	bf	1f
4:
	mov	#0, r0
3:
	add	#8, r15
	mov.l	@r15+, r8
	rts
	 mov.l	@r15+, r9
1:
	/* Check whether the time expired.  */
	mov	#-ETIMEDOUT, r1
	cmp/eq	r0, r1
	bf	2b
6:
	bra	3b
	 mov	#ETIMEDOUT, r0

.L1k2:
	.word	1000
	.align	2
.L1g2:
	.long	1000000000
	.size	__lll_timedwait_tid,.-__lll_timedwait_tid
#endif
