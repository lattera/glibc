/* Copyright (C) 2002 Free Software Foundation, Inc.
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

	.text

#ifndef UP
# define LOCK lock
#else
# define
#endif

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAKE		1

#define EINTR			4
#define EAGAIN			11
#define EWOULDBLOCK		EAGAIN
#define EINVAL			22
#define ETIMEDOUT		110


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
	movl	%gs:0, %edx
	subl	errno@gottpoff(%ebx), %edx
	movl	%eax, (%edx)
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


	.globl	__new_sem_trywait
	.type	__new_sem_trywait,@function
	.align	16
__new_sem_trywait:
	movl	4(%esp), %ecx

	movl	(%ecx), %eax
2:	testl	%eax, %eax
	jz	1f

	leal	-1(%eax), %edx
	LOCK
	cmpxchgl %edx, (%ecx)
	jne,pn	2b
	xorl	%eax, %eax
	ret

1:
#ifdef PIC
	call	__i686.get_pc_thunk.cx
#else
	movl	$3f, %ecx
3:
#endif
	addl	$_GLOBAL_OFFSET_TABLE_, %ecx
	movl	%gs:0, %edx
	subl	errno@gottpoff(%ecx), %edx
	movl	$EAGAIN, (%edx)
	orl	$-1, %eax
	ret
	.size	__new_sem_trywait,.-__new_sem_trywait
	.symver	__new_sem_trywait, sem_trywait@@GLIBC_2.1
#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
	.global	__old_sem_trywait
__old_sem_trywait = __new_sem_trywait
	.symver	__old_sem_trywait, sem_trywait@GLIBC_2.0
#endif


	.globl	sem_timedwait
	.type	sem_timedwait,@function
	.align	16
sem_timedwait:
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
	pushl	%edi
	pushl	%ebx
	subl	$8, %esp

	movl	%esp, %esi
	movl	28(%esp), %edi

	/* Check for invalid nanosecond field.  */
	cmpl	$1000000000, 4(%edi)
	movl	$EINVAL, %eax
	jae	6f

7:	xorl	%ecx, %ecx
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
	decl	%ecx
5:	testl	%ecx, %ecx
	movl	$ETIMEDOUT, %eax
	js	6f		/* Time is already up.  */

	movl	%ecx, (%esp)	/* Store relative timeout.  */
	movl	%edx, 4(%esp)
	movl	24(%esp), %ebx
	xorl	%ecx, %ecx
	movl	$SYS_futex, %eax
	xorl	%edx, %edx
	ENTER_KERNEL

	testl	%eax, %eax
	je,pt	9f
	cmpl	$-EWOULDBLOCK, %eax
	jne	3f

9:	movl	(%ebx), %eax
8:	testl	%eax, %eax
	je	7b

	leal	-1(%eax), %ecx
	LOCK
	cmpxchgl %ecx, (%ebx)
	jne,pn	8b

	addl	$8, %esp
	xorl	%eax, %eax
	popl	%ebx
	popl	%edi
	popl	%esi
	ret

3:	negl	%eax
6:
#ifdef PIC
	call	__i686.get_pc_thunk.bx
#else
	movl	$4f, %ebx
4:
#endif
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	%gs:0, %edx
	subl	errno@gottpoff(%ebx), %edx
	movl	%eax, (%edx)

	addl	$8, %esp
	orl	$-1, %eax
	popl	%ebx
	popl	%edi
	popl	%esi
	ret
	.size	sem_timedwait,.-sem_timedwait


	.globl	__new_sem_post
	.type	__new_sem_post,@function
	.align	16
__new_sem_post:
	pushl	%esi
	pushl	%ebx

	movl	12(%esp), %ebx
	movl	$1, %edx
	LOCK
	xaddl	%edx, (%ebx)

	xorl	%esi, %esi
	movl	$SYS_futex, %eax
	movl	$FUTEX_WAKE, %ecx
	incl	%edx
	ENTER_KERNEL

	testl	%eax, %eax
	js	1f

	popl	%ebx
	popl	%esi
	ret

1:
#ifdef PIC
	call	__i686.get_pc_thunk.bx
#else
	movl	$4f, %ebx
4:
#endif
	addl	$_GLOBAL_OFFSET_TABLE_, %ebx
	movl	%gs:0, %edx
	subl	errno@gottpoff(%ebx), %edx
	movl	$EINVAL, (%edx)

	orl	$-1, %eax
	popl	%ebx
	popl	%esi
	ret
	.size	__new_sem_post,.-__new_sem_post
	.symver	__new_sem_post, sem_post@@GLIBC_2.1
#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
	.global	__old_sem_post
__old_sem_post = __new_sem_post
	.symver	__old_sem_post, sem_post@GLIBC_2.0
#endif


#ifdef PIC
	.section .gnu.linkonce.t.__i686.get_pc_thunk.bx,"ax",@progbits
	.globl	__i686.get_pc_thunk.bx
	.hidden	__i686.get_pc_thunk.bx
	.type	__i686.get_pc_thunk.bx,@function
__i686.get_pc_thunk.bx:
	movl (%esp), %ebx;
	ret
	.size	__i686.get_pc_thunk.bx,.-__i686.get_pc_thunk.bx


	.section .gnu.linkonce.t.__i686.get_pc_thunk.cx,"ax",@progbits
	.globl	__i686.get_pc_thunk.cx
	.hidden	__i686.get_pc_thunk.cx
	.type	__i686.get_pc_thunk.cx,@function
__i686.get_pc_thunk.cx:
	movl (%esp), %ecx;
	ret
	.size	__i686.get_pc_thunk.cx,.-__i686.get_pc_thunk.cx
#endif
