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
	movl %eax, C_SYMBOL_NAME(__curbrk)
	xorl %eax, %eax
	ret

weak_alias (__brk, brk)
