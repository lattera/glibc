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
#include <pthread-errnos.h>

#ifdef UP
# define LOCK
#else
# define LOCK lock
#endif

#define SYS_futex		202
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

/* For the calculation see asm/vsyscall.h.  */
#define VSYSCALL_ADDR_vgettimeofday	0xffffffffff600000


	.text

/* int pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			       const struct timespec *abstime)  */
	.globl	__pthread_cond_timedwait
	.type	__pthread_cond_timedwait, @function
	.align	16
__pthread_cond_timedwait:
.LSTARTCODE:
	pushq	%r12
.Lpush_r12:
	pushq	%r13
.Lpush_r13:
	pushq	%r14
.Lpush_r14:
#define FRAME_SIZE 80
	subq	$FRAME_SIZE, %rsp
.Lsubq:

	cmpq	$1000000000, 8(%rdx)
	movq	$EINVAL, %rax
	jae	18f

	/* Stack frame:

	   rsp + 80
	            +--------------------------+
	   rsp + 48 | cleanup buffer           |
	            +--------------------------+
	   rsp + 40 | old wake_seq value       |
	            +--------------------------+
	   rsp + 24 | timeout value            |
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
	movq	%rdx, %r13

	je	22f
	movq	%rsi, dep_mutex(%rdi)

	/* Get internal lock.  */
22:	movl	$1, %esi
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %esi, (%rdi)
#else
	cmpxchgl %esi, cond_lock(%rdi)
#endif
	jnz	1f

	/* Unlock the mutex.  */
2:	movq	16(%rsp), %rdi
	xorq	%rsi, %rsi
	callq	__pthread_mutex_unlock_usercnt

	testl	%eax, %eax
	jne	16f

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
	leaq	48(%rsp), %rdi
	movq	%rsp, %rdx
	callq	__pthread_cleanup_push

	/* Get and store current wakeup_seq value.  */
	movq	8(%rsp), %rdi
	movq	wakeup_seq(%rdi), %r9
	movl	broadcast_seq(%rdi), %edx
	movq	%r9, 40(%rsp)
	movl	%edx, 4(%rsp)

	/* Get the current time.  */
8:
#ifdef __NR_clock_gettime
	/* Get the clock number.  Note that the field in the condvar
	   structure stores the number minus 1.  */
	movq	8(%rsp), %rdi
	movl	cond_nwaiters(%rdi), %edi
	andl	$((1 << clock_bits) - 1), %edi
	/* Only clocks 0 and 1 are allowed so far.  Both are handled in the
	   kernel.  */
	leaq	24(%rsp), %rsi
	movq	$__NR_clock_gettime, %rax
	syscall
# ifndef __ASSUME_POSIX_TIMERS
	cmpq	$-ENOSYS, %rax
	je	19f
# endif

	/* Compute relative timeout.  */
	movq	(%r13), %rcx
	movq	8(%r13), %rdx
	subq	24(%rsp), %rcx
	subq	32(%rsp), %rdx
#else
	leaq	24(%rsp), %rdi
	xorq	%rsi, %rsi
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	callq	*%rax

	/* Compute relative timeout.  */
	movq	32(%rsp), %rax
	movq	$1000, %rdx
	mul	%rdx		/* Milli seconds to nano seconds.  */
	movq	(%r13), %rcx
	movq	8(%r13), %rdx
	subq	24(%rsp), %rcx
	subq	%rax, %rdx
#endif
	jns	12f
	addq	$1000000000, %rdx
	decq	%rcx
12:	testq	%rcx, %rcx
	movq	8(%rsp), %rdi
	movq	$-ETIMEDOUT, %r14
	js	6f

	/* Store relative timeout.  */
21:	movq	%rcx, 24(%rsp)
	movq	%rdx, 32(%rsp)

	movl	cond_futex(%rdi), %r12d

	/* Unlock.  */
	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	3f

4:	callq	__pthread_enable_asynccancel
	movl	%eax, (%rsp)

	leaq	24(%rsp), %r10
	xorq	%rsi, %rsi	/* movq $FUTEX_WAIT, %rsi */
	movq	%r12, %rdx
	addq	$cond_futex, %rdi
	movq	$SYS_futex, %rax
	syscall
	movq	%rax, %r14

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
	jne	5f

6:	movl	broadcast_seq(%rdi), %edx

	movq	woken_seq(%rdi), %rax

	movq	wakeup_seq(%rdi), %r9

	cmpl	4(%rsp), %edx
	jne	23f

	cmpq	40(%rsp), %r9
	jbe	15f

	cmpq	%rax, %r9
	ja	9f

15:	cmpq	$-ETIMEDOUT, %r14
	jne	8b

13:	incq	wakeup_seq(%rdi)
	incl	cond_futex(%rdi)
	movq	$ETIMEDOUT, %r14
	jmp	14f

23:	xorq	%r14, %r14
	jmp	24f

9:	xorq	%r14, %r14
14:	incq	woken_seq(%rdi)

24:	subl	$(1 << clock_bits), cond_nwaiters(%rdi)

	/* Wake up a thread which wants to destroy the condvar object.  */
	cmpq	$0xffffffffffffffff, total_seq(%rdi)
	jne	25f
	movl	cond_nwaiters(%rdi), %eax
	andl	$~((1 << clock_bits) - 1), %eax
	jne	25f

	addq	$cond_nwaiters, %rdi
	movq	$SYS_futex, %rax
	movq	$FUTEX_WAKE, %rsi
	movl	$1, %edx
	syscall
	subq	$cond_nwaiters, %rdi

25:	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	10f

	/* Remove cancellation handler.  */
11:	movq	48+CLEANUP_PREV(%rsp), %rdx
	movq	%rdx, %fs:CLEANUP

	movq	16(%rsp), %rdi
	callq	__pthread_mutex_cond_lock

	testq	%rax, %rax
	cmoveq	%r14, %rax

18:	addq	$FRAME_SIZE, %rsp
.Laddq:
	popq	%r14
.Lpop_r14:
	popq	%r13
.Lpop_r13:
	popq	%r12
.Lpop_r12:

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
16:	movq	8(%rsp), %rdi
	movq	%rax, (%rsp)
	LOCK
#if cond_lock == 0
	decl	(%rdi)
#else
	decl	cond_lock(%rdi)
#endif
	jne	17f

#if cond_lock != 0
	addq	$cond_lock, %rdi
#endif
	callq	__lll_mutex_unlock_wake

17:	movq	(%rsp), %rax
	jmp	18b

#if defined __NR_clock_gettime && !defined __ASSUME_POSIX_TIMERS
	/* clock_gettime not available.  */
19:	leaq	24(%rsp), %rdi
	xorq	%rsi, %rsi
	movq	$VSYSCALL_ADDR_vgettimeofday, %rax
	callq	*%rax

	/* Compute relative timeout.  */
	movq	32(%rsp), %rax
	movq	$1000, %rdx
	mul	%rdx		/* Milli seconds to nano seconds.  */
	movq	(%r13), %rcx
	movq	8(%r13), %rdx
	subq	24(%rsp), %rcx
	subq	%rax, %rdx
	jns	20f
	addq	$1000000000, %rdx
	decq	%rcx
20:	testq	%rcx, %rcx
	movq	8(%rsp), %rdi
	movq	$-ETIMEDOUT, %r14
	js	6b
	jmp	21b
#endif
.LENDCODE:
	.size	__pthread_cond_timedwait, .-__pthread_cond_timedwait
versioned_symbol (libpthread, __pthread_cond_timedwait, pthread_cond_timedwait,
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
	.byte	0x40+.Lpush_r13-.Lpush_r12	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 24
	.byte	0x8d				# DW_CFA_offset %r13
	.uleb128 3
	.byte	0x40+.Lpush_r14-.Lpush_r13	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 32
	.byte	0x84				# DW_CFA_offset %r14
	.uleb128 4
	.byte	0x40+.Lsubq-.Lpush_r14		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 32+FRAME_SIZE
	.byte	3				# DW_CFA_advance_loc2
	.2byte	.Laddq-.Lsubq
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 32
	.byte	0x40+.Lpop_r14-.Laddq		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 24
	.byte	0xce				# DW_CFA_restore %r14
	.byte	0x40+.Lpop_r13-.Lpop_r14	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
	.byte	0xcd				# DW_CFA_restore %r13
	.byte	0x40+.Lpop_r12-.Lpop_r13	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 8
	.byte	0xcc				# DW_CFA_restore %r12
	.byte	0x40+.LSbl1-.Lpop_r12		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 32+FRAME_SIZE
	.byte	0x8c				# DW_CFA_offset %r12
	.uleb128 2
	.byte	0x8d				# DW_CFA_offset %r13
	.uleb128 3
	.byte	0x84				# DW_CFA_offset %r14
	.uleb128 4
	.align	8
.LENDFDE:
