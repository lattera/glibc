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

#ifndef UP
# define LOCK lock
#else
# define
#endif

#define SYS_futex		240
#define FUTEX_WAKE		1

#define EWOULDBLOCK		11


	.text

	.globl	__new_sem_wait
	.type	__new_sem_wait,@function
	.align	16
__new_sem_wait:
	pushl	%ebx
	pushl	%esi

	movl	12(%esp), %ebx

3:	movl	(%ebx), %eax
2:	testl	%eax, %eax
	je,pn	1f

	leal	-1(%eax), %edx
	LOCK
	cmpxchgl %edx, (%ebx)
	jne,pn	2b
	xorl	%eax, %eax

	popl	%esi
	popl	%ebx
	ret

1:	xorl	%esi, %esi
	movl	$SYS_futex, %eax
	movl	%esi, %ecx
	movl	%esi, %edx
	ENTER_KERNEL

	testl	%eax, %eax
	je	3b
	cmpl	$-EWOULDBLOCK, %eax
	je	3b
	negl	%eax
#ifdef PIC
	call	__i686.get_pc_thunk.bx
#else
	movl	$4f, %ebx
4:
#endif
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
#if USE___THREAD
	movl	%gs:0, %edx
	subl	errno@gottpoff(%ebx), %edx
	movl	%eax, (%edx)
#else
	movl	%eax, %edx
	call	__errno_location@plt
	movl	%edx, (%eax)
#endif
	orl	$-1, %eax
	popl	%esi
	popl	%ebx
	ret
	.size	__new_sem_wait,.-__new_sem_wait
	.symver	__new_sem_wait, sem_wait@@GLIBC_2.1
#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
	.global	__old_sem_wait
__old_sem_wait = __new_sem_wait
	.symver	__old_sem_wait, sem_wait@GLIBC_2.0
#endif
