/* Copyright (C) 2000 Free Software Foundation, Inc.
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

ENTRY (__syscall)
	mov	r4, r3			// syscall number
	mov	r5, r4			// p1
	mov	r6, r5			// p2
	mov	r7, r6			// p3
	mov.l	@r15, r7		// p4
	mov.l	@(4,r15), r0		// p5
	mov.l	@(8,r15), r1		// p6
	mov.l	@(12,r15), r2		// p7
	trapa	#0x17
	mov     r0, r1
	mov	#-12, r2
	shad	r2, r1
	not	r1, r1			// r1=0 means r0 = -1 to -4095
	tst	r1, r1			// i.e. error in linux
	bf	1f
	mov.l	.L2, r1
#ifdef SHARED
	mov	r0, r4
	mov.l	r12, @-r15
	sts.l	pr, @-r15
        mov.l	0f, r12
	mova	0f, r0
	add	r0, r12
	mova	.L2, r0
	add	r0, r1
	jsr	@r1
	 nop
	lds.l	@r15+, pr
	rts
	 mov.l	@r15+, r12
	.align 2
0:
	.long	_GLOBAL_OFFSET_TABLE_
#else
	jmp	@r1
	 mov	r0, r4
#endif
1:
	rts
	 nop

	.align	2
.L2:
	.long	PLTJMP(C_SYMBOL_NAME(__syscall_error))
PSEUDO_END (__syscall)

weak_alias (__syscall, syscall)
