/* Copyright (C) 1991, 92, 93, 95, 97, 98 Free Software Foundation, Inc.
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
#ifdef PIC
	ldr r1, 1f
	ldr r2, _cb_addr
2:	add r1, pc, r1
	add r1, r1, r2
#else
	ldr r1, _cb_addr
#endif
	str r0, [r1]
	mov r0, $0
	DO_RET (r14)
#ifdef PIC
1:	.long _GLOBAL_OFFSET_TABLE_ - 2b - 8
_cb_addr:
	.long C_SYMBOL_NAME(__curbrk)(GOTOFF)
#else
_cb_addr:
	.long C_SYMBOL_NAME(__curbrk)
#endif

weak_alias (__brk, brk)
