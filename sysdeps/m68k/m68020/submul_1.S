/* mc68020 __mpn_submul_1 -- Multiply a limb vector with a limb and subtract
   the result from a second limb vector.

Copyright (C) 1992, 1994 Free Software Foundation, Inc.

This file is part of the GNU MP Library.

The GNU MP Library is free software; you can redistribute it and/or modify
it under the terms of the GNU Library General Public License as published by
the Free Software Foundation; either version 2 of the License, or (at your
option) any later version.

The GNU MP Library is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU Library General Public
License for more details.

You should have received a copy of the GNU Library General Public License
along with the GNU MP Library; see the file COPYING.LIB.  If not, write to
the Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA. */

/*
  INPUT PARAMETERS
  res_ptr	(sp + 4)
  s1_ptr	(sp + 8)
  size		(sp + 12)
  s2_limb	(sp + 16)
*/

#include "asm-syntax.h"

	TEXT
	ALIGN
	GLOBL	___mpn_submul_1

LAB(___mpn_submul_1)

#define res_ptr a0
#define s1_ptr a1
#define size d2
#define s2_limb d4

/* Save used registers on the stack.  */
	INSN2(movem,l	,MEM_PREDEC(sp),d2-d5)

/* Copy the arguments to registers.  Better use movem?  */
	INSN2(move,l	,res_ptr,MEM_DISP(sp,20))
	INSN2(move,l	,s1_ptr,MEM_DISP(sp,24))
	INSN2(move,l	,size,MEM_DISP(sp,28))
	INSN2(move,l	,s2_limb,MEM_DISP(sp,32))

	INSN2(eor,w	,size,#1)
	INSN1(clr,l	,d1)
	INSN1(clr,l	,d5)
	INSN2(lsr,l	,size,#1)
	bcc	L1
	INSN2(subq,l	,size,#1)
	INSN2(sub,l	,d0,d0)		/* (d0,cy) <= (0,0) */

LAB(Loop)
	INSN2(move,l	,d3,MEM_POSTINC(s1_ptr))
	INSN2(mulu,l	,d1:d3,s2_limb)
	INSN2(addx,l	,d3,d0)
	INSN2(addx,l	,d1,d5)
	INSN2(sub,l	,MEM_POSTINC(res_ptr),d3)
LAB(L1)	INSN2(move,l	,d3,MEM_POSTINC(s1_ptr))
	INSN2(mulu,l	,d0:d3,s2_limb)
	INSN2(addx,l	,d3,d1)
	INSN2(addx,l	,d0,d5)
	INSN2(sub,l	,MEM_POSTINC(res_ptr),d3)

	dbf	size,Loop
	INSN2(addx,l	,d0,d5)
	INSN2(sub,l	,size,#0x10000)
	bcc	Loop

/* Restore used registers from stack frame.  */
	INSN2(movem,l	,d2-d5,MEM_POSTINC(sp))

	rts
