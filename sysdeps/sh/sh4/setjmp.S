/* setjmp for SH4.
   Copyright (C) 1999, 2000 Free Software Foundation, Inc.
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
#define _SETJMP_H
#define _ASM
#include <bits/setjmp.h>

ENTRY (__sigsetjmp)
	/* Save registers */
	add	#JB_SIZE, r4
	fmov.s	fr15, @-r4
	fmov.s	fr14, @-r4
	fmov.s	fr13, @-r4
	fmov.s	fr12, @-r4
	sts.l	fpscr, @-r4
	stc.l	gbr, @-r4
	sts.l	pr, @-r4
	mov.l	r15, @-r4
	mov.l	r14, @-r4
	mov.l	r13, @-r4
	mov.l	r12, @-r4
	mov.l	r11, @-r4
	mov.l	r10, @-r4
	mov.l	r9, @-r4
	mov.l	r8, @-r4

	/* Make a tail call to __sigjmp_save; it takes the same args.  */
#ifdef SHARED
	mov.l	1f, r1
	mova	1f, r0
	bra	2f
	 add	r1, r0
	.align	2
1:
	.long	_GLOBAL_OFFSET_TABLE_
2:
	mov.l	.L1, r1
	mov.l	@(r0,r1), r1
	jmp	@r1
	 nop
	.align	2
.L1:
	.long	C_SYMBOL_NAME(__sigjmp_save@GOT)
#else
	mov.l	.L1, r1
	jmp	@r1
	 nop
	.align	2
.L1:
	.long	C_SYMBOL_NAME(__sigjmp_save)
#endif
END (__sigsetjmp)
