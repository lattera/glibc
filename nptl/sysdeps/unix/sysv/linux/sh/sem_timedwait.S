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
#include <pthread-errnos.h>
#include "lowlevel-atomic.h"


#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

	.text

	.globl	sem_timedwait
	.type	sem_timedwait,@function
	.align	5
sem_timedwait:
	mov.l	@r4, r0
2:
	tst	r0, r0
	bt	1f
	mov	r0, r3
	mov	r0, r6
	add	#-1, r3
	CMPXCHG (r6, @r4, r3, r2)
	bf/s	2b
	 mov	r2, r0
	rts
	 mov	#0, r0

1:
	/* Check whether the timeout value is valid.  */
	mov.l	r12, @-r15
	mov.l	r9, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15
	add	#-8, r15
	mov	r4, r8
	mov	r5, r9

	/* Check for invalid nanosecond field.  */
	mov.l	@(4,r9), r0
	mov.l	.L1g, r1
	cmp/hs	r1, r0
	bt/s	6f
	 mov	#EINVAL, r0
7:
	/* Compute relative timeout.  */
	mov	r15, r4
	mov	#0, r5
	mov	#SYS_gettimeofday, r3
	trapa	#0x12
	SYSCALL_INST_PAD

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

	/* Store relative timeout.  */
	mov.l	r2, @r15
	mov.l	r3, @(4,r15)

	/* Futex call.  */
	mov	r8, r4
	mov	#FUTEX_WAIT, r5
	mov	#0, r6
	mov	r15, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	tst	r0, r0
	bt	9f
	cmp/eq	#-EWOULDBLOCK, r0
	bf	3f
9:
	mov.l	@r8, r0
8:
	tst	r0, r0
	bt	7b

	mov	r0, r3
	mov	r0, r4
	add	#-1, r3
	CMPXCHG (r4, @r8, r3, r2)
	bf/s	8b
	 mov	r2, r0

	add	#8, r15
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	mov.l	@r15+, r12
	rts
	 mov	#0, r0

3:
	neg	r0, r0
6:
	mov	r0, r8
	mova	.Lgot2, r0
	mov.l	.Lgot2, r12
	add	r0, r12

#if USE___THREAD
	mov.l	.Lerrno2, r0
	stc	gbr, r1
	mov.l	@(r0, r12), r0
	add	r1, r0
	mov.l	r8, @r0
#else
	mov.l	.Lerrloc2, r1
	bsrf	r1
	 nop
.Lerrloc2b:
	mov.l	r8, @r0
#endif
	add	#8, r15
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r9
	mov.l	@r15+, r12
	rts
	 mov	#-1, r0

.L1k:
	.word	1000
	.align	2
.L1g:
	.long	1000000000
.Lgot2:
	.long	_GLOBAL_OFFSET_TABLE_
#if USE___THREAD
.Lerrno2:
	.long	errno@GOTTPOFF
#else
.Lerrloc2:
	.long	__errno_location@PLT-(.Lerrloc2b+2-.)
#endif
	.size	sem_timedwait,.-sem_timedwait
