/* Copyright (C) 2002, 2003, 2004, 2005 Free Software Foundation, Inc.
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
#include <pthread-errnos.h>

#ifndef UP
# define LOCK lock
#else
# define
#endif

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAKE		1


	.text

	.globl	sem_timedwait
	.type	sem_timedwait,@function
	.align	16
	cfi_startproc
sem_timedwait:
	/* First check for cancellation.  */
	movl	%gs:CANCELHANDLING, %eax
	andl	$0xfffffff9, %eax
	cmpl	$8, %eax
	je	10f

	movl	4(%esp), %ecx

	movl	(%ecx), %eax
2:	testl	%eax, %eax
	je,pn	1f

	leal	-1(%eax), %edx
	LOCK
	cmpxchgl %edx, (%ecx)
	jne,pn	2b

	xorl	%eax, %eax
	ret

	/* Check whether the timeout value is valid.  */
1:	pushl	%esi
	cfi_adjust_cfa_offset(4)
	pushl	%edi
	cfi_adjust_cfa_offset(4)
	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	subl	$12, %esp
	cfi_adjust_cfa_offset(12)

	movl	32(%esp), %edi
	cfi_offset(7, -12)		/* %edi */

	/* Check for invalid nanosecond field.  */
	cmpl	$1000000000, 4(%edi)
	movl	$EINVAL, %esi
	cfi_offset(6, -8)		/* %esi */
	jae	6f

	cfi_offset(3, -16)		/* %ebx */
7:	call	__pthread_enable_asynccancel
	movl	%eax, 8(%esp)

	xorl	%ecx, %ecx
	movl	%esp, %ebx
	movl	%ecx, %edx
	movl	$SYS_gettimeofday, %eax
	ENTER_KERNEL

	/* Compute relative timeout.  */
	movl	4(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%edi), %ecx
	movl	4(%edi), %edx
	subl	(%esp), %ecx
	subl	%eax, %edx
	jns	5f
	addl	$1000000000, %edx
	subl	$1, %ecx
5:	testl	%ecx, %ecx
	movl	$ETIMEDOUT, %esi
	js	6f		/* Time is already up.  */

	movl	%ecx, (%esp)	/* Store relative timeout.  */
	movl	%edx, 4(%esp)
	movl	28(%esp), %ebx
	xorl	%ecx, %ecx
	movl	%esp, %esi
	movl	$SYS_futex, %eax
	xorl	%edx, %edx
	ENTER_KERNEL
	movl	%eax, %esi

	movl	8(%esp), %eax
	call	__pthread_disable_asynccancel

	testl	%esi, %esi
	je,pt	9f
	cmpl	$-EWOULDBLOCK, %esi
	jne	3f

9:	movl	(%ebx), %eax
8:	testl	%eax, %eax
	je	7b

	leal	-1(%eax), %ecx
	LOCK
	cmpxchgl %ecx, (%ebx)
	jne,pn	8b

	addl	$12, %esp
	cfi_adjust_cfa_offset(-12)
	xorl	%eax, %eax
	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(3)
	popl	%edi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(7)
	popl	%esi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(6)
	ret

	cfi_adjust_cfa_offset(24)
	cfi_offset(6, -8)		/* %esi */
	cfi_offset(7, -12)	 	/* %edi */
	cfi_offset(3, -16)		/* %ebx */
3:	negl	%esi
6:
#ifdef PIC
	call	__i686.get_pc_thunk.bx
#else
	movl	$4f, %ebx
4:
#endif
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
#if USE___THREAD
# ifdef NO_TLS_DIRECT_SEG_REFS
	movl	errno@gotntpoff(%ebx), %edx
	addl	%gs:0, %edx
	movl	%esi, (%edx)
# else
	movl	errno@gotntpoff(%ebx), %edx
	movl	%esi, %gs:(%edx)
# endif
#else
	call	__errno_location@plt
	movl	%esi, (%eax)
#endif

	addl	$12, %esp
	cfi_adjust_cfa_offset(-12)
	orl	$-1, %eax
	popl	%ebx
	cfi_adjust_cfa_offset(-4)
	cfi_restore(3)
	popl	%edi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(7)
	popl	%esi
	cfi_adjust_cfa_offset(-4)
	cfi_restore(6)
	ret

10:	/* Canceled.  */
	movl	$0xffffffff, %gs:RESULT
	LOCK
	orl	$0x10, %gs:CANCELHANDLING
	movl	%gs:CLEANUP_JMP_BUF, %eax
	jmp	HIDDEN_JUMPTARGET (__pthread_unwind)
	cfi_endproc
	.size	sem_timedwait,.-sem_timedwait
