/* Copyright (C) 1991, 1992, 1993, 1995 Free Software Foundation, Inc.
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
C_LABEL(__curbrk)
#ifdef PIC
	.long 0
#else
# ifdef HAVE_GNU_LD
	.long C_SYMBOL_NAME(_end)
# else
	.long C_SYMBOL_NAME(end)
# endif
#endif

.text

	SYSCALL_ERROR_HANDLER
ENTRY (__brk)
	movl %ebx, %edx
	movl $SYS_ify(brk), %eax
	movl 4(%esp), %ebx
	int $0x80
	movl %edx, %ebx

	cmpl 4(%esp), %eax
	jne syscall_error

#ifdef PIC
	/* Standard PIC nonsense to store into `__curbrk' through the GOT.  */
	call here
here:	popl %ecx
	addl $_GLOBAL_OFFSET_TABLE_+[.-here], %ecx
	movl C_SYMBOL_NAME(__curbrk@GOT)(%ecx), %ecx
	movl %eax, (%ecx)
#else
	movl %eax, C_SYMBOL_NAME(__curbrk)
#endif
	xorl %eax, %eax
	ret

weak_alias (__brk, brk)
