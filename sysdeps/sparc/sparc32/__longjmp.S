/* Copyright (C) 1991, 1993, 1996, 1997 Free Software Foundation, Inc.
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
   License along with the GNU C Library; see the file COPYING.LIB.  If not,
   write to the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
   Boston, MA 02111-1307, USA.  */

#include <sysdep.h>

#define _ASM 1
#include <bits/setjmp.h>
#define ENV(reg) [%g1 + (reg * 4)]

ENTRY(__longjmp)
	/* Store our arguments in global registers so we can still
	   use them while unwinding frames and their register windows.  */
	mov %o0, %g1		/* ENV in %g1 */
     	orcc %o1, %g0, %g2	/* VAL in %g2 */
     	be,a 0f			/* Branch if zero; else skip delay slot.  */
	 mov 1, %g2		/* Delay slot only hit if zero: VAL = 1.  */
0:
	/* Cache target FP in register %g3.  */
	ld ENV(JB_FP), %g3

	/* Now we will loop, unwinding the register windows up the stack
	   until the restored %fp value matches the target value in %g3.  */

LOC(loop):
	cmp %fp, %g3		/* Have we reached the target frame? */
	bl,a LOC(loop)		/* Loop while current fp is below target.  */
	 restore		/* Unwind register window in delay slot.  */
	be,a LOC(found)		/* Better have hit it exactly.  */
	 ld ENV(JB_SP), %o0	/* Delay slot: extract target SP.  */

LOC(bogus):
	/* Get here only if the jmp_buf or stack is clobbered.  */
	call C_SYMBOL_NAME(abort)
	 nop
	unimp 0

LOC(found):
	/* We have unwound register windows so %fp matches the target.  */
	cmp %o0, %sp		/* Check jmp_buf SP vs register window.  */
	bge,a LOC(sp_ok)	/* Saved must not be deeper than register.  */
	 mov %o0, %sp		/* OK, install new SP.  */
	b,a LOC(bogus)		/* Bogus, we lose.  */

LOC(sp_ok):
	ld ENV(JB_PC), %o0	/* Extract target return PC.  */
	jmp %o0 + 8		/* Return there.  */
	 mov %g2, %o0		/* Delay slot: set return value.  */

END(__longjmp)
