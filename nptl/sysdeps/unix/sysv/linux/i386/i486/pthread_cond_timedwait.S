/* Copyright (C) 2002, 2003 Free Software Foundation, Inc.
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

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1


	.text

/* int pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			       const struct timespec *abstime)  */
	.globl	__pthread_cond_timedwait
	.type	__pthread_cond_timedwait, @function
	.align	16
__pthread_cond_timedwait:
.LSTARTCODE:
	pushl	%ebp
.Lpush_ebp:
	pushl	%edi
.Lpush_edi:
	pushl	%esi
.Lpush_esi:
	pushl	%ebx
.Lpush_ebx:

	movl	20(%esp), %ebx
	movl	28(%esp), %ebp

	/* Get internal lock.  */
	movl	$1, %eax
	LOCK
#if cond_lock == 0
	xaddl	%eax, (%ebx)
#else
	xaddl	%eax, cond_lock(%ebx)
#endif
	testl	%eax, %eax
	jne	1f

	/* Unlock the mutex.  */
2:	pushl	24(%esp)
.Lpush4:
	call	__pthread_mutex_unlock_internal

	testl	%eax, %eax
	jne	16f

	addl	$1, total_seq(%ebx)
	adcl	$0, total_seq+4(%ebx)

	/* Install cancellation handler.  */
#ifdef PIC
	call	__i686.get_pc_thunk.cx
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	leal	__condvar_cleanup@GOTOFF(%ecx), %eax
#else
	leal	__condvar_cleanup, %eax
#endif
	subl	$40, %esp
.Lsubl:
	leal	28(%esp), %edx
	movl	%esp, 8(%esp)
	movl	%eax, 4(%esp)
	movl	%edx, (%esp)
	call	__pthread_cleanup_push

	/* Address of the mutex.  */
	movl	68(%esp), %ecx
	/* Get and store current wakeup_seq value.  */
	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx
	movl	%edi, 20(%esp)
	movl	%edx, 24(%esp)
	/* Prepare structure passed to cancellation handler.  */
	movl	%ecx, (%esp)
	movl	%ebx, 4(%esp)

	/* Unlock.  */
8:	LOCK
#if cond_lock == 0
	subl	$1, (%ebx)
#else
	subl	$1, cond_lock(%ebx)
#endif
	jne	3f

4:	leal	8(%esp), %eax
	call	__pthread_enable_asynccancel_2

	/* Get the current time.  */
	movl	%ebx, %edx
#ifdef __NR_clock_gettime
	/* Get the clock number.  Note that the field in the condvar
	   structure stores the number minus 1.  */
	movl	cond_clock(%ebx), %ebx
	/* Only clocks 0 and 1 are allowed.  Both are handled in the
	   kernel.  */
	leal	12(%esp), %ecx
	movl	$__NR_clock_gettime, %eax
	ENTER_KERNEL
# ifndef __ASSUME_POSIX_TIMERS
	cmpl	$-ENOSYS, %eax
	je	19f
# endif
	movl	%edx, %ebx

	/* Compute relative timeout.  */
	movl	(%ebp), %ecx
	movl	4(%ebp), %edx
	subl	12(%esp), %ecx
	subl	16(%esp), %edx
#else
	/* Get the current time.  */
	leal	12(%esp), %ebx
	xorl	%ecx, %ecx
	movl	$SYS_gettimeofday, %eax
	ENTER_KERNEL
	movl	%edx, %ebx

	/* Compute relative timeout.  */
	movl	16(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%ebp), %ecx
	movl	4(%ebp), %edx
	subl	12(%esp), %ecx
	subl	%eax, %edx
#endif
	jns	12f
	addl	$1000000000, %edx
	subl	$1, %ecx
12:	testl	%ecx, %ecx
	js	13f

	/* Store relative timeout.  */
21:	movl	%ecx, 12(%esp)
	movl	%edx, 16(%esp)
	leal	12(%esp), %esi
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%edi, %edx
	addl	$wakeup_seq, %ebx
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	subl	$wakeup_seq, %ebx
	movl	%eax, %esi

	movl	8(%esp), %eax
	call	__pthread_disable_asynccancel

	/* Lock.  */
	movl	$1, %eax
	LOCK
#if cond_lock == 0
	xaddl	%eax, (%ebx)
#else
	xaddl	%eax, cond_lock(%ebx)
#endif
	testl	%eax, %eax
	jne	5f

6:	movl	woken_seq(%ebx), %eax
	movl	woken_seq+4(%ebx), %ecx

	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx

	cmpl	24(%esp), %edx
	ja	7f
	jb	15f
	cmpl	20(%esp), %edi
	jbe	15f

7:	cmpl	%ecx, %edx
	ja	9f
	jb	15f
	cmp	%eax, %edi
	ja	9f

15:	cmpl	$-ETIMEDOUT, %esi
	jne	8b

13:	addl	$1, wakeup_seq(%ebx)
	adcl	$0, wakeup_seq+4(%ebx)
	movl	$ETIMEDOUT, %esi
	jmp	14f

9:	xorl	%esi, %esi
14:	addl	$1, woken_seq(%ebx)
	adcl	$0, woken_seq+4(%ebx)

	LOCK
#if cond_lock == 0
	subl	$1, (%ebx)
#else
	subl	$1, cond_lock(%ebx)
#endif
	jne	10f

	/* Remove cancellation handler.  */
11:	movl	28+CLEANUP_PREV(%esp), %edx
	movl	%edx, %gs:CLEANUP

	/* Trick ahead:	 (%esp) contains the address of the mutex.  */
	call	__pthread_mutex_lock_internal
	addl	$44, %esp
.Laddl:

	/* We return the result of the mutex_lock operation if it failed.  */
	testl	%eax, %eax
	cmovel	%esi, %eax

18:	popl	%ebx
.Lpop_ebx:
	popl	%esi
.Lpop_esi:
	popl	%edi
.Lpop_edi:
	popl	%ebp
.Lpop_ebp:

	ret

	/* Initial locking failed.  */
1:
.LSbl1:
#if cond_lock == 0
	movl	%ebx, %ecx
#else
	leal	cond_lock(%ebx), %ecx
#endif
	call	__lll_mutex_lock_wait
	jmp	2b

	/* Unlock in loop requires waekup.  */
3:
.LSbl2:
#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake
	jmp	4b

	/* Locking in loop failed.  */
5:
#if cond_lock == 0
	movl	%ebx, %ecx
#else
	leal	cond_lock(%ebx), %ecx
#endif
	call	__lll_mutex_lock_wait
	jmp	6b

	/* Unlock after loop requires waekup.  */
10:
#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake
	jmp	11b

	/* The initial unlocking of the mutex failed.  */
16:
.LSbl3:
	movl	%eax, (%esp)
	LOCK
#if cond_lock == 0
	subl	$1, (%ebx)
#else
	subl	$1, cond_lock(%ebx)
#endif
	jne	17f

#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake

17:	popl	%eax
	jmp	18b

#if defined __NR_clock_gettime && !defined __ASSUME_POSIX_TIMERS
	/* clock_gettime not available.  */
.LSbl4:
19:	leal	12(%esp), %ebx
	xorl	%ecx, %ecx
	movl	$SYS_gettimeofday, %eax
	ENTER_KERNEL
	movl	%edx, %ebx

	/* Compute relative timeout.  */
	movl	16(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%ebp), %ecx
	movl	4(%ebp), %edx
	subl	12(%esp), %ecx
	subl	%eax, %edx
	jns	20f
	addl	$1000000000, %edx
	subl	$1, %ecx
20:	testl	%ecx, %ecx
	js	13b
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
	.sleb128 -4				# Data alignment factor.
	.byte	8				# Return address register
						# column.
#ifdef SHARED
	.uleb128 1				# Augmentation value length.
	.byte	0x1b				# Encoding: DW_EH_PE_pcrel
						# + DW_EH_PE_sdata4.
#endif
	.byte 0x0c				# DW_CFA_def_cfa
	.uleb128 4
	.uleb128 4
	.byte	0x88				# DW_CFA_offset, column 0x8
	.uleb128 1
	.align 4
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
	.byte	0x40+.Lpush_ebp-.LSTARTCODE	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 8
	.byte	0x85				# DW_CFA_offset %ebp
	.uleb128 2
	.byte	0x40+ .Lpush_edi-.Lpush_ebp	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 12
	.byte	0x87				# DW_CFA_offset %edi
	.uleb128 3
	.byte	0x40+.Lpush_esi-.Lpush_edi	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
	.byte	0x86				# DW_CFA_offset %esi
	.uleb128 4
	.byte	0x40+.Lpush_ebx-.Lpush_esi	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 20
	.byte	0x83				# DW_CFA_offset %ebx
	.uleb128 5
	.byte	0x40+.Lpush4-.Lpush_ebx		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 24
	.byte	0x40+.Lsubl-.Lpush4		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 64
	.byte	0x40+.Laddl-.Lsubl		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 20
	.byte	0x40+.Lpop_ebx-.Laddl		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 16
	.byte	0xc3				# DW_CFA_restore %ebx
	.byte	0x40+.Lpop_esi-.Lpop_ebx	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 12
	.byte	0xc6				# DW_CFA_restore %esi
	.byte	0x40+.Lpop_edi-.Lpop_esi	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 8
	.byte	0xc7				# DW_CFA_restore %edi
	.byte	0x40+.Lpop_ebp-.Lpop_edi	# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 4
	.byte	0xc5				# DW_CFA_restore %ebp
	.byte	0x40+.LSbl1-.Lpop_edi		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 20
	.byte	0x40+.LSbl2-.LSbl1		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 64
	.byte	0x85				# DW_CFA_offset %ebp
	.uleb128 2
	.byte	0x87				# DW_CFA_offset %edi
	.uleb128 3
	.byte	0x86				# DW_CFA_offset %esi
	.uleb128 4
	.byte	0x83				# DW_CFA_offset %ebx
	.uleb128 5
	.byte	0x40+.LSbl3-.LSbl2		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 24
#if defined __NR_clock_gettime && !defined __ASSUME_POSIX_TIMERS
	.byte	0x40+.LSbl4-.LSbl3		# DW_CFA_advance_loc+N
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 64
#endif
	.align	4
.LENDFDE:
