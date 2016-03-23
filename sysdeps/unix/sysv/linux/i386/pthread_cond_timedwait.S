/* Copyright (C) 2002-2016 Free Software Foundation, Inc.
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
   License along with the GNU C Library; if not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#include <shlib-compat.h>
#include <lowlevellock.h>
#include <lowlevelcond.h>
#include <pthread-errnos.h>
#include <pthread-pi-defines.h>
#include <kernel-features.h>
#include <stap-probe.h>

	.text

/* int pthread_cond_timedwait (pthread_cond_t *cond, pthread_mutex_t *mutex,
			       const struct timespec *abstime)  */
	.globl	__pthread_cond_timedwait
	.type	__pthread_cond_timedwait, @function
	.align	16
__pthread_cond_timedwait:
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

	pushl	%ebp
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%ebp, 0)
	pushl	%edi
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%edi, 0)
	pushl	%esi
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%esi, 0)
	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	cfi_rel_offset(%ebx, 0)

	movl	20(%esp), %ebx
	movl	28(%esp), %ebp

	LIBC_PROBE (cond_timedwait, 3, %ebx, 24(%esp), %ebp)

	cmpl	$1000000000, 4(%ebp)
	movl	$EINVAL, %eax
	jae	18f

	/* Stack frame:

	   esp + 32
		    +--------------------------+
	   esp + 24 | timeout value            |
		    +--------------------------+
	   esp + 20 | futex pointer            |
		    +--------------------------+
	   esp + 16 | pi-requeued flag         |
		    +--------------------------+
	   esp + 12 | old broadcast_seq value  |
		    +--------------------------+
	   esp +  4 | old wake_seq value       |
		    +--------------------------+
	   esp +  0 | old cancellation mode    |
		    +--------------------------+
	*/

#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
# ifdef PIC
	LOAD_PIC_REG (cx)
	cmpl	$0, __have_futex_clock_realtime@GOTOFF(%ecx)
# else
	cmpl	$0, __have_futex_clock_realtime
# endif
	je	.Lreltmo
#endif

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
	addl	$(1 << nwaiters_shift), cond_nwaiters(%ebx)

#ifdef __ASSUME_FUTEX_CLOCK_REALTIME
# define FRAME_SIZE 24
#else
# define FRAME_SIZE 32
#endif
	subl	$FRAME_SIZE, %esp
	cfi_adjust_cfa_offset(FRAME_SIZE)
	cfi_remember_state

	/* Get and store current wakeup_seq value.  */
	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx
	movl	broadcast_seq(%ebx), %eax
	movl	%edi, 4(%esp)
	movl	%edx, 8(%esp)
	movl	%eax, 12(%esp)

	/* Reset the pi-requeued flag.  */
	movl	$0, 16(%esp)

	cmpl	$0, (%ebp)
	movl	$-ETIMEDOUT, %esi
	js	6f

8:	movl	cond_futex(%ebx), %edi
	movl	%edi, 20(%esp)

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

	leal	(%ebp), %esi
#if FUTEX_PRIVATE_FLAG > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex(%ebx)
	sete	%cl
	je	40f

	movl	dep_mutex(%ebx), %edi
	/* Requeue to a non-robust PI mutex if the PI bit is set and
	   the robust bit is not set.  */
	movl	MUTEX_KIND(%edi), %eax
	andl	$(ROBUST_BIT|PI_BIT), %eax
	cmpl	$PI_BIT, %eax
	jne	40f

	movl	$(FUTEX_WAIT_REQUEUE_PI|FUTEX_PRIVATE_FLAG), %ecx
	/* The following only works like this because we only support
	   two clocks, represented using a single bit.  */
	testl	$1, cond_nwaiters(%ebx)
	/* XXX Need to implement using sete instead of a jump.  */
	jne	42f
	orl	$FUTEX_CLOCK_REALTIME, %ecx

42:	movl	20(%esp), %edx
	addl	$cond_futex, %ebx
.Ladd_cond_futex_pi:
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	subl	$cond_futex, %ebx
.Lsub_cond_futex_pi:
	movl	%eax, %esi
	/* Set the pi-requeued flag only if the kernel has returned 0. The
	   kernel does not hold the mutex on ETIMEDOUT or any other error.  */
	cmpl	$0, %eax
	sete	16(%esp)
	je	41f

	/* When a futex syscall with FUTEX_WAIT_REQUEUE_PI returns
	   successfully, it has already locked the mutex for us and the
	   pi_flag (16(%esp)) is set to denote that fact.  However, if another
	   thread changed the futex value before we entered the wait, the
	   syscall may return an EAGAIN and the mutex is not locked.  We go
	   ahead with a success anyway since later we look at the pi_flag to
	   decide if we got the mutex or not.  The sequence numbers then make
	   sure that only one of the threads actually wake up.  We retry using
	   normal FUTEX_WAIT only if the kernel returned ENOSYS, since normal
	   and PI futexes don't mix.

	   Note that we don't check for EAGAIN specifically; we assume that the
	   only other error the futex function could return is EAGAIN (barring
	   the ETIMEOUT of course, for the timeout case in futex) since
	   anything else would mean an error in our function.  It is too
	   expensive to do that check for every call (which is  quite common in
	   case of a large number of threads), so it has been skipped.  */
	cmpl	$-ENOSYS, %eax
	jne	41f
	xorl	%ecx, %ecx

40:	subl	$1, %ecx
	movl	$0, 16(%esp)
#ifdef __ASSUME_PRIVATE_FUTEX
	andl	$FUTEX_PRIVATE_FLAG, %ecx
#else
	andl	%gs:PRIVATE_FUTEX, %ecx
#endif
	addl	$FUTEX_WAIT_BITSET, %ecx
	/* The following only works like this because we only support
	   two clocks, represented using a single bit.  */
	testl	$1, cond_nwaiters(%ebx)
	jne	30f
	orl	$FUTEX_CLOCK_REALTIME, %ecx
30:
	movl	20(%esp), %edx
	movl	$0xffffffff, %ebp
	addl	$cond_futex, %ebx
.Ladd_cond_futex:
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	subl	$cond_futex, %ebx
.Lsub_cond_futex:
	movl	28+FRAME_SIZE(%esp), %ebp
	movl	%eax, %esi

41:	movl	(%esp), %eax
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
	cmpl	12(%esp), %eax
	jne	23f

	movl	woken_seq(%ebx), %eax
	movl	woken_seq+4(%ebx), %ecx

	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx

	cmpl	8(%esp), %edx
	jne	7f
	cmpl	4(%esp), %edi
	je	15f

7:	cmpl	%ecx, %edx
	jne	9f
	cmp	%eax, %edi
	jne	9f

15:	cmpl	$-ETIMEDOUT, %esi
	je	28f

	/* We need to go back to futex_wait.  If we're using requeue_pi, then
	   release the mutex we had acquired and go back.  */
	movl	16(%esp), %edx
	test	%edx, %edx
	jz	8b

	/* Adjust the mutex values first and then unlock it.  The unlock
	   should always succeed or else the kernel did not lock the mutex
	   correctly.  */
	movl	dep_mutex(%ebx), %eax
	call	__pthread_mutex_cond_lock_adjust
	movl	dep_mutex(%ebx), %eax
	xorl	%edx, %edx
	call	__pthread_mutex_unlock_usercnt
	jmp	8b

28:	addl	$1, wakeup_seq(%ebx)
	adcl	$0, wakeup_seq+4(%ebx)
	addl	$1, cond_futex(%ebx)
	movl	$ETIMEDOUT, %esi
	jmp	14f

23:	xorl	%esi, %esi
	jmp	24f

9:	xorl	%esi, %esi
14:	addl	$1, woken_seq(%ebx)
	adcl	$0, woken_seq+4(%ebx)

24:	subl	$(1 << nwaiters_shift), cond_nwaiters(%ebx)

	/* Wake up a thread which wants to destroy the condvar object.  */
	movl	total_seq(%ebx), %eax
	andl	total_seq+4(%ebx), %eax
	cmpl	$0xffffffff, %eax
	jne	25f
	movl	cond_nwaiters(%ebx), %eax
	andl	$~((1 << nwaiters_shift) - 1), %eax
	jne	25f

	addl	$cond_nwaiters, %ebx
	movl	$SYS_futex, %eax
#if FUTEX_PRIVATE_FLAG > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex-cond_nwaiters(%ebx)
	sete	%cl
	subl	$1, %ecx
#ifdef __ASSUME_PRIVATE_FUTEX
	andl	$FUTEX_PRIVATE_FLAG, %ecx
#else
	andl	%gs:PRIVATE_FUTEX, %ecx
#endif
	addl	$FUTEX_WAKE, %ecx
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

11:	movl	24+FRAME_SIZE(%esp), %eax
	/* With requeue_pi, the mutex lock is held in the kernel.  */
	movl	16(%esp), %ecx
	testl	%ecx, %ecx
	jnz	27f

	call	__pthread_mutex_cond_lock
26:	addl	$FRAME_SIZE, %esp
	cfi_adjust_cfa_offset(-FRAME_SIZE)

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
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebx)
	popl	%esi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%esi)
	popl	%edi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%edi)
	popl	%ebp
	cfi_adjust_cfa_offset(-4)
	cfi_restore(%ebp)

	ret

	cfi_restore_state

27:	call	__pthread_mutex_cond_lock_adjust
	xorl	%eax, %eax
	jmp	26b

	cfi_adjust_cfa_offset(-FRAME_SIZE);
	/* Initial locking failed.  */
1:
#if cond_lock == 0
	movl	%ebx, %edx
#else
	leal	cond_lock(%ebx), %edx
#endif
#if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
#if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
#endif
	call	__lll_lock_wait
	jmp	2b

	/* The initial unlocking of the mutex failed.  */
16:
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
#if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
#if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
#endif
	call	__lll_unlock_wake

	movl	%esi, %eax
	jmp	18b

	cfi_adjust_cfa_offset(FRAME_SIZE)

	/* Unlock in loop requires wakeup.  */
3:
#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
#if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
#if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
#endif
	call	__lll_unlock_wake
	jmp	4b

	/* Locking in loop failed.  */
5:
#if cond_lock == 0
	movl	%ebx, %edx
#else
	leal	cond_lock(%ebx), %edx
#endif
#if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
#if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
#endif
	call	__lll_lock_wait
	jmp	6b

	/* Unlock after loop requires wakeup.  */
10:
#if cond_lock == 0
	movl	%ebx, %eax
#else
	leal	cond_lock(%ebx), %eax
#endif
#if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
#if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
#endif
	call	__lll_unlock_wake
	jmp	11b

#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
	cfi_adjust_cfa_offset(-FRAME_SIZE)
.Lreltmo:
	/* Get internal lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
# if cond_lock == 0
	cmpxchgl %edx, (%ebx)
# else
	cmpxchgl %edx, cond_lock(%ebx)
# endif
	jnz	101f

	/* Store the reference to the mutex.  If there is already a
	   different value in there this is a bad user bug.  */
102:	cmpl	$-1, dep_mutex(%ebx)
	movl	24(%esp), %eax
	je	117f
	movl	%eax, dep_mutex(%ebx)

	/* Unlock the mutex.  */
117:	xorl	%edx, %edx
	call	__pthread_mutex_unlock_usercnt

	testl	%eax, %eax
	jne	16b

	addl	$1, total_seq(%ebx)
	adcl	$0, total_seq+4(%ebx)
	addl	$1, cond_futex(%ebx)
	addl	$(1 << nwaiters_shift), cond_nwaiters(%ebx)

	subl	$FRAME_SIZE, %esp
	cfi_adjust_cfa_offset(FRAME_SIZE)

	/* Get and store current wakeup_seq value.  */
	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx
	movl	broadcast_seq(%ebx), %eax
	movl	%edi, 4(%esp)
	movl	%edx, 8(%esp)
	movl	%eax, 12(%esp)

	/* Reset the pi-requeued flag.  */
	movl	$0, 16(%esp)

	/* Get the current time.  */
108:	movl	%ebx, %edx
# ifdef __NR_clock_gettime
	/* Get the clock number.  */
	movl	cond_nwaiters(%ebx), %ebx
	andl	$((1 << nwaiters_shift) - 1), %ebx
	/* Only clocks 0 and 1 are allowed so far.  Both are handled in the
	   kernel.  */
	leal	24(%esp), %ecx
	movl	$__NR_clock_gettime, %eax
	ENTER_KERNEL
	movl	%edx, %ebx

	/* Compute relative timeout.  */
	movl	(%ebp), %ecx
	movl	4(%ebp), %edx
	subl	24(%esp), %ecx
	subl	28(%esp), %edx
# else
	/* Get the current time.  */
	leal	24(%esp), %ebx
	xorl	%ecx, %ecx
	movl	$__NR_gettimeofday, %eax
	ENTER_KERNEL
	movl	%edx, %ebx

	/* Compute relative timeout.  */
	movl	28(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%ebp), %ecx
	movl	4(%ebp), %edx
	subl	24(%esp), %ecx
	subl	%eax, %edx
# endif
	jns	112f
	addl	$1000000000, %edx
	subl	$1, %ecx
112:	testl	%ecx, %ecx
	movl	$-ETIMEDOUT, %esi
	js	106f

	/* Store relative timeout.  */
121:	movl	%ecx, 24(%esp)
	movl	%edx, 28(%esp)

	movl	cond_futex(%ebx), %edi
	movl	%edi, 20(%esp)

	/* Unlock.  */
	LOCK
# if cond_lock == 0
	subl	$1, (%ebx)
# else
	subl	$1, cond_lock(%ebx)
# endif
	jne	103f

.LcleanupSTART2:
104:	call	__pthread_enable_asynccancel
	movl	%eax, (%esp)

	leal	24(%esp), %esi
# if FUTEX_PRIVATE_FLAG > 255
	xorl	%ecx, %ecx
# endif
	cmpl	$-1, dep_mutex(%ebx)
	sete	%cl
	subl	$1, %ecx
# ifdef __ASSUME_PRIVATE_FUTEX
	andl	$FUTEX_PRIVATE_FLAG, %ecx
# else
	andl	%gs:PRIVATE_FUTEX, %ecx
# endif
# if FUTEX_WAIT != 0
	addl	$FUTEX_WAIT, %ecx
# endif
	movl	20(%esp), %edx
	addl	$cond_futex, %ebx
.Ladd_cond_futex2:
	movl	$SYS_futex, %eax
	ENTER_KERNEL
	subl	$cond_futex, %ebx
.Lsub_cond_futex2:
	movl	%eax, %esi

141:	movl	(%esp), %eax
	call	__pthread_disable_asynccancel
.LcleanupEND2:


	/* Lock.  */
	movl	$1, %edx
	xorl	%eax, %eax
	LOCK
# if cond_lock == 0
	cmpxchgl %edx, (%ebx)
# else
	cmpxchgl %edx, cond_lock(%ebx)
# endif
	jnz	105f

106:	movl	broadcast_seq(%ebx), %eax
	cmpl	12(%esp), %eax
	jne	23b

	movl	woken_seq(%ebx), %eax
	movl	woken_seq+4(%ebx), %ecx

	movl	wakeup_seq(%ebx), %edi
	movl	wakeup_seq+4(%ebx), %edx

	cmpl	8(%esp), %edx
	jne	107f
	cmpl	4(%esp), %edi
	je	115f

107:	cmpl	%ecx, %edx
	jne	9b
	cmp	%eax, %edi
	jne	9b

115:	cmpl	$-ETIMEDOUT, %esi
	je	28b

	jmp	8b

	cfi_adjust_cfa_offset(-FRAME_SIZE)
	/* Initial locking failed.  */
101:
# if cond_lock == 0
	movl	%ebx, %edx
# else
	leal	cond_lock(%ebx), %edx
# endif
# if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
# endif
	cmpl	$-1, dep_mutex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
# if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
# endif
	call	__lll_lock_wait
	jmp	102b

	cfi_adjust_cfa_offset(FRAME_SIZE)

	/* Unlock in loop requires wakeup.  */
103:
# if cond_lock == 0
	movl	%ebx, %eax
# else
	leal	cond_lock(%ebx), %eax
# endif
# if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
# endif
	cmpl	$-1, dep_mutex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
# if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
# endif
	call	__lll_unlock_wake
	jmp	104b

	/* Locking in loop failed.  */
105:
# if cond_lock == 0
	movl	%ebx, %edx
# else
	leal	cond_lock(%ebx), %edx
# endif
# if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
# endif
	cmpl	$-1, dep_mutex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
# if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
# endif
	call	__lll_lock_wait
	jmp	106b
#endif

	.size	__pthread_cond_timedwait, .-__pthread_cond_timedwait
versioned_symbol (libpthread, __pthread_cond_timedwait, pthread_cond_timedwait,
		  GLIBC_2_3_2)


	.type	__condvar_tw_cleanup2, @function
__condvar_tw_cleanup2:
	subl	$cond_futex, %ebx
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
	movl	%ebx, %edx
#else
	leal	cond_lock(%ebx), %edx
#endif
#if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
#if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
#endif
	call	__lll_lock_wait

1:	movl	broadcast_seq(%ebx), %eax
	cmpl	12(%esp), %eax
	jne	3f

	/* We increment the wakeup_seq counter only if it is lower than
	   total_seq.  If this is not the case the thread was woken and
	   then canceled.  In this case we ignore the signal.  */
	movl	total_seq(%ebx), %eax
	movl	total_seq+4(%ebx), %edi
	cmpl	wakeup_seq+4(%ebx), %edi
	jb	6f
	ja	7f
	cmpl	wakeup_seq(%ebx), %eax
	jbe	7f

6:	addl	$1, wakeup_seq(%ebx)
	adcl	$0, wakeup_seq+4(%ebx)
	addl	$1, cond_futex(%ebx)

7:	addl	$1, woken_seq(%ebx)
	adcl	$0, woken_seq+4(%ebx)

3:	subl	$(1 << nwaiters_shift), cond_nwaiters(%ebx)

	/* Wake up a thread which wants to destroy the condvar object.  */
	xorl	%edi, %edi
	movl	total_seq(%ebx), %eax
	andl	total_seq+4(%ebx), %eax
	cmpl	$0xffffffff, %eax
	jne	4f
	movl	cond_nwaiters(%ebx), %eax
	andl	$~((1 << nwaiters_shift) - 1), %eax
	jne	4f

	addl	$cond_nwaiters, %ebx
	movl	$SYS_futex, %eax
#if FUTEX_PRIVATE_FLAG > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex-cond_nwaiters(%ebx)
	sete	%cl
	subl	$1, %ecx
#ifdef __ASSUME_PRIVATE_FUTEX
	andl	$FUTEX_PRIVATE_FLAG, %ecx
#else
	andl	%gs:PRIVATE_FUTEX, %ecx
#endif
	addl	$FUTEX_WAKE, %ecx
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
#if (LLL_SHARED-LLL_PRIVATE) > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex(%ebx)
	setne	%cl
	subl	$1, %ecx
	andl	$(LLL_SHARED-LLL_PRIVATE), %ecx
#if LLL_PRIVATE != 0
	addl	$LLL_PRIVATE, %ecx
#endif
	call	__lll_unlock_wake

	/* Wake up all waiters to make sure no signal gets lost.  */
2:	testl	%edi, %edi
	jnz	5f
	addl	$cond_futex, %ebx
#if FUTEX_PRIVATE_FLAG > 255
	xorl	%ecx, %ecx
#endif
	cmpl	$-1, dep_mutex-cond_futex(%ebx)
	sete	%cl
	subl	$1, %ecx
#ifdef __ASSUME_PRIVATE_FUTEX
	andl	$FUTEX_PRIVATE_FLAG, %ecx
#else
	andl	%gs:PRIVATE_FUTEX, %ecx
#endif
	addl	$FUTEX_WAKE, %ecx
	movl	$SYS_futex, %eax
	movl	$0x7fffffff, %edx
	ENTER_KERNEL

	/* Lock the mutex only if we don't own it already.  This only happens
	   in case of PI mutexes, if we got cancelled after a successful
	   return of the futex syscall and before disabling async
	   cancellation.  */
5:	movl	24+FRAME_SIZE(%esp), %eax
	movl	MUTEX_KIND(%eax), %ebx
	andl	$(ROBUST_BIT|PI_BIT), %ebx
	cmpl	$PI_BIT, %ebx
	jne	8f

	movl	(%eax), %ebx
	andl	$TID_MASK, %ebx
	cmpl	%ebx, %gs:TID
	jne	8f
	/* We managed to get the lock.  Fix it up before returning.  */
	call	__pthread_mutex_cond_lock_adjust
	jmp	9f

8:	call	__pthread_mutex_cond_lock

9:	movl	%esi, (%esp)
.LcallUR:
	call	_Unwind_Resume
	hlt
.LENDCODE:
	cfi_endproc
	.size	__condvar_tw_cleanup, .-__condvar_tw_cleanup


	.section .gcc_except_table,"a",@progbits
.LexceptSTART:
	.byte	DW_EH_PE_omit			# @LPStart format (omit)
	.byte	DW_EH_PE_omit			# @TType format (omit)
	.byte	DW_EH_PE_sdata4			# call-site format
						# DW_EH_PE_sdata4
	.uleb128 .Lcstend-.Lcstbegin
.Lcstbegin:
	.long	.LcleanupSTART-.LSTARTCODE
	.long	.Ladd_cond_futex_pi-.LcleanupSTART
	.long	__condvar_tw_cleanup-.LSTARTCODE
	.uleb128  0
	.long	.Ladd_cond_futex_pi-.LSTARTCODE
	.long	.Lsub_cond_futex_pi-.Ladd_cond_futex_pi
	.long	__condvar_tw_cleanup2-.LSTARTCODE
	.uleb128  0
	.long	.Lsub_cond_futex_pi-.LSTARTCODE
	.long	.Ladd_cond_futex-.Lsub_cond_futex_pi
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
#ifndef __ASSUME_FUTEX_CLOCK_REALTIME
	.long	.LcleanupSTART2-.LSTARTCODE
	.long	.Ladd_cond_futex2-.LcleanupSTART2
	.long	__condvar_tw_cleanup-.LSTARTCODE
	.uleb128  0
	.long	.Ladd_cond_futex2-.LSTARTCODE
	.long	.Lsub_cond_futex2-.Ladd_cond_futex2
	.long	__condvar_tw_cleanup2-.LSTARTCODE
	.uleb128  0
	.long	.Lsub_cond_futex2-.LSTARTCODE
	.long	.LcleanupEND2-.Lsub_cond_futex2
	.long	__condvar_tw_cleanup-.LSTARTCODE
	.uleb128  0
#endif
	.long	.LcallUR-.LSTARTCODE
	.long	.LENDCODE-.LcallUR
	.long	0
	.uleb128  0
.Lcstend:


#ifdef SHARED
	.hidden DW.ref.__gcc_personality_v0
	.weak	DW.ref.__gcc_personality_v0
	.section .gnu.linkonce.d.DW.ref.__gcc_personality_v0,"aw",@progbits
	.align	4
	.type	DW.ref.__gcc_personality_v0, @object
	.size	DW.ref.__gcc_personality_v0, 4
DW.ref.__gcc_personality_v0:
	.long   __gcc_personality_v0
#endif
