/* Copyright (C) 2002, 2003, 2004 Free Software Foundation, Inc.
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
#include <shlib-compat.h>
#include <lowlevelcond.h>
#include <tcb-offsets.h>

#ifdef UP
# define LOCK
#else
# define LOCK lock
#endif

#define SYS_futex		202
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.text

	.align	16
	.type	__condvar_cleanup, @function
	.globl	__condvar_cleanup
	.hidden	__condvar_cleanup
__condvar_cleanup:
	pushq	%r12

	/* Get internal lock.  */
	movq	%rdi, %r8
	movq	8(%rdi), %rdi
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, cond_lock(%rdi)
#endif
	jz	1f

#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_lock_wait
#if cond_lock != 0
	subq	$cond_lock, %rdi
#endif

1:	movl	broadcast_seq(%rdi), %edx
	cmpl	4(%r8), %edx
	jne	3f

	incq	wakeup_seq(%rdi)
	incq	woken_seq(%rdi)
	incl	cond_futex(%rdi)

3:	subl	$(1 << clock_bits), cond_nwaiters(%rdi)

	/* Wake up a thread which wants to destroy the condvar object.  */
	xorq	%r12, %r12
	cmpq	$0xffffffffffffffff, total_seq(%rdi)
	jne	4f
	movl	cond_nwaiters(%rdi), %eax
	andl	$~((1 << clock_bits) - 1), %eax
	jne	4f

	addq	$cond_nwaiters, %rdi
	movq	$SYS_futex, %rax
	movq	$FUTEX_WAKE, %rsi
	movl	$1, %edx
	syscall
	subq	$cond_nwaiters, %rdi
	movq	$1, %r12

4:	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	je	2f
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_unlock_wake

	/* Wake up all waiters to make sure no signal gets lost.  */
2:	testq	%r12, %r12
	jnz	5f
	addq	$cond_futex, %rdi
	movq	$FUTEX_WAKE, %rsi
	movl	$0x7fffffff, %edx
	movq	$SYS_futex, %rax
	syscall

5:	movq	16(%r8), %rdi
	callq	__pthread_mutex_cond_lock

	popq	%r12

	retq
	.size	__condvar_cleanup, .-__condvar_cleanup


/* int pthread_cond_wait (pthread_cond_t *cond, pthread_mutex_t *mutex)  */
	.globl	__pthread_cond_wait
	.type	__pthread_cond_wait, @function
	.align	16
__pthread_cond_wait:
.LSTARTCODE:
	pushq	%r12
.Lpush_r12:
#define FRAME_SIZE 64
	subq	$FRAME_SIZE, %rsp
.Lsubq:
	/* Stack frame:

	   rsp + 64
	            +--------------------------+
	   rsp + 32 | cleanup buffer           |
		    +--------------------------+
	   rsp + 24 | old wake_seq value       |
	            +--------------------------+
	   rsp + 16 | mutex pointer            |
	            +--------------------------+
	   rsp +  8 | condvar pointer          |
	            +--------------------------+
	   rsp +  4 | old broadcast_seq value  |
	            +--------------------------+
	   rsp +  0 | old cancellation mode    |
	            +--------------------------+
	*/

	cmpq	$-1, dep_mutex(%rdi)

		/* Prepare structure passed to cancellation handler.  */
	movq	%rdi, 8(%rsp)
	movq	%rsi, 16(%rsp)

	je	15f
	movq	%rsi, dep_mutex(%rdi)

	/* Get internal lock.  */
15:	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, cond_lock(%rdi)
#endif
	jne	1f

	/* Unlock the mutex.  */
2:	movq	16(%rsp), %rdi
	xorq	%rsi, %rsi
	callq	__pthread_mutex_unlock_usercnt

	testl	%eax, %eax
	jne	12f

	movq	8(%rsp), %rdi
	incq	total_seq(%rdi)
	incl	cond_futex(%rdi)
	addl	$(1 << clock_bits), cond_nwaiters(%rdi)

	/* Install cancellation handler.  */
#ifdef PIC
	leaq	__condvar_cleanup(%rip), %rsi
#else
	leaq	__condvar_cleanup, %rsi
#endif
	leaq	32(%rsp), %rdi
	movq	%rsp, %rdx
	callq	__pthread_cleanup_push

	/* Get and store current wakeup_seq value.  */
	movq	8(%rsp), %rdi
	movq	wakeup_seq(%rdi), %r9
	movl	broadcast_seq(%rdi), %edx
	movq	%r9, 24(%rsp)
	movl	%edx, 4(%rsp)

	/* Unlock.  */
8:	movl	cond_futex(%rdi), %r12d
	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	3f

4:	callq	__pthread_enable_asynccancel
	movl	%eax, (%rsp)

	movq	8(%rsp), %rdi
	xorq	%r10, %r10
	movq	%r12, %rdx
	addq	$cond_futex-cond_lock, %rdi
	movq	$SYS_futex, %rax
	movq	%r10, %rsi	/* movq $FUTEX_WAIT, %rsi */
	syscall

	movl	(%rsp), %edi
	callq	__pthread_disable_asynccancel

	/* Lock.  */
	movq	8(%rsp), %rdi
	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, cond_lock(%rdi)
#endif
	jnz	5f

6:	movl	broadcast_seq(%rdi), %edx

	movq	woken_seq(%rdi), %rax

	movq	wakeup_seq(%rdi), %r9

	cmpl	4(%rsp), %edx
	jne	16f

	cmpq	24(%rsp), %r9
	jbe	8b

	cmpq	%rax, %r9
	jna	8b

	incq	woken_seq(%rdi)

	/* Unlock */
16:	subl	$(1 << clock_bits), cond_nwaiters(%rdi)

	/* Wake up a thread which wants to destroy the condvar object.  */
	cmpq	$0xffffffffffffffff, total_seq(%rdi)
	jne	17f
	movl	cond_nwaiters(%rdi), %eax
	andl	$~((1 << clock_bits) - 1), %eax
	jne	17f

	addq	$cond_nwaiters, %rdi
	movq	$SYS_futex, %rax
	movq	$FUTEX_WAKE, %rsi
	movl	$1, %edx
	syscall
	subq	$cond_nwaiters, %rdi

17:	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	10f

	/* Remove cancellation handler.  */
11:	movq	32+CLEANUP_PREV(%rsp), %rdx
	movq	%rdx, %fs:CLEANUP

	movq	16(%rsp), %rdi
	callq	__pthread_mutex_cond_lock
14:	addq	$FRAME_SIZE, %rsp
.Laddq:

	popq	%r12
.Lpop_r12:

	/* We return the result of the mutex_lock operation.  */
	retq

	/* Initial locking failed.  */
1:
.LSbl1:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_lock_wait
	jmp	2b

	/* Unlock in loop requires wakeup.  */
3:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_unlock_wake
	jmp	4b

	/* Locking in loop failed.  */
5:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_lock_wait
#if cond_lock != 0
	subq	$cond_lock, %rdi
#endif
	jmp	6b

	/* Unlock after loop requires wakeup.  */
10:
#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_unlock_wake
	jmp	11b

	/* The initial unlocking of the mutex failed.  */
12:	movq	%rax, %r10
	movq	8(%rsp), %rdi
	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	13f

#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_unlock_wake

13:	movq	%r10, %rax
	jmp	14b
.LENDCODE:
	.size	__pthread_cond_wait, .-__pthread_cond_wait
versioned_symbol (libpthread, __pthread_cond_wait, pthread_cond_wait,
		  GLIBC_2_3_2)


	.section .eh_frame,"a",@progbits
.LSTARTFRAME:
	.long	L(ENDCIE)-L(STARTCIE)		# Length of the CIE.
.LSTARTCIE:
	.long	0				# CIE ID.
	.byte	1				# Version number.
#ifdef SHARED
	.string	"zR"				# NUL-terminated augmentation
						# string.
#else
	.ascii	"\0"				# NUL-terminated augmentation
						# string.
#endif
	.uleb128 1				# Code alignment factor.
	.sleb128 -8				# Data alignment factor.
	.byte	16				# Return address register
						# column.
#ifdef SHARED
	.uleb128 1				# Augmentation value length.
	.byte	0x1b				# Encoding: DW_EH_PE_pcrel
						# + DW_EH_PE_sdata4.
#endif
	.byte 0x0c				# DW_CFA_def_cfa
	.uleb128 7
	.uleb128 8
	.byte	0x90				# DW_CFA_offset, column 0x8
	.uleb128 1
	.align 8
.LENDCIE:

	.long	.LENDFDE-.LSTARTFDE		# Length of the FDE.
.LSTARTFDE:
	.long	.LSTARTFDE-.LSTARTFRAME		# CIE pointer.
#ifdef SHARED
	.long	.LSTARTCODE-.			# PC-relative start address
						# of the code
#else
	.long	.LSTARTCODE			# Start address of the code.
#endif
	.long	.LENDCODE-.LSTARTCODE		# Length of the code.
#ifdef SHARED
	.uleb128 0				# No augmentation data.
#endif
	.byte	0x40+.Lpush_r12-.LSTARTCODE	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
	.byte	0x8c				# DW_CFA_offset %r12
	.uleb128 2
	.byte	0x40+.Lsubq-.Lpush_r12		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16+FRAME_SIZE
	.byte	3				# DW_CFA_advance_loc2
	.2byte	.Laddq-.Lsubq
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
	.byte	0x40+.Lpop_r12-.Laddq		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 8
	.byte	0xcc				# DW_CFA_restore %r12
	.byte	0x40+.LSbl1-.Lpop_r12		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 80
	.byte	0x8c				# DW_CFA_offset %r12
	.uleb128 2
	.align	8
.LENDFDE:
