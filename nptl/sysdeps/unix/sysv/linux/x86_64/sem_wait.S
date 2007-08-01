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


	.text

	.globl	sem_wait
	.type	sem_wait,@function
	.align	16
sem_wait:
.LSTARTCODE:
	pushq	%r12
.Lpush_r12:
	pushq	%r13
.Lpush_r13:
	movq	%rdi, %r13

#if VALUE == 0
	movl	(%r13), %eax
#else
	movl	VALUE(%r13), %eax
#endif
2:	testl	%eax, %eax
	je	1f

	leal	-1(%rax), %edx
	LOCK
#if VALUE == 0
	cmpxchgl %edx, (%r13)
#else
	cmpxchgl %edx, VALUE(%r13)
#endif
	jne	2b

7:	xorl	%eax, %eax

9:	popq	%r13
.Lpop_r13:
	popq	%r12
.Lpop_r12:

	retq

.Lafter_retq:
1:	LOCK
	addq	$1, NWAITERS(%r13)

.LcleanupSTART:
6:	call	__pthread_enable_asynccancel
	movl	%eax, %r8d

	xorq	%r10, %r10
	movl	$SYS_futex, %eax
	movq	%r13, %rdi
#if FUTEX_WAIT == 0
	movl	PRIVATE(%rdi), %esi
#else
	movl	$FUTEX_WAIT, %esi
	orl	PRIVATE(%rdi), %esi
#endif
	xorl	%edx, %edx
	syscall
	movq	%rax, %r12

	movl	%r8d, %edi
	call	__pthread_disable_asynccancel
.LcleanupEND:

	testq	%r12, %r12
	je	3f
	cmpq	$-EWOULDBLOCK, %r12
	jne	4f

3:
#if VALUE == 0
	movl	(%r13), %eax
#else
	movl	VALUE(%r13), %eax
#endif
5:	testl	%eax, %eax
	je	6b

	leal	-1(%rax), %edx
	LOCK
#if VALUE == 0
	cmpxchgl %edx, (%r13)
#else
	cmpxchgl %edx, VALUE(%r13)
#endif
	jne	5b

	LOCK
	subq	$1, NWAITERS(%r13)
	jmp	7b

4:	negq	%r12
#if USE___THREAD
	movq	errno@gottpoff(%rip), %rdx
	movl	%r12d, %fs:(%rdx)
#else
	callq	__errno_location@plt
	movl	%r12d, (%rax)
#endif
	orl	$-1, %eax

	LOCK
	subq	$1, NWAITERS(%r13)

	jmp 9b
	.size	sem_wait,.-sem_wait


	.type	sem_wait_cleanup,@function
sem_wait_cleanup:
	LOCK
	subq	$1, NWAITERS(%r13)
	movq	%rax, %rdi
.LcallUR:
	call	_Unwind_Resume@PLT
	hlt
.LENDCODE:
	.size	sem_wait_cleanup,.-sem_wait_cleanup


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
	.uleb128 sem_wait_cleanup-.LSTARTCODE
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
	.long	.Lpop_r13-.Lpush_r13
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
	.uleb128 24
	.byte   0x8c				# DW_CFA_offset %r12
        .uleb128 2
	.byte   0x8d				# DW_CFA_offset %r13
        .uleb128 3
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
