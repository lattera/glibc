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

	.text

#ifdef UP
# define LOCK
#else
# define LOCK lock
#endif

#define SYS_gettimeofday	__NR_gettimeofday
#define SYS_futex		240
#define FUTEX_WAIT		0
#define FUTEX_WAKE		1

#define EWOULDBLOCK		11
#define EINVAL			22
#define ETIMEDOUT		110

#define cond_lock	 0
#define cond_nr_wakers	 4
#define cond_nr_sleepers 8


	.global	__lll_cond_wait
	.type	__lll_cond_wait,@function
	.hidden	__lll_cond_wait
	.align	16
__lll_cond_wait:
	pushl	%esi
	pushl	%ebx

	xorl	%esi, %esi

	leal	cond_nr_wakers(%eax), %ebx

4:	movl	(%ebx), %edx
	testl	%edx, %edx
	jne	1f

	LOCK
	decl	cond_lock-cond_nr_wakers(%ebx)
	jne	2f

3:	xorl	%ecx, %ecx
	movl	$SYS_futex, %eax
	int	$0x80

	movl	$1, %eax
	LOCK
	xaddl	%eax, cond_lock-cond_nr_wakers(%ebx)
	testl	%eax, %eax
	je	4b

	leal	cond_lock-cond_nr_wakers(%ebx), %ecx
	/* Preserves %ebx, %edx, %edi, %esi.  */
	call	__lll_mutex_lock_wait
	jmp	4b

1:	decl	(%ebx)

	popl	%ebx
	popl	%esi
	ret

2:	leal	cond_lock-cond_nr_wakers(%ebx), %eax
	/* Preserves %ebx, %ecx, %edx, %edi, %esi.  */
	call	__lll_mutex_unlock_wake
	jmp	3b
	.size	__lll_cond_wait,.-__lll_cond_wait


	.global	__lll_cond_timedwait
	.type	__lll_cond_timedwait,@function
	.hidden	__lll_cond_timedwait
	.align	16
__lll_cond_timedwait:
	/* Check for a valid timeout value.  */
	cmpl	$1000000000, 4(%edx)
	jae	1f

	pushl	%ebp
	pushl	%edi
	pushl	%esi
	pushl	%ebx

	/* Stack frame for the timespec and timeval structs.  */
	subl	$8, %esp

	leal	cond_nr_wakers(%eax), %ebp	/* cond */
	movl	%edx, %edi			/* timeout */

9:	movl	(%ebp), %esi
	testl	%esi, %esi
	jne	5f

	LOCK
	decl	cond_lock-cond_nr_wakers(%ebp)
	jne	6f

	/* Get current time.  */
7:	movl	%esp, %ebx
	xorl	%ecx, %ecx
	movl	$SYS_gettimeofday, %eax
	int	$0x80

	/* Compute relative timeout.  */
	movl	4(%esp), %eax
	movl	$1000, %edx
	mul	%edx		/* Milli seconds to nano seconds.  */
	movl	(%edi), %ecx
	movl	4(%edi), %edx
	subl	(%esp), %ecx
	subl	%eax, %edx
	jns	3f
	addl	$1000000000, %edx
	decl	%ecx
3:	testl	%ecx, %ecx
	js	4f		/* Time is already up.  */

	movl	%ecx, (%esp)	/* Store relative timeout.  */
	movl	%edx, 4(%esp)
	movl	%esi, %edx
	movl	%esp, %esi
	xorl	%ecx, %ecx	/* movl $FUTEX_WAIT, %ecx */
	movl	%ebp, %ebx
	movl	$SYS_futex, %eax
	int	$0x80

	movl	%eax, %edx

	movl	$1, %eax
	LOCK
	xaddl	%eax, cond_lock-cond_nr_wakers(%ebp)
	testl	%eax, %eax
	jne	8f

	cmpl	$-ETIMEDOUT, %edx
	jne	9b

4:	movl	$ETIMEDOUT, %eax
	jmp	2f

5:	decl	(%ebp)
	xorl	%eax, %eax

2:	addl	$8, %esp
	popl	%ebx
	popl	%esi
	popl	%edi
	popl	%ebp
	ret

6:	leal	cond_lock-cond_nr_wakers(%ebp), %eax
	/* Preserves %ebx, %ecx, %edx, %edi, %esi.  */
	call	__lll_mutex_unlock_wake
	jmp	7b

8:	leal	cond_lock-cond_nr_wakers(%ebp), %ecx
	/* Preserves %ebx, %edx, %edi, %esi.  */
	call	__lll_mutex_lock_wait
	jmp	5b

1:	movl	$EINVAL, %eax
	ret
	.size	__lll_cond_timedwait,.-__lll_cond_timedwait


	.global	__lll_cond_wake
	.type	__lll_cond_wake,@function
	.hidden	__lll_cond_wake
	.align	16
__lll_cond_wake:
	pushl	%esi
	pushl	%ebx

	movl	%eax, %ebx

	movl	$1, %eax
	LOCK
	xaddl	%eax, (%ebx)
	testl	%eax, %eax
	jne	1f

2:	leal	cond_nr_wakers(%ebx), %ebx
	cmpl	$0, cond_nr_sleepers-cond_nr_wakers(%ebx)
	je	3f

	incl	(%ebx)
	jz	5f

6:	movl	$FUTEX_WAKE, %ecx
	xorl	%esi, %esi
	movl	%ecx, %edx	/* movl $1, %edx */
	movl	$SYS_futex, %eax
	int	$0x80

3:	LOCK
	decl	cond_lock-cond_nr_wakers(%ebx)
	je,pt	4f

	leal	cond_lock-cond_nr_wakers(%ebx), %eax
	call	__lll_mutex_unlock_wake

4:	popl	%ebx
	popl	%esi
	ret

1:	movl	%ebx, %ecx
	call	__lll_mutex_lock_wait
	jmp	2b

5:	movl	$0x80000000, (%ebx)
	jmp	6b
	.size	__lll_cond_wake,.-__lll_cond_wake


	.global	__lll_cond_broadcast
	.type	__lll_cond_broadcast,@function
	.hidden	__lll_cond_broadcast
	.align	16
__lll_cond_broadcast:
	pushl	%esi
	pushl	%ebx

	movl	%eax, %ebx
	movl	$0x8000000, %edx

	movl	$1, %eax
	LOCK
	xaddl	%eax, (%ebx)
	testl	%eax, %eax
	jne	1f

2:	leal	cond_nr_wakers(%ebx), %ebx
	cmpl	$0, cond_nr_sleepers-cond_nr_wakers(%ebx)
	je	3f

	orl	%edx, (%ebx)

6:	movl	$FUTEX_WAKE, %ecx
	xorl	%esi, %esi
	movl	$SYS_futex, %eax
	int	$0x80

3:	LOCK
	decl	cond_lock-cond_nr_wakers(%ebx)
	je,pt	4f

	leal	cond_lock-cond_nr_wakers(%ebx), %eax
	call	__lll_mutex_unlock_wake

4:	popl	%ebx
	popl	%esi
	ret

1:	movl	%ebx, %ecx
	call	__lll_mutex_lock_wait
	jmp	2b
	.size	__lll_cond_broadcast,.-__lll_cond_broadcast
