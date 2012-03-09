/* setjmp for am33.
   Copyright (C) 2001, 2004 Free Software Foundation, Inc.
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
   License along with the GNU C Library.  If not, see
   <http://www.gnu.org/licenses/>.  */

#include <sysdep.h>
#define _ASM
#define _SETJMP_H
#include <bits/setjmp.h>
#include <asm-syntax.h>


ENTRY (__sigsetjmp)
.Lsigsetjmp:
	/* Save registers.  */
	mov d0,a0
	mov d2,(0,a0)
	mov d3,(4,a0)
	mov mdr,d0
	mov d0,(8,a0)
	/* Restore d0 for __sigjmp_save.  */
	mov a0,d0
	mov a2,(12,a0)
	mov a3,(16,a0)
	mov sp,a1
	mov a1,(20,a0)
	add 24,a0
	mov r4,(a0+)
	mov r5,(a0+)
	mov r6,(a0+)
	mov r7,(a0+)
#ifdef __AM33_2__
	fmov fs4,(a0+)
	fmov fs5,(a0+)
	fmov fs6,(a0+)
	fmov fs7,(a0+)
	fmov fs8,(a0+)
	fmov fs9,(a0+)
	fmov fs10,(a0+)
	fmov fs11,(a0+)
	fmov fs12,(a0+)
	fmov fs13,(a0+)
	fmov fs14,(a0+)
	fmov fs15,(a0+)
	fmov fs16,(a0+)
	fmov fs17,(a0+)
	fmov fs18,(a0+)
	fmov fs19,(a0+)
#endif
	/* Make a tail call to __sigjmp_save; it takes the same args.  */
	jmp __sigjmp_save
END (__sigsetjmp)

/* BSD `_setjmp' entry point to `sigsetjmp (..., 1)'.  */
ENTRY (setjmp)
	/* Tail-call setsetjmp with savesigs==1.  */
	mov 1,d1
	bra .Lsigsetjmp
END (setjmp)

/* BSD `_setjmp' entry point to `sigsetjmp (..., 0)'.  */
ENTRY (_setjmp)
	/* Tail-call setsetjmp with savesigs==0.  */
	clr d1
	bra .Lsigsetjmp
END (_setjmp)
