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
#include <shlib-compat.h>
#include <lowlevelcond.h>
#include <kernel-features.h>
#include "lowlevel-atomic.h"

#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1
#define FUTEX_REQUEUE		3

#define EINVAL			22

	.text

	/* int pthread_cond_signal (pthread_cond_t *cond) */
	.globl	__pthread_cond_signal
	.type	__pthread_cond_signal, @function
	.align	5
__pthread_cond_signal:
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	mov	r4, r8

	/* Get internal lock.  */
	mov	#0, r3
	mov	#1, r4
#if cond_lock != 0
	CMPXCHG (r3, @(cond_lock,r8), r4, r2)
#else
	CMPXCHG (r3, @r8, r4, r2)
#endif
	bf	1f
2:
	mov.l	@(total_seq+4,r8),r0
	mov.l	@(total_seq,r8),r1
	mov.l	@(wakeup_seq+4,r8), r2
	cmp/hi	r2, r0
	bt	3f
	cmp/hi	r0, r2
	bt	4f
	mov.l	@(wakeup_seq,r8), r2
	cmp/hi	r2, r1
	bf	4f

3:
	/* Bump the wakeup number.  */
	mov	#1, r2
	mov	#0, r3
	clrt
	mov.l	@(wakeup_seq,r8),r0
	mov.l	@(wakeup_seq+4,r8),r1
	addc	r2, r0
	addc	r3, r1
	mov.l	r0,@(wakeup_seq,r8)
	mov.l	r1,@(wakeup_seq+4,r8)
	mov.l	@(cond_futex,r8),r0
	add	r2, r0
	mov.l	r0,@(cond_futex,r8)

	/* Wake up one thread.  */
	mov	r8, r4
	add	#cond_futex, r4
	mov	#FUTEX_WAKE, r5
	mov	#1, r6
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

4:
	/* Unlock.  */
#if cond_lock != 0
	DEC (@(cond_lock,r8), r2)
#else
	DEC (@r8, r2)
#endif
	tst	r2, r2
	bf	5f
6:
	mov	#0, r0
	lds.l	@r15+, pr
	rts
	 mov.l	@r15+, r8

1:
	/* Initial locking failed.  */
	mov	r8, r5
#if cond_lock != 0
	add	#cond_lock, r5
#endif
	mov.l	.Lmwait4, r1
	bsrf	r1
	 mov	r2, r4
.Lmwait4b:
	bra	2b
	 nop

5:
	/* Unlock in loop requires wakeup.  */
	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov.l	.Lmwake4, r1
	bsrf	r1
	 nop
.Lmwake4b:
	bra	6b
	 nop

	.align	2
.Lmwait4:
	.long	__lll_mutex_lock_wait-.Lmwait4b
.Lmwake4:
	.long	__lll_mutex_unlock_wake-.Lmwake4b
	.size	__pthread_cond_signal, .-__pthread_cond_signal
versioned_symbol (libpthread, __pthread_cond_signal, pthread_cond_signal,
		  GLIBC_2_3_2)

