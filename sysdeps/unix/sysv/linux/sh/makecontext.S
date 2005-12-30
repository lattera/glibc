/* Create new context.
   Copyright (C) 2005 Free Software Foundation, Inc.
   This file is part of the GNU C Library.

   The GNU C Library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.

   The GNU C Library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the GNU
   Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public
   License along with the GNU C Library; if not, write to the Free
   Software Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
   02111-1307 USA.  */

#include <sysdep.h>

#include "ucontext_i.h"

/* void __makecontext (struct ucontext *ucp, void (*func)(), int argc, ...);
  __makecontext sets up a stack and registers for context to run a given
  function.  The registers are set up like this:
    r4-r7: parameters 1 to 4
    r8	 : uc_link from ucontext structure
    pc	 : (*func) pointer
    pr	 : address of exitcode
    r15	 : stack pointer for func.  */

	.text
	.align	5
ENTRY(__makecontext)
	mov	#4, r3
	mov.l	@(oSS_SP,r4), r1
	mov.l	@(oSS_SIZE,r4), r2
	add	r1, r2
	cmp/gt	r6, r3
	bf/s	1f
	 shlr2	r2
	sub	r6, r2
	add	r3, r2
1:
	shll2	r2
	mov	#oR15, r0
	mov.l	@(oLINK,r4), r1
	mov.l	r2, @(r0,r4)
	mov.l	r1, @(oR8,r4)
	mov	#oPC, r0
	mov.l	r5, @(r0,r4)

	cmp/pl	r6
	bf/s	.L1
	 dt	r6
	mov.l	r7, @(oR4,r4)
	cmp/pl	r6
	bf/s	.L1
	 dt	r6
	mov.l	@(0,r15), r1
	mov.l	r1, @(oR5,r4)
	cmp/pl	r6
	bf/s	.L1
	 dt	r6
	mov.l	@(4,r15), r1
	mov.l	r1, @(oR6,r4)
	cmp/pl	r6
	bf/s	.L1
	 dt	r6
	mov.l	@(8,r15), r1
	mov.l	r1, @(oR7,r4)
	mov	#12,r0
.L0:
	cmp/pl	r6
	bf/s	.L1
	 dt	r6
	mov.l	@(r0,r15), r1
	mov.l	r1, @r2
	add	#4, r0
	bra	.L0
	 add	#4, r2
.L1:
#ifdef PIC
	mova	.Lexitcode, r0
#else
	mov.l	.L2, r0
#endif
	add	#oPR, r4
	rts
	 mov.l	r0, @r4
#ifndef PIC
	.align	2
.L2:
	.long	.Lexitcode
#endif
	cfi_endproc

	.align	5
.Lexitcode:
	tst	r8, r8
	bt/s	2f
	 mov	r8, r4
#ifdef PIC
	mova	.Lgot, r0
	mov.l	.Lgot, r12
	add	r0, r12
	mov.l	.L3, r1
	bsrf	r1
.LPCS0:
	 nop
#else
	mov.l	.L3, r1
	jsr	@r1
	 nop
#endif
2:
	mov.l	.L4, r1
#ifdef PIC
	add	r12, r1
#endif
	jsr	@r1
	 mov	r0, r4
0:
	bra	0b
	 nop

	.align	2
#ifdef PIC
.Lgot:
	.long	_GLOBAL_OFFSET_TABLE_
.L3:
	.long	__setcontext@PLT-(.LPCS0+2-(.))
.L4:
	.long	HIDDEN_JUMPTARGET(exit)@GOTOFF
#else
.L3:
	.long	__setcontext
.L4:
	.long	HIDDEN_JUMPTARGET(exit)
#endif
	cfi_startproc
PSEUDO_END(__makecontext)

weak_alias (__makecontext, makecontext)
