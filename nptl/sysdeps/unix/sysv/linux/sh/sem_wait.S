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
#include <pthread-errnos.h>
#include <tcb-offsets.h>
#include "lowlevel-atomic.h"


#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

	.text

	.globl	__new_sem_wait
	.type	__new_sem_wait,@function
	.align	5
	cfi_startproc
__new_sem_wait:
	/* First check for cancellation.  */
	stc	gbr, r0
	mov.w	.Lchand, r1
	mov.l	@(r0,r1), r0
	mov	#0xf9, r1
	and	r1, r0
	cmp/eq	#8, r0
	bt	5f

	mov.l	r8, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r8, 0)
	mov.l	r10, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r10, 0)
	mov.l	r12, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (r12, 0)
	sts.l	pr, @-r15
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset (pr, 0)
	mov	r4, r8
3:
	mov.l	@r8, r0
2:
	tst	r0, r0
	bt	1f
	mov	r0, r3
	mov	r0, r4
	add	#-1, r3
	CMPXCHG (r4, @r8, r3, r2)
	bf/s	2b
	 mov	r2, r0
	bra	9f
	 mov	#0, r0

1:
	mov.l	.Lenable0, r1
	bsrf	r1
	 nop
.Lenable0b:
	mov	r0, r10

	mov	r8, r4
	mov	#FUTEX_WAIT, r5
	mov	#0, r6
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	mov.l	.Ldisable0, r1
	mov	r10, r4
	bsrf	r1
	 mov	r0, r10
.Ldisable0b:	
	mov	r10, r0

	tst	r0, r0
	bt	3b
	cmp/eq	#-EWOULDBLOCK, r0
	bt	3b
	neg	r0, r0

	mov	r0, r8
	mova	.Lgot0, r0
	mov.l	.Lgot0, r12
	add	r0, r12

#if USE___THREAD
	mov.l	.Lerrno0, r0
	stc	gbr, r1
	mov.l	@(r0, r12), r0
	add	r1, r0
	mov.l	r8, @r0
#else
	mov.l	.Lerrloc0, r1
	bsrf	r1
	 nop
.Lerrloc0b:
	mov.l	r8, @r0
#endif
	mov	#-1, r0
9:
	lds.l	@r15+, pr
	mov.l	@r15+, r12
	mov.l	@r15+, r10
	rts
	 mov.l	@r15+, r8
5:
	/* Canceled.  */
	stc	gbr, r0
	mov.w	.Lresult, r1
	mov	#-1, r2
	mov.l	r2, @(r0,r1)
	mov.w	.Lchand, r0
	or.b	#0x10, @(r0,gbr)
	stc	gbr, r0
	mov.w	.Lclbuf, r1
	mov.l	.Lunwind, r2
	braf	r2
	 mov.l	@(r0,r1), r4
.Lunwindb:
	cfi_endproc

.Lchand:
	.word	CANCELHANDLING - TLS_PRE_TCB_SIZE
.Lresult:
	.word	RESULT - TLS_PRE_TCB_SIZE
.Lclbuf:
	.word	CLEANUP_JMP_BUF - TLS_PRE_TCB_SIZE
	.align	2
.Lgot0:
	.long	_GLOBAL_OFFSET_TABLE_
#if USE___THREAD
.Lerrno0:
	.long	errno@GOTTPOFF
#else
.Lerrloc0:
	.long	__errno_location@PLT-(.Lerrloc0b-.)
#endif
.Lenable0:
	.long	__pthread_enable_asynccancel-.Lenable0b
.Ldisable0:
	.long	__pthread_disable_asynccancel-.Ldisable0b
.Lunwind:
	.long	HIDDEN_JUMPTARGET (__pthread_unwind)-.Lunwindb
	.size	__new_sem_wait,.-__new_sem_wait
	versioned_symbol(libpthread, __new_sem_wait, sem_wait, GLIBC_2_1)
