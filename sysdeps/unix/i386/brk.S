/* Copyright (C) 1991, 1992, 1993, 1995, 1997 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

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

#ifndef	SYS_brk
#define	SYS_brk	17
#endif

.data
.globl C_SYMBOL_NAME(__curbrk)
C_LABEL(__curbrk)
#ifdef	HAVE_GNU_LD
	.long C_SYMBOL_NAME(_end)
#else
	.long C_SYMBOL_NAME(end)
#endif

.text
SYSCALL__ (brk, 1)
	movl 4(%esp), %eax
#ifdef	PIC
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
