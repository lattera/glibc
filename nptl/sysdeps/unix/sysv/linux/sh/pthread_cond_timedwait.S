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
#include <shlib-compat.h>
#include <lowlevelcond.h>
#include <pthread-errnos.h>
#include "lowlevel-atomic.h"

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.text

/* int pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			       const struct timespec *abstime)  */
	.globl	__pthread_cond_timedwait
	.type	__pthread_cond_timedwait, @function
	.align	5
__pthread_cond_timedwait:
	mov.l	r12, @-r15
	mov.l	r10, @-r15
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	add	#-48, r15
	mov	r4, r8
	mov	r5, r9
	mov	r6, r10

	/* Get internal lock.  */
	mov	#1, r3
#if cond_lock != 0
	XADD (r3, @(cond_lock,r8), r2)
#else
	XADD (r3, @r8, r2)
#endif
	tst	r2, r2
	bt	2f
	bra	1f
	 nop
2:
	/* Unlock the mutex.  */
	mov.l	.Lmunlock1, r1
	bsrf	r1
	 mov	r9, r4
.Lmunlock1b:

	mov	#1, r2
	mov	#0, r3

	clrt
	mov.l	@(total_seq,r8),r0
	mov.l	@(total_seq+4,r8),r1
	addc	r2, r0
	addc	r3, r1
	mov.l	r0,@(total_seq,r8)
	mov.l	r1,@(total_seq+4,r8)

	/* Install cancellation handler.  */
#ifdef PIC
	mova	.Lgot1, r0
	mov.l	.Lgot1, r12
	add	r0, r12
	mov.l	.Lccleanup1, r5
	add	r12, r5
#else
	mov.l	.Lccleanup1, r5
#endif
	mov	r15, r4
	add	#28, r4

	mov.l	.Lccpush1, r1
	bsrf	r1
	 mov	r8, r6
.Lccpush1b:

	/* Get and store current wakeup_seq value.  */
	mov.l	@(wakeup_seq,r8), r0
	mov.l	@(wakeup_seq+4,r8), r1
	mov.l	r0, @(12,r15)
	mov.l	r1, @(16,r15)

	/* Unlock.  */
8:
#if cond_lock != 0
	DEC (@(cond_lock,r8), r2)
#else
	DEC (@r8, r2)
#endif
	tst	r2, r2
	bt	4f
	bra	3f
	 nop
4:
	mov.l	.Lenable1, r1
	bsrf	r1
	 nop
.Lenable1b:
	mov.l	r0, @r15

	/* Get current time.  */
	mov	r15, r4
	add	#4, r4
	mov	#0, r5
	mov	#SYS_gettimeofday, r3
	trapa	#0x12
	SYSCALL_INST_PAD

	/* Compute relative timeout.  */
	mov.l	@(8,r15), r0
	mov.w	.L1k, r1
	dmulu.l	r0, r1		/* Milli seconds to nano seconds.  */
	mov.l	@r10, r2
	mov.l	@(4,r10), r3
	mov.l	@(4,r15), r0
	sts	macl, r1
	sub	r0, r2
	clrt
	subc	r1, r3
	bf	12f
	mov.l	.L1g, r1
	add	r1, r3
	add	#-1, r2
12:
	cmp/pz	r2
	bf	13f		/* Time is already up.  */

	/* Store relative timeout.  */
	mov.l	r2, @(4,r15)
	mov.l	r3, @(8,r15)

	mov	r15, r7
	add	#4, r7
	mov	#FUTEX_WAIT, r5
	mov.l	@(12,r15), r6
	mov	r8, r4
	add	#wakeup_seq, r4
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD
	mov.l	r0, @(20,r15)

	mov.l	.Ldisable1, r1
	bsrf	r1
	 mov.l	@r15, r4
.Ldisable1b:

	/* Lock.  */
	mov	#1, r3
#if cond_lock != 0
	XADD (r3, @(cond_lock,r8), r2)
#else
	XADD (r3, @r8, r2)
#endif
	tst	r2, r2
	bf	5f
6:
	mov.l	@(woken_seq,r8), r0
	mov.l	@(woken_seq+4,r8), r1

	mov.l	@(wakeup_seq,r8), r2
	mov.l	@(wakeup_seq+4,r8), r3

	mov.l	@(16,r15), r5
	cmp/hi	r5, r1
	bt	7f
	cmp/hi	r1, r5
	bt	15f

	mov.l	@(12,r15), r5
	cmp/hi	r0, r5
	bt	15f
7:
	cmp/hi	r1, r3
	bt	9f
	cmp/hi	r3, r1
	bt	15f
	cmp/hi	r0, r2
	bt	9f
15:
	mov.l	@(20,r15),r0
	cmp/eq	#-ETIMEDOUT, r0
	bf	8b
13:
	mov	#1, r2
	mov	#0, r3

	clrt
	mov.l	@(wakeup_seq,r8),r0
	mov.l	@(wakeup_seq+4,r8),r1
	addc	r2, r0
	addc	r3, r1
	mov.l	r0,@(wakeup_seq,r8)
	mov.l	r1,@(wakeup_seq+4,r8)
	mov	#ETIMEDOUT, r0
	bra	14f
	 mov.l	r0, @(24,r15)

9:
	mov	#0, r0
	mov.l	r0, @(24,r15)
14:
	mov	#1, r2
	mov	#0, r3

	clrt
	mov.l	@(woken_seq,r8),r0
	mov.l	@(woken_seq+4,r8),r1
	addc	r2, r0
	addc	r3, r1
	mov.l	r0,@(woken_seq,r8)
	mov.l	r1,@(woken_seq+4,r8)

#if cond_lock != 0
	DEC (@(cond_lock,r8), r2)
#else
	DEC (@r8, r2)
#endif
	tst	r2, r2
	bf	10f

11:
	/* Remove cancellation handler.  */
	mov	r15, r4
	add	#28, r4
	mov.l	.Lcpop1, r1
	bsrf	r1
	 mov	#0, r5
.Lcpop1b:

	mov	r9, r4
	mov.l	.Lmlocki1, r1
	bsrf	r1
	 mov	#0, r5
.Lmlocki1b:

	mov.l	@(24,r15), r0

	add	#48, r15

	/* We return the result of the mutex_lock operation.  */
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	mov.l	@r15+, r10
	rts
	 mov.l	@r15+, r12
	ret

.L1k:
	.word	1000
	.align	2
.Lmunlock1:
	.long	__pthread_mutex_unlock_internal-.Lmunlock1b
#ifdef PIC
.Lgot1:
	.long	_GLOBAL_OFFSET_TABLE_
.Lccleanup1:
	.long	__condvar_cleanup@GOTOFF
#else
.Lccleanup1:
	.long	__condvar_cleanup
#endif
.Lccpush1:
	.long	__pthread_cleanup_push-.Lccpush1b
.Lenable1:
	.long	__pthread_enable_asynccancel-.Lenable1b
.Ldisable1:
	.long	__pthread_disable_asynccancel-.Ldisable1b
.Lcpop1:
	.long	__pthread_cleanup_pop-.Lcpop1b
.Lmlocki1:
	.long	__pthread_mutex_lock_internal-.Lmlocki1b
.L1g:
	.long	1000000000

1:
	/* Initial locking failed.  */
	mov	r8, r5
#if cond_lock != 0
	add	#cond_lock, r5
#endif
	mov.l	.Lmwait2, r1
	bsrf	r1
	 mov	r2, r4
.Lmwait2b:
	bra	2b
	 nop

3:
	/* Unlock in loop requires waekup.  */
	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov.l	.Lmwake2, r1
	bsrf	r1
	 nop
.Lmwake2b:
	bra	4b
	 nop

5:
	/* Locking in loop failed.  */
	mov	r8, r5
#if cond_lock != 0
	add	#cond_lock, r5
#endif
	mov.l	.Lmwait3, r1
	bsrf	r1
	 mov	r2, r4
.Lmwait3b:
	bra	6b
	 nop

10:
	/* Unlock after loop requires waekup.  */
	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov.l	.Lmwake3, r1
	bsrf	r1
	 nop
.Lmwake3b:
	bra	11b
	 nop

	.align	2
.Lmwait2:
	.long	__lll_mutex_lock_wait-.Lmwait2b
.Lmwake2:
	.long	__lll_mutex_unlock_wake-.Lmwake2b
.Lmwait3:
	.long	__lll_mutex_lock_wait-.Lmwait3b
.Lmwake3:
	.long	__lll_mutex_unlock_wake-.Lmwake3b
	.size	__pthread_cond_timedwait, .-__pthread_cond_timedwait
versioned_symbol (libpthread, __pthread_cond_timedwait, pthread_cond_timedwait,
		  GLIBC_2_3_2)
