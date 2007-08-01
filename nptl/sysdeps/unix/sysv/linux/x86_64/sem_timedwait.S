/* Copyright (C) 2002, 2003, 2005, 2007 Free Software Foundation, Inc.
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
.Lpush_r12:
	pushq	%r13
.Lpush_r13:
	pushq	%r14
.Lpush_r14:
	subq	$24, %rsp
.Lsubq:

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
.Laddq:
	popq	%r14
.Lpop_r14:
	popq	%r13
.Lpop_r13:
	popq	%r12
.Lpop_r12:
	retq

.Lafter_retq:
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
	.size	sem_timedwait_cleanup,.-sem_timedwait_cleanup


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	0xff				# @LPStart format (omit)
	.byte	0xff				# @TType format (omit)
	.byte	0x01				# call-site format
						# DW_EH_PE_uleb128
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


	.section .eh_frame,"a",@progbits
.LSTARTFRAME:
	.long	.LENDCIE-.LSTARTCIE		# Length of the CIE.
.LSTARTCIE:
	.long	0				# CIE ID.
	.byte	1				# Version number.
#ifdef SHARED
	.string	"zPLR"				# NUL-terminated augmentation
						# string.
#else
	.string	"zPL"				# NUL-terminated augmentation
						# string.
#endif
	.uleb128 1				# Code alignment factor.
	.sleb128 -8				# Data alignment factor.
	.byte	16				# Return address register
						# column.
#ifdef SHARED
	.uleb128 7				# Augmentation value length.
	.byte	0x9b				# Personality: DW_EH_PE_pcrel
						# + DW_EH_PE_sdata4
						# + DW_EH_PE_indirect
	.long	DW.ref.__gcc_personality_v0-.
	.byte	0x1b				# LSDA Encoding: DW_EH_PE_pcrel
						# + DW_EH_PE_sdata4.
	.byte	0x1b				# FDE Encoding: DW_EH_PE_pcrel
						# + DW_EH_PE_sdata4.
#else
	.uleb128 10				# Augmentation value length.
	.byte	0x0				# Personality: absolute
	.quad	__gcc_personality_v0
	.byte	0x0				# LSDA Encoding: absolute
#endif
	.byte 0x0c				# DW_CFA_def_cfa
	.uleb128 7
	.uleb128 8
	.byte	0x90				# DW_CFA_offset, column 0x10
	.uleb128 1
	.align 8
.LENDCIE:

	.long	.LENDFDE-.LSTARTFDE		# Length of the FDE.
.LSTARTFDE:
	.long	.LSTARTFDE-.LSTARTFRAME		# CIE pointer.
#ifdef SHARED
	.long	.LSTARTCODE-.			# PC-relative start address
						# of the code.
	.long	.LENDCODE-.LSTARTCODE		# Length of the code.
	.uleb128 4				# Augmentation size
	.long	.LexceptSTART-.
#else
	.quad	.LSTARTCODE			# Start address of the code.
	.quad	.LENDCODE-.LSTARTCODE		# Length of the code.
	.uleb128 8				# Augmentation size
	.quad	.LexceptSTART
#endif

	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpush_r12-.LSTARTCODE
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
	.byte   0x8c				# DW_CFA_offset %r12
        .uleb128 2
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpush_r13-.Lpush_r12
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 24
	.byte   0x8d				# DW_CFA_offset %r13
        .uleb128 3
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpush_r14-.Lpush_r13
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 32
	.byte   0x8e				# DW_CFA_offset %r14
        .uleb128 4
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lsubq-.Lpush_r14
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 56
	.byte	4				# DW_CFA_advance_loc4
	.long	.Laddq-.Lsubq
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 32
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpop_r14-.Laddq
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 24
	.byte	0xce				# DW_CFA_restore %r14
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpop_r13-.Lpop_r14
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
	.byte	0xcd				# DW_CFA_restore %r13
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpop_r12-.Lpop_r13
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 8
	.byte	0xcc				# DW_CFA_restore %r12
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lafter_retq-.Lpop_r12
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 56
	.byte   0x8c				# DW_CFA_offset %r12
        .uleb128 2
	.byte   0x8d				# DW_CFA_offset %r13
        .uleb128 3
	.byte   0x8e				# DW_CFA_offset %r14
        .uleb128 4
	.align	8
.LENDFDE:


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
