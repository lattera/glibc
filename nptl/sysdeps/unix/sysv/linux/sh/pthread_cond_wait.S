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
#include "lowlevel-atomic.h"

#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.text

	.align	5
	.type	__condvar_cleanup, @function
	.globl	__condvar_cleanup
	.hidden	__condvar_cleanup
__condvar_cleanup:
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	mov	r4, r8

	/* Get internal lock.  */
	mov	#1, r3
#if cond_lock != 0
	XADD (r3, @(cond_lock,r8), r2)
#else
	XADD (r3, @r8, r2)
#endif
	tst	r2, r2
	bt	1f
	mov	r8, r5
#if cond_lock != 0
	add	#cond_lock, r5
#endif
	mov	r2, r4
	mov.l	.Lwait0, r1
	bsrf	r1
	 nop
.Lwait0b:
1:
	mov	#1, r2
	mov	#0, r3

	clrt
	mov.l	@(wakeup_seq,r8),r0
	mov.l	@(wakeup_seq+4,r8),r1
	addc	r2, r0
	addc	r3, r1
	mov.l	r0,@(wakeup_seq,r8)
	mov.l	r1,@(wakeup_seq+4,r8)

	clrt
	mov.l	@(woken_seq,r8),r0
	mov.l	@(woken_seq+4,r8),r1
	addc	r2, r0
	addc	r3, r1
	mov.l	r0,@(woken_seq,r8)
	mov.l	r1,@(woken_seq+4,r8)

	/* Release internal lock.  */
#if cond_lock != 0
	DEC (@(cond_lock,r8), r2)
#else
	DEC (@r8, r2)
#endif
	tst	r2, r2
	bt	2f

	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov.l	.Lwake0, r1
	bsrf	r1
	 nop
.Lwake0b:
2:
	lds.l	@r15+, pr
	rts
	 mov.l	@r15+, r8

	.align	2
.Lwait0:	
	.long	__lll_mutex_lock_wait-.Lwait0b
.Lwake0:
	.long	__lll_mutex_unlock_wake-.Lwake0b
	.size	__condvar_cleanup, .-__condvar_cleanup


/* int pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex)  */
	.globl	__pthread_cond_wait
	.type	__pthread_cond_wait, @function
	.align	5
__pthread_cond_wait:
	mov.l	r12, @-r15
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	add	#-32, r15
	mov	r4, r8
	mov	r5, r9

	/* Get internal lock.  */
	mov	#1, r3
#if cond_lock != 0
	XADD (r3, @(cond_lock,r8), r2)
#else
	XADD (r3, @r8, r2)
#endif
	tst	r2, r2
	bf	1f
2:	
	/* Unlock the mutex.  */
	mov.l	.Lmunlock0, r1
	bsrf	r1
	 mov	r9, r4
.Lmunlock0b:

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
	mova	.Lgot0, r0
	mov.l	.Lgot0, r12
	add	r0, r12
	mov.l	.Lccleanup0, r5
	add	r12, r5
#else
	mov.l	.Lccleanup0, r5
#endif
	mov	r15, r4
	add	#12, r4

	mov.l	.Lccpush0, r1
	bsrf	r1
	 mov	r8, r6
.Lccpush0b:

	/* Get and store current wakeup_seq value.  */
	mov.l	@(wakeup_seq,r8), r0
	mov.l	@(wakeup_seq+4,r8), r1
	mov.l	r0, @(4,r15)
	mov.l	r1, @(8,r15)

8:
	/* Unlock.  */
#if cond_lock != 0
	DEC (@(cond_lock,r8), r2)
#else
	DEC (@r8, r2)
#endif
	tst	r2, r2
	bf	3f
4:
	mov.l	.Lenable0, r1
	bsrf	r1
	 nop
.Lenable0b:
	mov.l	r0, @r15

	mov	#0, r7
	mov	#FUTEX_WAIT, r5
	mov.l	@(4,r15), r6
	mov	r8, r4
	add	#wakeup_seq, r4
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov.l	.Ldisable0, r1
	bsrf	r1
	 mov.l	@r15, r4
.Ldisable0b:	

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

	mov.l	@(8,r15), r5
	cmp/hi	r5, r1
	bt	7f
	cmp/hi	r1, r5
	bt	8b

	mov.l	@(4,r15), r5
	cmp/hi	r0, r5
	bt	8b
7:
	cmp/hi	r1, r3
	bt	9f
	cmp/hi	r3, r1
	bt	8b
	cmp/hi	r0, r2
	bf	8b
9:
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
	add	#12, r4
	mov.l	.Lcpop0, r1
	bsrf	r1
	 mov	#0, r5
.Lcpop0b:

	mov	r9, r4
	mov.l	.Lmlocki0, r1
	bsrf	r1
	 mov	#0, r5
.Lmlocki0b:

	add	#32, r15

	/* We return the result of the mutex_lock operation.  */
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	rts
	 mov.l	@r15+, r12

	.align	2
.Lmunlock0:
	.long	__pthread_mutex_unlock_internal-.Lmunlock0b
#ifdef PIC
.Lgot0:
	.long	_GLOBAL_OFFSET_TABLE_
.Lccleanup0:
	.long	__condvar_cleanup@GOTOFF
#else
.Lccleanup0:
	.long	__condvar_cleanup
#endif
.Lccpush0:
	.long	__pthread_cleanup_push-.Lccpush0b
.Lenable0:
	.long	__pthread_enable_asynccancel-.Lenable0b
.Ldisable0:
	.long	__pthread_disable_asynccancel-.Ldisable0b
.Lcpop0:
	.long	__pthread_cleanup_pop-.Lcpop0b
.Lmlocki0:
	.long	__pthread_mutex_lock_internal-.Lmlocki0b

1:
	/* Initial locking failed.  */
	mov	r8, r5
#if cond_lock != 0
	add	#cond_lock, r5
#endif
	mov.l	.Lmwait0, r1
	bsrf	r1
	 mov	r2, r4
.Lmwait0b:
	bra	2b
	 nop
3:
	/* Unlock in loop requires waekup.  */
	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov.l	.Lmwake0, r1
	bsrf	r1
	 nop
.Lmwake0b:
	bra	4b
	 nop

5:
	/* Locking in loop failed.  */
	mov	r8, r5
#if cond_lock != 0
	add	#cond_lock, r5
#endif
	mov.l	.Lmwait1, r1
	bsrf	r1
	 mov	r2, r4
.Lmwait1b:
	bra	6b
	 nop

10:
	/* Unlock after loop requires wakeup.  */
	mov	r8, r4
#if cond_lock != 0
	add	#cond_lock, r4
#endif
	mov.l	.Lmwake1, r1
	bsrf	r1
	 nop
.Lmwake1b:
	bra	11b
	 nop

	.align	2
.Lmwait0:
	.long	__lll_mutex_lock_wait-.Lmwait0b
.Lmwake0:
	.long	__lll_mutex_unlock_wake-.Lmwake0b
.Lmwait1:
	.long	__lll_mutex_lock_wait-.Lmwait1b
.Lmwake1:
	.long	__lll_mutex_unlock_wake-.Lmwake1b
	.size	__pthread_cond_wait, .-__pthread_cond_wait
versioned_symbol (libpthread, __pthread_cond_wait, pthread_cond_wait,
		  GLIBC_2_3_2)
