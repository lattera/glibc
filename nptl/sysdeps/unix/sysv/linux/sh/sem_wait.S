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

	.globl	__new_sem_wait
	.type	__new_sem_wait,@function
	.align	5
__new_sem_wait:
	mov.l	r12, @-r15
	mov.l	r8, @-r15
	sts.l	pr, @-r15
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
	bf	2b
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r12
	rts
	 mov	#0, r0

1:
	mov	r8, r4
	mov	#FUTEX_WAIT, r5
	mov	#0, r6
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

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
	lds.l	@r15+, pr
	mov.l	@r15+, r8
	mov.l	@r15+, r12
	rts
	 mov	#-1, r0

	.align	2
.Lgot0:
	.long	_GLOBAL_OFFSET_TABLE_
#if USE___THREAD
.Lerrno0:
	.long	errno@GOTTPOFF
#else
.Lerrloc0:
	.long	__errno_location@PLT-(.Lerrloc0b+2-.)
#endif
	.size	__new_sem_wait,.-__new_sem_wait
	.symver	__new_sem_wait, sem_wait@@GLIBC_2.2
