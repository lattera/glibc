/* Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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

ENTRY (__libc_pipe)
	mov	#+__NR_pipe, r3
	trapa	#0x10
	mov     r0, r3
	mov	#-12, r2
	shad	r2, r3
	not	r3, r3			// r1=0 means r0 = -1 to -4095
	tst	r3, r3			// i.e. error in linux
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
	mov.l	r0, @r4
	mov.l	r1, @(4, r4)
	rts
	 mov	#0, r0
	.align	2
.L2:
	.long	PLTJMP(C_SYMBOL_NAME(__syscall_error))
PSEUDO_END (__libc_pipe)

weak_alias (__libc_pipe, __pipe)
weak_alias (__libc_pipe, pipe)
