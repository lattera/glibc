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

	cmpl	$1000000000, 4(%ebp)
	movl	$EINVAL, %eax
	jae	18f

	/* Get internal lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %edx, (%ebx)
#else
	cmpxchgl %edx, cond_lock(%ebx)
#endif
	jnz	1f

	/* Store the reference to the mutex.  If there is already a
	   different value in there this is a bad user bug.  */
2:	cmpl	$-1, dep_mutex(%ebx)
	movl	24(%esp), %eax
	je	17f
	movl	%eax, dep_mutex(%ebx)

	/* Unlock the mutex.  */
17:	xorl	%edx, %edx
	call	__pthread_mutex_unlock_usercnt

	testl	%eax, %eax
	jne	16f

	addl	$1, total_seq(%ebx)
	adcl	$0, total_seq+4(%ebx)
	addl	$1, cond_futex(%ebx)
	addl	$(1 << clock_bits), cond_nwaiters(%ebx)

#define FRAME_SIZE 24
	subl	$FRAME_SIZE, %esp
.Lsubl:

	/* Get and store current wakeup_seq value.  */
	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx
	movl	broadcast_seq(%ebx), %eax
	movl	%edi, 12(%esp)
	movl	%edx, 16(%esp)
	movl	%eax, 20(%esp)

	/* Get the current time.  */
8:	movl	%ebx, %edx
#ifdef __NR_clock_gettime
	/* Get the clock number.  */
	movl	cond_nwaiters(%ebx), %ebx
	andl	$((1 << clock_bits) - 1), %ebx
	/* Only clocks 0 and 1 are allowed so far.  Both are handled in the
	   kernel.  */
	leal	4(%esp), %ecx
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
	subl	4(%esp), %ecx
	subl	8(%esp), %edx
#else
	/* Get the current time.  */
	leal	4(%esp), %ebx
	xorl	%ecx, %ecx
	movl	$SYS_gettimeofday, %eax
	ENTER_KERNEL
	movl	%edx, %ebx

	/* Compute relative timeout.  */
	movl	8(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%ebp), %ecx
	movl	4(%ebp), %edx
	subl	4(%esp), %ecx
	subl	%eax, %edx
#endif
	jns	12f
	addl	$1000000000, %edx
	subl	$1, %ecx
12:	testl	%ecx, %ecx
	movl	$-ETIMEDOUT, %esi
	js	6f

	/* Store relative timeout.  */
21:	movl	%ecx, 4(%esp)
	movl	%edx, 8(%esp)

	movl	cond_futex(%ebx), %edi

	/* Unlock.  */
	LOCK
#if cond_lock == 0
	subl	$1, (%ebx)
#else
	subl	$1, cond_lock(%ebx)
#endif
	jne	3f

.LcleanupSTART:
4:	call	__pthread_enable_asynccancel
	movl	%eax, (%esp)

	leal	4(%esp), %esi
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%edi, %edx
	addl	$cond_futex, %ebx
.Ladd_cond_futex:
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	subl	$cond_futex, %ebx
.Lsub_cond_futex:
	movl	%eax, %esi

	movl	(%esp), %eax
	call	__pthread_disable_asynccancel
.LcleanupEND:

	/* Lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %edx, (%ebx)
#else
	cmpxchgl %edx, cond_lock(%ebx)
#endif
	jnz	5f

6:	movl	broadcast_seq(%ebx), %eax
	cmpl	20(%esp), %eax
	jne	23f

	movl	woken_seq(%ebx), %eax
	movl	woken_seq+4(%ebx), %ecx

	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx

	cmpl	16(%esp), %edx
	jne	7f
	cmpl	12(%esp), %edi
	je	15f

7:	cmpl	%ecx, %edx
	jne	9f
	cmp	%eax, %edi
	jne	9f

15:	cmpl	$-ETIMEDOUT, %esi
	jne	8b

	addl	$1, wakeup_seq(%ebx)
	adcl	$0, wakeup_seq+4(%ebx)
	addl	$1, cond_futex(%ebx)
	movl	$ETIMEDOUT, %esi
	jmp	14f

23:	xorl	%esi, %esi
	jmp	24f

9:	xorl	%esi, %esi
14:	addl	$1, woken_seq(%ebx)
	adcl	$0, woken_seq+4(%ebx)

24:	subl	$(1 << clock_bits), cond_nwaiters(%ebx)

	/* Wake up a thread which wants to destroy the condvar object.  */
	movl	total_seq(%ebx), %eax
	andl	total_seq+4(%ebx), %eax
	cmpl	$0xffffffff, %eax
	jne	25f
	movl	cond_nwaiters(%ebx), %eax
	andl	$~((1 << clock_bits) - 1), %eax
	jne	25f

	addl	$cond_nwaiters, %ebx
	movl	$SYS_futex, %eax
	movl	$FUTEX_WAKE, %ecx
	movl	$1, %edx
	ENTER_KERNEL
	subl	$cond_nwaiters, %ebx

25:	LOCK
#if cond_lock == 0
	subl	$1, (%ebx)
#else
	subl	$1, cond_lock(%ebx)
#endif
	jne	10f

	/* Remove cancellation handler.  */
11:	movl	24+FRAME_SIZE(%esp), %eax
	call	__pthread_mutex_cond_lock
	addl	$FRAME_SIZE, %esp
.Laddl:

	/* We return the result of the mutex_lock operation if it failed.  */
	testl	%eax, %eax
#ifdef HAVE_CMOV
	cmovel	%esi, %eax
#else
	jne	22f
	movl	%esi, %eax
22:
#endif

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

	/* Unlock in loop requires wakeup.  */
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

	/* Unlock after loop requires wakeup.  */
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
	LOCK
#if cond_lock == 0
	subl	$1, (%ebx)
#else
	subl	$1, cond_lock(%ebx)
#endif
	jne	18b

	movl	%eax, %esi
#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake

	movl	%esi, %eax
	jmp	18b

#if defined __NR_clock_gettime && !defined __ASSUME_POSIX_TIMERS
	/* clock_gettime not available.  */
.LSbl4:
19:	leal	4(%esp), %ebx
	xorl	%ecx, %ecx
	movl	$SYS_gettimeofday, %eax
	ENTER_KERNEL
	movl	%edx, %ebx

	/* Compute relative timeout.  */
	movl	8(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%ebp), %ecx
	movl	4(%ebp), %edx
	subl	4(%esp), %ecx
	subl	%eax, %edx
	jns	20f
	addl	$1000000000, %edx
	subl	$1, %ecx
20:	testl	%ecx, %ecx
	movl	$-ETIMEDOUT, %esi
	js	6b
	jmp	21b
#endif
	.size	__pthread_cond_timedwait, .-__pthread_cond_timedwait
versioned_symbol (libpthread, __pthread_cond_timedwait, pthread_cond_timedwait,
		  GLIBC_2_3_2)


	.type	__condvar_tw_cleanup2, @function
__condvar_tw_cleanup2:
	subl	$cond_futex, %ebx
.LSbl5:
	.size	__condvar_tw_cleanup2, .-__condvar_tw_cleanup2
	.type	__condvar_tw_cleanup, @function
__condvar_tw_cleanup:
	movl	%eax, %esi

	/* Get internal lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
#if cond_lock == 0
	cmpxchgl %edx, (%ebx)
#else
	cmpxchgl %edx, cond_lock(%ebx)
#endif
	jz	1f

#if cond_lock == 0
	movl	%ebx, %ecx
#else
	leal	cond_lock(%ebx), %ecx
#endif
	call	__lll_mutex_lock_wait

1:	movl	broadcast_seq(%ebx), %eax
	cmpl	20(%esp), %eax
	jne	3f

	addl	$1, wakeup_seq(%ebx)
	adcl	$0, wakeup_seq+4(%ebx)

	addl	$1, cond_futex(%ebx)

	addl	$1, woken_seq(%ebx)
	adcl	$0, woken_seq+4(%ebx)

3:	subl	$(1 << clock_bits), cond_nwaiters(%ebx)

	/* Wake up a thread which wants to destroy the condvar object.  */
	xorl	%edi, %edi
	movl	total_seq(%ebx), %eax
	andl	total_seq+4(%ebx), %eax
	cmpl	$0xffffffff, %eax
	jne	4f
	movl	cond_nwaiters(%ebx), %eax
	andl	$~((1 << clock_bits) - 1), %eax
	jne	4f

	addl	$cond_nwaiters, %ebx
	movl	$SYS_futex, %eax
	movl	$FUTEX_WAKE, %ecx
	movl	$1, %edx
	ENTER_KERNEL
	subl	$cond_nwaiters, %ebx
	movl	$1, %edi

4:	LOCK
#if cond_lock == 0
	subl	$1, (%ebx)
#else
	subl	$1, cond_lock(%ebx)
#endif
	je	2f

#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
	call	__lll_mutex_unlock_wake

	/* Wake up all waiters to make sure no signal gets lost.  */
2:	testl	%edi, %edi
	jnz	5f
	addl	$cond_futex, %ebx
	movl	$FUTEX_WAKE, %ecx
	movl	$SYS_futex, %eax
	movl	$0x7fffffff, %edx
	ENTER_KERNEL

5:	movl	24+FRAME_SIZE(%esp), %eax
	call	__pthread_mutex_cond_lock

	movl	%esi, (%esp)
.LcallUR:
	call	_Unwind_Resume
	hlt
.LENDCODE:
	.size	__condvar_tw_cleanup, .-__condvar_tw_cleanup


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	0xff				# @LPStart format (omit)
	.byte	0xff				# @TType format (omit)
	.byte	0x0b				# call-site format
						# DW_EH_PE_sdata4
	.uleb128 .Lcstend-.Lcstbegin
.Lcstbegin:
	.long	.LcleanupSTART-.LSTARTCODE
	.long	.Ladd_cond_futex-.LcleanupSTART
	.long	__condvar_tw_cleanup-.LSTARTCODE
	.uleb128  0
	.long	.Ladd_cond_futex-.LSTARTCODE
	.long	.Lsub_cond_futex-.Ladd_cond_futex
	.long	__condvar_tw_cleanup2-.LSTARTCODE
	.uleb128  0
	.long	.Lsub_cond_futex-.LSTARTCODE
	.long	.LcleanupEND-.Lsub_cond_futex
	.long	__condvar_tw_cleanup-.LSTARTCODE
	.uleb128  0
	.long	.LcallUR-.LSTARTCODE
	.long	.LENDCODE-.LcallUR
	.long	0
	.uleb128  0
.Lcstend:


	.section .eh_frame,"a",@progbits
.LSTARTFRAME:
	.long	L(ENDCIE)-L(STARTCIE)		# Length of the CIE.
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
	.sleb128 -4				# Data alignment factor.
	.byte	8				# Return address register
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
	.uleb128 6				# Augmentation value length.
	.byte	0x0				# Personality: absolute
	.long	__gcc_personality_v0
	.byte	0x0				# LSDA Encoding: absolute
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
	.uleb128 4				# Augmentation size
#ifdef SHARED
	.long	.LexceptSTART-.
#else
	.long	.LexceptSTART
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
	.byte	2				# DW_CFA_advance_loc1
	.byte	.Lsubl-.Lpush_ebx
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 20+FRAME_SIZE
	.byte	3				# DW_CFA_advance_loc2
	.2byte	.Laddl-.Lsubl
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
	.uleb128 20+FRAME_SIZE
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
	.uleb128 20
#if defined __NR_clock_gettime && !defined __ASSUME_POSIX_TIMERS
	.byte	0x40+.LSbl4-.LSbl3		# DW_CFA_advance_loc+N
#else
	.byte	4				# DW_CFA_advance_loc4
	.long	.LSbl5-.LSbl3
#endif
	.byte	14				# DW_CFA_def_cfa_offset
	.uleb128 20+FRAME_SIZE
	.align	4
.LENDFDE:

#ifdef SHARED
	.hidden DW.ref.__gcc_personality_v0
	.weak   DW.ref.__gcc_personality_v0
	.section .gnu.linkonce.d.DW.ref.__gcc_personality_v0,"aw",@progbits
	.align 4
	.type   DW.ref.__gcc_personality_v0, @object
	.size   DW.ref.__gcc_personality_v0, 4
DW.ref.__gcc_personality_v0:
	.long   __gcc_personality_v0
#endif
