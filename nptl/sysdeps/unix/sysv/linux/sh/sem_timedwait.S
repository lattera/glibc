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
#include <shlib-compat.h>
#include <pthread-errnos.h>
#include <tcb-offsets.h>
#include <structsem.h>
#include <lowlevellock.h>
#include "lowlevel-atomic.h"


#if VALUE != 0
# error "code needs to be rewritten for VALUE != 0"
#endif

	.text

	.globl	sem_timedwait
	.type	sem_timedwait,@function
	.align	5
	cfi_startproc
sem_timedwait:
.LSTARTCODE:
#ifdef SHARED
	cfi_personality(DW_EH_PE_pcrel | DW_EH_PE_sdata4 | DW_EH_PE_indirect,
			DW.ref.__gcc_personality_v0)
	cfi_lsda(DW_EH_PE_pcrel | DW_EH_PE_sdata4, .LexceptSTART)
#else
	cfi_personality(DW_EH_PE_absptr, __gcc_personality_v0)
	cfi_lsda(DW_EH_PE_absptr, .LexceptSTART)
#endif
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
	mov.l	r8, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r8, 0)
	mov.l	r9, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r9, 0)
	mov.l	r10, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r10, 0)
	mov.l	r12, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (r12, 0)
	sts.l	pr, @-r15
	cfi_adjust_cfa_offset (4)
	cfi_rel_offset (pr, 0)
	add	#-8, r15
	cfi_adjust_cfa_offset (8)

	mov	r4, r8
	mov	r5, r9

	/* Check for invalid nanosecond field.  */
	mov.l	@(4,r9), r0
	mov.l	.L1g, r1
	cmp/hs	r1, r0
	bt/s	.Lerrno_exit
	 mov	#EINVAL, r10
	INC (@(NWAITERS,r8),r2)

7:
	/* Compute relative timeout.  */
	mov	r15, r4
	mov	#0, r5
	mov	#__NR_gettimeofday, r3
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
	bf/s	6f		/* Time is already up.  */
	 mov	#ETIMEDOUT, r0

	/* Store relative timeout.  */
	mov.l	r2, @r15
	mov.l	r3, @(4,r15)

.LcleanupSTART:
	mov.l	.Lenable0, r1
	bsrf	r1
	 nop
.Lenable0b:
	mov	r0, r10

	mov	r8, r4
#if FUTEX_WAIT == 0
	mov.l	@(PRIVATE,r8), r5
#else
	mov.l	@(PRIVATE,r8), r5
	mov	#FUTEX_WAIT, r0
	or	r0, r5
#endif
	mov	#0, r6
	mov	r15, r7
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
.LcleanupEND:

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

	DEC (@(NWAITERS,r8), r2)
	mov	#0, r0

10:
	add	#8, r15
	lds.l	@r15+, pr
	mov.l	@r15+, r12
	mov.l	@r15+, r10
	mov.l	@r15+, r9
	mov.l	@r15+, r8
	rts
	 nop

3:
	neg	r0, r0
6:
	mov	r0, r10
	DEC (@(NWAITERS,r8), r2)
.Lerrno_exit:
	mova	.Lgot2, r0
	mov.l	.Lgot2, r12
	add	r0, r12

	mov.l	.Lerrno2, r0
	stc	gbr, r1
	mov.l	@(r0, r12), r0
	bra	.Lexit
	 add	r1, r0
	.align	2
.Lerrno2:
	.long	errno@GOTTPOFF
.Lexit:
	mov.l	r10, @r0
	bra	10b
	 mov	#-1, r0

.L1k:
	.word	1000
	.align	2
.L1g:
	.long	1000000000
.Lgot2:
	.long	_GLOBAL_OFFSET_TABLE_
.Lenable0:
	.long	__pthread_enable_asynccancel-.Lenable0b
.Ldisable0:
	.long	__pthread_disable_asynccancel-.Ldisable0b
	.size	sem_timedwait,.-sem_timedwait

	.type	sem_wait_cleanup,@function
sem_wait_cleanup:
	DEC (@(NWAITERS,r8), r2)
.LcallUR:
	mov.l	.Lresume, r1
#ifdef PIC
	add	r12, r1
#endif
	jsr	@r1
	 nop
	sleep

	.align	2
.Lresume:
#ifdef PIC
	.long	_Unwind_Resume@GOTOFF
#else
	.long	_Unwind_Resume
#endif
.LENDCODE:
	cfi_endproc
	.size	sem_wait_cleanup,.-sem_wait_cleanup


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	DW_EH_PE_omit			! @LPStart format (omit)
	.byte	DW_EH_PE_omit			! @TType format (omit)
	.byte	DW_EH_PE_uleb128		! call-site format
	.uleb128 .Lcstend-.Lcstbegin
.Lcstbegin:
	.uleb128 .LcleanupSTART-.LSTARTCODE
	.uleb128 .LcleanupEND-.LcleanupSTART
	.uleb128 sem_wait_cleanup-.LSTARTCODE
	.uleb128  0
	.uleb128 .LcallUR-.LSTARTCODE
	.uleb128 .LENDCODE-.LcallUR
	.uleb128 0
	.uleb128  0
.Lcstend:

#ifdef SHARED
	.hidden	DW.ref.__gcc_personality_v0
	.weak	DW.ref.__gcc_personality_v0
	.section .gnu.linkonce.d.DW.ref.__gcc_personality_v0,"aw",@progbits
	.align	4
	.type	DW.ref.__gcc_personality_v0, @object
	.size	DW.ref.__gcc_personality_v0, 4
DW.ref.__gcc_personality_v0:
	.long	__gcc_personality_v0
#endif
