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
#include <lowlevelrwlock.h>
#include <pthread-errnos.h>
#include <tcb-offsets.h>
#include "lowlevel-atomic.h"

#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.text

	.globl	__pthread_rwlock_rdlock
	.type	__pthread_rwlock_rdlock,@function
	.align	5
__pthread_rwlock_rdlock:
	mov.l	r12, @-r15
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	mov	r4, r8

	/* Get the lock.  */
	mov	#0, r3
	mov	#1, r4
#if MUTEX == 0
	CMPXCHG (r3, @r8, r4, r2)
#else
	CMPXCHG (r3, @(MUTEX,r8), r4, r2)
#endif
	bf	1f
2:
	mov.l	@(WRITER,r8), r0
	tst	r0, r0
	bf	14f
	mov.l	@(WRITERS_QUEUED,r8), r0
	tst	r0, r0
	bt	5f
	mov.l	@(FLAGS,r8), r0
	tst	r0, r0
	bt	5f
3:
	mov.l	@(READERS_QUEUED,r8), r0
	add	#1, r0
	mov.l	r0, @(READERS_QUEUED,r8)
	tst	r0, r0
	bt	4f

	mov.l	@(READERS_WAKEUP,r8), r9

#if MUTEX == 0
	DEC (@r8, r2)
#else
	DEC (@(MUTEX,r8), r2)
#endif
	tst	r2, r2
	bf	10f
11:
	mov	r8, r4
	add	#READERS_WAKEUP, r4
	mov	#FUTEX_WAIT, r5
	mov	r9, r6
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	/* Reget the lock.  */
	mov	#0, r3
	mov	#1, r4
#if MUTEX == 0
	CMPXCHG (r3, @r8, r4, r2)
#else
	CMPXCHG (r3, @(MUTEX,r8), r4, r2)
#endif
	bf	12f
13:
	mov.l	@(READERS_QUEUED,r8), r0
	add	#-1, r0
	bra	2b
	 mov.l	r0, @(READERS_QUEUED,r8)

5:
	mov	#0, r3
	mov.l	@(NR_READERS,r8), r0
	add	#1, r0
	mov.l	r0, @(NR_READERS,r8)
	tst	r0, r0
	bt	8f

9:
#if MUTEX == 0
	DEC (@r8, r2)
#else
	DEC (@(MUTEX,r8), r2)
#endif
	tst	r2, r2
	bf	6f
7:
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	mov.l	@r15+, r12
	rts
	 mov	r3, r0

1:
	mov	r8, r5
#if MUTEX != 0
	add	#MUTEX, r5
#endif
	mov	r2, r4
	mov.l	.Lwait0, r1
	bsrf	r1
	 nop
.Lwait0b:
	bra	2b
	 nop
14:
	stc	gbr, r1
	mov.w	.Ltidoff, r2
	add	r2, r1
	mov.l	@r1, r1
	cmp/eq	r1, r0
	bf	3b
	/* Deadlock detected.  */
	bra	9b
	 mov	#EDEADLK, r3

.Ltidoff:
	.word	TID - TLS_PRE_TCB_SIZE
	
6:
	mov	r8, r4
#if MUTEX != 0
	add	#MUTEX, r4
#endif
	mov.l	.Lwake0, r1
	bsrf	r1
	 nop
.Lwake0b:
	bra	7b
	 mov	#0, r3

8:
	/* Overflow.  */
	mov.l	@(NR_READERS,r8), r1
	add	#-1, r1
	mov.l	r1, @(NR_READERS,r8)
	bra	9b
	 mov	#EAGAIN, r3

4:
	/* Overflow.  */
	mov.l	@(READERS_QUEUED,r8), r1
	add	#-1, r1
	mov.l	r1, @(READERS_QUEUED,r8)
	bra	9b
	 mov	#EAGAIN, r3

10:
	mov	r8, r4
#if MUTEX != 0
	add	#MUTEX, r4
#endif
	mov.l	.Lwake1, r1
	bsrf	r1
	 nop
.Lwake1b:
	bra	11b
	 nop

12:
	mov	r8, r5
#if MUTEX != 0
	add	#MUTEX, r5
#endif
	mov	r2, r4
	mov.l	.Lwait1, r1
	bsrf	r1
	 nop
.Lwait1b:
	bra	13b
	 nop

	.align	2
.Lwait0:
	.long	__lll_mutex_lock_wait-.Lwait0b
.Lwake0:
	.long	__lll_mutex_unlock_wake-.Lwake0b
.Lwait1:
	.long	__lll_mutex_lock_wait-.Lwait1b
.Lwake1:
	.long	__lll_mutex_unlock_wake-.Lwake1b
	.size	__pthread_rwlock_rdlock,.-__pthread_rwlock_rdlock

	.globl	pthread_rwlock_rdlock
pthread_rwlock_rdlock = __pthread_rwlock_rdlock

	.globl	__pthread_rwlock_rdlock_internal
__pthread_rwlock_rdlock_internal = __pthread_rwlock_rdlock
