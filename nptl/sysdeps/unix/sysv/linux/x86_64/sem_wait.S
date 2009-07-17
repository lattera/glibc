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


	.text

	.globl	sem_wait
	.type	sem_wait,@function
	.align	16
sem_wait:
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

	leal	-1(%rax), %edx
	LOCK
#if VALUE == 0
	cmpxchgl %edx, (%rdi)
#else
	cmpxchgl %edx, VALUE(%rdi)
#endif
	jne	2b

	xorl	%eax, %eax
	retq

1:	pushq	%r12
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r12, 0)
	pushq	%r13
	cfi_adjust_cfa_offset(8)
	cfi_rel_offset(%r13, 0)
	movq	%rdi, %r13

	LOCK
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

	xorl	%eax, %eax

9:	popq	%r13
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r13)
	popq	%r12
	cfi_adjust_cfa_offset(-8)
	cfi_restore(%r12)

	retq

	cfi_adjust_cfa_offset(2 * 8)
	cfi_rel_offset(%r12, 8)
	cfi_rel_offset(%r13, 0)
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
	cfi_endproc
	.size	sem_wait_cleanup,.-sem_wait_cleanup


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	DW_EH_PE_omit			# @LPStart format
	.byte	DW_EH_PE_omit			# @TType format
	.byte	DW_EH_PE_uleb128		# call-site format
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
	.align	8
	.type	DW.ref.__gcc_personality_v0, @object
	.size	DW.ref.__gcc_personality_v0, 8
DW.ref.__gcc_personality_v0:
	.quad	__gcc_personality_v0
#endif
