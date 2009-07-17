/* Copyright (C) 2002, 2003, 2005, 2007, 2009 Free Software Foundation, Inc.
   This file is part of the GNU C Library.
   Contributed by Ulrich Drepper <drepper@redhat.com>, 2002.

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
#include <lowlevellock.h>
#include <shlib-compat.h>
#include <pthread-errnos.h>
#include <structsem.h>


/* For the calculation see asm/vsyscall.h.  */
#define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000

	.text

	.globl	sem_timedwait
	.type	sem_timedwait,@function
	.align	16
sem_timedwait:
.LSTARTCODE:
	cfi_startproc
#ifdef SHARED
	cfi_personality(DW_EH_PE_pcrel | DW_EH_PE_sdata4 | DW_EH_PE_indirect,
			DW.ref.__gcc_personality_v0)
	cfi_lsda(DW_EH_PE_pcrel | DW_EH_PE_sdata4, .LexceptSTART)
#else
	cfi_personality(DW_EH_PE_udata4, __gcc_personality_v0)
	cfi_lsda(DW_EH_PE_udata4, .LexceptSTART)
#endif
#if VALUE == 0
	movl	(%rdi), %eax
#else
	movl	VALUE(%rdi), %eax
#endif
2:	testl	%eax, %eax
	je	1f

	leaq	-1(%rax), %rdx
	LOCK
#if VALUE == 0
	cmpxchgl %edx, (%rdi)
#else
	cmpxchgl %edx, VALUE(%rdi)
#endif
	jne	2b

	xorl	%eax, %eax
	retq

	/* Check whether the timeout value is valid.  */
1:	pushq	%r12
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r12, 0)
	pushq	%r13
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r13, 0)
	pushq	%r14
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r14, 0)
	subq	$24, %rsp
	cfi_adjust_cfa_offset(24)

	movq	%rdi, %r12
	movq	%rsi, %r13

	/* Check for invalid nanosecond field.  */
	cmpq	$1000000000, 8(%r13)
	movl	$EINVAL, %r14d
	jae	6f

	LOCK
	addq	$1, NWAITERS(%r12)

7:	xorl	%esi, %esi
	movq	%rsp, %rdi
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	callq	*%rax

	/* Compute relative timeout.  */
	movq	8(%rsp), %rax
	movl	$1000, %edi
	mul	%rdi		/* Milli seconds to nano seconds.  */
	movq	(%r13), %rdi
	movq	8(%r13), %rsi
	subq	(%rsp), %rdi
	subq	%rax, %rsi
	jns	5f
	addq	$1000000000, %rsi
	decq	%rdi
5:	testq	%rdi, %rdi
	movl	$ETIMEDOUT, %r14d
	js	6f		/* Time is already up.  */

	movq	%rdi, (%rsp)	/* Store relative timeout.  */
	movq	%rsi, 8(%rsp)

.LcleanupSTART:
	call	__pthread_enable_asynccancel
	movl	%eax, 16(%rsp)

	movq	%rsp, %r10
#if VALUE == 0
	movq	%r12, %rdi
#else
	leaq	VALUE(%r12), %rdi
#endif
#if FUTEX_WAIT == 0
	movl	PRIVATE(%rdi), %esi
#else
	movl	$FUTEX_WAIT, %esi
	orl	PRIVATE(%rdi), %esi
#endif
	movl	$SYS_futex, %eax
	xorl	%edx, %edx
	syscall
	movq	%rax, %r14

	movl	16(%rsp), %edi
	call	__pthread_disable_asynccancel
.LcleanupEND:

	testq	%r14, %r14
	je	9f
	cmpq	$-EWOULDBLOCK, %r14
	jne	3f

9:
#if VALUE == 0
	movl	(%r12), %eax
#else
	movl	VALUE(%r12), %eax
#endif
8:	testl	%eax, %eax
	je	7b

	leaq	-1(%rax), %rcx
	LOCK
#if VALUE == 0
	cmpxchgl %ecx, (%r12)
#else
	cmpxchgl %ecx, VALUE(%r12)
#endif
	jne	8b

	xorl	%eax, %eax

10:	LOCK
	subq	$1, NWAITERS(%r12)

	addq	$24, %rsp
	cfi_adjust_cfa_offset(-24)
	popq	%r14
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r14)
	popq	%r13
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r13)
	popq	%r12
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r12)
	retq

	cfi_adjust_cfa_offset(3 * 8 + 24)
	cfi_rel_offset(%r12, 24 + 2 * 8)
	cfi_rel_offset(%r13, 24 + 1 * 8)
	cfi_rel_offset(%r14, 24)
3:	negq	%r14
6:
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	%r14d, %fs:(%rdx)
#else
	callq	__errno_location@plt
	movl	%r14d, (%rax)
#endif

	orl	$-1, %eax
	jmp	10b
	.size	sem_timedwait,.-sem_timedwait


	.type	sem_timedwait_cleanup,@function
sem_timedwait_cleanup:
	LOCK
	subq	$1, NWAITERS(%r12)
	movq	%rax, %rdi
.LcallUR:
	call	_Unwind_Resume@PLT
	hlt
.LENDCODE:
	cfi_endproc
	.size	sem_timedwait_cleanup,.-sem_timedwait_cleanup


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	DW_EH_PE_omit			# @LPStart format
	.byte	DW_EH_PE_omit			# @TType format
	.byte	DW_EH_PE_uleb128		# call-site format
	.uleb128 .Lcstend-.Lcstbegin
.Lcstbegin:
	.uleb128 .LcleanupSTART-.LSTARTCODE
	.uleb128 .LcleanupEND-.LcleanupSTART
	.uleb128 sem_timedwait_cleanup-.LSTARTCODE
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
	.align	8
	.type	DW.ref.__gcc_personality_v0, @object
	.size	DW.ref.__gcc_personality_v0, 8
DW.ref.__gcc_personality_v0:
	.quad	__gcc_personality_v0
#endif
