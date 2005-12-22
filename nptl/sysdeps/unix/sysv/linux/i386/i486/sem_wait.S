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

#include <sysdep.h>
#include <shlib-compat.h>
#include <pthread-errnos.h>

#ifndef UP
# define LOCK lock
#else
# define
#endif

#define SYS_futex		240
#define FUTEX_WAKE		1


	.text

	.globl	__new_sem_wait
	.type	__new_sem_wait,@function
	.align	16
	cfi_startproc
__new_sem_wait:
	/* First check for cancellation.  */
	movl	%gs:CANCELHANDLING, %eax
	andl	$0xfffffff9, %eax
	cmpl	$8, %eax
	je	5f

	pushl	%ebx
	cfi_adjust_cfa_offset(4)
	pushl	%esi
	cfi_adjust_cfa_offset(4)
	subl	$4, %esp
	cfi_adjust_cfa_offset(4)

	movl	16(%esp), %ebx
	cfi_offset(3, -8)		/* %ebx */

	cfi_offset(6, -12)		/* %esi */
3:	movl	(%ebx), %eax
2:	testl	%eax, %eax
	je	1f

	leal	-1(%eax), %edx
	LOCK
	cmpxchgl %edx, (%ebx)
	jne,pn	2b
	xorl	%eax, %eax

	movl	4(%esp), %esi
	cfi_restore(6)
	movl	8(%esp), %ebx
	cfi_restore(3)
	addl	$12, %esp
	cfi_adjust_cfa_offset(-12)
	ret

	cfi_adjust_cfa_offset(12)
	cfi_offset(3, -8)		/* %ebx */
	cfi_offset(6, -12)		/* %esi */
1:	call	__pthread_enable_asynccancel
	movl	%eax, (%esp)

	xorl	%esi, %esi
	movl	$SYS_futex, %eax
	movl	%esi, %ecx
	movl	%esi, %edx
	ENTER_KERNEL
	movl	%eax, %esi

	movl	(%esp), %eax
	call	__pthread_disable_asynccancel

	testl	%esi, %esi
	je	3b
	cmpl	$-EWOULDBLOCK, %esi
	je	3b
	negl	%esi
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
	orl	$-1, %eax
	movl	4(%esp), %esi
	cfi_restore(6)
	movl	8(%esp), %ebx
	cfi_restore(3)
	addl	$12, %esp
	cfi_adjust_cfa_offset(-12)
	ret

5:	/* Canceled.  */
	movl	$0xffffffff, %gs:RESULT
	LOCK
	orl	$0x10, %gs:CANCELHANDLING
	movl	%gs:CLEANUP_JMP_BUF, %eax
	jmp	HIDDEN_JUMPTARGET (__pthread_unwind)
	cfi_endproc
	.size	__new_sem_wait,.-__new_sem_wait
	versioned_symbol(libpthread, __new_sem_wait, sem_wait, GLIBC_2_1)
#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
	.global	__old_sem_wait
__old_sem_wait = __new_sem_wait
	compat_symbol(libpthread, __old_sem_wait, sem_wait, GLIBC_2_0)
#endif
