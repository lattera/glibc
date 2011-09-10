/* Copyright (C) 2002, 2003, 2005, 2007, 2011 Free Software Foundation, Inc.
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
#include <lowlevellock.h>

	.text

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
	jne	2b
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
#ifdef NO_TLS_DIRECT_SEG_REFS
	movl	errno@gotntpoff(%ecx), %edx
	addl	%gs:0, %edx
	movl	$EAGAIN, (%edx)
#else
	movl	errno@gotntpoff(%ecx), %edx
	movl	$EAGAIN, %gs:(%edx)
#endif
	orl	$-1, %eax
	ret
	.size	__new_sem_trywait,.-__new_sem_trywait
	versioned_symbol(libpthread, __new_sem_trywait, sem_trywait, GLIBC_2_1)
#if SHLIB_COMPAT(libpthread, GLIBC_2_0, GLIBC_2_1)
	.global	__old_sem_trywait
__old_sem_trywait = __new_sem_trywait
	compat_symbol(libpthread, __old_sem_trywait, sem_trywait, GLIBC_2_0)
#endif


#ifdef PIC
	.section .gnu.linkonce.t.__i686.get_pc_thunk.cx,"ax",@progbits
	.globl	__i686.get_pc_thunk.cx
	.hidden	__i686.get_pc_thunk.cx
	.type	__i686.get_pc_thunk.cx,@function
__i686.get_pc_thunk.cx:
	movl (%esp), %ecx;
	ret
	.size	__i686.get_pc_thunk.cx,.-__i686.get_pc_thunk.cx
#endif
