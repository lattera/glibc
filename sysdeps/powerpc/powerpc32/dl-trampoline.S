/* PLT trampolines.  PPC32 version.
   Copyright (C) 2005 Free Software Foundation, Inc.
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

	.section ".text"
	.align 2
	.globl _dl_runtime_resolve
	.type _dl_runtime_resolve,@function
_dl_runtime_resolve:
 # We need to save the registers used to pass parameters, and register 0,
 # which is used by _mcount; the registers are saved in a stack frame.
	stwu 1,-64(1)
	stw 0,12(1)
	stw 3,16(1)
	stw 4,20(1)
 # The code that calls this has put parameters for `fixup' in r12 and r11.
	mr 3,12
	stw 5,24(1)
	mr 4,11
	stw 6,28(1)
	mflr 0
 # We also need to save some of the condition register fields
	stw 7,32(1)
	stw 0,48(1)
	stw 8,36(1)
	mfcr 0
	stw 9,40(1)
	stw 10,44(1)
	stw 0,8(1)
	bl _dl_fixup@local
 # 'fixup' returns the address we want to branch to.
	mtctr 3
 # Put the registers back...
	lwz 0,48(1)
	lwz 10,44(1)
	lwz 9,40(1)
	mtlr 0
	lwz 8,36(1)
	lwz 0,8(1)
	lwz 7,32(1)
	lwz 6,28(1)
	mtcrf 0xFF,0
	lwz 5,24(1)
	lwz 4,20(1)
	lwz 3,16(1)
	lwz 0,12(1)
 # ...unwind the stack frame, and jump to the PLT entry we updated.
	addi 1,1,64
	bctr
	.size	 _dl_runtime_resolve,.-_dl_runtime_resolve

	.align 2
	.globl _dl_prof_resolve
	.type _dl_prof_resolve,@function
_dl_prof_resolve:
 # We need to save the registers used to pass parameters, and register 0,
 # which is used by _mcount; the registers are saved in a stack frame.
	stwu 1,-64(1)
        stw 0,12(1)
	stw 3,16(1)
	stw 4,20(1)
 # The code that calls this has put parameters for `fixup' in r12 and r11.
	mr 3,12
	stw 5,24(1)
	mr 4,11
	stw 6,28(1)
	mflr 5
 # We also need to save some of the condition register fields.
	stw 7,32(1)
	stw 5,48(1)
	stw 8,36(1)
	mfcr 0
	stw 9,40(1)
	stw 10,44(1)
	stw 0,8(1)
	bl _dl_profile_fixup@local
 # 'fixup' returns the address we want to branch to.
	mtctr 3
 # Put the registers back...
	lwz 0,48(1)
	lwz 10,44(1)
	lwz 9,40(1)
	mtlr 0
	lwz 8,36(1)
	lwz 0,8(1)
	lwz 7,32(1)
	lwz 6,28(1)
	mtcrf 0xFF,0
	lwz 5,24(1)
	lwz 4,20(1)
	lwz 3,16(1)
        lwz 0,12(1)
 # ...unwind the stack frame, and jump to the PLT entry we updated.
	addi 1,1,64
	bctr
	.size	 _dl_prof_resolve,.-_dl_prof_resolve
