/* Copyright (C) 1995 Free Software Foundation, Inc.
This file is part of the GNU C Library.

The GNU C Library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

The GNU C Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with the GNU C Library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.  */

#include <sysdep.h>

.data
.globl C_SYMBOL_NAME(__curbrk)

.text
.globl C_SYMBOL_NAME(syscall_error)

ENTRY (__sbrk)

	movl %ebx, %edx

#ifdef PIC
	/* Standard PIC nonsense to access `__curbrk' through the GOT.  */
	call .L0
.L0:	popl %ecx
	addl $_GLOBAL_OFFSET_TABLE_+[.-.L0], %ecx

	movl C_SYMBOL_NAME(__curbrk@GOT)(%ecx), %ecx
	movl (%ecx), %ebx
	movl %ebx, %eax
	testl %ebx, %ebx
	jne .L1

	/* We have to initialize `__curbrk' first.  */
	movl $SYS_ify(brk), %eax
	int $0x80

	movl %eax, (%ecx)
	movl %eax, %ebx
	.align 16, 0x90

.L1:
#else
	movl C_SYMBOL_NAME(__curbrk), %ebx
	movl %ebx, %eax
#endif
	
	addl 4(%esp), %ebx
	cmpl %ebx, %eax
	je .L2

	movl $SYS_ify(brk), %eax
	int $0x80

	cmpl %eax, %ebx
	jne .L3

#ifdef PIC
	xchgl %eax, (%ecx)
#else
	xchgl %eax, C_SYMBOL_NAME(__curbrk)
#endif

.L2:
	movl %edx, %ebx
	ret

	.align 16
.L3:
	movl %edx, %ebx
	jmp JUMPTARGET(syscall_error)

weak_alias (__sbrk, sbrk)
