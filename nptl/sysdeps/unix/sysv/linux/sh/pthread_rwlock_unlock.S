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
#include <kernel-features.h>
#include "lowlevel-atomic.h"


	.text

	.globl	__pthread_rwlock_unlock
	.type	__pthread_rwlock_unlock,@function
	.align	5
	cfi_startproc
__pthread_rwlock_unlock:
	mov.l	r12, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r12, 0)
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
	bf	5f
	mov.l	@(NR_READERS,r8), r0
	add	#-1, r0
	mov.l	r0, @(NR_READERS,r8)
	tst	r0, r0
	bf	6f
5:
	mov	#0, r0
	mov.l	r0, @(WRITER,r8)
	mov	#1, r6
	mov	r8, r4
	add	#WRITERS_WAKEUP, r4
	mov.l	@(WRITERS_QUEUED,r8), r0
	tst	r0, r0
	bf	0f

	/* If also no readers waiting nothing to do.  */
	mov.l	@(READERS_QUEUED,r8), r0
	tst	r0, r0
	bt	6f

	mov	#-1, r6
	shlr	r6		/* r6 = 0x7fffffff */
	mov	r8, r4
	add	#READERS_WAKEUP, r4

0:
	mov.l	@r4, r0
	add	#1, r0
	mov.l	r0, @r4
#if MUTEX == 0
	DEC (@r8, r2)
#else
	DEC (@(MUTEX,r8), r2)
#endif
	tst	r2, r2
	bf	7f

8:
#ifdef __ASSUME_PRIVATE_FUTEX
	mov	#PSHARED, r0
	mov.b	@(r0,r8), r5
	mov	#(FUTEX_PRIVATE_FLAG|FUTEX_WAKE), r0
	xor	r0, r5
	extu.b	r5, r5
#else
	mov	#PSHARED, r0
	mov.b	@(r0,r8), r5
	extu.b	r5, r5
	mov	#FUTEX_WAKE, r0
	or	r0, r5
	stc	gbr, r1
	mov.w	.Lpfoff, r2
	add	r2, r1
	mov.l	@r1, r0
	xor	r0, r5
#endif
	mov	#SYS_futex, r3
	mov	#0, r7
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	cfi_remember_state
	lds.l	@r15+, pr
	cfi_adjust_cfa_offset (-4)
	cfi_restore (pr)
	mov.l	@r15+, r8
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r8)
	mov.l	@r15+, r12
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r12)
	rts
	 mov	#0, r0
	cfi_restore_state
6:
#if MUTEX == 0
	DEC (@r8, r2)
#else
	DEC (@(MUTEX,r8), r2)
#endif
	tst	r2, r2
	bf	3f
4:
	cfi_remember_state
	lds.l	@r15+, pr
	cfi_adjust_cfa_offset (-4)
	cfi_restore (pr)
	mov.l	@r15+, r8
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r8)
	mov.l	@r15+, r12
	cfi_adjust_cfa_offset (-4)
	cfi_restore (r12)
	rts
	 mov	#0, r0
	cfi_restore_state

1:
	mov	r8, r5
#if MUTEX != 0
	add	#MUTEX, r5
#endif
	mov	#PSHARED, r0
	mov.b	@(r0,r8), r6
	extu.b	r6, r6
	mov.l	.Lwait8, r1
	bsrf	r1
	 mov	r2, r4
.Lwait8b:
	bra	2b
	 nop
3:
	mov	r8, r4
#if MUTEX != 0
	add	#MUTEX, r4
#endif
	mov	#PSHARED, r0
	mov.b	@(r0,r8), r5
	extu.b	r5, r5
	mov.l	.Lwake8, r1
	bsrf	r1
	 nop
.Lwake8b:
	bra	4b
	 nop

7:
	mov.l	r4, @-r15
	cfi_adjust_cfa_offset (4)
	mov.l	r6, @-r15
	cfi_adjust_cfa_offset (4)
	mov	r8, r4
#if MUTEX != 0
	add	#MUTEX, r4
#endif
	mov	#PSHARED, r0
	mov.b	@(r0,r8), r5
	extu.b	r5, r5
	mov.l	.Lwake9, r1
	bsrf	r1
	 nop
.Lwake9b:

	mov.l	@r15+, r6
	cfi_adjust_cfa_offset (-4)
	bra	8b
	 mov.l	@r15+, r4

	cfi_endproc

#ifndef __ASSUME_PRIVATE_FUTEX
.Lpfoff:
	.word	PRIVATE_FUTEX - TLS_PRE_TCB_SIZE
#endif
	.align	2
.Lwait8:	
	.long	__lll_lock_wait-.Lwait8b
.Lwake8:
	.long	__lll_unlock_wake-.Lwake8b
.Lwake9:
	.long	__lll_unlock_wake-.Lwake9b
	.size	__pthread_rwlock_unlock,.-__pthread_rwlock_unlock

strong_alias (__pthread_rwlock_unlock, pthread_rwlock_unlock)
hidden_def (__pthread_rwlock_unlock)
