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
#include "lowlevel-atomic.h"


#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

	.text

	.globl	__new_sem_post
	.type	__new_sem_post,@function
	.align	5
__new_sem_post:
	mov	#1, r3
	XADD (r3, @r4, r2)

	mov	#FUTEX_WAKE, r5
	mov	r2, r6
	add	#1, r6
	mov	#0, r7
	mov	#SYS_futex, r3
	extu.b	r3, r3
	trapa	#0x14
	SYSCALL_INST_PAD

	cmp/pz	r0
	bf	1f
	rts
	 mov	#0, r0

1:
	mov	#EINVAL, r2
	mova	.Lgot3, r0
	mov.l	.Lgot3, r12
	add	r0, r12

#if USE___THREAD
	mov.l	.Lerrno3, r0
	stc	gbr, r1
	mov.l	@(r0, r12), r0
	add	r1, r0
#else
	mov.l	.Lerrloc3, r1
	bsrf	r1
	 nop
.Lerrloc3b:
#endif
	mov.l	r2, @r0
	lds.l	@r15+, pr
	mov.l	@r15+, r12
	rts
	 mov	#-1, r0

	.align	2
.Lgot3:
	.long	_GLOBAL_OFFSET_TABLE_
#if USE___THREAD
.Lerrno3:
	.long	errno@GOTTPOFF
#else
.Lerrloc3:
	.long	__errno_location@PLT-(.Lerrloc3b-.)
#endif
	.size	__new_sem_post,.-__new_sem_post
	versioned_symbol(libpthread, __new_sem_post, sem_post, GLIBC_2_1)
