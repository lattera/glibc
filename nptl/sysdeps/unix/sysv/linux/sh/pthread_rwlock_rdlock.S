/* Copyright (C) 2003-2012 Free Software Foundation, Inc.
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

#include <sysdep.h>
#include <lowlevellock.h>
#include <lowlevelrwlock.h>
#include <pthread-errnos.h>
#include <tcb-offsets.h>
#include <kernel-features.h>
#include "lowlevel-atomic.h"


	.text

	.globl	__pthread_rwlock_rdlock
	.type	__pthread_rwlock_rdlock,@function
	.align	5
	cfi_startproc
__pthread_rwlock_rdlock:
	mov.l	r12, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r12, 0)
	mov.l	r9, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r9, 0)
	mov.l	r8, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r8, 0)
	sts.l	pr, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (pr, 0)
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
	mov	#FLAGS, r0
	mov.b	@(r0,r8), r0
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
#ifdef __ASSUME_PRIVATE_FUTEX
	mov	#PSHARED, r0
	mov.b	@(r0,r8), r5
	mov	#(FUTEX_PRIVATE_FLAG|FUTEX_WAIT), r0
	xor	r0, r5
	extu.b	r5, r5
#else
	mov	#PSHARED, r0
	mov.b	@(r0,r8), r5
	extu.b	r5, r5
# if FUTEX_WAIT != 0
	mov	#FUTEX_WAIT, r0
	or	r0, r5
# endif
	stc	gbr, r1
	mov.w	.Lpfoff, r2
	add	r2, r1
	mov.l	@r1, r0
	xor	r0, r5
#endif
	mov	r8, r4
	add	#READERS_WAKEUP, r4
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
	cfi_remember_state
	lds.l	@r15+, pr
	cfi_adjust_cfa_offset (-4)
	cfi_restore (pr)
	mov.l	@r15+, r8
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r8)
	mov.l	@r15+, r9
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r9)
	mov.l	@r15+, r12
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r12)
	rts
	 mov	r3, r0
	cfi_restore_state

#ifndef __ASSUME_PRIVATE_FUTEX
.Lpfoff:
	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE
#endif

1:
	mov	r8, r5
#if MUTEX != 0
	add	#MUTEX, r5
#endif
	mov	#PSHARED, r0
	mov.b	@(r0,r8), r6
	extu.b	r6, r6
	mov.l	.Lwait0, r1
	bsrf	r1
	 mov	r2, r4
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
	mov	#PSHARED, r0
	mov.b	@(r0,r8), r5
	extu.b	r5, r5
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
	mov	#PSHARED, r0
	mov.b	@(r0,r8), r5
	extu.b	r5, r5
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
	mov	#PSHARED, r0
	mov.b	@(r0,r8), r6
	extu.b	r6, r6
	mov.l	.Lwait1, r1
	bsrf	r1
	 mov	r2, r4
.Lwait1b:
	bra	13b
	 nop
	cfi_endproc

	.align	2
.Lwait0:
	.long	__lll_lock_wait-.Lwait0b
.Lwake0:
	.long	__lll_unlock_wake-.Lwake0b
.Lwait1:
	.long	__lll_lock_wait-.Lwait1b
.Lwake1:
	.long	__lll_unlock_wake-.Lwake1b
	.size	__pthread_rwlock_rdlock,.-__pthread_rwlock_rdlock

strong_alias (__pthread_rwlock_rdlock, pthread_rwlock_rdlock)
hidden_def (__pthread_rwlock_rdlock)
