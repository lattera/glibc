/* Copyright (C) 2002, 2003, 2005 Free Software Foundation, Inc.
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

#ifndef UP
# define LOCK lock
#else
# define LOCK
#endif

#define SYS_futex	202
#define FUTEX_WAKE	1

	.comm	__fork_generation, 4, 4

	.text


	.globl	__pthread_once
	.type	__pthread_once,@function
	.align	16
__pthread_once:
.LSTARTCODE:
	testl	$2, (%rdi)
	jz	1f
	xorl	%eax, %eax
	retq

	/* Preserve the function pointer.  */
1:	pushq	%rsi
.Lpush_rsi:
	xorq	%r10, %r10

	/* Not yet initialized or initialization in progress.
	   Get the fork generation counter now.  */
6:	movl	(%rdi), %eax

5:	movl	%eax, %edx

	testl	$2, %eax
	jnz	4f

	andl	$3, %edx
	orl	__fork_generation(%rip), %edx
	orl	$1, %edx

	LOCK
	cmpxchgl %edx, (%rdi)
	jnz	5b

	/* Check whether another thread already runs the initializer.  */
	testl	$1, %eax
	jz	3f	/* No -> do it.  */

	/* Check whether the initializer execution was interrupted
	   by a fork.  */
	xorl	%edx, %eax
	testl	$0xfffffffc, %eax
	jnz	3f	/* Different for generation -> run initializer.  */

	/* Somebody else got here first.  Wait.  */
#if FUTEX_WAIT == 0
	xorl	%esi, %esi
#else
	movl	$FUTEX_WAIT, %esi
#endif
	movl	$SYS_futex, %eax
	syscall
	jmp	6b

	/* Preserve the pointer to the control variable.  */
3:	pushq	%rdi
.Lpush_rdi:

.LcleanupSTART:
	callq	*8(%rsp)
.LcleanupEND:

	/* Get the control variable address back.  */
	popq	%rdi
.Lpop_rdi:

	/* Sucessful run of the initializer.  Signal that we are done.  */
	LOCK
	incl	(%rdi)

	/* Wake up all other threads.  */
	movl	$0x7fffffff, %edx
	movl	$FUTEX_WAKE, %esi
	movl	$SYS_futex, %eax
	syscall

4:	addq	$8, %rsp
.Ladd:
	xorl	%eax, %eax
	retq

	.size	__pthread_once,.-__pthread_once


	.globl	__pthread_once_internal
__pthread_once_internal = __pthread_once

	.globl	pthread_once
pthread_once = __pthread_once


	.type	clear_once_control,@function
	.align	16
clear_once_control:
	movq	(%rsp), %rdi
	movq	%rax, %r8
	movl	$0, (%rdi)

	movl	$0x7fffffff, %edx
	movl	$FUTEX_WAKE, %esi
	movl	$SYS_futex, %eax
	syscall

	movq	%r8, %rdi
.LcallUR:
	call	_Unwind_Resume@PLT
	hlt
.LENDCODE:
	.size	clear_once_control,.-clear_once_control


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
	.uleb128 clear_once_control-.LSTARTCODE
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
	.long	.Lpush_rsi-.LSTARTCODE
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpush_rdi-.Lpush_rsi
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 24
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpop_rdi-.Lpush_rdi
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
	.byte	4				# DW_CFA_advance_loc4
	.long	.Ladd-.Lpop_rdi
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 8
	.byte	4				# DW_CFA_advance_loc4
	.long	clear_once_control-.Ladd
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 24
#if 0
	.byte	4				# DW_CFA_advance_loc4
	.long	.Lpop_rdi2-clear_once_control
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
#endif
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
