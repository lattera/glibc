/* mc68020 __mpn_add_n -- Add two limb vectors of the same length > 0 and store
   sum in a third limb vector.

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
  s2_ptr	(sp + 16)
  size		(sp + 12)
*/

#include "asm-syntax.h"

	TEXT
	ALIGN
	GLOBL	___mpn_add_n

LAB(___mpn_add_n)
/* Save used registers on the stack.  */
	INSN2(move,l	,MEM_PREDEC(sp),d2)
	INSN2(move,l	,MEM_PREDEC(sp),a2)

/* Copy the arguments to registers.  Better use movem?  */
	INSN2(move,l	,a2,MEM_DISP(sp,12))
	INSN2(move,l	,a0,MEM_DISP(sp,16))
	INSN2(move,l	,a1,MEM_DISP(sp,20))
	INSN2(move,l	,d2,MEM_DISP(sp,24))

	INSN2(eor,w	,d2,#1)
	INSN2(lsr,l	,d2,#1)
	bcc L1
	INSN2(subq,l	,d2,#1)		/* clears cy as side effect */

LAB(Loop)
	INSN2(move,l	,d0,MEM_POSTINC(a0))
	INSN2(move,l	,d1,MEM_POSTINC(a1))
	INSN2(addx,l	,d0,d1)
	INSN2(move,l	,MEM_POSTINC(a2),d0)
LAB(L1)	INSN2(move,l	,d0,MEM_POSTINC(a0))
	INSN2(move,l	,d1,MEM_POSTINC(a1))
	INSN2(addx,l	,d0,d1)
	INSN2(move,l	,MEM_POSTINC(a2),d0)

	dbf d2,Loop			/* loop until 16 lsb of %4 == -1 */
	INSN2(subx,l	,d0,d0)		/* d0 <= -cy; save cy as 0 or -1 in d0 */
	INSN2(sub,l	,d2,#0x10000)
	bcs L2
	INSN2(add,l	,d0,d0)		/* restore cy */
	bra Loop

LAB(L2)
	INSN1(neg,l	,d0)

/* Restore used registers from stack frame.  */
	INSN2(move,l	,a2,MEM_POSTINC(sp))
	INSN2(move,l	,d2,MEM_POSTINC(sp))

	rts
